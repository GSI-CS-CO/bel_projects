/** @file fbas-snoop-saftbus.cpp
 *  @brief Command-line tool to measure the latency of the FBAS C2 messaging.
 *
 *  Copyright (C) 2026 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  This tool requires the running saftbusd daemon.
 *
 *  @author Enkhbold Ochirsuren <e.ochirsuren@gsi.de>
 *
 *  @bug Unknown!
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/**
 * Derived from: https://github.com/GSI-CS-CO/saftlib/tree/master/examples/EventSnoopTool/saft-snoop-saftbus.cpp
 *
 * Build (yocto): unset LD_LIBRARY_PATH &&
 *                source /common/usr/embedded/yocto/fesa/current/sdk/environment-setup-core2-64-ffos-linux
 * Build:         make fbas-snoop-saftbus
 * Run (devel):   export SAFTBUS_SOCKET_PATH=/tmp/saftbus &&
 *                saftbusd libsaft-service.so tr0:dev/wbm0 &&
 *                ./fbas-snoop-saftbus tr0 0x1fcbfcb000000000 0xffffffffffffff00 0 0x1fcbfcb000000042
 *
 */

#include <SAFTd_Proxy.hpp>
#include <TimingReceiver_Proxy.hpp>
#include <SoftwareActionSink_Proxy.hpp>
#include <SoftwareCondition_Proxy.hpp>

#include <CommonFunctions.h>

#include <memory>
#include <iostream>

#include <csignal>  // signal handling
#include <cmath>    // sqrt

namespace stats {

  typedef struct metrics {
    int count;
    double old_mean;
    double new_mean;
    double old_variance;
    double new_variance;
  } metrics_t;

  metrics_t latency; // measured C2 messaging latency
  metrics_t rate;    // measured C2 messaging rate

  uint64_t test_event_id; // C2 event id
  uint64_t test_event_ts; // C2 event timestamp
  uint64_t stop_event_id; // event id to trigger measurement reports

  void clear(metrics_t& s)
  {
    s.count = 0;
    s.new_mean = 0;
    s.old_mean = 0;
    s.new_variance = 0;
    s.old_variance = 0;
  }

  void calculate(double x, metrics_t& s)
  {
    s.count++;

    if (s.count == 1) {
      s.old_mean = x;
      s.new_mean = x;
      s.old_variance = 0;
      s.new_variance = 0;
    }
    else {
      s.new_mean = s.old_mean + (x - s.old_mean)/s.count;
      s.new_variance = s.old_variance + (x - s.old_mean) * (x - s.new_mean);
      s.old_mean = s.new_mean;
      s.old_variance = s.new_variance;
    }
  }

  int count(metrics_t& s)
  {
    return s.count;
  }

  double mean(metrics_t& s)
  {
    return (( s.count > 0) ? s.new_mean : 0.0);
  }

  double variance(metrics_t& s)
  {
    return (( s.count > 1 ) ? s.new_variance/(s.count - 1) : 0.0);
  }

  double std_dev(metrics_t& s)
  {
    return std::sqrt(variance(s));
  }

  void report(metrics_t& s)
  {
    std::cout << std::dec << "mean = " << mean(s) << " "
              << "dev = " << std_dev(s) << " "
              << "count = " << count(s) << " " << std::endl;
  }

  void report_all()
  {
      std::cout << "c2 latency [ns]: ";
      report(latency);

      std::cout << "c2 rate [ns]: ";
      report(rate);
  }

  void clear_all()
  {
      clear(latency);
      clear(rate);
      test_event_ts = 0;
  }

}

volatile std::sig_atomic_t termination_requested = 0;

void sig_handler(int sig)
{
  termination_requested = sig;
}

void check_termination()
{
  if (termination_requested) {
    stats::report_all();

    exit(termination_requested);
  }
}

void on_action(uint64_t event, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags) {

  if (event == stats::stop_event_id) {
    stats::report_all();
    stats::clear_all();
    return;
  }

  if (event == stats::test_event_id) {
    if (stats::test_event_ts) {
      stats::calculate((deadline.getTAI() - stats::test_event_ts), stats::rate);
    }
    stats::test_event_ts = deadline.getTAI();
  }

  stats::calculate((executed.getTAI() - deadline.getTAI()), stats::latency);
}

int main(int argc, char *argv[]) {
  if (argc != 6) {
    std::cerr << "usage: " << argv[0] << " <saftlib-device> <c2_event> <event_mask> <offset> <stop_event>" << std::endl;
    std::cerr << "example: " << argv[0] << " tr0 0x1fcbfcb000000000 0xffffffffffffff00 0 0x1fcbfcb000000042" << std::endl;
    return 0;
  }

  uint64_t snoop_args[4];

  for (int i = 2; i < 6; ++i) {
    std::istringstream argin(argv[i]);
    argin >> std::hex >> snoop_args[i-2];
    if (!argin) {
      std::cerr << "cannot read snoop argument from " << argv[i] << std::endl;
      return 1;
    }
  }

  uint64_t &c2     = snoop_args[0];
  uint64_t &mask   = snoop_args[1];
  uint64_t &offset = snoop_args[2];
  stats::test_event_id = c2;
  stats::test_event_ts = 0;
  stats::stop_event_id = snoop_args[3];

  std::cout << std::hex << "c2=" << c2 << " "
            << "mask=" << mask << " "
            << "offset=" << offset << " "
            << "stop=" << stats::stop_event_id << std::endl;

  auto saftd = saftlib::SAFTd_Proxy::create();
  auto tr = saftlib::TimingReceiver_Proxy::create(saftd->getDevices()[argv[1]]);
  auto softwareActionSink_obj_path = tr->NewSoftwareActionSink("");
  auto softwareActionSink          = saftlib::SoftwareActionSink_Proxy::create(softwareActionSink_obj_path);
  auto condition_obj_path          = softwareActionSink->NewCondition(true, c2, mask, offset);
  auto sw_condition                = saftlib::SoftwareCondition_Proxy::create(condition_obj_path);

  sw_condition->setAcceptEarly(true);
  sw_condition->setAcceptLate(true);
  sw_condition->setAcceptDelayed(true);
  sw_condition->setAcceptConflict(true);

  sw_condition->SigAction.connect(sigc::ptr_fun(&on_action));

  signal(SIGINT, sig_handler);

  for (;;) {
    //saftbus::Loop::get_default().iteration(true);
    saftlib::wait_for_signal();
    // saftbus::SignalGroup::get_global().wait_for_signal();
    check_termination();
  }

  return 0;
}

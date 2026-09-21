/** @file fbas-snoop-saftbus.cpp
 *  @brief Command-line tool to snoop the FBAS C2 events by using the saftbus.
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
 *                ./fbas-snoop-saftbus tr0 0xffffffffff00 0xffffffffff00 0
 *
 */

#include <SAFTd_Proxy.hpp>
#include <TimingReceiver_Proxy.hpp>
#include <SoftwareActionSink_Proxy.hpp>
#include <SoftwareCondition_Proxy.hpp>

#include <CommonFunctions.h>

#include <memory>
#include <iostream>

void on_action(uint64_t event, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags) {
  std::cout << "event " << event << " "
            << "param " << param << " "
            << "deadline " << deadline.getTAI() << " "
            << "executed " << executed.getTAI() << " "
            << "flags " << flags << " "
            << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 5) {
    std::cerr << "usage: " << argv[0] << " <saftlib-device> <id> <mask> <param>" << std::endl;
    return 0;
  }

  uint64_t snoop_args[3];
  uint64_t &id    = snoop_args[0];
  uint64_t &mask  = snoop_args[1];
  uint64_t &param = snoop_args[2];
  for (int i = 2; i < 5; ++i) {
    std::istringstream argin(argv[i]);
    argin >> snoop_args[i-2];
    if (!argin) {
      std::cerr << "cannot read snoop argument from " << argv[i] << std::endl;
      return 1;
    }
  }

  auto saftd = saftlib::SAFTd_Proxy::create();
  auto tr = saftlib::TimingReceiver_Proxy::create(saftd->getDevices()[argv[1]]);
  auto softwareActionSink_obj_path = tr->NewSoftwareActionSink("");
  auto softwareActionSink          = saftlib::SoftwareActionSink_Proxy::create(softwareActionSink_obj_path);
  auto condition_obj_path          = softwareActionSink->NewCondition(true, id, mask, param);
  auto sw_condition                = saftlib::SoftwareCondition_Proxy::create(condition_obj_path);

  sw_condition->setAcceptEarly(true);
  sw_condition->setAcceptLate(true);
  sw_condition->setAcceptDelayed(true);
  sw_condition->setAcceptConflict(true);

  sw_condition->SigAction.connect(sigc::ptr_fun(&on_action));

  for (;;) {
    //saftbus::Loop::get_default().iteration(true);
    saftlib::wait_for_signal();
    // saftbus::SignalGroup::get_global().wait_for_signal();
  }

  return 0;
}

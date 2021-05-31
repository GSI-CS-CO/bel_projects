// Synopsis 
// ==================================================================================================== 
// WhiteRabbit to MIL gateway control application 

// Defines 
// ==================================================================================================== 
#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

// Includes 
// ==================================================================================================== 
#include <stdio.h>
#include <iostream>
#include <deque>
#include <cassert>
#include <getopt.h>
#include <unistd.h>
#include <algorithm>

#include "SAFTd.h"
#include "EmbeddedCPUActionSink.h"
#include "EmbeddedCPUCondition.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "WbmActionSink.h"
#include "WbmCondition.h"
#include "TimingReceiver.h"
#include "WrMilGateway.h"
#include "wr_mil_gw_regs.h"
#include "Output.h"
#include "OutputCondition.h"

#ifdef USEMASP
  #include "MASP/Emitter/StatusEmitter.h"
  #include "MASP/StatusDefinition/DeviceStatus.h"
  #include "MASP/Util/Logger.h"
  #include "MASP/Common/StatusNames.h"
  #include "MASP/Emitter/End_of_scope_status_emitter.h"
  #include <boost/thread/thread.hpp> // (sleep)
  #include <iostream>
  #include <string>
#endif // USEMASP


// Namespaces 
// ==================================================================================================== 
using namespace saftlib;
using namespace std;

// Globals 
// ==================================================================================================== 
static const char *deviceName = NULL; // Name of the device 
static const char *program    = NULL; // Name of the application 

// Prototypes 
// ==================================================================================================== 
static void wrmilgw_help (void);
bool op_ready(bool receiver_locked, bool firmware_running, int firmware_state);
void print_firmware_state(uint32_t firmware_state);
void print_event_source(uint32_t event_source);
void print_in_use(bool in_use);
void print_info1(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw);
void print_info2(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw);
void print_info3(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw);
void createCondition(std::shared_ptr<TimingReceiver_Proxy> receiver, uint64_t eventID);
void destroyGatewayConditions(std::shared_ptr<TimingReceiver_Proxy> receiver);
void on_locked(bool is_locked);
void on_firmware_running(bool is_running);
void on_firmware_state(uint32_t state);
void on_event_source(uint32_t source);
void on_in_use(bool in_use);
std::string mil_description(uint8_t mil);

// Function wrmilgw_help() 
// ==================================================================================================== 
static void wrmilgw_help (void)
{
  // Print arguments and options 
  std::cout << "wrmilgw-ctl for SAFTlib" << std::endl;
  std::cout << "Usage: " << program << " <unique device name> [OPTIONS]" << std::endl;
  std::cout << std::endl;
  std::cout << "Arguments/[OPTIONS]:" << std::endl;
  // std::cout << "  -c <id> <mask> <offset> <tag>: Create a new condition" << std::endl;
  std::cout << "  -i                            Show gateway information. Repeat the option"          << std::endl;
  std::cout << "                                 to get more detailed information, e.g. -iii"         << std::endl;
  std::cout << "  -R                            Read register content"                                << std::endl;
  std::cout << "  -m                            Start monitoring loop"                                << std::endl;
  std::cout << "  -g                            Show received MIL events in monitoring loop"          << std::endl;
  std::cout << "  -b                            Bugfix mode: show relevent WR-Events together"        << std::endl;
  std::cout << "                                             with events that trigger MIL-generation" << std::endl;
  std::cout << "                                             together with snooped MIL events"        << std::endl;
  std::cout << "  -H                            Show MIL-event histogram"                             << std::endl;
  std::cout << "  -s                            Start WR-MIL Gateway as SIS18 Pulszentrale"           << std::endl;
  std::cout << "  -e                            Start WR-MIL Gateway as ESR   Pulszentrale"           << std::endl;
  std::cout << "  -l <latency>                  Set MIL event latency [us]"                           << std::endl;
  std::cout << "  -t <trigger>                  Set UTC-trigger event [0..255]"                       << std::endl;
  std::cout << "  -o <offset>                   Set UTC-offset [s] (value is added to WR-time)"       << std::endl;
  std::cout << "  -d <delay>                    Set Trigger-UTC delay [us]"                           << std::endl;
  std::cout << "  -u <delay>                    Set UTC-UTC delay [us]"                               << std::endl;
  std::cout << "  -r                            Pause gateway for 1 s, and reset"                     << std::endl;
  std::cout << "  -k                            Kill gateway. Only LM32 reset can recover."           << std::endl;
  std::cout << "                                 This is useful before flashing new firmware."        << std::endl;
  std::cout << "  -c                            No color on console ouput"                            << std::endl;
#ifdef USEMASP
  std::cout << "  -P                            MASP emitter in productive mode"                      << std::endl;
#endif
  std::cout << "  -h                            Print help (this message)"                            << std::endl;
  std::cout << std::endl;
}

std::string red_color       = "\033[1;31m";
std::string green_color     = "\033[1;32m";
std::string default_color   = "\033[0m";

const int key_width = 25;
const int value_width = 15;

class BugfixMode {
public:
  struct EvtID {
    uint64_t evtid;
    saftlib::Time deadline;
    int fid, gid, evtno, flags, bpc, sid, bpid, res;
    uint32_t e[5];
    EvtID(uint64_t id, saftlib::Time dl) : 
      evtid(id),
      deadline(dl),
      fid  ((id >> 60) & 0xf    ),
      gid  ((id >> 48) & 0xfff  ),
      evtno((id >> 36) & 0xfff  ),
      flags((id >> 32) & 0xf    ),
      bpc  ((id >> 34) & 0x1    ),
      sid  ((id >> 20) & 0xfff  ),
      bpid ((id >>  6) & 0x3fff ),
      res  ( id        & 0x3f   )   {}
    bool make_mil(int &mil) {
      if ( (gid == 0x12c) && ((evtno&0xff00) == 0) ) {
        int tophalf = sid&0xff;
        int virtacc = tophalf &0xf;
        if (evtno >= 200 && evtno <= 208) {
          // tophalf = tophalf;
        } else if (evtno == 255) {
          tophalf = (2<<4) | 0xf;
        } else {
          tophalf = (2<<4) | virtacc;
        }
        mil = (tophalf<<8)|(evtno&0xff);
        return true;
      }
      return false;
    }
    uint32_t decode_mil_utc_timestamp() {
      // uint64_t UTC_offset_ms = 1199142000000;
      // std::cerr << std::hex << e[0] << " " << e[1] << " " << e[2] << " " << e[3] << " " << e[4] << " " << std::endl;
      uint32_t time = 0; 
      time |= e[1]&0x3f; time <<= 8;
      time |= e[2]     ; time <<= 8;
      time |= e[3]     ; time <<= 8;
      time |= e[4];
      // std::cerr << "time = 0x" << std::hex << std::setw(8) << std::setfill('0') <<                time  << " = " << std::dec <<                time << std::endl;
      return time;
    }
    void encode_mil_utc_timestamp() // e must be an array with lenth 5
    {
      // std::cerr << "encode " << deadline.getTAI() << std::endl;
      uint64_t msNow  = deadline.getTAI() / UINT64_C(1000000); // conversion from ns to ms (since 1970)
      uint64_t ms2008 = UINT64_C(1199142000000); // miliseconds at 01/01/2008  (since 1970)
                                                 // the number was caluclated using: date --date='01/01/2008' +%s
      uint64_t mil_timestamp_ms = msNow - ms2008;
      uint32_t mil_ms           = mil_timestamp_ms % 1000;
      uint32_t mil_sec          = mil_timestamp_ms / 1000;

      e[0]  = (( mil_ms>>2      ) & 0xff) /*<< 8) | 0xe0*/;
      e[1]  = (((mil_ms&0x3)<<6 ) & 0xff) /*<< 8) | 0xe1*/;
      e[1] |= (( mil_sec>>24    ) & 0xff) /*<< 8) | 0xe1*/;
      e[2]  = (( mil_sec>>16    ) & 0xff) /*<< 8) | 0xe2*/;
      e[3]  = (( mil_sec>>8     ) & 0xff) /*<< 8) | 0xe3*/;
      e[4]  = (( mil_sec        ) & 0xff) /*<< 8) | 0xe4*/;
      assert(mil_sec == decode_mil_utc_timestamp());

      for (int i = 0; i < 5; ++i) {
        e[i] <<= 8;
        e[i] |= 0xe0 | i;
      }

      // std::cerr << "same? " << mil_sec << " = " << this->decode_mil_utc_timestamp() << std::endl;
    }
  };

  bool utc_time_trigger(int mil) {
    return (mil&0xff) == 0xf6;
  }


  void on_event_trigger(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags) {
    gen_q.push_back(id & 0xffff);
    gentime_q.push_back(deadline.getTAI());
    B2->WriteOutput(true);
    B2->WriteOutput(false);
  }
  void on_mil(uint32_t message) {
    std::cout << "mil-snoop:   0x" << std::hex << std::setw(4) << std::setfill('0') << message << std::endl;
    	mil_q.push_back(message);
    while (gen_q.size() > 0 && mil_q.size() > 0) {
      if (gen_q.front() != mil_q.front()) {
        std::cerr << "missing event on MIL bus: 0x" << std::setw(4) << std::setfill('0') << gen_q.front() << " at TAI " << gentime_q.front() << std::endl;
        gen_q.pop_front();
        gentime_q.pop_front();
        // blink B1 whenever there was a missing MIL. This can be used as a trigger to look at the broken MIL event on an oscilloscope
        B1->WriteOutput(true);
        B1->WriteOutput(false);
      } else {
        gen_q.pop_front();
        gentime_q.pop_front();
        mil_q.pop_front();
      }
    }
  }

  std::shared_ptr<saftlib::Output_Proxy> B1;
  std::shared_ptr<saftlib::Output_Proxy> B2;

private:
  std::deque<uint64_t> gentime_q; 
  std::deque<uint32_t> gen_q;
  std::deque<uint32_t> mil_q;
};

// this will be called, in case we are snooping for events
static void on_action(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, std::shared_ptr<WrMilGateway_Proxy> wrmilgw)
{
  bool late     = flags&1;
  bool early    = flags&2;
  bool conflict = flags&4;
  //bool delayed  = flags&8;

  if (late) {
    std::cout << "late MIL event: "       << (id & 0xff) << "   " << executed-deadline << " ns" << std::endl;
    wrmilgw->IncrementLateMilEvents();
  }
  if (early) {
    std::cout << "early MIL event: "      << (id & 0xff) << "   " << executed-deadline << " ns" << std::endl;
  }
  if (conflict) {
    std::cout << "conflicting MIL event " << (id & 0xff) << "   " << std::endl;
  }
} 

static void on_mil(uint32_t message) {
  std::cout << "MIL: 0x" 
            << std::hex << std::setfill('0') << std::setw(4) << message << std::dec 
            << "  " << mil_description(0xff&message)
            << std::endl;
}

bool op_ready(bool receiver_locked, bool firmware_running, int firmware_state)
{
  return     (receiver_locked  == true)
          && (firmware_running == true)
          && (firmware_state   == WR_MIL_GW_STATE_CONFIGURED);
}

void print_firmware_state(uint32_t firmware_state)
{
  switch(firmware_state) {
    case WR_MIL_GW_STATE_INIT:
      std::cout << red_color << std::setw(value_width) << std::right << "INIT" << default_color << std::endl;
    break;         
    case WR_MIL_GW_STATE_UNCONFIGURED:
      std::cout << red_color << std::setw(value_width) << std::right << "UNCONFIGURED" << default_color << std::endl;
    break; 
    case WR_MIL_GW_STATE_CONFIGURED:
      std::cout << green_color << std::setw(value_width) << std::right << "CONFIGURED" << default_color << std::endl;
    break;   
    case WR_MIL_GW_STATE_PAUSED:
      std::cout << red_color << std::setw(value_width) << std::right << "PAUSED" << default_color << std::endl;
    break;       
    default:
      std::cout << red_color << std::setw(value_width) << std::right << "UNKNOWN" << default_color << std::endl;
  }  
}

void print_event_source(uint32_t event_source)
{
  switch(event_source) {
    case WR_MIL_GW_EVENT_SOURCE_SIS:
      std::cout << green_color << std::setw(value_width) << std::right << "SIS18" << default_color << std::endl;
    break;
    case WR_MIL_GW_EVENT_SOURCE_ESR:
      std::cout << green_color << std::setw(value_width) << std::right << "ESR" << default_color << std::endl;
    break;
    default:
      std::cout << red_color << std::setw(value_width) << std::right << "UNKNOWN" << default_color << std::endl;
  }  
}

void print_in_use(bool in_use) 
{
  if (in_use) {
    std::cout << green_color << std::setw(value_width) << std::right << "YES" << default_color << std::endl;
  } else {
    std::cout << red_color << std::setw(value_width) << std::right << "NO" << default_color << std::endl;
  }
}

// Print basic info
void print_info1(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw)
{
  auto receiver_locked  = receiver->getLocked();
  auto firmware_running = wrmilgw->getFirmwareRunning();
  auto firmware_state   = wrmilgw->getFirmwareState(); 

  std::cout << std::setw(key_width) << std::left << "Gateway OP_READY:";
  if (op_ready(receiver_locked, firmware_running, firmware_state)) {
    std::cout << green_color << std::setw(value_width) << std::right << "YES" << default_color << std::endl;
  } else {
    std::cout << red_color << std::setw(value_width) << std::right << "NO" << default_color << std::endl;
  }

  auto in_use = wrmilgw->getInUse();
  std::cout << std::setw(key_width) << std::left << "Gateway in use:";
  print_in_use(in_use);
}

// print number of events info
void print_info2(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw)
{
  auto receiver_locked  = receiver->getLocked();
  auto firmware_running = wrmilgw->getFirmwareRunning();
  auto firmware_state   = wrmilgw->getFirmwareState(); 
  auto event_source     = wrmilgw->getEventSource();

  std::cout << std::endl;
  std::cout << std::setw(key_width) << std::left << "TimingReceiver status:";
  if (receiver_locked) {
    std::cout << green_color << std::setw(value_width) << std::right << "LOCKED" << default_color << std::endl;
  } else {
    std::cout << red_color << std::setw(value_width) << std::right << "NOT LOCKED" << default_color << std::endl;
  }

  std::cout << std::setw(key_width) << std::left << "Gateway firmware:";
  if (firmware_running) {
    std::cout << green_color << std::setw(value_width) << std::right << "RUNNING" << default_color << std::endl;
  } else {
    std::cout << red_color << std::setw(value_width) << std::right << "STALLED" << default_color << std::endl;
  }

  std::cout << std::setw(key_width) << std::left << "Gateway state:";
  print_firmware_state(firmware_state);

  std::cout << std::setw(key_width) << std::left << "Source type:";
  print_event_source(event_source);

  std::cout << std::setw(key_width) << std::left << "Total MIL events:" 
            << std::setw(value_width) << std::right << wrmilgw->getNumMilEvents()
            << std::endl;
  std::cout << std::setw(key_width) << std::left << "Late MIL events:" 
            << std::setw(value_width) << std::right << wrmilgw->getNumLateMilEvents()
            << std::endl;
}

// print timnig and UTC-trigger configuration info
void print_info3(std::shared_ptr<TimingReceiver_Proxy> receiver, std::shared_ptr<WrMilGateway_Proxy> wrmilgw)
{
  std::cout << std::endl;
  std::cout << std::setw(key_width) << std::left << "MIL event latency:" 
            << std::setw(value_width) << std::right << wrmilgw->getEventLatency() << " us"
            << std::endl;
  std::cout << std::setw(key_width) << std::left << "UTC event trigger:" 
            << std::setw(value_width) << std::right << static_cast<int>(wrmilgw->getUtcTrigger())
            << std::endl;
  std::cout << std::setw(key_width) << std::left << "UTC offset:" 
            << std::setw(value_width) << std::right << wrmilgw->getUtcOffset()/1000 << " s"
            << std::endl;
  std::cout << std::setw(key_width) << std::left << "UTC-UTC delay:" 
            << std::setw(value_width) << std::right << wrmilgw->getUtcUtcDelay() << " us"
            << std::endl;
  std::cout << std::setw(key_width) << std::left << "Trigger-UTC delay:" 
            << std::setw(value_width) << std::right << wrmilgw->getTriggerUtcDelay() << " us"
            << std::endl;
}

const auto SIS18EventID = UINT64_C(0x112c000000000000);
const auto   ESREventID = UINT64_C(0x1154000000000000);

void createCondition(std::shared_ptr<WrMilGateway_Proxy> wrmilgw, std::shared_ptr<TimingReceiver_Proxy> receiver, uint64_t eventID)
{
  {
    // create the embedded CPU action sink for SIS18 WR events
    std::map<std::string, std::string> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
    if (e_cpus.size() != 1)  {
      throw saftbus::Error(saftbus::Error::FAILED, "No embedded CPU action sink found");
    }
    std::shared_ptr<EmbeddedCPUActionSink_Proxy> e_cpu 
        = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);
    auto eventMask = UINT64_C(0xfffff00000000000);
    auto offset    = INT64_C(-1000)*wrmilgw->getEventLatency(); // set the negative latency as offset so that the execution will be at 0
    auto tag       = UINT32_C(0x4);

    // Destroy all unowned conditions
    std::vector< std::string > all_conditions = e_cpu->getAllConditions();  
    for (unsigned int condition_it = 0; condition_it < all_conditions.size(); condition_it++) {
      std::shared_ptr<EmbeddedCPUCondition_Proxy> destroy_condition = EmbeddedCPUCondition_Proxy::create(all_conditions[condition_it]);
      if (destroy_condition->getDestructible() && (destroy_condition->getOwner() == "")) { 
        destroy_condition->Destroy();
      }
    }

    e_cpu->Own();
    e_cpu->setMinOffset(-300000000);
    e_cpu->setMaxOffset(300000000);
    std::shared_ptr<EmbeddedCPUCondition_Proxy> condition
       = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(true, eventID, eventMask, offset, tag));
    // Accept every kind of event 
    condition->setAcceptConflict(true);
    condition->setAcceptDelayed(true);
    condition->setAcceptEarly(true);
    condition->setAcceptLate(true);
    condition->Disown();
    e_cpu->Disown();
  }
  {
  // create a Wbm Action sink
    /* Search for embedded CPU channel */
    map<std::string, std::string> acwbms = receiver->getInterfaces()["WbmActionSink"];
    if (acwbms.size() != 1)
    {
      throw saftbus::Error(saftbus::Error::FAILED, "No Wbm action sink found");
    }
    std::shared_ptr<WbmActionSink_Proxy> acwbm = WbmActionSink_Proxy::create(acwbms.begin()->second);
    // record macro sequence to access the mil piggy
    std::vector<std::vector<uint32_t> > commands(1, std::vector<uint32_t>(3)); // a sequence with one command
    commands[0][0] = 0x9004;     // WB-adr.
    commands[0][1] = 0x000000aa; // payload (will not be used)
    commands[0][2] = 0x0000003f; // use low part of eventID as WB-data
    uint macroIdx = 0;
    acwbm->setEnable(false);
    acwbm->ClearAllMacros();
    acwbm->RecordMacro(macroIdx, commands); // program macro index 0
    acwbm->setEnable(true);

    std::vector< std::string > all_conditions = acwbm->getAllConditions();  
    for (unsigned int condition_it = 0; condition_it < all_conditions.size(); condition_it++) {
      std::shared_ptr<WbmCondition_Proxy> destroy_condition = WbmCondition_Proxy::create(all_conditions[condition_it]);
      if (destroy_condition->getDestructible() && (destroy_condition->getOwner() == "")) { 
        destroy_condition->Destroy();
      }
    }

    acwbm->Own();
    auto eventID   = UINT64_C(0xffffffff00000000);
    auto eventMask = UINT64_C(0xffffffff00000000);
    auto offset    = -0;
    auto Tag       = macroIdx; // to target the recorded macro sequence
    std::shared_ptr<WbmCondition_Proxy> condition
       = WbmCondition_Proxy::create(acwbm->NewCondition(true, eventID, eventMask, offset, Tag));
    // Accept every kind of event 
    condition->setAcceptConflict(true);
    condition->setAcceptDelayed(true);
    condition->setAcceptEarly(true);
    condition->setAcceptLate(true);
    condition->Disown();
    acwbm->Disown();
  }
}
void destroyGatewayConditions(std::shared_ptr<TimingReceiver_Proxy> receiver)
{
  // Get the conditions
  std::map<std::string, std::string> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
  if (e_cpus.size() != 1)  {
    throw saftbus::Error(saftbus::Error::FAILED, "No embedded CPU action sink found");
  }
  std::shared_ptr<EmbeddedCPUActionSink_Proxy> e_cpu 
      = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);
  std::vector< std::string > all_conditions = e_cpu->getAllConditions();
 
  // Destroy conditions if possible
  for (auto condition_name: all_conditions)  {
    std::shared_ptr<EmbeddedCPUCondition_Proxy> condition = EmbeddedCPUCondition_Proxy::create(condition_name);
    if (condition->getDestructible() && condition->getOwner() == "" &&
        condition->getMask() == UINT64_C(0xfffff00000000000) &&
        condition->getOffset() == INT64_C(-100000) &&
        (condition->getID() == SIS18EventID || 
         condition->getID() == ESREventID) )  {
      condition->Destroy();
    }
  }  
}

// Signal callbacks 
// ==================================================================================================== 
void on_locked(bool is_locked) 
{
  if (is_locked) {
    std::cout << "got WR-Lock" << std::endl;
  } else {
    std::cout << "WR-Lock lost!" << std::endl;
  }
}

void on_firmware_running(bool is_running) 
{
  if (is_running) {
    std::cout << "firmware started" << std::endl;
  } else {
    std::cout << "firmware stopped" << std::endl;
  }
}

void on_firmware_state(uint32_t state) 
{
  std::cout << "firmware state changed to "; 
  print_firmware_state(state);
}

void on_event_source(uint32_t source) 
{
  std::cout << "source type changed to    "; 
  print_event_source(source);
}

void on_in_use(bool in_use) 
{
  if (in_use) {
    std::cout << "gateway used" << std::endl; 
  } else {
    std::cout << "gateway idle, generating EVT_INTERNAL_FILL events" << std::endl;
  }
}


// Function main() 
// ==================================================================================================== 
int main (int argc, char** argv)
{
  // Helpers 
  int     opt            = 0;
  char   *pEnd           = NULL;
  int     info           =  0;
  int     utc_utc_delay  = -1;
  int     trig_utc_delay = -1;
  int64_t utc_offset     = -1;
  int     utc_trigger    = -1;
  int     mil_latency    = -1;
  bool    reset          =  0;
  bool    kill           =  0;
  bool    wait_for_firmware = false;
  bool    configSIS18    = false;
  bool    configESR      = false;
  bool    show_help      = false;
  bool    monitoring     = false;
  bool    receive_mil    = false;
  bool    bugfix_mode    = false;
  bool    clearStat      = false;
  bool    lateHist       = false;
  bool    show_histogram = false;
  bool    read_registers = false;
  bool    request_fill   = false;
#ifdef USEMASP
  bool    masp_productive= false;
#endif

  // Get the application name 
  program = argv[0]; 
  
  // Parse arguments 
  //while ((opt = getopt(argc, argv, "c:dgxzlvh")) != -1)
  while ((opt = getopt(argc, argv, "l:d:u:o:t:wsehrkifPcCLmgbM:HR")) != -1) 
  {
    switch (opt)
    {
      case 'm': { monitoring = true; break; }
      case 'g': { receive_mil = true; break; }
      case 'b': { bugfix_mode = true; break; }
      case 'H': { show_histogram = true; break; }
      case 'R': { read_registers = true; break; }
      case 'c': { red_color = green_color = default_color = ""; break; }
      case 'C': { clearStat = true; break; }
      case 'L': { lateHist = true; break; }
      case 'w': { wait_for_firmware = true; break;}
      case 's': { configSIS18 = true; break; }
      case 'e': { configESR   = true; break; }
      case 'i': { ++info; break; } // more info by putting -i multiple times
      case 'f': { request_fill = true; break; }
#ifdef USEMASP
      case 'P': { masp_productive = true; break; }
#endif
      case 'l': {
         if (argv[optind-1] != NULL) { mil_latency = strtoull(argv[optind-1], &pEnd, 0); }
         else                        { std::cerr << "Error: missing latency value [us]" << std::endl; return -1; } 
         break;
      }
      case 't': {
         if (argv[optind-1] != NULL) { utc_trigger = strtoull(argv[optind-1], &pEnd, 0); }
         else                        { std::cerr << "Error: missing trigger value 0..255" << std::endl; return -1; } 
         break;
      }
      case 'o': {
         if (argv[optind-1] != NULL) { utc_offset = strtoull(argv[optind-1], &pEnd, 0); }
         else                        { std::cerr << "Error: missing offset value [s]" << std::endl; return -1; } 
         break;
      }
      case 'd': {
         if (argv[optind-1] != NULL) { trig_utc_delay = strtoull(argv[optind-1], &pEnd, 0); }
         else                        { std::cerr << "Error: missing delay value [us]" << std::endl; return -1; } 
         break;
      }
      case 'u': {
         if (argv[optind-1] != NULL) { utc_utc_delay = strtoull(argv[optind-1], &pEnd, 0); }
         else                        { std::cerr << "Error: missing delay value [us]" << std::endl; return -1; } 
         break;
      }
      case 'r': { reset = true; break; }
      case 'k': { kill  = true; break; }
      case 'h': { show_help       = true; break; }
      default:  { std::cout << "Unknown argument..." << std::endl; show_help = true; break; }
    }
    // Break loop if help is needed 
    if (show_help) { break; }
  }
    
  // Does the user need help 
  if (show_help)
  {
    wrmilgw_help();
    return (-1);
  }
  
  // Get the device name 
  deviceName = argv[optind];
  
  // Try to connect to saftd 
  try 
  {
    // Search for device name 
    if (deviceName == NULL)  { 
      std::cerr << "Missing device name!" << std::endl;
      wrmilgw_help();
      return -1;
    }
    map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();
    if (devices.find(deviceName) == devices.end())  {
      std::cerr << "Device '" << deviceName << "' does not exist!" << std::endl;
      return -1;
    }


    // Get TimingReceiver Proxy
    std::shared_ptr<TimingReceiver_Proxy> receiver = TimingReceiver_Proxy::create(devices[deviceName]);

    // Search for WR-MIL Gateway 
    map<std::string, std::string> wrmilgws = receiver->getInterfaces()["WrMilGateway"];
    if (wrmilgws.size() != 1) {
      std::cerr << "Device '" << receiver->getName() << "' has no WR-MIL Gateway!" << std::endl;
      return -1;
    }
    // Get Gateway Proxy
    std::shared_ptr<WrMilGateway_Proxy> wrmilgw = WrMilGateway_Proxy::create(wrmilgws.begin()->second);

    if (wait_for_firmware) {
      for (int i = 0; i < 10; ++i) {
        if (wrmilgw->getFirmwareRunning()) {
          break;
        }
        if (i == 9) {
          std::cerr << "timeout while waiting for firmware" << std::endl;
          return 1;
        }
        sleep(1);
      }
    }


    if (clearStat) {
      wrmilgw->ClearStatistics();
    }

    if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_UNCONFIGURED) {
      ///////////////////////////////////////////
      // Gateway configuration (only allowed  if 
      //        firmware is not configured yet)
      ///////////////////////////////////////////

      // set MIL latency
      if (mil_latency >= 0) {
        std::cout << "Setting MIL event latency to " << mil_latency << " us." << std::endl;
        wrmilgw->setEventLatency(mil_latency);
      }
      // set utc_trigger
      if (utc_trigger >= 0) {
        std::cout << "Setting trigger for UTC event generation to event number." << utc_trigger << std::endl;
        wrmilgw->setUtcTrigger(utc_trigger);
      }
      // UTC offset in seconds
      if (utc_offset >= 0) {
        std::cout << "Setting UTC offset to " << utc_offset << " seconds." << std::endl;
        wrmilgw->setUtcOffset(utc_offset);
      }
      // set delays
      if (trig_utc_delay >= 0) {
        std::cout << "Setting delay between trigger and first UTC event to  " << trig_utc_delay << " us." << std::endl;
        wrmilgw->setTriggerUtcDelay(trig_utc_delay);
      }
      if (utc_utc_delay >= 0) {
        std::cout << "Setting delay between two generated UTC events to  " << utc_utc_delay << " us." << std::endl;
        wrmilgw->setUtcUtcDelay(utc_utc_delay);
      }
    } else if (mil_latency   >= 0 ||
              utc_trigger    >= 0 ||
              utc_offset     >= 0 ||
              trig_utc_delay >= 0 || 
              utc_utc_delay  >= 0) {
      std::cerr << red_color << "You cannot change Gateway configuration while Gateway is running" << default_color << std::endl;
      std::cerr << " Reset Gateway first (option -r)" << std::endl;
    }

    if (wrmilgw->getFirmwareRunning()) {
      ///////////////////////////////////////////
      // Gateway actions (only if firmware runs)
      ///////////////////////////////////////////

      if (request_fill) {
        std::cerr << "send fill request" << std::endl;
        wrmilgw->RequestFillEvent();
      }

      // config and start Gateway
      if (configSIS18 && configESR) {
        std::cerr << red_color << "You cannot configure Gateway as SIS18 _and_ ESR" << default_color << std::endl;
        std::cerr << " choose SIS18 (option -s) or ESR (option -e)" << std::endl;
        return -1;
      } else if (configSIS18) {
        if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_UNCONFIGURED) {
          std::cout << "Starting WR-MIL Gateway as SIS18 Pulszentrale" << std::endl;
          destroyGatewayConditions(receiver);
          createCondition(wrmilgw, receiver, SIS18EventID);
          wrmilgw->StartSIS18();
        } else if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_CONFIGURED) {
          std::cerr << red_color << "Gateway is already configured and running." << default_color << std::endl;
          std::cerr << " If you want to change the configuration" << std::endl;
          std::cerr << " you need to reset first (option -r)" << std::endl;
        }
      } else if (configESR) {
        if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_UNCONFIGURED) {
          std::cout << "Starting WR-MIL Gateway as ESR Pulszentrale" << std::endl;
          destroyGatewayConditions(receiver);
          createCondition(wrmilgw, receiver, ESREventID);
          wrmilgw->StartESR();
        } else if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_CONFIGURED) {
          std::cerr << red_color << "Gateway is already configured and running." << default_color << std::endl;
          std::cerr << " If you want to change the configuration" << std::endl;
          std::cerr << " you need to reset first (option -r)" << std::endl;
        }
      }  

      if (kill) {
        std::cout << "Killing Gateway." << std::endl;
        destroyGatewayConditions(receiver);
        wrmilgw->KillGateway();
      }
      if (reset) {
        std::cout << "Pausing Gateway firmware, resetting after 1 second." << std::endl;
        destroyGatewayConditions(receiver);
        wrmilgw->ResetGateway();
      }
    } else if (configESR   ||
               configSIS18 || 
               kill        ||
               reset) { // Firmware not running
      std::cerr << red_color << "Cannot act on gateway because firmware is not running" << default_color << std::endl;
    }

    // Print Info (as much as asked for)
    if (info > 0) { 
      print_info1(receiver, wrmilgw);
    } 
    if (info > 1) {
      print_info2(receiver, wrmilgw);
    }
    if (info > 2) {
      print_info3(receiver, wrmilgw);
    }

    if (read_registers) {
      std::vector<std::string> registerNames;
      registerNames.push_back("MAGIC_NUMBER ");
      registerNames.push_back("COMMAND      ");
      registerNames.push_back("UTC_TRIGGER  ");
      registerNames.push_back("UTC_DELAY    ");
      registerNames.push_back("TRIG_UTC_DELA");
      registerNames.push_back("EVENT_SOURCE ");
      registerNames.push_back("LATENCY      ");
      registerNames.push_back("STATE        ");
      registerNames.push_back("UTC_OFFSET_HI");
      registerNames.push_back("UTC_OFFSET_LO");
      registerNames.push_back("NUM_EVENTS_HI");
      registerNames.push_back("NUM_EVENTS_LO");
      registerNames.push_back("LATE_EVENTS  ");
      auto registerContent = wrmilgw->getRegisterContent();
      unsigned idx = 0;
      for (auto reg: registerContent) {
        std::cout << "0x" << std::hex << std::setfill('0') << std::setw(2) << idx << " " 
                  << registerNames[idx] << " : " 
                  << "0x" << std::hex << std::setfill('0') << std::setw(8) << reg 
                  << std::dec << std::endl;
        ++idx;          
      }
    }

    if (lateHist) {
      auto histogram = wrmilgw->getLateHistogram();
      std::cout << "late mil events:" << std::endl;
      std::cout << std::setw(10) << "delay[us]" << std::setw(10) << "count" << std::endl;
      for (unsigned i = 0; i < histogram.size(); ++i) {
        std::cout << "  < " << std::dec << std::setw(5) << (1<<(i+10))/1000 << std::setw(10) << histogram[i] << std::endl;
      }
    }

    if (show_histogram) {
      std::cout << "MIL event histogram" << std::endl;
      auto histogram = wrmilgw->getMilHistogram();
      double max = 0;
      for (auto bin: histogram) {
        if (max < bin) {
          max = bin;
        }
      }
      if (max == 0) {
        std::cout << "histogram empty" << std::endl;
      } else {
        std::cout << "max value = " << max << std::endl;
        std::cout << std::setw (10) << "EVT_NO" << std::setw(10) << "count" << std::endl;
        for (unsigned int i = 0; i < histogram.size(); ++i) {
          if (histogram[i]) {
            std::cout << std::setw (10) << std::dec << i << std::setw(10) << histogram[i] << std::endl;
          }
        }
      }
    }

    ///////////////////////////////////////////
    // Snoop for Events received on MIl Piggy
    ///////////////////////////////////////////
    if (receive_mil) {
      std::cout << "MIL monitoring anabled" << std::endl;
      wrmilgw->SigReceivedMilEvent.connect(sigc::ptr_fun(on_mil));
      while(true) {
        saftlib::wait_for_signal();
      }
      return 0;
    }

    ///////////////////////////////////////////
    // Bugfix Mode
    // snoop for the WR-events that are supposed to be translated to MIL
    // snoop for 0xffffffff00000000 events (the ones that trigger the MIL-piggy)
    // snoop for MIL events 
    // normally, all three sholud should be present.
    // if one is missing. report to std::cerr
    ///////////////////////////////////////////
    if (bugfix_mode) {
      BugfixMode bugfix_mode;
      auto source = wrmilgw->getEventSource();
      auto eventID = SIS18EventID;
      // auto eventMask = UINT64_C(0xfffff00000000000);
      if (source == WR_MIL_GW_EVENT_SOURCE_ESR) {
        eventID = ESREventID;
      }
      std::cerr << "Looking for missing MIL events" << std::endl;
      auto sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
      auto TriggerCondition = SoftwareCondition_Proxy::create(sink->NewCondition(true, 0xffffffff00000000, 0xffffffff00000000, 0));
      TriggerCondition->setAcceptLate(true);
      TriggerCondition->setAcceptEarly(true);
      TriggerCondition->setAcceptConflict(true);
      TriggerCondition->setAcceptDelayed(true);
      TriggerCondition->SigAction.connect(sigc::mem_fun(bugfix_mode, &BugfixMode::on_event_trigger));

      wrmilgw->SigReceivedMilEvent.connect(sigc::mem_fun(bugfix_mode, &BugfixMode::on_mil));

      // configure the outputs
      bugfix_mode.B1 = saftlib::Output_Proxy::create("/de/gsi/saftlib/tr0/outputs/B1");
      bugfix_mode.B2 = saftlib::Output_Proxy::create("/de/gsi/saftlib/tr0/outputs/B2");

      bugfix_mode.B1->Own();
      bugfix_mode.B2->Own();

      bugfix_mode.B1->WriteOutput(false);
      bugfix_mode.B2->WriteOutput(false);

      bugfix_mode.B1->setOutputEnable(true);
      bugfix_mode.B2->setOutputEnable(true);

      // create a 16 us long pulse on ouput B2 when a MIL event is sent to the MIL piggy by the ECA wb-master output channel
      auto condition1 = OutputCondition_Proxy::create(bugfix_mode.B2->NewCondition(true, eventID, 0xffffffff00000000,     0, 1));
      auto condition2 = OutputCondition_Proxy::create(bugfix_mode.B2->NewCondition(true, eventID, 0xffffffff00000000, 16000, 0));
      condition1->setAcceptConflict(true);
      condition1->setAcceptDelayed(true);
      condition1->setAcceptEarly(true);
      condition1->setAcceptLate(true);
      condition2->setAcceptConflict(true);
      condition2->setAcceptDelayed(true);
      condition2->setAcceptEarly(true);
      condition2->setAcceptLate(true);


      while(true) {
        saftlib::wait_for_signal();
      }
      return 0;
    }


    ///////////////////////////////////////////
    // Monitoring Loop
    ///////////////////////////////////////////
    if (monitoring) {

      // snoop for late MIL events 
      std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
      std::shared_ptr<SoftwareCondition_Proxy> condition 
        = SoftwareCondition_Proxy::create(sink->NewCondition(false, 0xffffffff00000000, 0xffffffff00000000, 0));
      // Accept all errors
      condition->setAcceptLate(true);
      condition->setAcceptEarly(true);
      condition->setAcceptConflict(true);
      condition->setAcceptDelayed(true);
      condition->SigAction.connect(sigc::bind(sigc::ptr_fun(&on_action), wrmilgw) );
      condition->setActive(true);


      // switch off colors in monitoring mode
      red_color = green_color = default_color = "";
      // if monitoring was called on a running gateway and 
      //  no SIS18/ESR information was given, then get this
      //  information from the gateway.
      if (configSIS18 == false && configESR == false) {
        auto source = wrmilgw->getEventSource();
        if (source == WR_MIL_GW_EVENT_SOURCE_SIS) {
          configSIS18 = true;
        } else if (source == WR_MIL_GW_EVENT_SOURCE_ESR) {
          configESR = true;
        }
      }

    
    ///////////////////////////////////////////
    // MASP relevant stuff
    ///////////////////////////////////////////
#ifdef USEMASP
        // send MASP status
      std::string nomen("U_WR2MIL_");
      if (configSIS18) {
        nomen.append("SIS18");
      }
      if (configESR) {
        nomen.append("ESR");
      }
      char hostname_cstr[100];
      gethostname(hostname_cstr,100);
      std::string hostname(hostname_cstr);
      std::string source_id(nomen);
      source_id.append(".");
      source_id.append(hostname);
      MASP::StatusEmitter emitter(MASP::StatusEmitterConfig(
          MASP::StatusEmitterConfig::CUSTOM_EMITTER_DEFAULT(),
          source_id, masp_productive ));
      //printf ("prepare MASP status emitter with source_id: %s, nomen: %s, productive: %\n");
      std::cout << "prepare MASP status emitter with "
                << "source_id: " << source_id 
                << ",  nomen: " << nomen
                << ",  productive: " << (masp_productive?"true":"false")
                << std::endl;

      MASP::no_logger no_log;
      MASP::Logger::middleware_logger = &no_log;
#endif  // USEMASP

      // connect some callbacks
      receiver->SigLocked.connect(sigc::ptr_fun(&on_locked));
      wrmilgw->SigFirmwareRunning.connect(sigc::ptr_fun(&on_firmware_running));
      wrmilgw->SigFirmwareState.connect(sigc::ptr_fun(&on_firmware_state));
      wrmilgw->SigEventSource.connect(sigc::ptr_fun(&on_event_source));
      wrmilgw->SigInUse.connect(sigc::ptr_fun(&on_in_use));

      bool opReady = op_ready(receiver->getLocked(), 
                              wrmilgw->getFirmwareRunning(),
                              wrmilgw->getFirmwareState());
      // initially set the opReady state
      wrmilgw->setOpReady(opReady);

      std::cout << "start monitoring loop" << std::endl;
      while(true) {
        saftlib::wait_for_signal();
        // check OP_READY and notify on change
        bool new_opReady = op_ready(receiver->getLocked(), 
                                    wrmilgw->getFirmwareRunning(),
                                    wrmilgw->getFirmwareState());
#ifdef USEMASP
        {
          MASP::End_of_scope_status_emitter scoped_emitter(nomen,emitter);
          scoped_emitter.set_OP_READY(new_opReady);
        }
#endif 
        if (new_opReady != opReady) {
          std::cout << "OP_READY=" << (new_opReady?"YES":"NO ") << std::endl;
          wrmilgw->setOpReady(new_opReady);
          wrmilgw->UpdateOLED();
          opReady = new_opReady;
        }
      }
    }
    
  } 
  catch (const saftbus::Error& error)
  {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  // Done 
  return (0);
}


std::string mil_description(uint8_t mil) {
  switch(mil) {
    case 0: return std::string("EVT_PZ_ChanEnd  SIS/ESR PZ only: mark end of channel  ");
    case 1: return std::string("EVT_START_RF  Power to RF cavities  ");
    case 2: return std::string("EVT_START_IQ  Begin of beam production  ");
    case 3: return std::string("EVT_IQ_HEATING  Begin of ion source arc, ECR RF   ");
    case 4: return std::string("EVT_PREP_BEAM_ON  Switch on chopper, read act. values   ");
    case 5: return std::string("EVT_IQ_GAS_ON   Start of heading gas (ion source)   ");
    case 6: return std::string("EVT_BEAM_ON   Valid beam  ");
    case 8: return std::string("EVT_BEAM_OFF  End of beam production  ");
    case 10: return std::string("EVT_STOP_IQ   Switch IQ off   ");
    case 12: return std::string("EVT_STOP_RF   Switch RF off   ");
    case 16: return std::string("EVT_PREP_NEXT_ACC   Prepare next acc., write set values   ");
    case 17: return std::string("EVT_AUX_PRP_NXT_ACC   Set values in magnet prep. cycles   ");
    case 18: return std::string("EVT_RF_PREP_NXT_ACC   Begin of RF extra cycle   ");
    case 19: return std::string("EVT_PREP_UNI_DIAG   Prepare diagnostic devices, Unilac  ");
    case 20: return std::string("EVT_PREP_AUX  Trigger BIF beam diagnostics  ");
    case 22: return std::string("EVT_PREP_EXP  Pretrigger for Experiments  ");
    case 24: return std::string("EVT_RF_AUX_TRIG   Trigger ADC in RF extra cycles  ");
    case 25: return std::string("EVT_MAGN_DOWN   Set magnets to zero current   ");
    case 26: return std::string("EVT_SD_AUX_START  Beam diagnostics aux start trigger  ");
    case 27: return std::string("EVT_SD_AUX_STOP   Beam diagnostics aux stop trigger   ");
    case 28: return std::string("EVT_PRETRIG_BEAM  Magnets on flattop, PG trigger  ");
    case 29: return std::string("EVT_UNI_END_CYCLE   End of a UNILAC cycle   ");
    case 30: return std::string("EVT_READY_TO_SIS  10 ms before beam transfer  ");
    case 31: return std::string("EVT_SRC_DST_ID  Source/Destination of beam  ");
    case 32: return std::string("EVT_START_CYCLE   First Event in a cycle  ");
    case 33: return std::string("EVT_START_BFELD   Start of B-Field  ");
    case 34: return std::string("EVT_PEAK_UP   Peaking trip up   ");
    case 35: return std::string("EVT_INJECT  B-field reached injection level   ");
    case 36: return std::string("EVT_UNI_TRANS   Demand UNILAC beam  ");
    case 37: return std::string("EVT_UNI_ACKN  Acknowledge from Unilac   ");
    case 38: return std::string("EVT_UNI_READY   Unilac ready, transfer in preparation   ");
    case 39: return std::string("EVT_MB_LOAD   Start Bumper magnet ramp up   ");
    case 40: return std::string("EVT_MB_TRIGGER  Start active Bumper Ramp (down)   ");
    case 41: return std::string("EVT_INJECT_END  Start of injection from unilac  ");
    case 42: return std::string("EVT_TIMEOUT_1   Trigger timeout channel 1   ");
    case 43: return std::string("EVT_RAMP_START  Start of acc/deacc ramp in magnets  ");
    case 44: return std::string("EVT_PREP_INJ  Prepare devices for next Injection  ");
    case 45: return std::string("EVT_FLATTOP   Magnets reached Flattop   ");
    case 46: return std::string("EVT_EXTR_START_SLOW   Start of extraction   ");
    case 47: return std::string("EVT_MK_LOAD_1   Load Kicker for HF-triggered puls   ");
    case 48: return std::string("EVT_MK_LOAD_2   Load Kicker for Timinggenerator-triggered puls  ");
    case 49: return std::string("EVT_KICK_START_1  Start Kicker for HF-triggered extraction  ");
    case 50: return std::string("EVT_TIMEOUT_2   Trigger timeout channel 2   ");
    case 51: return std::string("EVT_EXTR_END  End of extraction   ");
    case 52: return std::string("EVT_FLATTOP_END   End of Flattop (Magnets) reached  ");
    case 53: return std::string("EVT_PREP_EXTR   Prepare devices for next Extraction   ");
    case 54: return std::string("EVT_PEAK_DOWN   Peaking strip down  ");
    case 55: return std::string("EVT_END_CYCLE   End of a cycle  ");
    case 56: return std::string("EVT_SYNCH   Trigger all function generators   ");
    case 57: return std::string("EVT_EXTR_BUMP   Start of closed orbit distortion  ");
    case 58: return std::string("EVT_SIS_ACK_TO_ESR  SIS acknowledge to ESR  ");
    case 59: return std::string("EVT_SIS_READY_1   SIS ready for extraction to ESR   ");
    case 60: return std::string("EVT_SIS_READY_2   SIS ready for extraction to ESR   ");
    case 61: return std::string("EVT_TRANS_START_1   Begin of transmission to ESR  ");
    case 62: return std::string("EVT_TRANS_START_2   Begin of transmission to ESR  ");
    case 63: return std::string("EVT_PHASE_SYNCH_GATE_1  Begin of first phase synchronisation between ESR- and SIS-HF  ");
    case 64: return std::string("EVT_TIMEOUT_3   Trigger timeout channel 3   ");
    case 65: return std::string("EVT_PHASE_SYNCH_GATE_2  Begin of second phase synchronisation between ESR- and SIS-HF   ");
    case 66: return std::string("EVT_TIMEOUT_4   Trigger timeout channel 4   ");
    case 67: return std::string("EVT_TIMEOUT_5   Trigger timeout channel 5   ");
    case 68: return std::string("EVT_TIMEOUT_6   Trigger timeout channel 6   ");
    case 69: return std::string("EVT_KICK_START_2  Start Kicker for TG-triggered extraction  ");
    case 70: return std::string("EVT_UNI_PREP  Demand setting of TK (200 ms before beam)   ");
    case 71: return std::string("EVT_INJ_BUMP  Closed orbit destortion for reinjection   ");
    case 72: return std::string("EVT_RE_INJ_START  SIS is ready for reinjection  ");
    case 73: return std::string("EVT_RE_INJ_END  End of reinjection  ");
    case 74: return std::string("EVT_PREP_RE_INJ   Prepare devices for Reinjection   ");
    case 75: return std::string("EVT_PREP_KICK_1   Prepare kicker: load capacitor 1  ");
    case 76: return std::string("EVT_PREP_KICK_2   Prepare kicker: load capacitor 2  ");
    case 77: return std::string("EVT_MK_LOAD_RE_INJ  Load Kicker for Reinjection   ");
    case 78: return std::string("EVT_EXTR_STOP_SLOW  End of slow extraction  ");
    case 79: return std::string("EVT_ASYNC_TRANS   Transfer to ESR without HF synchron.  ");
    case 80: return std::string("EVT_EMA_START   Start EMA meassurement gate   ");
    case 81: return std::string("EVT_HF_BM_START   Vorbereitung Strahlgym.   ");
    case 82: return std::string("EVT_HF_BM_AMPH2   Start Ampl., Ph. Kav. 2 Strahlgym.  ");
    case 83: return std::string("EVT_HF_BM_FREQ2   Start Freq. Kav. 2 Strahlgym.   ");
    case 84: return std::string("EVT_HF_BM_AMPH12  Start Ampl., Ph. Kav. 1+2 Strahlgym.  ");
    case 85: return std::string("EVT_HF_BM_FREQ1   Start Freq. Kav. 1 Strahlgym.   ");
    case 86: return std::string("EVT_HF_BM_AMPH1   Start Ampl., Ph. Kav. 1 Strahlgym.  ");
    case 87: return std::string("EVT_TG_CLEAR  Clear Timinggenerator   ");
    case 88: return std::string("EVT_MQ_START1   Load Q-Kicker (1st shot)  ");
    case 89: return std::string("EVT_MQ_START2   Load Q-Kicker (2nd shot)  ");
    case 90: return std::string("EVT_MQ_MESS   Trigger for Q-Wert measurement  ");
    case 91: return std::string("EVT_INT_FLAT  Start of internal Flattop   ");
    case 92: return std::string("EVT_DT_MESS   Trigger for DTML measurement  ");
    case 93: return std::string("EVT_DX_MESS   Trigger for DX measurement  ");
    case 94: return std::string("EVT_TG_SWITCH   Umschalten der TG synchronisation   ");
    case 95: return std::string("EVT_START_LECROY  Start measurement with LeCroy   ");
    case 96: return std::string("EVT_GAP_POS_MESS  Messevent zwischen den Zyklen   ");
    case 97: return std::string("EVT_GAP_TRA_MESS  Messevent zwischen den Zyklen   ");
    case 98: return std::string("EVT_GAP_SCR_MESS  Messevent zwischen den Zyklen   ");
    case 99: return std::string("EVT_GAP_DTS_MESS  Messevent für schnelle Trafos   ");
    case 100: return std::string("EVT_SIS_TRANS1_ESR  First transmission complete to ESR  ");
    case 101: return std::string("EVT_SIS_TRANS2_ESR  Second transmission complete to ESR   ");
    case 102: return std::string("EVT_PREP_DIAGNOSE   ...   ");
    case 103: return std::string("EVT_PREP_DG   Vorbereitung Gitterhardware   ");
    case 104: return std::string("EVT_DG_TRIGGER  Trigger Messung Gitterhardware  ");
    case 105: return std::string("EVT_KICK_READY  Ext. Synchronisat. für Kicker   ");
    case 106: return std::string("EVT_KICK_GATE   Start externe Synchr. für Kicker  ");
    case 107: return std::string("EVT_PREP_DTI  Entklemmung TK trafos SIS timing  ");
    case 108: return std::string("EVT_INJ_READY   Einzelne Unilac-Injektion erfolgt   ");
    case 109: return std::string("EVT_MHB   Multi-Harmonischer-Betrieb von GS00BE_S (SIS18)   ");
    case 128: return std::string("EVT_ESR_READY_1   ESR ready for beam transfer   ");
    case 129: return std::string("EVT_ESR_READY_2   ESR ready for beam transfer   ");
    case 130: return std::string("EVT_ESR_REQ_TO_SIS  ESR beam request to SIS   ");
    case 131: return std::string("EVT_EIN1  ESR ????? 1   ");
    case 132: return std::string("EVT_EIN2  ESR ????? 2   ");
    case 133: return std::string("EVT_MAN1  ESR manipulation 1  ");
    case 134: return std::string("EVT_MAN2  ESR manipulation 2  ");
    case 135: return std::string("EVT_PHASE_SYNCH_1   Phase is 1st time synchronized between ESR- and SIS-HF  ");
    case 136: return std::string("EVT_PHASE_SYNCH_2   Phase is 2nd time synchronized between ESR- and SIS-HF  ");
    case 137: return std::string("EVT_NO_BEAM   There is no beam in ESR (for diagnostics)   ");
    case 138: return std::string("EVT_DT_STOP   Stop of DTML measurement  ");
    case 139: return std::string("EVT_DT_READ   Read data for DTML measurement  ");
    case 140: return std::string("EVT_DX_STOP   Stop of DX measurement  ");
    case 141: return std::string("EVT_DX_READ   Read data for DX measurement  ");
    case 142: return std::string("EVT_LEXT  ESR start with ?????????????  ");
    case 143: return std::string("EVT_PSTACK  ESR start with ?????????????  ");
    case 144: return std::string("EVT_STACK   ESR start with ?????????????  ");
    case 145: return std::string("EVT_ESR_TRANS_SIS   Transmission complete to SIS  ");
    case 146: return std::string("EVT_ECE_HV_VAR  Stepwise variation of ECE voltage   ");
    case 147: return std::string("EVT_ECE_HV_ON   Write set value for pulsed ECE HV   ");
    case 148: return std::string("EVT_ECE_HV_MESS   Read actual value of pulsed ECE HV  ");
    case 149: return std::string("EVT_BUNCH_ROTATE  Start with bunch rotation   ");
    case 150: return std::string("EVT_ESR_REQ_REINJ   ESR request SIS reinjection   ");
    case 151: return std::string("EVT_RESET   Start of 11th Ramp in magnets   ");
    case 152: return std::string("EVT_AUS1  Start of 11th Ramp in magnets   ");
    case 153: return std::string("EVT_AUS2  Start of 11th Ramp in magnets   ");
    case 154: return std::string("EVT_RAMP_11   Start of 11th Ramp in magnets   ");
    case 155: return std::string("EVT_RAMP_12   Start of 12th Ramp in magnets   ");
    case 156: return std::string("EVT_RAMP_13   Start of 13th Ramp in magnets   ");
    case 157: return std::string("EVT_RAMP_14   Start of 14th Ramp in magnets   ");
    case 158: return std::string("EVT_RAMP_15   Start of 15th Ramp in magnets   ");
    case 159: return std::string("EVT_RAMP_16   Start of 16th Ramp in magnets   ");
    case 160: return std::string("EVT_EBEAM_ON  Electron beam on  ");
    case 161: return std::string("EVT_EBEAM_OFF   Electron beam off   ");
    case 162: return std::string("EVT_UDET_IN   Move detector (charge changed) in   ");
    case 163: return std::string("EVT_UDET_OUT  Move detector out   ");
    case 180: return std::string("EVT_TIMING_LOCAL  Take local timing   ");
    case 181: return std::string("EVT_TIMING_EXTERN   Switch to extern timing   ");
    case 199: return std::string("EVT_INTERNAL_FILL   PZ: Fill long event delays (>10s)   ");
    case 200: return std::string("EVT_DATA_START  First data transfer event   ");
    case 201: return std::string("EVT_DATA_0  Data transfer event   ");
    case 202: return std::string("EVT_DATA_1  Data transfer event   ");
    case 203: return std::string("EVT_DATA_2  Data transfer event   ");
    case 204: return std::string("EVT_DATA_3  Data transfer event   ");
    case 205: return std::string("EVT_DATA_4  Data transfer event   ");
    case 206: return std::string("EVT_DATA_5  Data transfer event   ");
    case 207: return std::string("EVT_DATA_6  Data transfer event   ");
    case 208: return std::string("EVT_DATA_7  Data transfer event   ");
    case 209: return std::string("EVT_TIME_1  Time stamp, most sign. bits   ");
    case 210: return std::string("EVT_TIME_2  Time stamp, medium sign. bits   ");
    case 211: return std::string("EVT_TIME_3  Time stamp, least sign. bits  ");
    case 224: return std::string("EVT_UTC_1   Time stamp UTC bits 32..39  ");
    case 225: return std::string("EVT_UTC_2   Time stamp UTC bits 24..31  ");
    case 226: return std::string("EVT_UTC_3   Time stamp UTC bits 16..23  ");
    case 227: return std::string("EVT_UTC_4   Time stamp UTC bits 8..15   ");
    case 228: return std::string("EVT_UTC_5   Time stamp UTC bits 0.. 7   ");
    case 245: return std::string("EVT_END_CMD_EXEC  End of command evaluation within gap  ");
    case 246: return std::string("EVT_BEGIN_CMD_EXEC  Start of command evaluation within gap  ");
    case 247: return std::string("EVT_GAP_INFO  PZ information about next cycle   ");
    case 248: return std::string("EVT_COLL_DET  PZ detected collision of 2 asynch. events   ");
    case 249: return std::string("EVT_TIMING_DIAG   For diagnostics in timing system  ");
    case 250: return std::string("EVT_SUP_CYCLE_START   Supercycle starts   ");
    case 251: return std::string("EVT_GET_EC_TIME   Synchronous reading of time of all ECs  ");
    case 252: return std::string("EVT_SET_EC_TIME   Synchronous setting of time in all ECs  ");
    case 253: return std::string("EVT_RESERVED  In older systems : change vrtacc event  ");
    case 254: return std::string("EVT_EMERGENCY   Emergency event   ");
    case 255: return std::string("EVT_COMMAND   Command event  ");
  }
  return std::string("unkonw");
}

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
#include <giomm.h>

#include "SAFTd.h"
#include "EmbeddedCPUActionSink.h"
#include "EmbeddedCPUCondition.h"
#include "TimingReceiver.h"
#include "WrMilGateway.h"
#include "wr_mil_gw_regs.h"

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
void print_firmware_state(guint32 firmware_state);
void print_event_source(guint32 event_source);
void print_in_use(bool in_use);
void print_info1(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw);
void print_info2(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw);
void print_info3(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw);
void createCondition(Glib::RefPtr<TimingReceiver_Proxy> receiver, guint64 eventID);
void destroyGatewayConditions(Glib::RefPtr<TimingReceiver_Proxy> receiver);
void on_locked(bool is_locked);
void on_firmware_running(bool is_running);
void on_firmware_state(guint32 state);
void on_event_source(guint32 source);
void on_num_late_mil_events(guint32 total, guint32 since_last_signal, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw);
void on_in_use(bool in_use);

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
  std::cout << "  -i                            Show gateway information. Repeat the option"    << std::endl;
  std::cout << "                                 to get more detailed information, e.g. -iii"   << std::endl;
  std::cout << "  -m                            Start monitoring loop"                          << std::endl;
  std::cout << "  -H                            Show MIL-event histogram"                       << std::endl;
  std::cout << "  -s                            Start WR-MIL Gateway as SIS18 Pulszentrale"     << std::endl;
  std::cout << "  -e                            Start WR-MIL Gateway as ESR   Pulszentrale"     << std::endl;
  std::cout << "  -l <latency>                  Set MIL event latency [us]"                     << std::endl;
  std::cout << "  -t <trigger>                  Set UTC-trigger event [0..255]"                 << std::endl;
  std::cout << "  -o <offset>                   Set UTC-offset [s] (value is added to WR-time)" << std::endl;
  std::cout << "  -d <delay>                    Set Trigger-UTC delay [us]"                     << std::endl;
  std::cout << "  -u <delay>                    Set UTC-UTC delay [us]"                         << std::endl;
  std::cout << "  -r                            Pause gateway for 1 s, and reset"               << std::endl;
  std::cout << "  -k                            Kill gateway. Only LM32 reset can recover."     << std::endl;
  std::cout << "                                 This is useful before flashing new firmware."  << std::endl;
  std::cout << "  -c                            No color on console ouput"                      << std::endl;
  std::cout << "  -h                            Print help (this message)"                      << std::endl;
  std::cout << std::endl;
}

std::string red_color       = "\033[1;31m";
std::string green_color     = "\033[1;32m";
std::string default_color   = "\033[0m";

const int key_width = 25;
const int value_width = 15;

bool op_ready(bool receiver_locked, bool firmware_running, int firmware_state)
{
  return     (receiver_locked  == true)
          && (firmware_running == true)
          && (firmware_state   == WR_MIL_GW_STATE_CONFIGURED);
}

void print_firmware_state(guint32 firmware_state)
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

void print_event_source(guint32 event_source)
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
void print_info1(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw)
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
void print_info2(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw)
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
void print_info3(Glib::RefPtr<TimingReceiver_Proxy> receiver, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw)
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

void createCondition(Glib::RefPtr<TimingReceiver_Proxy> receiver, guint64 eventID)
{
  // create the embedded CPU action sink for SIS18 WR events
  std::map<Glib::ustring, Glib::ustring> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
  if (e_cpus.size() != 1)  {
    throw IPC_METHOD::Error(IPC_METHOD::Error::FAILED, "No embedded CPU action sink found");
  }
  Glib::RefPtr<EmbeddedCPUActionSink_Proxy> e_cpu 
      = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);
  auto eventMask = UINT64_C(0xfffff00000000000);
  auto offset    = INT64_C(-100000);
  auto tag       = UINT32_C(0x4);

  Glib::RefPtr<EmbeddedCPUCondition_Proxy> condition 
      = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(true, eventID, eventMask, offset, tag));
  // Accept every kind of event 
  condition->setAcceptConflict(true);
  condition->setAcceptDelayed(true);
  condition->setAcceptEarly(true);
  condition->setAcceptLate(true);
  condition->Disown();
}
void destroyGatewayConditions(Glib::RefPtr<TimingReceiver_Proxy> receiver)
{
  // Get the conditions
  std::map<Glib::ustring, Glib::ustring> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
  if (e_cpus.size() != 1)  {
    throw IPC_METHOD::Error(IPC_METHOD::Error::FAILED, "No embedded CPU action sink found");
  }
  Glib::RefPtr<EmbeddedCPUActionSink_Proxy> e_cpu 
      = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);
  std::vector< Glib::ustring > all_conditions = e_cpu->getAllConditions();
 
  // Destroy conditions if possible
  for (auto condition_name: all_conditions)  {
    Glib::RefPtr<EmbeddedCPUCondition_Proxy> condition = EmbeddedCPUCondition_Proxy::create(condition_name);
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
    std::cout << "WR-MIL-Gateway: got WR-Lock" << std::endl;
  } else {
    std::cout << "WR-MIL-Gateway: WR-Lock lost!" << std::endl;
  }
}

void on_firmware_running(bool is_running) 
{
  if (is_running) {
    std::cout << "WR-MIL-Gateway: firmware started" << std::endl;
  } else {
    std::cout << "WR-MIL-Gateway: firmware stopped" << std::endl;
  }
}

void on_firmware_state(guint32 state) 
{
  std::cout << "WR-MIL-Gateway: firmware state changed to "; 
  print_firmware_state(state);
}

void on_event_source(guint32 source) 
{
  std::cout << "WR-MIL-Gateway: source type changed to    "; 
  print_event_source(source);
}

void on_num_late_mil_events(guint32 total, guint32 since_last_signal, Glib::RefPtr<WrMilGateway_Proxy> wrmilgw) 
{
  // If gateway is reset, callback will be called with total=0
  // To prevent this to be displayed as "late MIL event detected", 
  //  we treat this as a special case.
  if (total > 0) {
    std::cout << "WR-MIL-Gateway: late MIL event detected. Total/New = " 
              << total << '/' << since_last_signal
              << ". Histogram = ";
    auto histogram = wrmilgw->getLateHistogram();
    for (auto bin: histogram) {
      std::cout << bin << " ";
    }
    std::cout << std::endl;
  }
}

void on_in_use(bool in_use) 
{
  if (in_use) {
    std::cout << "WR-MIL-Gateway: gateway is in use (seeing MIL events)" << std::endl; 
  } else {
    std::cout << "WR-MIL-Gateway: gateway not used (no MIL events for more than 10 seconds)" << std::endl;
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
  bool    configSIS18    = false;
  bool    configESR      = false;
  bool    show_help      = false;
  bool    monitoring     = false;
  bool    clearStat      = false;
  bool    lateHist       = false;
  bool    show_histogram = false;
  
  // Get the application name 
  program = argv[0]; 
  
  // Parse arguments 
  //while ((opt = getopt(argc, argv, "c:dgxzlvh")) != -1)
  while ((opt = getopt(argc, argv, "l:d:u:o:t:sehrkicCLmM:H")) != -1) 
  {
    switch (opt)
    {
      case 'm': { monitoring = true; break; }
      case 'H': { show_histogram = true; break; }
      case 'c': { red_color = green_color = default_color = ""; break; }
      case 'C': { clearStat = true; break; }
      case 'L': { lateHist = true; break; }
      case 's': { configSIS18 = true; break; }
      case 'e': { configESR   = true; break; }
      case 'i': { ++info; break; } // more info by putting -i multiple times
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
  
  // Initialize Glib stuff 
  Gio::init();
  
  // Try to connect to saftd 
  try 
  {
    // Search for device name 
    if (deviceName == NULL)  { 
      std::cerr << "Missing device name!" << std::endl;
      wrmilgw_help();
      return -1;
    }
    map<Glib::ustring, Glib::ustring> devices = SAFTd_Proxy::create()->getDevices();
    if (devices.find(deviceName) == devices.end())  {
      std::cerr << "Device '" << deviceName << "' does not exist!" << std::endl;
      return -1;
    }


    saftlib::SignalGroup signal_group;

    // Get TimingReceiver Proxy
    Glib::RefPtr<TimingReceiver_Proxy> receiver = TimingReceiver_Proxy::create(devices[deviceName], signal_group);

    // Search for WR-MIL Gateway 
    map<Glib::ustring, Glib::ustring> wrmilgws = receiver->getInterfaces()["WrMilGateway"];
    if (wrmilgws.size() != 1) {
      std::cerr << "Device '" << receiver->getName() << "' has no WR-MIL Gateway!" << std::endl;
      return -1;
    }
    // Get Gateway Proxy
    Glib::RefPtr<WrMilGateway_Proxy> wrmilgw = WrMilGateway_Proxy::create(wrmilgws.begin()->second, signal_group);

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

      // config and start Gateway
      if (configSIS18 && configESR) {
        std::cerr << red_color << "You cannot configure Gateway as SIS18 _and_ ESR" << default_color << std::endl;
        std::cerr << " choose SIS18 (option -s) or ESR (option -e)" << std::endl;
        return -1;
      } else if (configSIS18) {
        if (wrmilgw->getFirmwareState() == WR_MIL_GW_STATE_UNCONFIGURED) {
          std::cout << "Starting WR-MIL Gateway as SIS18 Pulszentrale" << std::endl;
          destroyGatewayConditions(receiver);
          createCondition(receiver, SIS18EventID);
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
          createCondition(receiver, ESREventID);
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
    // Monitoring Loop
    ///////////////////////////////////////////
    if (monitoring) {
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

      // connect some callbacks
      receiver->SigLocked.connect(sigc::ptr_fun(&on_locked));
      wrmilgw->SigFirmwareRunning.connect(sigc::ptr_fun(&on_firmware_running));
      wrmilgw->SigFirmwareState.connect(sigc::ptr_fun(&on_firmware_state));
      wrmilgw->SigEventSource.connect(sigc::ptr_fun(&on_event_source));
      wrmilgw->SigNumLateMilEvents.connect(sigc::bind(sigc::ptr_fun(&on_num_late_mil_events), wrmilgw));
      wrmilgw->SigInUse.connect(sigc::ptr_fun(&on_in_use));

      bool opReady = op_ready(receiver->getLocked(), 
                              wrmilgw->getFirmwareRunning(),
                              wrmilgw->getFirmwareState());

      std::cout << "WrMilGateway: starting monitoring loop" << std::endl;
      while(true) {
        signal_group.wait_for_signal();
        // check OP_READY and notify on change
        bool new_opReady = op_ready(receiver->getLocked(), 
                                    wrmilgw->getFirmwareRunning(),
                                    wrmilgw->getFirmwareState());
#ifdef USEMASP
        {
          MASP::End_of_scope_status_emitter scoped_emitter(nomen,emitter);
          scoped_emitter.set_OP_READY(op_ready);
        }
#endif 
        if (new_opReady != opReady) {
          std::cout << "WR-MIL-Gateway: OP_READY=" << (new_opReady?"YES":"NO ") << std::endl;
          opReady = new_opReady;
        }
      }
    }
    
  } 
  catch (const Glib::Error& error)
  {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  // Done 
  return (0);
}

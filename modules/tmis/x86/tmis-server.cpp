// @file tmis-server.cpp
// @brief Timing Message Information Service: Server
// @author Dietrich Beck  <d.beck@gsi.de>
//
// Copyright (C) 2015 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
// Publishes timing message information.
//
//*****************************************************************************
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//*****************************************************************************
//

#define TMIS_SERVER_VERSION 0x000101  


#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

// dim includes
#include <dis.h>

// standard includes
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>

// saftlib includes
#include "SAFTd.h"
#include "TimingReceiver.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

using namespace std;

static const char* program;
bool absoluteTime   = false;
bool UTC            = false;          // show UTC instead of TAI
bool UTCleap        = false;

// dim server
#define NAMELEN    256
#define MESSAGELEN 4
// server name
char     disServerName[NAMELEN];

// timing message
char     disMessageName[NAMELEN];
uint64_t disMessage[4]; // 0: id, 1: param, 2: flags/TEF, 3: deadline
uint32_t disMessageId;

// message counter
char     disNmessageName[NAMELEN];
uint32_t disNmessage;
uint32_t disNmessageId;

// this will be called when receiving ECA actions
static void on_action(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags)
{
  disMessage[0]     = id;
  disMessage[1]     = param;
  disMessage[2]     = (uint64_t)(flags & 0xff) << 32;
  disMessage[3]     = deadline.getTAI();

  dis_update_service(disMessageId);

  disNmessage++;
  dis_update_service(disNmessageId);
} // on_action


using namespace saftlib;
using namespace std;

// display help
static void help(void) {
  std::cout << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name>" << std::endl;
  std::cout << std::endl;
  std::cout << "  -h                   display this help and exit" << std::endl;
  std::cout << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cout << std::endl;
  std::cout << "  <name> <eventID> <mask> <offset> configure server, offset is in ns, CTRL+C to exit (try 'hello 0x0 0x0 0' for ALL)" << std::endl;
  std::cout << "                       info: these values will be used as filter for publishing messages." << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "This tool provides the server for the Timing Messages Information Service." << std::endl;
  std::cout << std::endl;
  std::cout << "Tip: For using negative values when configuring the server, consider" << std::endl;
  std::cout << "using the special argument '--' to terminate option scanning." << std::endl << std::endl;

  std::cout << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  printf("Version %x, Licensed under the GPL v3.\n", TMIS_SERVER_VERSION);
} // help

int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  char *value_end;

  // variables snoop event
  uint64_t snoopID     = 0x0;
  uint64_t snoopMask   = 0x0;
  int64_t  snoopOffset = 0x0;

  int i;
  

  // variables inject event
  saftlib::Time eventTime;     // time for next event in PTP time
  saftlib::Time ppsNext;     // time for next PPS 
  saftlib::Time wrTime;     // current WR time

  // variables attach, remove
  char    *deviceName = NULL;
  char    *serverName = NULL;

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "hf")) != -1) {
    switch (opt) {
      case 'f' :
        useFirstDev = true;
        break;
      case 'h':
        help();
        return 0;
      default:
        std::cerr << program << ": bad getopt result" << std::endl;
        return 1;
    } // switch opt
  }   // while opt

  if (optind + 1>= argc) {
    std::cerr << program << " expecting two non-optional arguments: <device name> <server name>" << std::endl;
    help();
    return 1;
  }

  deviceName = argv[optind];
  serverName = argv[optind+1];

  if (optind+5  != argc) {
    std::cerr << program << ": expecting exactly three arguments: <eventID> <mask> <offset>" << std::endl;
    return 1;
  }

  snoopID     = strtoull(argv[optind+2], &value_end, 0);
  //std::cout << std::hex << snoopID << std::endl;
  if (*value_end != 0) {
    std::cerr << program << ": invalid eventID -- " << argv[optind+2] << std::endl;
    return 1;
  } // snoopID
  snoopMask     = strtoull(argv[optind+3], &value_end, 0);
  //std::cout << std::hex << snoopMask<< std::endl;
  if (*value_end != 0) {
    std::cerr << program << ": invalid mask -- " << argv[optind+3] << std::endl;
    return 1;
  } // mask
  snoopOffset   = strtoll(argv[optind+4], &value_end, 0);
  //std::cout << std::hex << snoopOffset<< std::endl;
  if (*value_end != 0) {
    std::cerr << program << ": invalid offset -- " << argv[optind+4] << std::endl;
    return 1;
  } // offset
  

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  }
  
  try {

    // init service data and names
    sprintf(disServerName, "tmis_%s", serverName);
    
    sprintf(disMessageName, "%s_message", disServerName);
    for (i=0; i<4; i++) disMessage[i] = 0x0;

    sprintf(disNmessageName, "%s_nmessage", disServerName);
    disNmessage = 0;

    // create services and start server
    // the DIM C interface is used for simplicity, consider migration to the C++ interface
    disMessageId  = dis_add_service(disMessageName,  "X:4", disMessage,   sizeof(disMessage),  0, 0);
    disNmessageId = dis_add_service(disNmessageName, "I:1", &disNmessage, sizeof(disNmessage), 0, 0);
    dis_start_serving(disServerName);


    // initialize required stuff
    std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

    // get a specific device
    map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();
    std::shared_ptr<TimingReceiver_Proxy> receiver;
    if (useFirstDev) {
      receiver = TimingReceiver_Proxy::create(devices.begin()->second);
    } else {
      if (devices.find(deviceName) == devices.end()) {
        std::cerr << "Device '" << deviceName << "' does not exist" << std::endl;
        return -1;
      } // find device
      receiver = TimingReceiver_Proxy::create(devices[deviceName]);
    } //if(useFirstDevice);

    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));

    // snoop tr
    std::shared_ptr<SoftwareCondition_Proxy> condition 
      = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, snoopMask, snoopOffset));
    // Accept all errors
    condition->setAcceptLate(true);
    condition->setAcceptEarly(true);
    condition->setAcceptConflict(true);
    condition->setAcceptDelayed(true);
    condition->SigAction.connect(sigc::ptr_fun(&on_action));
    condition->setActive(true);
    while(true) {
      saftlib::wait_for_signal();
    }
  } // eventSnoop
  
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }

  return 0;
}


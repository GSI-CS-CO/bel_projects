// @file tmis-client.cpp
// @brief Timing Message Information Service: Client
// @author Dietrich Beck  <d.beck@gsi.de>
//
// Copyright (C) 2015 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
// Subscbribes to timing message information.
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

#define TMIS_CLIENT_VERSION 0x000101  

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

#include <dic.hxx>

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

using namespace saftlib;
using namespace std;

// global variables
static const char* program;
static uint32_t pmode = PMODE_NONE;   // how data are printed (hex, dec, verbosity)
bool absoluteTime   = false;
bool UTC            = false;          // show UTC instead of TAI
bool UTCleap        = false;
bool printMessage   = false;
bool injectMessage  = false;

// variables snoop event
uint64_t snoopId     = 0x0;
uint64_t snoopMask   = 0x0;
int64_t  snoopOffset = 0x0;
  
std::shared_ptr<TimingReceiver_Proxy> myReceiver;

// dim helpers
#define NAMELEN    256
#define MESSAGELEN 4
uint32_t no_link_32 = 0xdeadbeef;
uint64_t no_link_64 = 0xdeadbeefce420651;
uint64_t no_link_message[MESSAGELEN] = {no_link_64,0,0,0};

// dim client
char     disPrefix[NAMELEN];

// timing message
char     dicMessageName[NAMELEN];
uint64_t dicMessage[MESSAGELEN]; // 0: id, 1: param, 2: flags/tef, 3: deadline

// message counter
char     dicNmessageName[NAMELEN];
uint32_t dicNmessage;

// use-case 'serve by buffer'
class timingNmessageBuffer : public DimInfo
{
  void infoHandler()
  {
    dicNmessage = (uint32_t)getInt();
  } // infoHandler

public :
  timingNmessageBuffer(const char *name) : DimInfo(name, (int)no_link_32) {};
};


// use-case 'serve by buffer'
class timingMessageBuffer : public DimInfo
{
  void infoHandler()
  {
    uint64_t *data;
    int i, len;

    data = (uint64_t *)getData();
    len = getSize()/sizeof(uint64_t);

    // check service size
    //if (len != MESSAGELEN) return;
    for(i = 0; i < len; i++) dicMessage[i] = data[i];
  } // infoHandler

public :
  timingMessageBuffer(const char *name) : DimInfo(name, no_link_message, MESSAGELEN*sizeof(uint64_t)) {};
};


// use case 'on-action'
class timingMessageOnAction : public DimInfo
{
  void infoHandler()
  {
    uint64_t *data;
    int      len;
    saftlib::Time deadline;
    saftlib::Time wrTime;     // current WR time
    uint64_t      id;
    uint64_t      param;
    uint16_t      flags;
    
    data = (uint64_t *)getData();
    len = getSize()/sizeof(uint64_t);

    // check service size
    if (len != MESSAGELEN) return;

    // check if service is available
    if (data[0] == no_link_64) {
      std::cout << "timing message: NO_LINK"  << std::endl;
      return;
    } // if noLink

    id       = data[0];
    param    = data[1];
    flags    = (uint16_t)((data[2] >> 32) & 0xff);
    deadline = saftlib::makeTimeTAI(data[3]);

    if ((snoopMask != 0x0) && ((id & snoopMask) != snoopId)) return;
    
    if (printMessage) {
      std::cout << "tDeadline: " << tr_formatDate(deadline, pmode);
      std::cout << tr_formatActionEvent(id, pmode);
      std::cout << tr_formatActionParam(param, 0xFFFFFFFF, pmode);
      std::cout << tr_formatActionFlags(flags, 0, pmode);
      std::cout << std::endl;
    } // if printMessage

    if (injectMessage) {
      wrTime    = myReceiver->CurrentTime() + snoopOffset; // add one ms
      myReceiver->InjectEvent(id, param, wrTime);
    } // if injectEvent
    
  } // infoHandler
public :
  timingMessageOnAction(const char *name) : DimInfo(name, no_link_message, MESSAGELEN*sizeof(uint64_t)) {};
};


//using namespace saftlib;
//using namespace std;

// display help
static void help(void) {
  std::cout << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name>" << std::endl;
  std::cout << std::endl;
  std::cout << "  -h                   display this help and exit" << std::endl;
  std::cout << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cout << "  -d                   display values in dec format" << std::endl;
  std::cout << "  -x                   display values in hex format" << std::endl;
  std::cout << "  -v                   more verbosity, usefull with command 'snoop'" << std::endl;
  std::cout << "  -p                   print message to screen" << std::endl;
  std::cout << "  -i                   inject message into local timing receiver" << std::endl;
  std::cout << "  -U                   display/inject absolute time in UTC instead of TAI" << std::endl;
  std::cout << "  -L                   used with command 'inject' and -U: if injected UTC second is ambiguous choose the later one" << std::endl;
  std::cout << std::endl;
  std::cout << "  <server name> <eventID> <mask> <offset> configure client, offset is in ns, CTRL+C to exit (try 'hello 0x0 0x0 0' for ALL)" << std::endl;
  std::cout << "                       info: these values will be used as filter when injecting messages into the local TR (option '-i')" << std::endl;
  std::cout << "                       or when printing message data to the screen)" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "This tool provides a client for the Timing Messages Information Service." << std::endl;
  std::cout << std::endl;
  std::cout << "Tip: For using negative values when configuring the client, consider" << std::endl;  
  std::cout << "using the special argument '--' to terminate option scanning." << std::endl << std::endl;

  std::cout << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  printf("Version %x, Licensed under the GPL v3.\n", TMIS_CLIENT_VERSION);
  std::cout << std::endl;
} // help


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  char *value_end;

  // variables attach, remove
  char    *deviceName = NULL;
  char    *serverName = NULL;

  pmode       = PMODE_NONE;
  
  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "dxvpihfUL")) != -1) {
    switch (opt) {
      case 'f' :
        useFirstDev = true;
        break;
      case 'i':
        injectMessage = true;
        break;
      case 'p':
        printMessage = true;
        break;
      case 'U':
        UTC = true;
        pmode = pmode + PMODE_UTC;
        break;
      case 'L':
        if (UTC) {
          UTCleap = true;
        } else {
          std::cerr << "-L only works with -U" << std::endl;
          return -1;
        } // else 'L'
        break;
      case 'd':
        pmode = pmode + PMODE_DEC;
        break;
      case 'x':
        pmode = pmode + PMODE_HEX;
        break;
      case 'v':
        pmode = pmode + PMODE_VERBOSE;
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
  
  snoopId     = strtoull(argv[optind+2], &value_end, 0);
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
    if (injectMessage) {
      // initialize required stuff
      std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

      // get device
      map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();
      std::shared_ptr<TimingReceiver_Proxy> receiver;
      if (useFirstDev) {
        receiver = TimingReceiver_Proxy::create(devices.begin()->second);
      } else {
        if (devices.find(deviceName) == devices.end()) {
          std::cerr << "Device '" << deviceName << "' does not exist" << std::endl;
          return -1;
        } // find device
      receiver   = TimingReceiver_Proxy::create(devices[deviceName]);
      myReceiver = receiver;
      } //if(useFirstDevice);
    } // if inject device

    sleep(1);
    
    // init service data and names
    sprintf(disPrefix, "tmis_%s", serverName);
    
    sprintf(dicMessageName,  "%s_message",  disPrefix);
    sprintf(dicNmessageName, "%s_nmessage", disPrefix);

    // subscribe to services
    timingMessageOnAction tm(dicMessageName);
    timingNmessageBuffer  nm(dicNmessageName);



    while (1) {
      sleep(1);
      if (!printMessage) {
        std::cout << dicNmessage << std::endl;
      } // if !printMessage
    } // while
    
 
  } catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }

  return 0;
}


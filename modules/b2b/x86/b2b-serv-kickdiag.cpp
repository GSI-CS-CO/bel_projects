/*******************************************************************************************
 *  b2b-serv-kickdiag.cpp
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 16-dec-2024
 *
 * publishes additional diagnostic data of the kicker
 *
 * this is experimental, as this information is not retrieved via a timing message (sent by 
 * the b2b system) but it is obtained from local ECA actions at the kicker frontend
 *
 * ------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
 *
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
 *
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 15-April-2019
 *********************************************************************************************/
#define B2B_SERV_KICKD_VERSION 0x000806

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

// dim includes
#include <dis.h>
#include <dis.hxx>

// standard includes
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>

// saftlib includes
#include "SAFTd.h"
#include "TimingReceiver.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "EmbeddedCPUActionSink.h"
#include "EmbeddedCPUCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// b2b includes
#include <common-lib.h>                 // COMMON
#include <b2blib.h>                     // API
#include <b2b.h>                        // FW

using namespace saftlib;
using namespace std;

#define FID          0x1                // format ID of timing messages

static const char* program;

// dim
#define DIMCHARSIZE 32                  // standard size for char services
#define DIMMAXSIZE  1024                // max size for service names
#define NAMELEN     256                 // max size for names

// tags for kicker diag
enum evtKTag{tagKRising, tagKFalling, tagKStart, tagKStop};

double    no_link_dbl   = NAN;          // indicates "no link" for missing DIM services of type double

// published services
char      disVersion[DIMCHARSIZE];
char      disHostname[DIMCHARSIZE];
uint32_t  disNTransfer;
double    disRisingOffs[B2B_NSID];      // offset of first rising edge to B2B_TRIGGER
double    disFallingOffs[B2B_NSID];     // offset of first falling edge to B2B_TRIGGER
uint32_t  disRisingN[B2B_NSID];         // number of rising edges; expectation value is 1
uint32_t  disFallingN[B2B_NSID];        // number of falling edges; expectation value is 1
double    disLen[B2B_NSID];             // lengh of signal
double    disSetLevel[B2B_NSID];        // set level of comparator for detection of probe signals

uint32_t  disVersionId      = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disNTransferId    = 0;
uint32_t  disRisingOffsId[B2B_NSID];
uint32_t  disFallingOffsId[B2B_NSID];
uint32_t  disRisingNId[B2B_NSID];
uint32_t  disFallingNId[B2B_NSID];
uint32_t  disLenId[B2B_NSID];
uint32_t  disSetLevelId[B2B_NSID];

// subscribed services
double    dicSetLevel;                  // set level of comparator value for detection of probe signals

uint32_t  dicSetLevelId; 


// local variables
uint32_t reqRing;                       // requested extraction ring
uint32_t reqMode;                       // requested mode; 0: extraction, 1: injection
uint32_t reqIO;                         // requested IO to which the magnet probe signal is connected
uint32_t reqEvtNo;                      // requested event number

// init setval
void initValues(uint32_t sid)
{
  disRisingOffs[sid]   = NAN;
  disFallingOffs[sid]  = NAN;
  disRisingN[sid]      = 0;
  disFallingN[sid]     = 0;
  disLen[sid]          = NAN;
  disSetLevel[sid]     = NAN;
} // initValues


// update values
void disUpdateValues(uint32_t sid)
{
  dis_update_service(disRisingOffsId[sid]);
  dis_update_service(disFallingOffsId[sid]);
  dis_update_service(disRisingNId[sid]);
  dis_update_service(disFallingNId[sid]);
  dis_update_service(disLenId[sid]);
  dis_update_service(disSetLevelId[sid]);
  
  disNTransfer++;
  dis_update_service(disNTransferId);
} // disUpdateSetval
 

// handle received timing message
static void timingMessage(uint32_t tag, saftlib::Time deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t isLate, uint32_t isEarly, uint32_t isConflict, uint32_t isDelayed)
{
  uint32_t            recSid;          // received SID

  static int          flagActive;      // flag: b2b is active
  static uint64_t     tStart;          // time of transfer
  static uint32_t     sid;             // Sequence ID

  recSid      = ((evtId  & 0x00000000fff00000) >> 20);

  // check ranges
  if (recSid  > B2B_NSID)                  return;
  if (tag > tagKStop)                      return;
  if ((!flagActive) && (tag != tagKStart)) return;

  switch (tag) {
    case tagKStart   :
      sid                          = recSid;
      tStart                       = deadline.getTAI();
      flagActive                   = 1;
      initValues(sid);
      //      printf("start %d\n", sid);
      break;
    case tagKStop    :
      flagActive                   = 0;
      if ((disRisingN[sid] > 0) && (disFallingN[sid] > 0)) disLen[sid] = disFallingOffs[sid] - disRisingOffs[sid];
      disSetLevel[sid]             = dicSetLevel;
      disUpdateValues(sid);
      //printf("stop %d, level %f\n", sid, disSetLevel[sid]);
      break;
    case tagKRising  :
      disRisingN[sid]++;
      if (disRisingN[sid] == 1)  disRisingOffs[sid]  = (float)(deadline.getTAI() - tStart);
      //      printf("rising %d\n", sid);
      break;
    case tagKFalling :
      disFallingN[sid]++;
      if (disFallingN[sid] == 1) disFallingOffs[sid] = (float)(deadline.getTAI() - tStart);
      //      printf("falling, sid %d,  N %d, offset %f\n", sid, disFallingN[sid], disFallingOffs[sid]);
      break;
    default         :
      ;
  } // switch tag
} // timingmessage


// this will be called in case no message was received
static void recTimeout()
{
  saftlib::Time time = saftlib::makeTimeTAI(12345); 
  
  timingMessage(42, time, 0x0, 0x0, 0x0, 0, 0, 0, 0);
} // recTimeout()


// this will be called when receiving ECA actions from software action queue
// informative: this routine is presently not used, as the softare action queue does not support the TEF field
static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int                 flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessag


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  // 'generic' services
  sprintf(name, "%s_version", prefix);
  sprintf(disVersion, "%s",  b2b_version_text(B2B_SERV_KICKD_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  sprintf(name, "%s_ntransfer", prefix);
  disNTransferId  = dis_add_service(name, "I", &disNTransfer, sizeof(disNTransfer), 0 , 0);

  // values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s_sid%02d_risingoffs",  prefix, i);
    disRisingOffsId[i]  = dis_add_service(name, "D:1", &(disRisingOffs[i]),  sizeof(double), 0, 0);

    sprintf(name, "%s_sid%02d_risingN",     prefix, i);
    disRisingNId[i]     = dis_add_service(name, "I:1", &(disRisingN[i]),     sizeof(uint32_t), 0, 0);

    sprintf(name, "%s_sid%02d_fallingoffs", prefix, i);
    disFallingOffsId[i] = dis_add_service(name, "D:1", &(disFallingOffs[i]), sizeof(double), 0, 0);

    sprintf(name, "%s_sid%02d_fallingN",    prefix, i);
    disFallingNId[i]    = dis_add_service(name, "I:1", &(disFallingN[i]),    sizeof(uint32_t), 0, 0);

    sprintf(name, "%s_sid%02d_len",         prefix, i);
    disLenId[i]         = dis_add_service(name, "D:1", &(disLen[i]),         sizeof(double), 0, 0);

    sprintf(name, "%s_sid%02d_level",       prefix, i);
    disSetLevelId[i]    = dis_add_service(name, "D:1", &(disSetLevel[i]),    sizeof(double), 0, 0);
  } // for i
} // disAddServices


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];

    sprintf(name, "%s_setlevel", prefix);
    dicSetLevelId      = dic_info_service_stamped(name, MONITORED, 0, &dicSetLevel, sizeof(double), 0, 0, &no_link_dbl, sizeof(double));
} // dicSubscribeServices

                        
using namespace saftlib;
using namespace std;

// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -r<index>            specify ring  (0: SIS18[default], 1: ESR, 2: CRYRING)" << std::endl;
  std::cerr << "  -m<index>            specify mode  (0: extraction[default], 1: injection)" << std::endl;
  std::cerr << "  -i<index>            specify input (1..N; [default is IO1])" << std::endl;
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool provides a server for additional kicker diagnostic data" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " tr0 -r0 pro'" << std::endl;
  std::cerr << std::endl;

  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << b2b_version_text(B2B_SERV_KICKD_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help

int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  char *tail;
  int i;

  // variables snoop event
  uint64_t snoopID     = 0x0;
  uint64_t tluIn       = 0x0;
  int      nCondition  = 0;

  // variables attach, remove
  char    *deviceName = NULL;
  char    *envName    = NULL;

  // 
  char     ringName[NAMELEN];
  char     prefix[NAMELEN*2];
  char     comparatorPrefix[NAMELEN*2];
  char     disName[DIMMAXSIZE];

  reqRing  = SIS18_RING;                // gid SIS18
  reqMode  = 0;                         // extraction
  reqIO    = 1;                         // magnet probe connected to IO1
  reqEvtNo = B2B_ECADO_B2B_TRIGGEREXT;  // extraction trigger
  tluIn    = B2B_ECADO_TLUINPUT1;       // input IO1


  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "r:m:i:hf")) != -1) {
    switch (opt) {
      case 'r' :
        switch (strtol(optarg, &tail, 0)) {
          case 0 : reqRing = SIS18_RING;   break;
          case 1 : reqRing = ESR_RING;     break;
          case 2 : reqRing = CRYRING_RING; break;
          default:
            std::cerr << "option -e: parameter out of range" << std::endl;
            return 1;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
        break;
      case 'm' :
        reqMode = strtol(optarg, &tail, 0);
        if (reqMode > 1) fprintf(stderr, "parameter out of range)\n");
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
        if (reqMode == 1) reqEvtNo = B2B_ECADO_B2B_TRIGGERINJ;
        else              reqEvtNo = B2B_ECADO_B2B_TRIGGEREXT;
        break;
      case 'i' :
        reqIO = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
        switch (reqIO) {
          case 1 : tluIn = B2B_ECADO_TLUINPUT1; break;
          case 2 : tluIn = B2B_ECADO_TLUINPUT2; break;
          case 3 : tluIn = B2B_ECADO_TLUINPUT3; break;
          case 4 : tluIn = B2B_ECADO_TLUINPUT4; break;
          case 5 : tluIn = B2B_ECADO_TLUINPUT5; break;
          default :
            std::cerr << "option -i: parameter out of range" << std::endl;
            return 1;
        } // switch reqIO
        break;
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

  if (optind >= argc) {
    std::cerr << program << " expecting one non-optional arguments: <device name>" << std::endl;
    help();
    return 1;
  }

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  if (!(optind+1 < argc)) return 1;
  
  deviceName = argv[optind];
  envName    = argv[optind+1];
  gethostname(disHostname, 32);

  switch(reqRing) {
    case SIS18_RING :
      nCondition = 4;
      sprintf(ringName, "sis18");
      break;
    case ESR_RING :
      nCondition = 4;
      sprintf(ringName, "esr");
      break;
    case CRYRING_RING :
      nCondition = 4;
      sprintf(ringName, "yr");
      break;
    default :
        std::cerr << "Ring '"<< reqRing << "' does not exist" << std::endl;
        return -1;;
  } // switch extRing
  
  // init DIM services
  // Ids
  disNTransferId        = 0;
  disVersionId          = 0;
  disHostnameId         = 0;
  disNTransferId        = 0;
  for (i=0; i< B2B_NSID; i++ ) {
    disRisingOffsId[i]  = 0;
    disFallingOffsId[i] = 0;
    disRisingNId[i]     = 0;
    disFallingNId[i]    = 0;
    disLenId[i]         = 0;
  } // for i
  // data
  disNTransfer          = 0;
  for (i=0; i< B2B_NSID; i++ ) initValues(i);
 
  if (!reqMode) sprintf(prefix, "b2b_%s_%s-kdde", envName, ringName);  // extraction
  else          sprintf(prefix, "b2b_%s_%s-kddi", envName, ringName);  // injection
  
  printf("%s: starting server using prefix %s\n", program, prefix);

  disAddServices(prefix);
  
  sprintf(disName, "%s", prefix);
  dis_start_serving(disName);

  if (!reqMode) sprintf(comparatorPrefix, "b2b_%s_%s-kse", envName, ringName);  // extraction
  else          sprintf(comparatorPrefix, "b2b_%s_%s-ksi", envName, ringName);  // injection

  printf("%s: subscribing to comparator server using prefix %s\n", program, comparatorPrefix);

  dicSubscribeServices(comparatorPrefix);
  
  try {
    // basic saftd stuff
    std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

    // connect to timing receiver
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

    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];
    uint32_t tag[nCondition];
    uint32_t tmpTag;

    // define conditions (ECA filter rules)

    // CMD_B2B_TRIGGER..., signals start of data collection
    tmpTag        = tagKStart;
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)reqRing  << 48) | ((uint64_t)reqEvtNo << 36);
    condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[0]        = tmpTag;
  
    // CMD_B2B_TRIGGER..., +100us (!), signals stop of data collection
    tmpTag        = tagKStop;        
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)reqRing  << 48) | ((uint64_t)reqEvtNo << 36);
    condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 100000));
    tag[1]        = tmpTag;

    // IO input rising edge
    tmpTag        = tagKRising;
    snoopID       = ((uint64_t)0xf << 60) | ((uint64_t)0xfff  << 48) | ((uint64_t)tluIn << 36 | (uint64_t)000000001);
    condition[2]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffffffffffffffff, 0));
    tag[2]        = tmpTag;

    // IO input falling edge
    tmpTag        = tagKFalling;
    snoopID       = ((uint64_t)0xf << 60) | ((uint64_t)0xfff  << 48) | ((uint64_t)tluIn << 36 | (uint64_t)000000000);
    condition[3]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffffffffffffffff, 0));
    tag[3]        = tmpTag;

    // let's go!
    for (i=0; i<nCondition; i++) {
      condition[i]->setAcceptLate(true);
      condition[i]->setAcceptEarly(true);
      condition[i]->setAcceptConflict(true);
      condition[i]->setAcceptDelayed(true);
      condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i

    while(true) {
      int mstimeout;

      mstimeout = 10000;
      
     if (saftlib::wait_for_signal(mstimeout) == 0) recTimeout();
    } // while true
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


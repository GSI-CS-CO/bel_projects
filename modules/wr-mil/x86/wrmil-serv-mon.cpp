/*******************************************************************************************
 *  wrmil-serv-mon.cpp
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 27-Nov-2024
 *
 * monitors WR-MIL gateway
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
#define WRMIL_SERV_MON_VERSION 0x000102

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
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// wr-mil includes
#include <common-lib.h>                 // COMMON
#include <wrmillib.h>                   // API
#include <wr-mil.h>                     // FW

// USE MASP
#ifdef USEMASP
// includes for MASP
#include "MASP/Emitter/End_of_scope_status_emitter.h"
#include "MASP/StatusDefinition/DeviceStatus.h"
#include "MASP/Util/Logger.h"
#include "MASP/Common/StatusNames.h"
#include <boost/thread/thread.hpp> // (sleep)
#include <iostream>
#include <string>
#include <limits.h>

std::string   maspNomen;
std::string   maspSourceId; 
bool          maspProductive;     // send to pro/dev masp
bool          maspSigOpReady;     // value for MASP signal OP_READY
bool          maspSigTransfer;    // value for MASP signal TRANSFER (custom emitter)

MASP::StatusEmitterConfig get_config() {
  char   hostname[HOST_NAME_MAX];

  gethostname(hostname, HOST_NAME_MAX);
  maspSourceId   = maspNomen + "." + std::string(hostname);

#ifdef PRODUCTIVE
  maspProductive = true;
#else
  maspProductive = false;
#endif

  MASP::StatusEmitterConfig config = MASP::StatusEmitterConfig(MASP::StatusEmitterConfig::CUSTOM_EMITTER_DEFAULT(), maspSourceId, maspProductive);

  return config;
}
#endif


using namespace saftlib;
using namespace std;

#define FID                0x1          // format ID of timing messages
#define UPDATE_TIME_MS    1000          // time for status updates [ms]
#define MATCH_WIN_US        20          // window for matching start and stop evts [us]
#define INITMINMAX 10000000000          // init value for min/max

static const char* program;

// dim
#define DIMCHARSIZE 32                  // standard size for char services
#define DIMMAXSIZE  1024                // max size for service names
#define NAMELEN     256                 // max size for names

// services
char      disVersion[DIMCHARSIZE];      // firmware version
char      disState[DIMCHARSIZE];        // firmware state
char      disHostname[DIMCHARSIZE];     // hostname
uint64_t  disStatus;                    // firmware status
monval_t  disMonData;

uint32_t  disVersionId      = 0;
uint32_t  disStateId        = 0;
uint32_t  disStatusId       = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disMonDataId      = 0;
uint32_t  disCmdClearId     = 0;


// local variables
monval_t  monData;                      // monitoring data
double    tAveOld;                      // helper for stats
double    tAveStreamOld;                // helper for stats
int       flagClear;                    // flag for clearing diag data;
uint32_t  matchWindow       = MATCH_WIN_US;
uint32_t  modeCompare       = 0;
uint64_t  offsetStart;                  // correction to be used for different monitoring types
uint64_t  offsetStop;                   // correction to be used for different monitoring types



uint64_t  one_us_ns = 1000;
uint64_t  one_ms_ns = 1000000;


// calc basic statistic properties
void calcStats(double *meanNew,         // new mean value, please remember for later
               double meanOld,          // old mean value (required for 'running stats')
               double *streamNew,       // new stream value, please remember for later
               double streamOld,        // old stream value (required for 'running stats')
               double val,              // the new value :-)
               uint32_t n,              // number of values (required for 'running stats')
               double *var,             // standard variance
               double *sdev             // standard deviation
               )
{
  // see  ”The Art of ComputerProgramming, Volume 2: Seminumerical Algorithms“, Donald Knuth, or
  // http://www.netzmafia.de/skripten/hardware/Control/auswertung.pdf
  if (n > 1) {
    *meanNew   = meanOld + (val - meanOld) / (double)n;
    *streamNew = streamOld + (val - meanOld)*(val - *meanNew);
    *var       = *streamNew / (double)(n - 1);
    *sdev      = sqrt(*var);
  }
  else {
    *meanNew = val;
    *var     = 0;
  }
} // calcStats


// clear statistics
void clearStats()
{
  // monData.gid      = 0x0;
  monData.nFwSnd     = 0x0;
  monData.nFwRecD    = 0x0;
  monData.nFwRecT    = 0x0;
  monData.nFwRecErr  = 0x0;
  monData.nFwBurst   = 0x0;
  monData.nStart     = 0x0;
  monData.nStop      = 0x0;
  monData.nMatch     = 0x0;
  monData.nFailSnd   = 0x0;
  monData.nFailEvt   = 0x0;
  monData.nFailOrder = 0x0;
  monData.tAct       = NAN;
  monData.tMin       =  INITMINMAX;
  monData.tMax       = -INITMINMAX;
  monData.tAve       = NAN;
  monData.tSdev      = NAN;        
  tAveOld            = NAN;
  tAveStreamOld      = NAN;
} // clearStats


// handle received timing message
static void timingMessage(uint64_t evtId, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  static int          flagMsgStart;    // flag: marks that a start message has been received
  static uint64_t     deadlineStart;     // deadline of MIL event sent
  static uint32_t     sndEvtNo;        // evtNo of MIL event sent
  
  uint32_t            mFid;            // FID 
  // uint32_t            mGid;            // GID
  uint32_t            mEvtNo;          // event number

  double              tDiff;           // time difference between stop and start; must be smaller than match window
  double              mean;            // mean value for statistics
  double              sdev;            // sdev for statistics
  double              stream;          // a stream value for statistics
  double              dummy;

  mFid        = ((evtId  & 0xf000000000000000) >> 60);
  // mGid        = ((evtId  & 0x0fff000000000000) >> 48);
  mEvtNo      = ((evtId  & 0x0000fff000000000) >> 36);

  stream      = 0;
  sdev        = 0;

  // check ranges
  if (mFid != FID)                        return;  // unexpected format of timing message
  if (tag   > tagStop)                    return;  // illegal tag
  if ((modeCompare == 2) && ((mEvtNo >= 0x0e0) && (mEvtNo <= 0x0e4))) return; // exclude UTC telegrams
  //printf("tag %d\n", tag);

  
  switch (tag) {
    case tagStart   :
      //if (mGid != monData.gid)            return;  // received GID does not match configuration
      monData.nStart++; 
      flagMsgStart                 = 1;
      deadlineStart                = deadline.getTAI();
      /*if ((deadlineStart - tStartOld) < dStartMin) dStartMin = deadlineStart - tStartOld;
        tStartOld = deadlineStart;*/
      sndEvtNo                     = mEvtNo;
      monData.tAct                 = NAN;
      //monData.tMin                 = NAN;
      //monData.tMax                 = NAN;
      //monData.tAve                 = NAN;
      //monData.tSdev                = NAN;
      break;
    case tagStop    :
      monData.nStop++;
      /*if ((deadline.getTAI() - tStopOld) < dStopMin) dStopMin = deadline.getTAI() - tStopOld;
        tStopOld = deadline.getTAI();*/

      // a MIL telegram has been received although no MIL telegram has been sent: give up
      if (!flagMsgStart){
        monData.nFailSnd++;
        return;
      } // if !flagMsgStart

      // evtNo of MIL received/sent telegrams do not match: give up
      if ((mEvtNo != sndEvtNo) && (modeCompare != 4)) {
        monData.nFailEvt++;
        return;  
      } // if !evtNo
      // assume we have a matching pair of sent and received MIL telegrams
      flagMsgStart                  = 0;
      tDiff                         = (double)((int64_t)(deadline.getTAI() - offsetStop - deadlineStart)) / (double)one_us_ns;

      // check causality
      if (tDiff < -1.0 * matchWindow){
        monData.nFailOrder++;
        return;
      } // if tDiff
      if (modeCompare == 4) tDiff  += (double)offsetStart / (double)one_us_ns;  // hacky
      monData.nMatch++;
      monData.tAct                  = tDiff;
      if (monData.tAct < monData.tMin) monData.tMin = monData.tAct;
      if (monData.tAct > monData.tMax) monData.tMax = monData.tAct;

      calcStats(&mean, tAveOld, &stream, tAveStreamOld, monData.tAct, monData.nMatch , &dummy, &sdev);
      tAveOld       = mean;
      tAveStreamOld = stream;
      monData.tAve  = mean;
      monData.tSdev = sdev;
      break;
    default         :
      ;
  } // switch tag 
} // timingmessage


// callback for command
void dis_cmd_clear(void *tag, void *buffer, int *size)
{
  flagClear = 1;
} // dis_cmd_clear


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];

  // 'generic' services
  sprintf(name, "%s_version_fw", prefix);
  sprintf(disVersion, "%s",  wrmil_version_text(WRMIL_SERV_MON_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_state", prefix);
  sprintf(disState, "%s", wrmil_state_text(COMMON_STATE_OPREADY));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  sprintf(name, "%s_status", prefix);
  disStatus       = 0x1;   
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  // monitoring data service
  sprintf(name, "%s_data", prefix);
  disMonDataId  = dis_add_service(name, "I:2;X:3;I:2;X:3;I:3;D:5", &(disMonData), sizeof(monval_t), 0, 0);

  // command clear
  sprintf(name, "%s_cmd_cleardiag", prefix);
  disCmdClearId = dis_add_cmnd(name, 0, dis_cmd_clear, 17);
} // disAddServices

                        
// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -h                   display this help and exit"                                  << std::endl;
  std::cerr << "  -e                   display version"                                             << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)"    << std::endl;
  std::cerr << "  -d                   start server publishing data"                                << std::endl;
  std::cerr << "  -c <mode>            type of comparison; default 0; this can be"                  << std::endl;
  std::cerr << "                       0: don't compare                                         "   << std::endl;
  std::cerr << "                       1: compare received MIL telegrams with WR timing messages"   << std::endl;
  std::cerr << "                       2: compare received MIL telegrams with sent MIL telegrams"   << std::endl;
  std::cerr << "                       3: as mode '2' but excludes UTC telegrams"                   << std::endl;
  std::cerr << "                       4: as mode '1' but discarding data from MIL piggy"           << std::endl;
  std::cerr << "                       5: compare sent MIL telegrams with WR timing messages"       << std::endl;
  std::cerr << "  -m <match windows>   [us] windows for matching start/stop evts; default 20"       << std::endl;
  std::cerr << std::endl;
  std::cerr << "  The paremter -s is mandatory"                                                     << std::endl;
  std::cerr << "  -s <MIL domain>      MIL domain; this can be"                                     << std::endl;
  std::cerr << "                       0: PZU_QR; UNILAC, Source Right"                             << std::endl;
  std::cerr << "                       1: PZU_QL; UNILAC, Source Left"                              << std::endl;     
  std::cerr << "                       2: PZU_QN; UNILAC, Source High Charge State Injector (HLI)"  << std::endl;
  std::cerr << "                       3: PZU_UN; UNILAC, High Charge State Injector (HLI)"         << std::endl;
  std::cerr << "                       4: PZU_UH; UNILAC, High Current Injector (HSI)"              << std::endl;
  std::cerr << "                       5: PZU_AT; UNILAC, Alvarez Cavities"                         << std::endl;
  std::cerr << "                       6: PZU_TK; UNILAC, Transfer Line"                            << std::endl;
  std::cerr << "                       7: SIS18_RING"                                               << std::endl;
  std::cerr << "                       8: ESR_RING"                                                 << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool monitors a White Rabbit -> MIL gateway."                                  << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " tr0 -s0 -d pro'"                                       << std::endl;
  std::cerr << std::endl;
  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << wrmil_version_text(WRMIL_SERV_MON_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int      opt;
  bool     useFirstDev    = false;
  bool     getVersion     = false;
  bool     startServer    = false;
  uint32_t gid=0xffffffff;                // gid for gateway
  uint32_t gidStart;                      // relevant to select the type of messages used as a start
  uint64_t idStop;                        // relevant to select the type of messages used as a stop
  uint64_t maskStop;                      // relevant to select the type of messages used as a stop
  

  char    *tail;

  // variables logging
  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of of gateway
  uint64_t actStatus      = 0;                 // actual status of gateway
  int      printFlag      = 0;

  // variables snoop event
  uint64_t snoopID     = 0x0;
  int      nCondition  = 2;

  int      tmpi;
  int      i;

  // variables attach, remove
  char    *deviceName = NULL;

  char     domainName[NAMELEN];          // name of MIL domain
  char     prefix[NAMELEN*3];            // prefix DIM services
  char     disName[DIMMAXSIZE];          // name of DIM server
  char     environment[NAMELEN];         // environment, typically either int or pro
  uint32_t verLib;                       // library version
  uint32_t verFw;                        // firmware version
  
  char     ebPath[1024];
  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "s:m:c:hefd")) != -1) {
    switch (opt) {
      case 'e' :
        getVersion  = true;
        break;
      case 'f' :
        useFirstDev = true;
        break;
      case 'd' :
        startServer = true;
        break;
      case 's' :
        tmpi        = strtoull(optarg, &tail, 0);
        if (*tail != 0) {std::cerr << "Specify a proper number, not " << optarg << "'%s'!" << std::endl; return 1;}
        switch (tmpi) {
          case 0: gid = PZU_QR;     sprintf(domainName, "%s", "pzu_qr");     break;
          case 1: gid = PZU_QL;     sprintf(domainName, "%s", "pzu_ql");     break;
          case 2: gid = PZU_QN;     sprintf(domainName, "%s", "pzu_qn");     break;
          case 3: gid = PZU_UN;     sprintf(domainName, "%s", "pzu_un");     break;
          case 4: gid = PZU_UH;     sprintf(domainName, "%s", "pzu_uh");     break;
          case 5: gid = PZU_AT;     sprintf(domainName, "%s", "pzu_at");     break;
          case 6: gid = PZU_TK;     sprintf(domainName, "%s", "pzu_tk");     break;
          case 7: gid = SIS18_RING; sprintf(domainName, "%s", "sis18_ring"); break;
          case 8: gid = ESR_RING;   sprintf(domainName, "%s", "esr_ring");   break;
          default: {std::cerr << "Specify a proper number, not " << tmpi << "'%s'!" << std::endl; return 1;} break;
        } // switch tmpi
#ifdef USEMASP
        switch (tmpi) {
          case 0: maspNomen = std::string("U_WR2MIL_PZUQR");    break;
          case 1: maspNomen = std::string("U_WR2MIL_PZUQL");    break;
          case 2: maspNomen = std::string("U_WR2MIL_PZUQN");    break;
          case 3: maspNomen = std::string("U_WR2MIL_PZUUN");    break;
          case 4: maspNomen = std::string("U_WR2MIL_PZUUH");    break;
          case 5: maspNomen = std::string("U_WR2MIL_PZUAT");    break;
          case 6: maspNomen = std::string("U_WR2MIL_PZUTK");    break;
          case 7: maspNomen = std::string("U_WR2MIL_SIS18");    break;
          case 8: maspNomen = std::string("U_WR2MIL_ESR");      break;
        } // switch tmpi
#endif
        break;
      case 'm':
        matchWindow = strtoull(optarg, &tail, 0);
        break;
      case 'c':
        modeCompare = strtoull(optarg, &tail, 0);
        break;
      case 'h':
        help();
        return 0;
        break;
      default:
        std::cerr << program << ": bad getopt result" << std::endl;
        return 1;
    } // switch opt
  }   // while opt

  if (optind >= argc) {
    std::cerr << program << ": expecting one non-optional arguments: <device name>" << std::endl;
    help();
    return 1;
  }

  if (gid == 0xffffffff) {
    std::cerr << program << ": parameter -s is non-optional\n";
    return 1;
  } // if gid

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  deviceName = argv[optind];
  gethostname(disHostname, 32);
  
  if (optind+1 < argc) {
    sprintf(environment, "%s", argv[++optind]);
    sprintf(prefix, "wrmil_%s_%s-mon", environment, domainName);
  } // if optind
  else
    sprintf(prefix, "wrmil_%s-mon", domainName);

  if (startServer) {
    printf("%s: starting server using prefix %s\n", program, prefix);

    clearStats();
    disAddServices(prefix);
    
    sprintf(disName, "%s", prefix);
    dis_start_serving(disName);
  } // if startServer
  
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


    sprintf(ebPath, "%s", receiver->getEtherbonePath().c_str());
    if ((status =  wrmil_firmware_open(&ebDevice, ebPath, 0, &cpu)) != COMMON_STATUS_OK) {
      std::cerr << program << ": can't open connection to lm32 firmware" << std::endl;
      exit(1);
    } // if status
    
    if (getVersion) {
      wrmil_version_library(&verLib);
      printf("wrmil: serv-sys / library / firmware /  version %s / %s",  wrmil_version_text(verLib), wrmil_version_text(WRMIL_SERV_MON_VERSION));     
      wrmil_version_firmware(ebDevice, &verFw);
      printf(" / %s\n",  wrmil_version_text(verFw));     
    } // if getVersion

#ifdef USEMASP 
    // optional: disable masp logging (default: log to stdout, can be customized)
    MASP::no_logger no_log;
    MASP::Logger::middleware_logger = &no_log;

    MASP::StatusEmitter emitter(get_config());
    std::cout << "wr-mil: emmitting to MASP as sourceId: " << maspSourceId << ", using nomen: " << maspNomen << ", environment pro: " << maspProductive << std::endl;
#else
    std::cout << "wr-mil: no MASP emitter!" << std::endl;
#endif // USEMASP

    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

    uint32_t tag[nCondition];
    uint32_t tmpTag;

    if (modeCompare) {
      // define conditions (ECA filter rules)

      // select if we use the received White Rabbit timing messages or the sent MIL telegrams as start
      switch (modeCompare) {
        case 1:
          // compare received MIL telegrams (stop) to received WR Timing messages (start)
          // received MIL telegrams have a deadline 1ms after timestamp,
          // thus we must add 1ms to the start action for matching
          gidStart    = gid;
          idStop      = ((uint64_t)LOC_MIL_REC << 48);
          maskStop    = 0xfffff00000000000;
          offsetStart = one_ms_ns;
          offsetStop  = one_us_ns * matchWindow;
          break;
        case 2 ... 3:
          // compare received MIL telegrams (stop) to sent MIL telegrams (start)
          // received MIL telegrams have a deadline 1ms after timestamp,
          // thus we must add 1ms to the start action for matching
          gidStart    = LOC_MIL_SEND;
          idStop      = ((uint64_t)LOC_MIL_REC << 48);
          maskStop    = 0xfffff00000000000;
          offsetStart = one_ms_ns + WRMIL_MILSEND_LATENCY;
          offsetStop  = one_us_ns * matchWindow;
          break;
        case 4:
          // compare timestamps of received MIL telegrams (stop) to received WR Timing messags (start)
          // this might be less precise as we can't check the event number of received MIL telegrams
          // we post-trigger the messages, to avoid late messages at the TLU
          gidStart    = gid;
          idStop      = ((uint64_t)LOC_TLU << 48) | ((uint64_t)WRMIL_ECADO_MIL_TLU << 36) | 0x1;
          maskStop    = 0xffffffffffffffff;
          offsetStart = WRMIL_POSTTRIGGER_TLU;
          offsetStop  = WRMIL_POSTTRIGGER_TLU + one_us_ns * matchWindow;
          break;
        case 5:
          // compare sent MIL telegrams (stop) to received WR Timing messags (start)
          gidStart    = gid;
          idStop      = ((uint64_t)LOC_MIL_SEND << 48);
          maskStop    = 0xfffff00000000000;
          offsetStart = -WRMIL_MILSEND_LATENCY;
          offsetStop  = one_us_ns * matchWindow;
          break;
        default:
          // bogus; unused
          gidStart    = 0;
          idStop      = 0;
          maskStop    = 0;
          offsetStart = 0;
          offsetStop  = 0;
          break;
      } // switch modeCompare

      // message that is injected locally by the lm32 firmware (triggering a rule on the ECA WB channel towards the MIL interface)
      // this message must be delayed by one ms to (hopefully) coincide with the received MIL telegram
      // we also do prefix matching of the first four bits of the evtNo (should be '0x0')
      tmpTag        = tagStart;
      snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gidStart << 48);
      condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffff00000000000, offsetStart));
      tag[0]        = tmpTag;
        
      // message that is injected locally by the lm32 firmware after detecting and decoding a receive MIL telegram
      // this message is delayed by 20us to avoid a) a collision b) to have a defined order of both messages
      // (this assumes that the 'true' offset between both messages is always smaller than 20us)
      // we also do prefix machting of the first four bits of the evtNo (should be '0x0')
      tmpTag        = tagStop;        
      snoopID       = ((uint64_t)FID << 60) | idStop;
      condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, maskStop, offsetStop));
      tag[1]        = tmpTag;
    
      // let's go!
      for (i=0; i<nCondition; i++) {
        condition[i]->setAcceptLate(true);
        condition[i]->setAcceptEarly(true);
        condition[i]->setAcceptConflict(true);
        condition[i]->setAcceptDelayed(true);
        condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&timingMessage), tag[i]));
        condition[i]->setActive(true);    
      } // for i
    } // if modeCompare

    saftlib::Time deadline_t;
    uint64_t      t_new, t_old, t_lastlog;
    uint32_t      tmp32a, tmp32b, tmp32c, tmp32f, tmp32g, tmp32h;
    int32_t       stmp32a;
    uint64_t      tmp64a;
    uint32_t      fwGid, fwEvtsRecErr, fwEvtsBurst, fwState, fwVersion, nBadStatus, nBadState;
    uint64_t      fwEvtsSnd, fwEvtsRecT, fwEvtsRecD, fwStatus;

    t_old     = comlib_getSysTime();
    t_lastlog = comlib_getSysTime();
    while(true) {
      // hier periodisch DIM service aktualisieren bzw. update auf Bildschirm bzw. update MASP
      monData.gid   = gid;
      monData.cMode = modeCompare;
      saftlib::wait_for_signal(UPDATE_TIME_MS / 10);

      t_new = comlib_getSysTime();
      if (((t_new - t_old) / one_ms_ns) > UPDATE_TIME_MS) {
        t_old      = t_new;

        // update firmware data
        wrmil_common_read(ebDevice, &fwStatus, &fwState, &nBadStatus, &nBadState, &fwVersion, &tmp32c, 0);
        wrmil_info_read(ebDevice, &tmp32a, &tmp32b, &tmp32c, &fwGid, &stmp32a, &tmp64a, &tmp32f, &tmp32g, &tmp32h, &fwEvtsSnd, &fwEvtsRecT, &fwEvtsRecD, &fwEvtsRecErr, &fwEvtsBurst, 0);
        if (fwGid != gid) fwStatus |= COMMON_STATUS_OUTOFRANGE; // signal an error

        disStatus  = fwStatus;
        sprintf(disState  , "%s", wrmil_state_text(fwState));
        sprintf(disVersion, "%s", wrmil_version_text(fwVersion));
               
        // update monitoring data
        monData.nFwSnd    = fwEvtsSnd;
        monData.nFwRecT   = fwEvtsRecT;
        monData.nFwRecErr = fwEvtsRecErr;
        monData.nFwBurst  = fwEvtsBurst;
        monData.nFwRecD   = fwEvtsRecD;
        disMonData        = monData;
        if (disMonData.tMin ==  INITMINMAX) disMonData.tMin = NAN;
        if (disMonData.tMax == -INITMINMAX) disMonData.tMax = NAN;

        if (startServer) {
          // update service data
          dis_update_service(disStatusId);
          dis_update_service(disStateId);
          dis_update_service(disVersionId);
          dis_update_service(disMonDataId);
        } // if startServer


        // logging
        printFlag      = 0;

        if (actState != fwState) {
          printFlag    = 1;
          actState     = fwState;
        } // if actstate
        if (actStatus  != fwStatus) {
          printFlag    = 1;
          actStatus    = fwStatus;
        } // if actstatus

        if (((t_new - t_lastlog) / one_ms_ns) > 60000) { // update once per minute
          printFlag = 1;
          t_lastlog = t_new;
        } // if lastlog

      if (printFlag) {
        // grrrr.... logger omits white spaces, thus the formatting becomes lousy
        // the following lines do padding with a dedicated character 
#define LENGID 11                                        // GID name with most characters is SIS18_RING requiring a MIL gateway
        char fill[LENGID+1];
        int  len;
        for (int i=0;i<LENGID;i++) fill[i] = '.';
        len = strlen(domainName);
        if (len < LENGID) sprintf(&(fill[LENGID - len]), "%s", domainName);

        // print to screen (with optional piping to logger)
        printf("env %s, gid %s", environment, fill);
        printf(", nSent %012lu", fwEvtsSnd);
         printf(", %s (%06u), ",  comlib_stateText(fwState), nBadState);
         if ((fwStatus >> COMMON_STATUS_OK) & 0x1) printf("OK   (%06u)\n", nBadStatus);
         else printf("NOTOK(%06u)\n", nBadStatus);
         // print set status bits (except OK)
         for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(fwStatus)*8); i++) {
           if ((fwStatus >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrmil_status_text(i));
         } // for i
         fflush(stdout);
      } // if printFlag
        

#ifdef USEMASP
      if (fwState  == COMMON_STATE_OPREADY) maspSigOpReady  = true;
      else                                  maspSigOpReady  = false;

      maspSigTransfer = true;   // ok, this is dummy for now, e.g. in case of MIL troubles or so
      
      // use masp end of scope emitter
      {  
        MASP::End_of_scope_status_emitter scoped_emitter(maspNomen, emitter);
        scoped_emitter.set_OP_READY(maspSigOpReady);
        // scoped_emitter.set_custom_status(DMUNIPZ_MASP_CUSTOMSIG, maspSigTransfer); disabled as our boss did not like it
      } // <--- status is send when the End_of_scope_emitter goes out of scope  
#endif
       
      } // if update

      // clear data
      if (flagClear) {
        clearStats();                           // clear server
        wrmil_cmd_cleardiag(ebDevice);          // clear fw diags
        
        flagClear = 0;
      } // if flagclear
    } // while true

    wrmil_firmware_close(ebDevice);    
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


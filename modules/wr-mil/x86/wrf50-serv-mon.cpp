/*******************************************************************************************
 *  wrf50-serv-mon.cpp
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 04-Jun-2024
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
#define WRF50_SERV_MON_VERSION 0x000004

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
#include <wr-f50.h>                     // FW

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
#define RISING_EDGE_ID     0x1          // last 4 bits of EvtId when a rising edge is detected for ECA/TLU input
#define UPDATE_TIME_MS    1000          // time for status updates [ms]
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

// local variables
monval_t  monData;                      // monitoring data
double    tAveOld;                      // helper for stats
double    tAveStreamOld;                // helper for stats
int       flagClear;                    // flag for clearing diag data;
uint32_t  matchWindow       = WRF50_POSTTRIGGER_TLU;
uint32_t  modeCompare       = 0;
uint64_t  offsetStart;                  // correction to be used for different monitoring types

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
  monData.nFwSnd     = 0x0;
  monData.nFwRecD    = 0x0;
  monData.nFwRecT    = 0x0;
  monData.nFwRecErr  = 0x0;
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
  static int          flagDmMsg;          // flag: marks that the 'start' message from Data Master has been received
  static uint64_t     deadlineDmMsgAct;   // deadline of act DM message
  static uint64_t     deadlineDmMsgPrev;  // deadline of previous DM message
  static int32_t      cyclenDmAct;        // length of act DM cycle
  static int32_t      cyclenDmPrev;       // length of previous DM cycle
  static uint64_t     deadlineF50Act;     // deadline of act mains cycle
  static uint64_t     deadlineF50Prev;    // deadline of previous mains cycle
  static int32_t      cyclenF50Act;       // length of act mains cycle
  static int32_t      cyclenF50Prev;      // length of previous mains cycle
  
  uint32_t            mFid;            // FID 
  uint32_t            mGid;            // GID

  double              mean;            // mean value for statistics
  double              sdev;            // sdev for statistics
  double              stream;          // a stream value for statistics
  double              dummy;
  
  int32_t             tmp;

  mFid        = ((evtId  & 0xf000000000000000) >> 60);
  mGid        = ((evtId  & 0x0fff000000000000) >> 48);

  stream      = 0;
  sdev        = 0;

  // check ranges
  if (mFid != FID)                        return;  // unexpected format of timing message
  if (mGid != PZU_F50)                    return;  // unexpected GID
  if (tag   > tagStop)                    return;  // illegal tag
  
  switch (tag) {
    case tagStart   :
      // cycle start message from Data Master has been received
      monData.nStart++; 
      deadlineDmMsgAct             = deadline.getTAI();

      if (monData.nStart > WRF50_N_STAMPS) {
        // assume we are in a valid regime
        flagDmMsg                  = 1;
        cyclenDmAct                = (int32_t)(deadlineDmMsgAct - deadlineDmMsgPrev);

        // check if DM is within limits
        if (cyclenDmAct > WRF50_CYCLELEN_MAX)                  printf("wr-f50: Data Master - actual cycle length exceeds maximum value : %10.3f us\n", (double)cyclenDmAct/1000.0);
        if (cyclenDmAct < WRF50_CYCLELEN_MIN)                  printf("wr-f50: Data Master - actual cycle length below minimum value   : %10.3f us\n", (double)cyclenDmAct/1000.0);
        tmp                        = (int32_t)(cyclenDmAct - cyclenDmPrev);
        if (abs(tmp)    > WRF50_LOCK_DIFFDM)                   printf("wr-f50: Data Master - actual and previous cycle length differ by: %10.3f us\n", (double)tmp/1000.0);
        cyclenDmPrev               = cyclenDmAct;
      }  // if nStart
      cyclenDmPrev                 = deadlineDmMsgAct -  deadlineDmMsgPrev;
      deadlineDmMsgPrev            = deadlineDmMsgAct;
      
      break;
    case tagStop    :
      // 50 Hz mains signal has been received
      monData.nStop++;
      deadlineF50Act               = deadline.getTAI() - (uint64_t)WRF50_POSTTRIGGER_TLU;

      if (monData.nStop >  WRF50_N_STAMPS + 1) {
        // assume we are in a valid regime

        // check if mains is wihin limits
        cyclenF50Act               = (int32_t)(deadlineF50Act - deadlineF50Prev);
        if (cyclenF50Act > WRF50_CYCLELEN_MAX)                 printf("wr-f50: 50 Hz mains - actual cycle length exceeds maximum value : %10.3f us\n", (double)cyclenF50Act/1000.0);
        if (cyclenF50Act < WRF50_CYCLELEN_MIN)                 printf("wr-f50: 50 Hz mains - actual cycle length exceeds minimum value : %10.3f us\n", (double)cyclenF50Act/1000.0);
        tmp                        = cyclenF50Act - cyclenF50Prev;
        if (abs(tmp) > WRF50_LOCK_DIFFMAINS)                   printf("wr-f50: 50 Hz mains - actual and previous cycle length differ by: %10.3f us\n", (double)tmp/1000.0);
        // cyclenF50Prev              = cyclenF50Act;

        // check if we Data Master is locked
        if (!flagDmMsg)                                        printf("wr-f50: missing cycle start message from Data Master\n");
        else {
          // we have a message from Data Master, check if Data Master is locked
          tmp                      = (int32_t)(deadlineDmMsgAct - deadlineF50Act);
          if (abs(tmp) > (WRF50_CYCLELEN_MIN / 360))           printf("wr-f50: Data Master not synched to 50 Hz mains, current offset  : %10.3f us\n", (double)tmp/1000.0);
          else {
            // calc stats
            monData.tAct                  = (double)tmp / 1000.0;
            monData.nMatch++;
            if (monData.tAct < monData.tMin) monData.tMin = monData.tAct;
            if (monData.tAct > monData.tMax) monData.tMax = monData.tAct;
          
            calcStats(&mean, tAveOld, &stream, tAveStreamOld, monData.tAct, monData.nMatch , &dummy, &sdev);
            tAveOld       = mean;
            tAveStreamOld = stream;
            monData.tAve  = mean;
            monData.tSdev = sdev;
          } // else abs

          flagDmMsg                = 0;
        } // else flagDmMsg
      } // if nStop
      cyclenF50Prev                = deadlineF50Act - deadlineF50Prev;
      deadlineF50Prev              = deadlineF50Act;

      break;
    default         :
      ;
  } // switch tag
  
  //printf("out tag %d, bpid %d\n", tag, bpid);
} // timingmessage


// this will be called when receiving ECA actions from software action queue
// informative: this routine is presently not used, as the softare action queue does not support the TEF field
/*static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int                 flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessage */


// call back for command
class RecvCommand : public DimCommand
{
  int  reset;
  void commandHandler() {flagClear = 1;}
public :
  RecvCommand(const char *name) : DimCommand(name,"C"){}
}; 


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];

  // 'generic' services
  sprintf(name, "%s_version_fw", prefix);
  sprintf(disVersion, "%s",  wrmil_version_text(WRF50_SERV_MON_VERSION));
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
  disMonDataId  = dis_add_service(name, "I:2;X:3;I:1;X:3;I:3;D:5", &(disMonData), sizeof(monval_t), 0, 0);
} // disAddServices

                        
// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -h                   display this help and exit"                                  << std::endl;
  std::cerr << "  -e                   display version"                                             << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)"    << std::endl;
  std::cerr << "  -d                   start server publishing data"                                << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool monitors a UNILAC 50 Hz synchronisation unit (wrf50)"                     << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " -d pro'"                                               << std::endl;
  std::cerr << std::endl;
  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << wrmil_version_text(WRF50_SERV_MON_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int      opt;
  bool     useFirstDev    = false;
  bool     getVersion     = false;
  bool     startServer    = false;

  char    *tail;


  // variables snoop event
  uint64_t snoopID     = 0x0;
  int      nCondition  = 2;

  char     tmp[752];
  int      i;

  // variables attach, remove
  char    *deviceName = NULL;

  char     domainName[NAMELEN];          // name of MIL domain
  char     prefix[NAMELEN*2];            // prefix DIM services
  char     disName[DIMMAXSIZE];          // name of DIM server
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
#ifdef USEMASP
        maspNomen = std::string("U_WRF50_SYNC");    break;
#endif
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

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  deviceName = argv[optind];
  gethostname(disHostname, 32);
  sprintf(domainName,"%s", "pzu_f50");
  
  if (optind+1 < argc) sprintf(prefix, "wrmil_%s_%s-mon", argv[++optind], domainName);
  else                 sprintf(prefix, "wrmil_%s-mon", domainName);

  if (startServer) {
    printf("%s: starting server using prefix %s\n", program, prefix);

    clearStats();
    disAddServices(prefix);
    // uuuuhhhh, mixing c++ and c  
    sprintf(tmp, "%s-cmd_cleardiag", prefix);
    RecvCommand cmdClearDiag(tmp);
    
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
    if ((status =  wrf50_firmware_open(&ebDevice, ebPath, 0, &cpu)) != COMMON_STATUS_OK) {
      std::cerr << program << ": can't open connection to lm32 firmware" << std::endl;
      exit(1);
    } // if status
    
    if (getVersion) {
      wrmil_version_library(&verLib);
      printf("wr-f50: serv-sys / library / firmware /  version %s / %s",  wrmil_version_text(verLib), wrmil_version_text(WRF50_SERV_MON_VERSION));     
      wrmil_version_firmware(ebDevice, &verFw);
      printf(" / %s\n",  wrmil_version_text(verFw));     
    } // if getVersion

#ifdef USEMASP 
    // optional: disable masp logging (default: log to stdout, can be customized)
    MASP::no_logger no_log;
    MASP::Logger::middleware_logger = &no_log;

    MASP::StatusEmitter emitter(get_config());
    std::cout << "wr-f50: emmitting to MASP as sourceId: " << maspSourceId << ", using nomen: " << maspNomen << ", environment pro: " << maspProductive << std::endl;
#else
    std::cout << "wr-f50: no MASP emitter!" << std::endl;
#endif // USEMASP

    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

    uint32_t tag[nCondition];
    uint32_t tmpTag;

    // define conditions (ECA filter rules)

    // message that is sent by the Data Master upon each cycle start of UNILAC; in an ideal world this message will have
    // have the same deadline as the 50 Hz mains TTL signal 
    tmpTag        = tagStart;
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)PZU_F50 << 48) | ((uint64_t)WRF50_ECADO_F50_DM << 36);
    condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffffffff00000000, 0));
    tag[0]        = tmpTag;

    // message that is injected locally by the ECA TLU when receiving a 50 Hz mains TTL signal
    tmpTag        = tagStop;
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)PZU_F50 << 48) | ((uint64_t)WRF50_ECADO_F50_TLU  << 36) | (uint64_t)RISING_EDGE_ID;
    condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffffffffffffffff, WRF50_POSTTRIGGER_TLU));
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


    saftlib::Time deadline_t;
    uint64_t      t_new, t_old;
    uint32_t      tmp32a, tmp32b, tmp32c, tmp32d;
    int32_t       stmp32a, stmp32b, stmp32c, stmp32d, stmp32e, stmp32f, stmp32g, stmp32h, stmp32i;
    uint32_t      fwTMainsAct, fwEvtsLate, fwState, fwVersion, fwLockState, fwNLocked, fwNCycles, fwMode, fwOffsDone;
    uint64_t      fwStatus, fwLockDate;
    int32_t       fwF50Offs;
    int           nUpdate = 0;

    t_old = comlib_getSysTime();
    while(true) {
      // irgendwo hier periodisch DIM service aktualisieren bzw. update auf Bildschirm bzw. update MASP
      monData.gid   = PZU_F50;
      monData.cMode = modeCompare;
      saftlib::wait_for_signal(UPDATE_TIME_MS / 10);

      t_new = comlib_getSysTime();
      if (((t_new - t_old) / one_ms_ns) > UPDATE_TIME_MS) {
        t_old      = t_new;
        nUpdate++;
        if (nUpdate > 10) {
           printf("wr-f50: act mains frequency                                     : %10.3f Hz\n", 1000000000.0/(double)fwTMainsAct);
           fflush(stdout);                                                                         // required for immediate writing (if stdout is piped to syslog)
           nUpdate = 0;
        } // argh ...

        // update firmware data
        wrmil_common_read(ebDevice, &fwStatus, &fwState, &tmp32a, &tmp32b, &fwVersion, &tmp32c, 0);
        wrf50_info_read(ebDevice, &fwF50Offs, &fwMode , &fwTMainsAct, &tmp32b, &tmp32c, &stmp32a, &stmp32b, &stmp32c, &stmp32g, &stmp32h, &stmp32i, &stmp32d, &stmp32e, &stmp32f, &fwLockState, &fwLockDate, &fwNLocked,
                        &fwNCycles, &fwEvtsLate, &fwOffsDone, &tmp32d, 0);

        disStatus  = fwStatus;
        sprintf(disState  , "%s", wrmil_state_text(fwState));
        sprintf(disVersion, "%s", wrmil_version_text(fwVersion));
               
        // update monitoring data
        monData.nFwSnd    = fwNCycles;
        monData.nFwRecT   = fwNCycles;
        monData.nFwRecErr = 0;
        monData.nFwRecD   = 0;
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

        //printf("wrmil-mon: fw snd %ld, recD %ld, recT %ld; mon snd %ld, rec %ld, match %ld, act %f, ave %f, sdev %f, min %f, max %f\n", monData.nFwSnd, monData.nFwRecT, monData.nFwRecT, monData.nStart, monData.nStop, monData.nMatch, monData.tAct, monData.tAve, monData.tSdev, monData.tMin, monData.tMax);

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
        clearStats();
        flagClear = 0;
      } // if flagclear
    } // while true

    wrf50_firmware_close(ebDevice);    
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


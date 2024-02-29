/*******************************************************************************************
 *  wrmil-serv-mon.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Feb-2024
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
#define WRMIL_SERV_MON_VERSION 0x000001

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

using namespace saftlib;
using namespace std;

#define FID                0x1          // format ID of timing messages
#define UPDATE_TIME_MS    1000          // time for status updates [ms]
#define MATCH_WIN_US        20          // window for matching start and stop evts [us]

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
uint32_t  matchWindow       = MATCH_WIN_US;



uint64_t  one_us_ns = 1000;
uint64_t  one_ms_ns = 1000000;

/*uint32_t  nNotSnd           = 0;
uint32_t  nBadEvt           = 0;
uint32_t  badEvt            = 0xfff;
uint64_t  tStartOld         = 0;
uint64_t  dStartMin         = 1000000000;
uint64_t  tStopOld          = 0;
uint64_t  dStopMin          = 1000000000;
#define   NMONI 400
int       nMon              = 0;
uint64_t  deadlineMon[NMONI];
uint32_t  evtNoMon[NMONI];
uint32_t  tagMon[NMONI];
uint32_t  flagsMon[NMONI];
*/


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
  monData.nFwSnd   = 0x0;
  monData.nFwRecD  = 0x0;
  monData.nFwRecT  = 0x0;
  monData.nSnd     = 0x0;
  monData.nRec     = 0x0;
  monData.nMatch   = 0x0;
  monData.tAct     = NAN;
  monData.tMin     = 1000000000;
  monData.tMax     = -1000000000;
  monData.tAve     = NAN;
  monData.tSdev    = NAN;        
  tAveOld          = NAN;
  tAveStreamOld    = NAN;
} // clearStats


// handle received timing message
static void timingMessage(uint64_t evtId, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  static int          flagMilSent;     // flag: marks that a MIL event has been sent
  static uint64_t     sndDeadline;     // deadline of MIL event sent
  
  static uint32_t     sndEvtNo;        // evtNo of MIL event sent
  
  uint32_t            mFid;            // FID 
  uint32_t            mGid;            // GID
  uint32_t            mEvtNo;          // event number

  double              mean;            // mean value for statistics
  double              sdev;            // sdev for statistics
  double              stream;          // a stream value for statistics
  double              dummy;

  mFid        = ((evtId  & 0xf000000000000000) >> 60);
  mGid        = ((evtId  & 0x0fff000000000000) >> 48);
  mEvtNo      = ((evtId  & 0x0000fff000000000) >> 36);

  stream      = 0;
  sdev        = 0;

  /*
  deadlineMon[nMon] = deadline.getTAI();
  evtNoMon[nMon]    = mEvtNo;
  tagMon[nMon]      = tag;
  flagsMon[nMon]    = flags;
  nMon++;
  if (nMon == NMONI) {
    for (int i=0; i<NMONI; i++) 
      printf("tag %d, evt %2x, deadline %lu, flags %u\n", tagMon[i], evtNoMon[i], deadlineMon[i], flagsMon[i]);
    exit(1);
  }
  */
  /*
  printf("tag %d, evt %2x, deadline %lu, flags %u\n", tag, mEvtNo, deadline.getTAI(), flags);
  */

  // check ranges
  if (mFid != FID)                        return;  // unexpected format of timing message
  if (tag   > tagStop)                    return;  // illegal tag
  //printf("tag %d\n", tag);

  
  switch (tag) {
    case tagStart   :
      if (mGid != monData.gid)            return;  // received GID does not match configuration
      monData.nSnd++; 
      flagMilSent                  = 1;
      sndDeadline                  = deadline.getTAI();
      /*if ((sndDeadline - tStartOld) < dStartMin) dStartMin = sndDeadline - tStartOld;
        tStartOld = sndDeadline;*/
      sndEvtNo                     = mEvtNo;
      monData.tAct                 = NAN;
      //monData.tMin                 = NAN;
      //monData.tMax                 = NAN;
      //monData.tAve                 = NAN;
      //monData.tSdev                = NAN;
      break;
    case tagStop    :
      monData.nRec++;
      /*if ((deadline.getTAI() - tStopOld) < dStopMin) dStopMin = deadline.getTAI() - tStopOld;
        tStopOld = deadline.getTAI();*/

      if (!flagMilSent)                   return;  // a MIL telegram has been received although no MIL telegram has been sent: give up
      flagMilSent                  = 0;            // we received a MIL telegram pair: after 'returning', we start waiting for a new pair
      if (mEvtNo != sndEvtNo)             return;  // evtNo of MIL received/sent telegrams do not match: give up*/

      // assume we have a matching pair of sent and received MIL telegrams
      monData.nMatch++;
      monData.tAct                  = (double)((int64_t)(deadline.getTAI() - one_us_ns * matchWindow - sndDeadline)) / (double)one_us_ns;
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
  
  //printf("out tag %d, bpid %d\n", tag, bpid);
} // timingmessage


// this will be called when receiving ECA actions from software action queue
// informative: this routine is presently not used, as the softare action queue does not support the TEF field
/*static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int                 flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessag*/


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
  disMonDataId  = dis_add_service(name, "I:1;X:6;D:5", &(disMonData), sizeof(monval_t), 0, 0);
} // disAddServices

                        
// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -h                   display this help and exit"                                  << std::endl;
  std::cerr << "  -e                   display version"                                             << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)"    << std::endl;
  std::cerr << "  -d                   start server publishing data"                                << std::endl;
  std::cerr << "  -m <match windows>   [us] windows for matching start/stop evts; default 20"       << std::endl;
  std::cerr << std::endl;
  std::cerr << "  The paremter -s is mandatory"                                                     << std::endl;
  std::cerr << "  -s <MIL domain>      MIL domain; this can be"                                     << std::endl;
  std::cerr << "                       0: PZU-QR; UNILAC, Source Right\n"                           << std::endl;
  std::cerr << "                       1: PZU-QL; UNILAC, Source Left\n"                            << std::endl;     
  std::cerr << "                       2: PZU-QN; UNILAC, Source High Charge State Injector (HLI)\n"<< std::endl;
  std::cerr << "                       3: PZU-UN; UNILAC, High Charge State Injector (HLI)\n"       << std::endl;
  std::cerr << "                       4: PZU-UH; UNILAC, High Current Injector (HSI)\n"            << std::endl;
  std::cerr << "                       5: PZU-AT; UNILAC, Alvarez Cavities\n"                       << std::endl;
  std::cerr << "                       6: PZU-TK; UNILAC, Transfer Line\n"                          << std::endl;
  std::cerr << "                       7: PZ-SIS18\n"                                               << std::endl;
  std::cerr << "                       8: PZ-ESR\n"                                                 << std::endl;
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

  char    *tail;


  // variables snoop event
  uint64_t snoopID     = 0x0;
  int      nCondition  = 2;

  char     tmp[752];
  int      tmpi;
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
  while ((opt = getopt(argc, argv, "s:m:hefd")) != -1) {
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
          case 4: gid = PZU_UH;     sprintf(domainName, "%s", "pzu_ut");     break;
          case 5: gid = PZU_AT;     sprintf(domainName, "%s", "pzu_at");     break;
          case 6: gid = PZU_TK;     sprintf(domainName, "%s", "pzu_tk");     break;
          case 7: gid = SIS18_RING; sprintf(domainName, "%s", "sis18_ring"); break;
          case 8: gid = ESR_RING;   sprintf(domainName, "%s", "esr_ring");   break;
          default: {std::cerr << "Specify a proper number, not " << tmpi << "'%s'!" << std::endl; return 1;} break;
        } // case tmpi
        break;
      case 'm':
        matchWindow = strtoull(optarg, &tail, 0);
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
  
  if (optind+1 < argc) sprintf(prefix, "wrmil_%s_%s-mon", argv[++optind], domainName);
  else                 sprintf(prefix, "wrmil_%s-mon", domainName);

  if (startServer) {
    printf("%s: starting server using prefix %s\n", program, prefix);

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


    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

    /*
    // search for embedded CPU channel
     map<std::string, std::string> e_cpus = receiver->getInterfaces()["EmbeddedCPUActionSink"];
    if (e_cpus.size() != 1)
    {
      std::cerr << "Device '" << receiver->getName() << "' has no embedded CPU!" << std::endl;
      return (-1);
    }
    // connect to embedded CPU
    std::shared_ptr<EmbeddedCPUActionSink_Proxy> e_cpu = EmbeddedCPUActionSink_Proxy::create(e_cpus.begin()->second);

    // create action sink for ecpu
    std::shared_ptr<EmbeddedCPUCondition_Proxy> condition[nCondition];
    */
    uint32_t tag[nCondition];
    uint32_t tmpTag;

    // define conditions (ECA filter rules)

    // message that is injected locally by the lm32 firmware (triggering a rule on the ECA WB channel towards the MIL interface)
    // this message must be delayed by one ms to (hopefully) coincide with the received MIL telegram
    tmpTag        = tagStart;
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48);
    condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, one_ms_ns));
    tag[0]        = tmpTag;
        
    // message that is injected locally by the lm32 firmware after detecting and decoding a receive MIL telegram
    // this message is delayed by 20us to avoid a) a collision b) to have a defined order of both messages
    // (this assumes that the 'true' offset between both messages is always smaller than 20us)
    tmpTag        = tagStop;        
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)LOC_MIL_REC << 48);
    condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, one_us_ns * matchWindow));
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
    uint32_t      tmp32a, tmp32b, tmp32c, tmp32d, tmp32e, tmp32f, tmp32g, tmp32h, tmp32i;
    int32_t       stmp32a;
    uint64_t      tmp64a;
    uint32_t      fwGid, fwLate, fwState, fwVersion;
    uint64_t      fwEvtsSnd, fwEvtsRecT, fwEvtsRecD, fwStatus;

    t_old = comlib_getSysTime();
    while(true) {
      /*
      t1 = comlib_getSysTime();
      ecaStatus = comlib_wait4ECAEvent(1, device, ecaq_base, &recTag, &deadline, &evtId, &param, &tef, &isLate, &isEarly, &isConflict, &isDelayed);
      t2 = comlib_getSysTime();
      tmp32 = t2 - t1; 
      if (tmp32 > 10000000) printf("%s: reading from ECA Q took %u [us]\n", program, tmp32 / 1000);
      if (ecaStatus == COMMON_STATUS_EB) { printf("eca EB error, device %x, address %x\n", device, (uint32_t)ecaq_base);}
      if (ecaStatus == COMMON_STATUS_OK) {
        deadline_t = saftlib::makeTimeTAI(deadline);
        //t2         = comlib_getSysTime(); printf("msg: tag %x, id %lx, tef %lx, dtu %lu\n", recTag, evtId, tef, (uint32_t)(t2 -t1));
        timingMessage(recTag, deadline_t, evtId, param, tef, isLate, isEarly, isConflict, isDelayed);
        }*/
      // irgendwo hier periodisch DIM service aktualisieren bzw. update auf Bildschirm bzw. update MASP
      monData.gid = gid;
      saftlib::wait_for_signal(UPDATE_TIME_MS / 10);

      t_new = comlib_getSysTime();
      if (((t_new - t_old) / one_ms_ns) > UPDATE_TIME_MS) {
        t_old      = t_new;

        // update firmware data
        wrmil_common_read(ebDevice, &fwStatus, &fwState, &tmp32a, &tmp32b, &fwVersion, &tmp32c, 0);
        wrmil_info_read(ebDevice, &tmp32a, &tmp32b, &tmp32c, &fwGid, &stmp32a, &tmp64a, &tmp32f, &tmp32g, &tmp32h, &fwEvtsSnd, &fwEvtsRecT, &fwEvtsRecD, &fwLate, &tmp32i, 0);
        if (fwGid != gid) fwStatus |= COMMON_STATUS_OUTOFRANGE; // signal an error

        disStatus  = fwStatus;
        sprintf(disState  , "%s", wrmil_state_text(fwState));
        sprintf(disVersion, "%s", wrmil_version_text(fwVersion));
               
        // update monitoring data
        monData.nFwSnd  = fwEvtsSnd;
        monData.nFwRecT = fwEvtsRecT;
        monData.nFwRecD = fwEvtsRecD;
        disMonData      = monData;

        if (startServer) {
          // update service data
          dis_update_service(disStatusId);
          dis_update_service(disStateId);
          dis_update_service(disVersionId);
          dis_update_service(disMonDataId);
        } // if startServer

        //printf("wrmil-mon: fw snd %ld, recD %ld, recT %ld; mon snd %ld, rec %ld, match %ld, act %f, ave %f, sdev %f, min %f, max %f\n", monData.nFwSnd, monData.nFwRecT, monData.nFwRecT, monData.nSnd, monData.nRec, monData.nMatch, monData.tAct, monData.tAve, monData.tSdev, monData.tMin, monData.tMax);
      } // if update

      // clear data
      if (flagClear) {
        clearStats();
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


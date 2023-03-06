/*******************************************************************************************
 *  b2b-serv-raw.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 06-Mar-2023
 *
 * publishes raw data of the b2b system
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
#define B2B_SERV_RAW_VERSION 0x000424

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
//#include "SoftwareActionSink.h"
//#include "SoftwareCondition.h"
#include "EmbeddedCPUActionSink.h"
#include "EmbeddedCPUCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// b2b includes
//#include <wb_devices.h>                 // wb_api
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

// services
char      disVersion[DIMCHARSIZE];
char      disState[DIMCHARSIZE];
char      disHostname[DIMCHARSIZE];
uint64_t  disStatus;
uint32_t  disNTransfer;
setval_t  disSetval[B2B_NSID];
getval_t  disGetval[B2B_NSID];

uint32_t  disVersionId      = 0;
uint32_t  disStateId        = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disStatusId       = 0;
uint32_t  disNTransferId    = 0;
uint32_t  disSetvalId[B2B_NSID];
uint32_t  disGetvalId[B2B_NSID];

// local variables
uint32_t reqExtRing;                    // requested extraction ring

uint32_t sid;                           // Sequence ID
uint32_t bpid;                          // Beam Process ID


// update set value
void disUpdateSetval(uint32_t sid, uint64_t tStart, setval_t setval)
{
  uint32_t secs;
  uint32_t msecs;
  
  b2b_t2secs(tStart, &secs, &msecs);
  msecs  /= 1000000;
  
  disSetval[sid] = setval;
  dis_set_timestamp(disSetvalId[sid], secs, msecs);
  dis_update_service(disSetvalId[sid]);
  
  disNTransfer++;
  dis_update_service(disNTransferId);
} // disUpdateSetval
 

// update get value
 void disUpdateGetval(uint32_t sid, uint64_t tStart, getval_t getval)
{
  uint32_t secs;
  uint32_t msecs;

  b2b_t2secs(tStart, &secs, &msecs);
  msecs  /= 1000000;
  
  disGetval[sid] = getval;
  dis_set_timestamp(disGetvalId[sid], secs, msecs);
  dis_update_service(disGetvalId[sid]);
} // disUpdateGetval


// handle received timing message
static void timingMessage(uint32_t tag, saftlib::Time deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t isLate, uint32_t isEarly, uint32_t isConflict, uint32_t isDelayed)
{
  uint32_t            recSid;          // received SID
  int                 flagErr;

  static int          flagActive;      // flag: b2b is active
  static setval_t     setval;          // set values
  static getval_t     getval;          // get values
  static uint64_t     tStart;          // time of transfer

  uint64_t one_ns_as = 1000000000;
  fdat_t   tmp;
  float    tmpf;

  recSid      = ((evtId  & 0x00000000fff00000) >> 20);

  // check ranges
  if (recSid  > B2B_NSID)                 return;
  if (tag > tagStop)                      return;
  if ((!flagActive) && (tag != tagStart)) return;
  //printf("tag %d\n", tag);
  // mark message as received
  getval.flagEvtRec  |= 0x1 << tag;
  getval.flagEvtLate |= isLate << tag;;
  
  switch (tag) {
    case tagStart   :
      sid                      = recSid;
      tStart                   = deadline.getUTC();
      flagActive               = 1;
      setval.flag_nok          = 0xfffffffe;                  // mode is 'ok'
      setval.mode              = 0;
      setval.ext_T             = 0;
      setval.ext_h             = 0;
      setval.ext_cTrig         = 0;
      setval.inj_T             = 0;
      setval.inj_h             = 0;
      setval.inj_cTrig         = 0;
      setval.cPhase            = 0;
      getval.flag_nok          = 0xffffffff;
      getval.ext_phase         = 0;
      getval.ext_phaseFract_ps = 0;
      getval.ext_phaseErr_ps   = 0;;
      getval.ext_dKickMon      = 0;
      getval.ext_dKickProb     = 0;
      getval.ext_diagPhase     = 0;
      getval.ext_diagMatch     = 0;
      getval.inj_phase         = 0;
      getval.inj_phaseFract_ps = 0;
      getval.inj_phaseErr_ps   = 0;;
      getval.inj_dKickMon      = 0;
      getval.inj_dKickProb     = 0;
      getval.inj_diagPhase     = 0;
      getval.inj_diagMatch     = 0;
      getval.flagEvtRec        = 0x1 << tag;
      getval.flagEvtErr        = 0;
      getval.flagEvtLate       = isLate << tag;
      getval.tCBS              = deadline.getTAI();
      getval.finOff            = 0;
      getval.prrOff            = 0;
      getval.preOff            = 0;
      getval.priOff            = 0;
      getval.kteOff            = 0;
      getval.ktiOff            = 0;
      break;
    case tagStop    :
      flagActive       = 0;
      disUpdateSetval(sid, tStart, setval);
      disUpdateGetval(sid, tStart, getval);      
      break;
    case tagPme     :
      setval.mode              = 2;
      setval.ext_h             = ((param & 0xff00000000000000) >> 56);
      setval.ext_T             = ((param & 0x00ffffffffffffff));    // [as]
      if (setval.ext_h) setval.flag_nok &= 0xfffffffb;              // if ok, reset bit ext_h invalid
      if (setval.ext_T) setval.flag_nok &= 0xfffffffd;              // if ok, reset bit ext_T invalid
      tmpf                     = comlib_half2float((uint16_t)((tef & 0xffff0000)  >> 16)); // [us, hfloat]; chk for NAN?
      setval.ext_cTrig         = round(tmpf * 1000.0);              // [ns]
      setval.flag_nok         &= 0xfffffff7;                        // if ok, reset bit ext_cTrig invalid
      tmpf                     = comlib_half2float((uint16_t)( tef & 0x0000ffff));         // [us, hfloat]; chk for NAN?
      setval.inj_cTrig         = round(tmpf * 1000.0);              // [ns]
      setval.flag_nok         &= 0xffffffbf;                        // / if ok, reset bit inj_cTrig invalid
      break;
    case tagPmi     :
      setval.mode              = 4;
      setval.inj_h             = ((param & 0xff00000000000000) >> 56);
      setval.inj_T             = ((param & 0x00ffffffffffffff));    // [as]
      if (setval.inj_h) setval.flag_nok &= 0xffffffdf;              // if ok, reset bit inj_h invalid
      if (setval.inj_T) setval.flag_nok &= 0xffffffef;              // if ok, reset bit inj_T invalid
      tmpf                     = comlib_half2float((uint16_t)((tef & 0xffff0000) >> 16));              // [us, hfloat]]
      setval.cPhase            = round(tmpf  * 1000);               // [ns]
      setval.flag_nok         &= 0xffffff7f;                        // if ok, reset cPhase invalid
      break;
    case tagPre     :
      getval.preOff            = param - getval.tCBS;
      getval.ext_phase         = param;
      getval.ext_phaseFract_ps = (int16_t)( tef & 0x0000ffff);
      getval.ext_phaseErr_ps   = (int16_t)((tef & 0xffff0000) >> 16);
      if (param) getval.flag_nok &= 0xfffffffe;
      flagErr                  = ((evtId & B2B_ERRFLAG_PMEXT) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagPri     :
      getval.priOff            = param - getval.tCBS;
      getval.inj_phase         = param;
      getval.inj_phaseFract_ps = (int16_t)( tef & 0x0000ffff);
      getval.inj_phaseErr_ps   = (int16_t)((tef & 0xffff0000) >> 16);
      if (param) getval.flag_nok &= 0xffffffdf;
      flagErr                  = ((evtId & B2B_ERRFLAG_PMINJ) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;     
    case tagKte     :
      if (!setval.mode) setval.mode = 1;                    // special case: extraction kickers shall fire upon CBS
      getval.kteOff            = deadline.getTAI() - getval.tCBS;
      tmpf                     = comlib_half2float((uint16_t)((tef & 0xffff0000) >> 16));        // [us, hfloat]
      getval.finOff            = round(tmpf * 1000.0);
      tmpf                     = comlib_half2float((uint16_t)(tef & 0x0000ffff));                // [us, hfloat]
      getval.prrOff            = round(tmpf * 1000.0);
      flagErr                  = ((evtId & B2B_ERRFLAG_CBU) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagKti     :
      if (setval.mode < 3) setval.mode = 3;
      getval.ktiOff            = deadline.getTAI() - getval.tCBS;
      flagErr                  = ((evtId    & 0x0000000000000010) >> 4);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagKde     :
      getval.ext_dKickProb     = param & 0x00000000ffffffff;
      getval.ext_dKickMon      = ((param & 0xffffffff00000000) >> 32);
      if (getval.ext_dKickProb != 0x7fffffff) getval.flag_nok &= 0xfffffffb;
      if (getval.ext_dKickMon  != 0x7fffffff) getval.flag_nok &= 0xfffffffd;
      flagErr                  = ((evtId & B2B_ERRFLAG_KDEXT) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagKdi     :
      getval.inj_dKickProb     = param & 0x00000000ffffffff;
      getval.inj_dKickMon      = ((param & 0xffffffff00000000) >> 32);
      if (getval.inj_dKickProb != 0x7fffffff) getval.flag_nok &= 0xffffff7f;
      if (getval.inj_dKickMon  != 0x7fffffff) getval.flag_nok &= 0xffffffbf;          
      flagErr                  = ((evtId & B2B_ERRFLAG_KDINJ) != 0);
      getval.flagEvtErr       |= flagErr << tag;
      break;
    case tagPde     :
      tmp.data                 = ((param & 0x00000000ffffffff));
      if (tmp.data != 0x7fffffff) {
        getval.flag_nok &= 0xffffffef;
        getval.ext_diagMatch   = (double)tmp.f;
      } // if ok
      tmp.data                 = ((param & 0xffffffff00000000) >> 32);
      if (tmp.data != 0x7fffffff) {
        getval.flag_nok &= 0xfffffff7;
        getval.ext_diagPhase   = (double)tmp.f;
      } // if ok
      break;
    case tagPdi     :
      tmp.data             = ((param & 0x00000000ffffffff));
      if (tmp.data != 0x7fffffff) {
        getval.flag_nok &= 0xfffffdff;
        getval.inj_diagMatch = (double)tmp.f;
      } // if ok
      tmp.data             = ((param & 0xffffffff00000000) >> 32);
      if (tmp.data != 0x7fffffff) {
        getval.flag_nok &= 0xfffffeff;
        getval.inj_diagPhase = (double)tmp.f;
      } // if ok
      break;
    default         :
      ;
  } // switch tag
  
  //printf("out tag %d, bpid %d\n", tag, bpid);
} // timingmessage


// this will be called when receiving ECA actions from software action queue
static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int                 flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessag


// call back for command
class RecvCommand : public DimCommand
{
  int  reset;
  void commandHandler() {disNTransfer = 0;}
public :
  RecvCommand(const char *name) : DimCommand(name,"C"){}
}; 


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  // 'generic' services
  sprintf(name, "%s-raw_version_fw", prefix);
  sprintf(disVersion, "%s",  b2b_version_text(B2B_SERV_RAW_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s-raw_state", prefix);
  sprintf(disState, "%s", b2b_state_text(COMMON_STATE_OPREADY));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s-raw_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", &disHostname, 32, 0 , 0);

  sprintf(name, "%s-raw_status", prefix);
  disStatus       = 0x1;   
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  sprintf(name, "%s-raw_ntransfer", prefix);
  disNTransferId  = dis_add_service(name, "I", &disNTransfer, sizeof(disNTransfer), 0 , 0);

  // set values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_setval", prefix, i);
    disSetvalId[i]  = dis_add_service(name, "I:1;I:1;X:1;I:1;F:1;X:1;I:1;F:2", &(disSetval[i]), sizeof(setval_t), 0, 0);
    dis_set_timestamp(disSetvalId[i], 1, 0);
  } // for i

  // set values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_getval", prefix, i);
    disGetvalId[i]  = dis_add_service(name, "I:1;X:1;I:4;F:2;X:1;I:4;F:2;I:3;X:1;I:6", &(disGetval[i]), sizeof(getval_t), 0, 0);
    dis_set_timestamp(disGetvalId[i], 1, 0);
  } // for i
} // disAddServices

                        
//using namespace saftlib;
//using namespace std;

// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -e<index>            specify extraction ring (0: SIS18[default], 1: ESR, 2: CRYRING)" << std::endl;
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool provides a server for raw b2b data." << std::endl;
  std::cerr << "Example1: '" << program << " tr0 -e0 pro'" << std::endl;
  std::cerr << std::endl;

  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << b2b_version_text(B2B_SERV_RAW_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help

int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  char *tail;


  // variables snoop event
  uint64_t snoopID     = 0x0;
  int      nCondition  = 0;

  char tmp[128];
  int i;

  // variables attach, remove
  char    *deviceName = NULL;

  char     ringName[DIMMAXSIZE];
  char     prefix[DIMMAXSIZE];
  char     disName[DIMMAXSIZE];


  reqExtRing  = SIS18_RING;


  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "e:hf")) != -1) {
    switch (opt) {
      case 'e' :
        switch (strtol(optarg, &tail, 0)) {
          case 0 : reqExtRing = SIS18_RING;   break;
          case 1 : reqExtRing = ESR_RING;     break;
          case 2 : reqExtRing = CRYRING_RING; break;
          default:
            std::cerr << "option -e: parameter out of range" << std::endl;
            return 1;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
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

  deviceName = argv[optind];
  gethostname(disHostname, 32);

  switch(reqExtRing) {
    case SIS18_RING :
      nCondition = 15;
      sprintf(ringName, "sis18");
      break;
    case ESR_RING :
      nCondition = 15;
      sprintf(ringName, "esr");
      break;
    case CRYRING_RING :
      nCondition = 7;
      sprintf(ringName, "yr");
      break;
    default :
        std::cerr << "Ring '"<< reqExtRing << "' does not exist" << std::endl;
        return -1;;
  } // switch extRing
  

  
  if (optind+1 < argc) sprintf(prefix, "b2b_%s_%s", argv[++optind], ringName);
  else                 sprintf(prefix, "b2b_%s", ringName);

  printf("%s: starting server using prefix %s\n", program, prefix);

  disAddServices(prefix);
  // uuuuhhhh, mixing c++ and c  
  sprintf(tmp, "%s-raw_cmd_cleardiag", prefix);
  RecvCommand cmdClearDiag(tmp);
  
  sprintf(disName, "%s-raw", prefix);
  dis_start_serving(disName);
  
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
    //std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    //std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

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
    uint32_t tag[nCondition];
    uint32_t tmpTag;

    // define conditions (ECA filter rules)
    switch (reqExtRing) {
      case SIS18_RING : 

        // SIS18, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[0]        = tmpTag;
        
        // SIS18, CMD_B2B_START, +100ms (!), signals stop of data collection
        tmpTag        = tagStop;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        tag[1]        = tmpTag;
        
        // SIS18 to extraction, PMEXT,
        tmpTag        = tagPme;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[2]        = tmpTag;

        // SIS18 to extraction, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[3]        = tmpTag;

        // SIS18 to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[4]        = tmpTag;

        // SIS18 to ESR, PMEXT
        tmpTag        = tagPme;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[5]        = tmpTag;

        // SIS18 to ESR, PMINJ
        tmpTag        = tagPmi;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[6]        = tmpTag;

        // SIS18 to ESR, PREXT
        tmpTag        = tagPre;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[7]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[7]        = tmpTag;

        // SIS18 to ESR, PRINJ
        tmpTag        = tagPri;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PRINJ << 36);
        condition[8]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[8]        = tmpTag;
   
        // SIS18 to ESR, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[9]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[9]        = tmpTag;

        // SIS18 to ESR, DIAGINJ
        tmpTag        = tagPdi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);
        condition[10] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[10]       = tmpTag;
        
        // SIS18 extraction kicker trigger
        tmpTag        = tagKte;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[11] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[11]       = tmpTag;
        
        // SIS18 extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[12] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[12]       = tmpTag;
        
        // ESR injection kicker trigger
        tmpTag        = tagKti;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGERINJ << 36);
        condition[13] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[13]       = tmpTag;
        
        // ESR injection kicker diagnostic
        tmpTag        = tagKdi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKINJ << 36);
        condition[14] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[14]       = tmpTag;
        
        break;
      case ESR_RING : 

        // ESR, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[0]        = tmpTag;
        
        // ESR, CMD_B2B_START, +100ms (!), signals stop of data collection 
        tmpTag        = tagStop;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        tag[1]        = tmpTag;
        
        // ESR to extraction, PMEXT, 
        tmpTag        = tagPme;       
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[2]        = tmpTag;

        // ESR to extraction, PREXT
        tmpTag        = tagPre;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[3]        = tmpTag;

        // ESR to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[4]        = tmpTag;
       
        // ESR to CRYRING, PMEXT
        tmpTag        = tagPme;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[5]        = tmpTag;

        // ESR to CRYRING, PMINJ
        tmpTag        = tagPmi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[6]        = tmpTag;

        // ESR to CRYRING, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[7]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[7]        = tmpTag;

        // ESR to CRYRING, PRINJ
        tmpTag        = tagPri;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_PRINJ << 36);
        condition[8]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[8]        = tmpTag;
   
        // ESR to CRYRING, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[9]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[9]        = tmpTag;

        // ESR to CRYRING, DIAGINJ
        tmpTag        = tagPdi;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_CRYRING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);
        condition[10] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[10]       = tmpTag;

        // ESR extraction kicker trigger
        tmpTag        = tagKte;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[11] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[11]       = tmpTag;
        
        // ESR extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[12] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[12]       = tmpTag;

        // CRYRING injection kicker trigger
        tmpTag        = tagKti;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGERINJ << 36);
        condition[13] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[13]       = tmpTag;
        
        // CRYRING injection kicker diagnostic
        tmpTag        = tagKdi;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKINJ << 36);
        condition[14] = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[14]       = tmpTag;

        break;
      case CRYRING_RING : 

        // CRYRING, CMD_B2B_START, signals start of data collection
        tmpTag        = tagStart;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[0]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[0]        = tmpTag;
        
        // CRYRING, CMD_B2B_START, +100ms (!), signals stop of data collection 
        tmpTag        = tagStop;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_START << 36);
        condition[1]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 100000000, tmpTag));
        tag[1]        = tmpTag;
        
        // CRYRING to extraction, PMEXT, 
        tmpTag        = tagPme;        
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[2]        = tmpTag;

        // CRYRING to extraction, PREXT
        tmpTag        = tagPre;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[3]        = tmpTag;

        // CRYRING to extraction, DIAGEXT
        tmpTag        = tagPde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[4]        = tmpTag;
       
        // CRYRING extraction kicker trigger
        tmpTag        = tagKte;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[5]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[5]        = tmpTag;
        
        // CRYRING extraction kicker diagnostic
        tmpTag        = tagKde;
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)CRYRING_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[6]  = EmbeddedCPUCondition_Proxy::create(e_cpu->NewCondition(false, snoopID, 0xfffffff000000000, 0, tmpTag));
        tag[6]        = tmpTag;

        break;
      default :
        std::cerr << "Extraction ring " << reqExtRing << " does not exit" << std::endl;
        exit(1);
    } // switch extRing

    // let's go!
    for (i=0; i<nCondition; i++) {
      condition[i]->setAcceptLate(true);
      condition[i]->setAcceptEarly(true);
      condition[i]->setAcceptConflict(true);
      condition[i]->setAcceptDelayed(true);
      //condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i


    eb_device_t   device;
    eb_address_t  ecaq_base;
    uint32_t      recTag;
    uint64_t      deadline;
    uint64_t      evtId; 
    uint64_t      param;  
    uint32_t      tef;
    uint32_t      isLate;
    uint32_t      isEarly;
    uint32_t      isConflict;
    uint32_t      isDelayed;
    saftlib::Time deadline_t;
    uint32_t      ecaStatus;
    eb_status_t   ebStatus;
    uint32_t      qIdx = 0;

    //uint64_t      t1, t2;
    //uint32_t      tmp32;
    //int           nEvt, nTo;          // number of received Events and timeout occurences
    //uint32_t      tEvtSum, tToSum;    // sum of time passed;
    //uint32_t      tEvtMin, tToMin;    // min time passed;
    //uint32_t      tEvtMax, tToMax;    // max time passed;
    //uint32_t      tEvtAve, tToAve;    // average time passed;

    //recTag = tagStop;
    //nEvt   = 0;
    //nTo    = 0;
        
    ebStatus = comlib_ecaq_open("dev/wbm1", qIdx, &device, &ecaq_base);
    while(true) {
      //      saftlib::wait_for_signal();

      //if (recTag == tagStop) {
        // received events
        //if (nEvt > 0) {
        //  tEvtAve = tEvtSum / nEvt;
        //  printf("rec evt processing stats [us]: min %4u, max %8u, ave %8u, n %d\n", tEvtMin, tEvtMax, tEvtAve, nEvt);
        //} // if nEvt
        //nEvt    = 0;
        //tEvtSum = 0;
        //tEvtMin = 0xffffffff;
        //tEvtMax = 0;

        // timeouts
        //if (nTo  > 0) {
        //  tToAve = tToSum / nTo;
        //  printf("timeout processing stats [us]: min %4u, max %8u, ave %8u, n %d\n", tToMin, tToMax, tToAve, nTo);          
        //} // if nTo
        //nTo    = 0;
        //tToSum = 0;
        //tToMin = 0xffffffff;;
        //tToMax = 0;
      // } // if recTag
      
      t1 = comlib_getSysTime();
      ecaStatus = comlib_wait4ECAEvent(1, device, ecaq_base, &recTag, &deadline, &evtId, &param, &tef, &isLate, &isEarly, &isConflict, &isDelayed);
      t2 = comlib_getSysTime();
      tmp32 = (t2 - t1) / 1000; // elapsed time [us]
      //if (ecaStatus != COMMON_STATUS_TIMEDOUT) {
      //  tEvtSum += tmp32;
      //  nEvt++;
      //  if (tmp32 < tEvtMin) tEvtMin = tmp32;
      //  if (tmp32 > tEvtMax) tEvtMax = tmp32;
      //} // if ecaStatus
      //else {
      //  tToSum += tmp32;
      //  nTo++;
      //  if (tmp32 < tToMin) tToMin = tmp32;
      //  if (tmp32 > tToMax) tToMax = tmp32;
      //} // else ecaStatus
      
      if (tmp32 > 10000000) printf("%s: reading from ECA Q took %u [us]\n", program, tmp32);
      if (ecaStatus == COMMON_STATUS_EB) { printf("eca EB error, device %x, address %x\n", device, ecaq_base);}
      if (ecaStatus == COMMON_STATUS_OK) {
        deadline_t = saftlib::makeTimeTAI(deadline);
        //t2         = comlib_getSysTime(); printf("msg: tag %x, id %lx, tef %lx, dtu %lu\n", recTag, evtId, tef, (uint32_t)(t2 -t1));
        timingMessage(recTag, deadline_t, evtId, param, tef, isLate, isEarly, isConflict, isDelayed);
      }
    } // while true
    comlib_ecaq_close(device);
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


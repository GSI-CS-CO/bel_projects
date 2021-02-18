/*******************************************************************************************
 *  b2b-serv-raw.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-Feb-2021
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
#define B2B_SERV_RAW_VERSION 0x000233

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

// saftlib includes
#include "SAFTd.h"
#include "TimingReceiver.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// b2b includes
#include <common-lib.h>                 // COMMON
#include <b2blib.h>                     // API
#include <b2b.h>                        // FW

using namespace std;


#define FID          0x1                // format ID of timing messages
#define EKSOFFSET    -500000            // offset for EVT_KICK_START


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


// this will be called when receiving ECA actions
static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  uint32_t            recSid;          // received SID
  int                 flagLate;
  int                 flagErr;

  static int          flagActive;      // flag: b2b is active
  static setval_t     setval;          // set values
  static getval_t     getval;          // get values
  static uint64_t     tStart;          // time of transfer

  recSid      = ((id    & 0x00000000fff00000) >> 20);
  flagLate    = flags & 0x1;

  // check ranges
  if (recSid  > B2B_NSID)                 return;
  if (tag > tagStop)                      return;
  if ((!flagActive) && (tag != tagStart)) return;
  //printf("tag %d\n", tag);
  // mark message as received
  getval.flagEvtRec  |= 0x1 << tag;
  getval.flagEvtLate |= flagLate << tag;;
  
  switch (tag) {
    case tagStart   :
      sid                  = recSid;
      tStart               = deadline.getUTC() - EKSOFFSET;
      flagActive           = 1;
      setval.flag_nok      = 0xfffffffe;                    // mode is 'ok'
      setval.mode          = 0;
      setval.ext_T         = 0;
      setval.ext_h         = 0;
      setval.ext_cTrig     = 0;
      setval.inj_T         = 0;
      setval.inj_h         = 0;
      setval.inj_cTrig     = 0;
      setval.cPhase        = 0;
      getval.flag_nok      = 0xffffffff;
      getval.ext_phase     = 0;
      getval.ext_dKickMon  = 0;
      getval.ext_dKickProb = 0;
      getval.ext_diagPhase = 0;
      getval.ext_diagMatch = 0;
      getval.inj_phase     = 0;
      getval.inj_dKickMon  = 0;
      getval.inj_dKickProb = 0;
      getval.inj_diagPhase = 0;
      getval.inj_diagMatch = 0;
      getval.flagEvtRec    = 0x1 << tag;
      getval.flagEvtErr    = 0;
      getval.flagEvtLate   = flagLate << tag;;
      getval.tEKS          = deadline.getTAI() - EKSOFFSET;;
      getval.doneOff       = 0;
      getval.preOff        = 0;
      getval.priOff        = 0;
      getval.kteOff        = 0;
      getval.ktiOff        = 0;
      break;
    case tagStop    :
      flagActive       = 0;
      disUpdateSetval(sid, tStart, setval);
      disUpdateGetval(sid, tStart, getval);      
      break;
    case tagPme     :
      setval.mode      = 2;
      setval.ext_h     = ((param & 0xff00000000000000) >> 56);
      setval.ext_T     = ((param & 0x00ffffffffffffff));
      if (setval.ext_h) setval.flag_nok &= 0xfffffffb;      // if ok, reset bit
      if (setval.ext_T) setval.flag_nok &= 0xfffffffd;      // if ok, reset bit
      break;
    case tagPmi     :
      setval.mode      = 4;
      setval.inj_h     = ((param & 0xff00000000000000) >> 56);
      setval.inj_T     = ((param & 0x00ffffffffffffff));
      if (setval.inj_h) setval.flag_nok &= 0xffffffef;
      if (setval.inj_T) setval.flag_nok &= 0xffffffdf;
      break;
    case tagPre     :
      getval.preOff      = (int32_t)(param - getval.tEKS);
      getval.ext_phase   = param;
      if (param) getval.flag_nok &= 0xfffffffe;
      flagErr            = ((id & B2B_ERRFLAG_PMEXT) != 0);
      getval.flagEvtErr |= flagErr << tag;
      break;
    case tagPri     :
      getval.priOff      = (int32_t)(param - getval.tEKS);
      getval.inj_phase   = param;
      if (param) getval.flag_nok &= 0xffffffdf;
      flagErr            = ((id & B2B_ERRFLAG_PMINJ) != 0);
      getval.flagEvtErr |= flagErr << tag;
      break;     
    case tagKte     :
      if (!setval.mode) setval.mode = 1;                    // special case: extraction kickers shall fire upon EKS
      getval.kteOff       = (int32_t)(deadline.getTAI() - getval.tEKS);
      setval.ext_cTrig    = ((param & 0x00000000ffffffff));
      getval.doneOff      = ((param & 0xffffffff00000000) >> 32);
      setval.flag_nok    &= 0xfffffff7;
      flagErr             = ((id & B2B_ERRFLAG_CBU) != 0);
      getval.flagEvtErr  |= flagErr << tag;
      break;
    case tagKti     :
      if (setval.mode < 3) setval.mode = 3;
      getval.ktiOff      = (int32_t)(deadline.getTAI() - getval.tEKS);
      setval.inj_cTrig   = ((param & 0x00000000ffffffff));
      setval.cPhase      = ((param & 0xffffffff00000000) >> 32);
      setval.flag_nok   &= 0xffffffbf;
      setval.flag_nok   &= 0xffffff7f;
      flagErr            = ((id    & 0x0000000000000010) >> 4);
      getval.flagEvtErr |= flagErr << tag;
      break;
    case tagKde     :
      getval.ext_dKickProb = ((param & 0x00000000ffffffff));
      getval.ext_dKickMon  = ((param & 0xffffffff00000000) >> 32);
      if (getval.ext_dKickProb != 0x7fffffff) getval.flag_nok &= 0xfffffffb;
      if (getval.ext_dKickMon  != 0x7fffffff) getval.flag_nok &= 0xfffffffd;
      flagErr              = ((id & B2B_ERRFLAG_KDEXT) != 0);
      getval.flagEvtErr   |= flagErr << tag;
      break;
    case tagKdi     :
      getval.inj_dKickProb = ((param & 0x00000000ffffffff));
      getval.inj_dKickMon  = ((param & 0xffffffff00000000) >> 32);
      if (getval.inj_dKickProb != 0x7fffffff) getval.flag_nok &= 0xffffff7f;
      if (getval.inj_dKickMon  != 0x7fffffff) getval.flag_nok &= 0xffffffbf;          
      flagErr              = ((id & B2B_ERRFLAG_KDINJ) != 0);
      getval.flagEvtErr   |= flagErr << tag;
      break;
    case tagPde     :
      getval.ext_diagMatch = ((param & 0x00000000ffffffff));
      getval.ext_diagPhase = ((param & 0xffffffff00000000) >> 32);
      if (getval.ext_diagMatch != 0x7fffffff) getval.flag_nok &= 0xffffffef;
      if (getval.ext_diagPhase != 0x7fffffff) getval.flag_nok &= 0xfffffff7;          
      break;
    case tagPdi     :
      getval.inj_diagMatch = ((param & 0x00000000ffffffff));
      getval.inj_diagPhase = ((param & 0xffffffff00000000) >> 32);
      if (getval.inj_diagMatch != 0x7fffffff) getval.flag_nok &= 0xfffffdff;
      if (getval.inj_diagPhase != 0x7fffffff) getval.flag_nok &= 0xfffffeff;          
      break;
    default         :
      ;
  } // switch tag
  
  //printf("out tag %d, bpid %d\n", tag, bpid);
} // on_action


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
    disSetvalId[i]  = dis_add_service(name, "I:1;I:1;X:1;I:2;X:1;I:2;I:1", &(disSetval[i]), sizeof(setval_t), 0, 0);
  } // for i

  // set values
  for (i=0; i< B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_getval", prefix, i);
    disGetvalId[i]  = dis_add_service(name, "I:1;X:1;I:4;X:1;I:4;I:3;X:1;I:5", &(disGetval[i]), sizeof(getval_t), 0, 0);
  } // for i
} // disAddServices

                        
using namespace saftlib;
using namespace std;

// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> [OPTIONS] <server name>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -e<index>            specify extraction ring (0:SIS18[default], 1: ESR)" << std::endl;
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool provides a server for raw b2b data." << std::endl;
  std::cerr << "Example1: '" << program << " tr1 -e0'" << std::endl;
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
          case 0 : reqExtRing = SIS18_RING; break;
          case 1 : reqExtRing = ESR_RING;   break;
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
      nCondition = 7;
      sprintf(ringName, "esr");
      break;
    default :
        std::cerr << "Ring '"<< reqExtRing << "' does not exist" << std::endl;
        return -1;;
  } // switch extRing
  

  
  if (optind+1 < argc) sprintf(prefix, "b2b_%s_%s", ringName, argv[++optind]);
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
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];
    uint32_t tag[nCondition];

    // define conditions (ECA filter rules)
    switch (reqExtRing) {
      case SIS18_RING : 

        // SIS18, EVT_KICK_START, EKSOFFSET, signals start of data collection
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_KICKSTART << 36);
        condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, EKSOFFSET));
        tag[0]        = tagStart;
        
        // SIS18, EVT_KICK_START, +100ms (!), signals stop of data collection 
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_KICKSTART << 36);
        condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 100000000));
        tag[1]        = tagStop;
        
        // SIS18 to extraction, PMEXT, 
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[2]        = tagPme;

        // SIS18 to extraction, PREXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[3]        = tagPre;

        // SIS18 to extraction, DIAGEXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[4]        = tagPde;

        // SIS18 to ESR, PMEXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[5]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[5]        = tagPme;

        // SIS18 to ESR, PMINJ
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PMINJ << 36);
        condition[6]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[6]        = tagPmi;

        // SIS18 to ESR, PREXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[7]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[7]        = tagPre;

        // SIS18 to ESR, PRINJ
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_PRINJ << 36);
        condition[8]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[8]        = tagPri;
   
        // SIS18 to ESR, DIAGEXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[9]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[9]        = tagPde;

        // SIS18 to ESR, DIAGINJ
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_B2B_ESR << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGINJ << 36);
        condition[10] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[10]       = tagPdi;
        
        // SIS18 extraction kicker trigger
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[11] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[11]       = tagKte;
        
        // SIS18 extraction kicker diagnostic
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)SIS18_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[12] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[12]       = tagKde;
        
        // ESR injection kicker trigger
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGERINJ << 36);
        condition[13] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[13]       = tagKti;
        
        // ESR injection kicker diagnostic
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKINJ << 36);
        condition[14] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[14]       = tagKdi;
        
        break;
      case ESR_RING : 

        // ESR, EVT_KICK_START, EKSOFFSET, signals start of data collection
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_KICKSTART2 << 36);
        condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, EKSOFFSET));
        tag[0]        = tagStart;
        
        // ESR, EVT_KICK_START, +100ms (!), signals stop of data collection 
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_KICKSTART2 << 36);
        condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 100000000));
        tag[1]        = tagStop;
        
        // ESR to extraction, PMEXT, 
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PMEXT << 36);
        condition[2]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[2]        = tagPme;

        // ESR to extraction, PREXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_PREXT << 36);
        condition[3]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[3]        = tagPre;

        // ESR to extraction, DIAGEXT
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_B2B_EXTRACT << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGEXT << 36);
        condition[4]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[4]        = tagPde;
       
        // ESR extraction kicker trigger
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_TRIGGEREXT << 36);
        condition[5]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[5]        = tagKte;
        
        // ESR extraction kicker diagnostic
        snoopID       = ((uint64_t)FID << 60) | ((uint64_t)ESR_RING << 48) | ((uint64_t)B2B_ECADO_B2B_DIAGKICKEXT << 36);
        condition[6]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[6]        = tagKde;

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
      condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i
    
    while(true) {
      saftlib::wait_for_signal();
    } // while true
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


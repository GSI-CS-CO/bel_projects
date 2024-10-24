/*******************************************************************************************
 *  unichop-serv-mon.cpp
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Oct-2024
 *
 * monitors uni-chop firmware
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
#define UNICHOP_SERV_MON_VERSION 0x000007

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

// uni-chop includes
#include <common-lib.h>                 // COMMON
#include <unichoplib.h>                 // API
#include <uni-chop.h>                   // FW

using namespace saftlib;
using namespace std;

#define FID                0x1          // format ID of timing messages
#define UPDATE_TIME_MS    1000          // time for status updates [ms]

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
monData_t disMonData[UNICHOP_NSID*2];   // max 16 virtaccs; 0..15: HLI; 16..31: HSI; contains chopper data;

uint32_t  disVersionId      = 0;
uint32_t  disStateId        = 0;
uint32_t  disStatusId       = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disMonDataId[UNICHOP_NSID*2];
uint32_t  disCmdClearId     = 0;

uint32_t  one_ms_ns = 1000;

void clearStats(int index)
{
  disMonData[index].cyclesN         = 0;
  disMonData[index].triggerLen      = 0;
  disMonData[index].triggerN        = 0;
  disMonData[index].triggerErrN     = 0;
  disMonData[index].triggerFlag     = 0;
  disMonData[index].triggerErr      = 0;
  disMonData[index].pulseStartT     = 0;
  disMonData[index].pulseStartN     = 0;
  disMonData[index].pulseStartErrN  = 0;
  disMonData[index].pulseStartFlag  = 0;
  disMonData[index].pulseStartErr   = 0;
  disMonData[index].pulseStopT      = 0;
  disMonData[index].pulseStopN      = 0;
  disMonData[index].pulseStopErrN   = 0;
  disMonData[index].pulseStopFlag   = 0;
  disMonData[index].pulseStopErr    = 0;
  disMonData[index].pulseLen        = 0;
} // clearStats


// update value
void disUpdateData(int index, uint64_t tChop, monData_t data)
{
  uint32_t secs;
  uint32_t msecs;
  
  unichop_t2secs(tChop, &secs, &msecs);
  msecs  /= 1000000;
  
  disMonData[index] = data;
  dis_set_timestamp(disMonDataId[index], secs, msecs);
  dis_update_service(disMonDataId[index]);
} // disUpdateData


// handle received timing message
static void timingMessage(uint64_t evtId, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  uint64_t            tChopUtc;        // time of chopper pulse
  monData_t           monData;
  
  uint32_t            mFid;            // FID 
  //uint32_t            mGid;            // GID
  uint32_t            mSid;            // SID
  //uint32_t            mEvtNo;          // event number
  int                 index;           // 0..15: HLI; 16..31: HSI
  uint32_t            triggerLen;
  uint32_t            pulseStart;
  uint32_t            pulseStop;

  mFid        = ((evtId  & 0xf000000000000000) >> 60);
  //mGid        = ((evtId  & 0x0fff000000000000) >> 48);
  mSid        = ((evtId  & 0x00000000fff00000) >> 20);
  //mEvtNo      = ((evtId  & 0x0000fff000000000) >> 36);

  // check ranges
  if (mFid != FID)                        return;  // unexpected format of timing message
  if (tag   > tagHSI)                     return;  // illegal tag
  if (mSid  > UNICHOP_NSID)               return;  // out of range

  index = -1; 
  switch (tag) {
    case tagHLI   :
      index = 0;
    case tagHSI   :                                // this is an OR, no break on purpose;
      if (index != 0) index = UNICHOP_NSID;

      index                     += mSid;
      triggerLen                 = ((param & 0xffff000000000000) >> 48);
      pulseStart                 = ((param & 0x0000ffff00000000) >> 32);
      pulseStop                  = ((param & 0x00000000ffff0000) >> 16);

      disMonData[index].cyclesN++;
      tChopUtc                   = deadline.getTAI();

      switch (triggerLen) {
        case 0x7fff :
          monData.triggerLen     = 0x7fffffff;
          monData.triggerFlag    = 0;
          monData.triggerErr     = 1;
          monData.triggerErrN++;
          break;
        case 0xffff :
          monData.triggerLen     = 0;
          monData.triggerFlag    = 0;
          monData.triggerErr     = 0;
          break;
        default :
          monData.triggerLen     = triggerLen;
          monData.triggerFlag    = 1;
          monData.triggerErr     = 0;        
          monData.triggerN++;
      } // switch triggerLen

      switch (pulseStart) {
        case 0x7fff :
          monData.pulseStartT    = 0x7fffffff;
          monData.pulseStartFlag = 0;
          monData.pulseStartErr  = 1;
          monData.pulseStartErrN++;
          break;
        case 0xffff :
          monData.pulseStartT    = 0;
          monData.pulseStartFlag = 0;
          monData.pulseStartErr  = 0;
          break;
        default :
          monData.pulseStartT    = pulseStart;
          monData.pulseStartFlag = 1;
          monData.pulseStartErr  = 0;        
          monData.pulseStartN++;
      } // switch pulseStart

      switch (pulseStop) {
        case 0x7fff :
          monData.pulseStopT     = 0x7fffffff;
          monData.pulseStopFlag  = 0;
          monData.pulseStopErr   = 1;
          monData.pulseStopErrN++;
          break;
        case 0xffff :
          monData.pulseStopT     = 0;
          monData.pulseStopFlag  = 0;
          monData.pulseStopErr   = 0;
          break;
        default :
          monData.pulseStopT     = pulseStop;
          monData.pulseStopFlag  = 1;
          monData.pulseStopErr   = 0;        
          monData.pulseStopN++;
      } // switch pulseStop

      if ((monData.pulseStopErr == 0) && (monData.pulseStartErr == 0)) monData.pulseLen = monData.pulseStopT - monData.pulseStartT;

      disUpdateData(index, tChopUtc, monData);
      break;
    default         :
      ;
  } // switch tag
} // timingmessage


// callback for command
void dis_cmd_clear(void *tag, void *address, int *size)
{
  int32_t *index;
  
  if (*size != sizeof(int32_t)) return;
  index = (int *)(address);

  clearStats(*index);
} // dis_cmd_clear


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  // 'generic' services
  sprintf(name, "%s_version_fw", prefix);
  sprintf(disVersion, "%s",  unichop_version_text(UNICHOP_SERV_MON_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_state", prefix);
  sprintf(disState, "%s", unichop_state_text(COMMON_STATE_OPREADY));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  sprintf(name, "%s_status", prefix);
  disStatus       = 0x1;   
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  // monitoring data service
  // HLI
  for (i=0; i < UNICHOP_NSID; i++) {
    sprintf(name, "%s_hli-data_sid%02d", prefix, i);
    disMonDataId[i]  = dis_add_service(name, "I:17", &(disMonData[i]), sizeof(monData_t), 0, 0);
  } // for i

  // HSI
  for (i=UNICHOP_NSID; i < 2*UNICHOP_NSID; i++) {
    sprintf(name, "%s_hsi-data_sid%02d", prefix, i-UNICHOP_NSID);
    disMonDataId[i]  = dis_add_service(name, "I:17", &(disMonData[i]), sizeof(monData_t), 0, 0);
  } // for i

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
  std::cerr << std::endl;
  std::cerr << "This tool monitors the HLI and HSI choppers"                                        << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " tr0 -d pro'"                                           << std::endl;
  std::cerr << std::endl;
  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << unichop_version_text(UNICHOP_SERV_MON_VERSION) << ". Licensed under the GPL v3." << std::endl;
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

  //char     tmp[752];
  int      i;

  // variables attach, remove
  char    *deviceName = NULL;
  char    *domainName = NULL;

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
  while ((opt = getopt(argc, argv, "hefd")) != -1) {
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
  } // if optind

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  deviceName = argv[optind];
  domainName = argv[++optind];
  gethostname(disHostname, 32);
  
  sprintf(prefix, "unichop_%s-mon", domainName);

  if (startServer) {
    printf("%s: starting server using prefix %s\n", program, prefix);

    for (i=0; i<UNICHOP_NSID*2; i++) clearStats(i);
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
    if ((status =  unichop_firmware_open(&ebDevice, ebPath, 0, &cpu)) != COMMON_STATUS_OK) {
      std::cerr << program << ": can't open connection to lm32 firmware" << std::endl;
      exit(1);
    } // if status
    
    if (getVersion) {
      unichop_version_library(&verLib);
      printf("unichop: serv-sys / library / firmware /  version %s / %s",  unichop_version_text(verLib), unichop_version_text(UNICHOP_SERV_MON_VERSION));     
      unichop_version_firmware(ebDevice, &verFw);
      printf(" / %s\n",  unichop_version_text(verFw));     
    } // if getVersion

    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

    uint32_t tag[nCondition];
    uint32_t tmpTag;

    // message containing info on HLI chopper
    tmpTag        = tagHLI;
    snoopID       = ((uint64_t)FID << 60 | (uint64_t)GID_LOCAL_ECPU_FROM << 48 | (uint64_t)UNICHOP_ECADO_HLISTOP << 36);
    condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[0]        = tmpTag;

    // message containing info on HSI chopper
    tmpTag        = tagHSI;
    snoopID       = ((uint64_t)FID << 60 | (uint64_t)GID_LOCAL_ECPU_FROM << 48 | (uint64_t)UNICHOP_ECADO_HSISTOP << 36);
    condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
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


    /*
    saftlib::Time deadline_t;
    int32_t       stmp32a;
    uint64_t      tmp64a;
    */

    uint64_t      t_new, t_old;
    uint32_t      fwState, fwVersion;
    uint64_t      fwStatus;
    uint32_t      tmp32a, tmp32b, tmp32c;



    t_old = comlib_getSysTime();
    while(true) {
      saftlib::wait_for_signal(UPDATE_TIME_MS / 10);

      t_new = comlib_getSysTime();
      if (((t_new - t_old) / one_ms_ns) > UPDATE_TIME_MS) {
        t_old      = t_new;

        // update firmware data
        unichop_common_read(ebDevice, &fwStatus, &fwState, &tmp32a, &tmp32b, &fwVersion, &tmp32c, 0);

        disStatus  = fwStatus;
        sprintf(disState  , "%s", unichop_state_text(fwState));
        sprintf(disVersion, "%s", unichop_version_text(fwVersion));
               
        if (startServer) {
          // update service data; monitoring data already updated inside callback function
          dis_update_service(disStatusId);
          dis_update_service(disStateId);
          dis_update_service(disVersionId);
        } // if startServer      
      } // if update

      /*
      // clear data
      if (flagClear) {
        clearStats();                             // clear server
        unichop_cmd_cleardiag(ebDevice);          // clear fw diags
        
        flagClear = 0;
      } // if flagclear
      */
    } // while true

    unichop_firmware_close(ebDevice);    
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }
  
  return 0;
} // main


/*******************************************************************************************
 *  sync-serv-mon.cpp
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 13-Jun-2025
 *
 * monitors event activity when checking synchronization between machines
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
#define SYNC_SERV_MON_VERSION 0x000002

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

// sync-mon includes
#include <common-lib.h>                 // COMMON
#include <syncmonlib.h>                 // API

using namespace saftlib;
using namespace std;

#define UPDATE_TIME_MS     1000         // time for status updates [ms]
#define FID                0x1          // format ID of timing messages

static const char* program;

// dim
#define DIMCHARSIZE        32           // standard size for char services
#define DIMMAXSIZE         1024         // max size for service names
#define DIMMAXMON          32           // max number of monitored message (rules)
#define NAMELEN            256          // max size for names

// services
char      disVersion[DIMCHARSIZE];      // software version
char      disHostname[DIMCHARSIZE];     // hostname
monval_t  disMonData[DIMMAXMON];        // monitoring data

uint32_t  disVersionId      = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disMonDataId[DIMMAXMON];
uint32_t  disCmdClearId     = 0;


// local variables
int       nMonData;                     // number of monitoring data
int       flagClear;                    // flag for clearing diag data;

uint64_t  one_us_ns = 1000;
uint64_t  one_ms_ns = 1000000;


// clear statistics
void clearStats()
{
  int i;

  for (i=0;i<DIMMAXMON; i++) disMonData[i] = smEmptyMonData();
} // clearStats


// handle received timing message
static void timingMessage(uint64_t evtId, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  uint32_t            mFid;            // FID 
  uint32_t            mGid;            // GID
  uint32_t            mEvtNo;          // event number
  uint32_t            mFlags;          // flags
  uint32_t            mSid;            // SID
  uint32_t            mBpid;           // BPID
  uint32_t            mEia;            // evt Id attributes

  mFid        = ((evtId  & 0xf000000000000000) >> 60);
  mGid        = ((evtId  & 0x0fff000000000000) >> 48);
  mEvtNo      = ((evtId  & 0x0000fff000000000) >> 36);
  mFlags      = ((evtId  & 0x0000000f00000000) >> 32);
  mSid        = ((evtId  & 0x00000000fff00000) >> 20);
  mBpid       = ((evtId  & 0x00000000000fffc0) >>  6);
  mEia        = ((evtId  & 0x000000000000003f)      );

  // check ranges
  if (mFid != FID)                        return;  // unexpected format of timing message
  if (tag   > (uint32_t)nMonData)         return;  // illegal tag

  disMonData[tag].fid      = mFid;
  disMonData[tag].gid      = mGid;
  disMonData[tag].evtNo    = mEvtNo;
  disMonData[tag].flags    = mFlags;
  disMonData[tag].sid      = mSid;
  disMonData[tag].bpid     = mBpid;
  disMonData[tag].eia      = mEia;
  disMonData[tag].param    = param;
  disMonData[tag].deadline = deadline.getTAI();
  disMonData[tag].dummy    = 0x0;
  disMonData[tag].counter++;

  dis_update_service(disMonDataId[tag]);

} // timingmessage


// callback for command
void dis_cmd_clear(void *tag, void *buffer, int *size)
{
  flagClear = 1;
} // dis_cmd_clear


// add all dim services
void disAddServices(char *prefix, char *domainName)
{
  char name[DIMMAXSIZE];
  int  i;

  // clear service IDs for monitoring data
  for (i=0; i<DIMMAXMON; i++) disMonDataId[i] = 0;

  // 'generic' services
  sprintf(name, "%s_version_fw", prefix);
  sprintf(disVersion, "%06x",  SYNC_SERV_MON_VERSION);
  disVersionId      = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_hostname", prefix);
  disHostnameId     = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  // monitoring data service
  for (i=0;i<nMonData;i++){
    sprintf(name, "%s_data%02d", prefix, i);
    sprintf(disMonData[i].domainName, "%s", domainName);
    disMonDataId[i] = dis_add_service(name, "I:7;X:2;I:2;C", &(disMonData[i]), sizeof(monval_t), 0, 0);
  }

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
  std::cerr << "  The paremter -s is mandatory"                                                     << std::endl;
  std::cerr << "  -s <what>            specifies what will be monitored; this can be"               << std::endl;
  std::cerr << "                       0: UNILAC Transfer Channel (reference group)"                << std::endl;
  std::cerr << "                       1: SIS18 injection (injection and main threads)"             << std::endl;     
  std::cerr << std::endl;
  std::cerr << "This tool monitors the transfer between machines at GSI and FAIR."                  << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << "tr0 -s0 pro'"                                           << std::endl;
  std::cerr << std::endl;
  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version %x06" << SYNC_SERV_MON_VERSION << ". Licensed under the GPL v3." << std::endl;
} // help


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int      opt;
  bool     useFirstDev    = false;
  bool     getVersion     = false;
  bool     startServer    = false;
  uint32_t gid=0xffffffff;               // gid for server
  char    *tail;

  // variables snoop event
  uint64_t snoopID     = 0x0;

  int      tmpi;
  int      i;

  // variables attach, remove
  char    *deviceName = NULL;

  char     domainName[NAMELEN];          // name of timing domain (UNILAC, SIS18, ...)
  char     prefix[NAMELEN*3];            // prefix DIM services
  char     disName[DIMMAXSIZE];          // name of DIM server
  char     environment[NAMELEN];         // environment, typically either int or pro
  
  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "s:dhef")) != -1) {
    switch (opt) {
      case 'e' :
        getVersion  = true;
        break;
      case 'f' :
        useFirstDev = true;
        break;
      case 'd':
        startServer = true;
        break;
      case 's' :
        tmpi        = strtoull(optarg, &tail, 0);
        if (*tail != 0) {std::cerr << "Specify a proper number, not " << optarg << "'%s'!" << std::endl; return 1;}
        switch (tmpi) {
          case 0: gid = GIDUNILACEXT;  sprintf(domainName, "%s", "unilac")   ;  nMonData = 2; break;
          case 1: gid = GIDSIS18INJ;   sprintf(domainName, "%s", "sis18-inj");  nMonData = 2; break;
          default: {std::cerr << "Specify a proper number, not " << tmpi << "'%s'!" << std::endl; return 1;} break;
        } // switch tmpi
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
    sprintf(prefix, "syncserv_%s_%s-mon", environment, domainName);
  } // if optind
  else
    sprintf(prefix, "syncserv_%s-mon", domainName);

  if (startServer) {
    printf("%s: starting server using prefix %s\n", program, prefix);

    clearStats();
    disAddServices(prefix, domainName);

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
    
    if (getVersion) printf("sync serv mon: version %06x\n", SYNC_SERV_MON_VERSION);
  
    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition[nMonData];

    uint32_t tag[nMonData];
    uint32_t tmpTag;

    // select timing message to monitor
    switch (gid) {
      case GIDUNILACEXT:
        // transfer from UNILAC
        if (nMonData != 2) {std::cerr << "wrong array size" << std::endl; return 1;}
        tmpTag             = 0;
        snoopID            = 0x0;
        snoopID           |= ((uint64_t)FID << 60);
        snoopID           |= ((uint64_t)gid << 48);
        snoopID           |= ((uint64_t)EVT_BEAM_ON << 36);
        condition[tmpTag]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[tmpTag]        = tmpTag;

        // hack to support legacy groups (for testing til July 2025
        tmpTag             = 1;
        snoopID            = 0x0;
        snoopID           |= ((uint64_t)FID << 60);
        snoopID           |= ((uint64_t)0x1c6 << 48);                    // legacy GID TK
        snoopID           |= ((uint64_t)EVT_BEAM_ON << 36);
        condition[tmpTag]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[tmpTag]        = 0;                                          // hack!

        break;
      case GIDSIS18INJ:
        // injection into SIS18
        if (nMonData != 2) {std::cerr << "wrong array size" << std::endl; return 1;}
        tmpTag             = 0;
        snoopID            = 0x0;
        snoopID           |= ((uint64_t)FID << 60);
        snoopID           |= ((uint64_t)gid << 48);
        snoopID           |= ((uint64_t)EVT_RAMP_START << 36);
        condition[tmpTag]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[tmpTag]        = tmpTag;

        tmpTag             = 1;
        snoopID            = 0x0;
        snoopID           |= ((uint64_t)FID << 60);
        snoopID           |= ((uint64_t)gid << 48);
        snoopID           |= ((uint64_t)EVT_MB_TRIGGER<< 36);
        condition[tmpTag]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
        tag[tmpTag]        = tmpTag;

        break;
    } // switch gid
    
    // let's go!
    for (i=0; i<nMonData; i++) {
      condition[i]->setAcceptLate(true);
      condition[i]->setAcceptEarly(true);
      condition[i]->setAcceptConflict(true);
      condition[i]->setAcceptDelayed(true);
      condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&timingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i

    while(true) {
      if (saftlib::wait_for_signal(UPDATE_TIME_MS / 10) == 0) {
        // timeout
        // evtl irgendwelchen Krempel auf den Bildschirm schreiben
      }     
    } // while true
    
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  } // catch
  
  return 0;
} // main


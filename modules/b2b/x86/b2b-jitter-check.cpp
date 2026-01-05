/*******************************************************************************************
 *  b2b-jitter-check.cpp
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 28-feb-2025
 *
 * checks jitter between two timing receivers connected via a Lemo cable 
 * the first timing receiver outputs a PPS pulse
 * the second timing receiver timestamps the pulse and does simple statistics
 * the data is published to the network
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
#define B2B_JITTER_CHECK_VERSION 0x000810

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
#include "Output.h"
#include "Input.h"
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

// services
char        disVersion[DIMCHARSIZE];
char        disHostname[DIMCHARSIZE];
char        disState[DIMCHARSIZE];
jitterChk_t disJitterChk;

uint32_t    disVersionId      = 0;
uint32_t    disHostnameId     = 0;
uint32_t    disStateId        = 0;
uint32_t    disJitterChkId    = 0;

// saftlib
std::shared_ptr<TimingReceiver_Proxy> ppsReceiver;   // TR for PPS and TS
std::shared_ptr<Output_Proxy>         ppsOutProxy;
std::shared_ptr<Input_Proxy>          ppsInProxy;
bool                                  ppsOutFound;
bool                                  ppsInFound;
char                                  ppsName[128];

std::shared_ptr<TimingReceiver_Proxy> tsReceiver;    // TR for TS
std::shared_ptr<Output_Proxy>         tsOutProxy;
std::shared_ptr<Input_Proxy>          tsInProxy;
bool                                  tsOutFound;
bool                                  tsInFound;
char                                  tsName[128];

// statistics
double   tMin;
double   tMax;
uint32_t tN;
double   tAveOld;
double   tStreamOld;


// clear statistics
void clearStats()
{
  tMin       = 999999999;
  tMax       = 0;
  tN         = 0;
  tAveOld    = 0;   
  tStreamOld = 0;
} // clearStats


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


// handle received timing message
static void timingMessage(uint32_t tag, saftlib::Time deadline, uint64_t evtId, uint64_t param, uint32_t tef, uint32_t isLate, uint32_t isEarly, uint32_t isConflict, uint32_t isDelayed)
{
  double    tAct;                   // fractional time (sub second part) of the deadline
  double    tAve;
  double    tmpStream        = 0;
  double    tSdev            = 0;
  double    dummy;

  if (tsReceiver->getLocked() && ppsReceiver->getLocked()) sprintf(disState, "%s", b2b_state_text(COMMON_STATE_OPREADY));
  else                                                     sprintf(disState, "%s", b2b_state_text(COMMON_STATE_ERROR));

  if (tag == 17) {
    tAct = deadline.getTAI() % 1000000000;
      
    // statistics
    tN++;
    calcStats(&tAve, tAveOld, &tmpStream, tStreamOld, tAct, tN , &dummy, &tSdev);
    tAveOld          = tAve;
    tStreamOld       = tmpStream;
    if (tAct < tMin) tMin = tAct;
    if (tAct > tMax) tMax = tAct;
    
    //printf("# %6d, act %f, ave %f, sdev %f, min %f, max %f\n", tN, tAct, tAve, tSdev, tMin, tMax);
    
    disJitterChk.ppsAct  = tAct;
    disJitterChk.ppsN    = tN;
    disJitterChk.ppsMean = tAve;
    disJitterChk.ppsSdev = tSdev;
    disJitterChk.ppsMin  = tMin;
    disJitterChk.ppsMax  = tMax;
  } // if locked
  else {
    disJitterChk.ppsAct  = NAN;
  } // else locked

  dis_update_service(disJitterChkId);
  dis_update_service(disStateId);
} // timingMessage


// this will be called in case no message was received
static void recTimeout()
{
  saftlib::Time time = saftlib::makeTimeTAI(12345); 
  
  timingMessage(42, time, 0x0, 0x0, 0x0, 0, 0, 0, 0);
} // recTimeout()


// this will be called when receiving ECA actions from software action queue
static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  int flagLate;

  flagLate    = flags & 0x1;

  timingMessage(tag, deadline, id, param, 0x0, flagLate, 0, 0, 0);
} // recTimingMessag


// call back for command
class RecvCommand : public DimCommand
{
  int  reset;
  void commandHandler() {
    clearStats();
  } // commandHandler
public :
  RecvCommand(const char *name) : DimCommand(name,"C"){}
}; 


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];

  // 'generic' services
  sprintf(name, "%s-jitter-check_version_fw", prefix);
  sprintf(disVersion, "%s",  b2b_version_text(B2B_JITTER_CHECK_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s-jitter-check_state", prefix);
  sprintf(disState, "%s", b2b_state_text(COMMON_STATE_ERROR));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s-jitter-check_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  // jitter data
  sprintf(name, "%s-jitter-check_data", prefix);
  disJitterChkId  = dis_add_service(name, "D:1;I:1;D:4", &(disJitterChk), sizeof(jitterChk_t), 0, 0);
} // disAddServices

                        
//using namespace saftlib;
//using namespace std;

// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name PPS> <device name TS> [OPTIONS] <server name prefix>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -p <n>               specify number of IO where PPS is generated (default '1')" << std::endl;
  std::cerr << "  -t <n>               specify number of IO where timestamp is acquired (default '1')" << std::endl;
  
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool uses two timing receivers where IOs are connected via a cable. A PPS pulse is generated on the first" << std::endl;
  std::cerr << "TR, while the a timestamp of the PPS is measured at the second TR. The result is published via DIM." << std::endl;
  std::cerr << "In case the first (PPS) TR is not used (but something else), the '-p' option can be omitted; in this case" << std::endl;
  std::cerr << "<device name PPS> shall be identical to <device name TS>" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Example1: '" << program << " tr0 tr1 -p2 -t2'" << std::endl;
  std::cerr << std::endl;

  std::cerr << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cerr << "Version " << b2b_version_text(B2B_JITTER_CHECK_VERSION) << ". Licensed under the GPL v3." << std::endl;
} // help


void ppsOutConfig()
{
  std::map< std::string, std::string > ins;  
  std::map< std::string, std::string > outs;
  bool                                 i_found  = false;
  bool                                 o_found  = false;

  outs = ppsReceiver->getOutputs();
  ins  = ppsReceiver->getInputs();

  for (std::map<std::string,std::string>::iterator it=outs.begin(); it!=outs.end(); ++it) {
    if (it->first == ppsName) {
      i_found = true;
      ppsOutProxy = Output_Proxy::create(it->second);
    } // if it
  } // for it

  for (std::map<std::string,std::string>::iterator ito=ins.begin(); ito!=ins.end(); ++ito) {
    if (ito->first == ppsName) {
      o_found = true;
      ppsInProxy = Input_Proxy::create(ito->second); 
    } // if it
  } // for it


  // input not found
  if (i_found == false) {
    std::cerr << "no input with the name " << ppsName << std::endl;
    exit(1);
  }

  // output not found
  if (o_found == false) {
    std::cerr << "no output with the name " << ppsName << std::endl;
    exit(1);
  }
  
  // reset input properties
  ppsInProxy->setEventEnable(false);
  ppsInProxy->setInputTermination(false);
  ppsInProxy->setGateIn(false);
  
  // set output properties
  ppsOutProxy->setOutputEnable(true);
  ppsOutProxy->setGateOut(true);
  ppsOutProxy->WriteOutput(false);
  ppsOutProxy->setPPSMultiplexer(true); 
} //ppsOutConfig


void tsInConfig()
{
  std::map< std::string, std::string > outs; 
  std::map< std::string, std::string > ins;
  bool                                 i_found  = false;
  bool                                 o_found  = false;

  ins  = tsReceiver->getInputs();
  outs = tsReceiver->getOutputs();
  

  for (std::map<std::string,std::string>::iterator it=ins.begin(); it!=ins.end(); ++it) {
    if (it->first == tsName) {
      i_found = true;
      tsInProxy  = Input_Proxy::create(it->second);
    } // if it
  } // for it

  for (std::map<std::string,std::string>::iterator ito=outs.begin(); ito!=outs.end(); ++ito) {
    if (ito->first == tsName) {
      o_found = true;
      tsOutProxy = Output_Proxy::create(ito->second); 
    } // if it
  } // for it

  // input not found
  if (i_found == false) {
    std::cerr << "no input with the name " << tsName << std::endl;
    exit(1);
  }

  // output not found
  if (o_found == false) {
    std::cerr << "no output with the name " << tsName << std::endl;
    exit(1);
  }

  // reset output properties
  tsOutProxy->setOutputEnable(false);
  tsOutProxy->setGateOut(false);
  tsOutProxy->setPPSMultiplexer(false);

  // set input properties
  tsInProxy->setInputTermination(true);
  tsInProxy->setGateIn(true);
} //tsInConfig


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  char *tail;


  // variables snoop event
  uint64_t snoopID     = 0x0;

  // variables TR
  char    *deviceNamePPS = NULL;
  char    *deviceNameTS  = NULL;
  int     ioPPS          = 1;
  int     ioTS           = 1;
  
  char    tmp[512];
  char    prefix[NAMELEN];
  char    disName[DIMMAXSIZE];

  uint64_t evtPrefix = 0xffffa00000000000;
  uint64_t ioPrefix  = 0x0;

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "p:t:h")) != -1) {
    switch (opt) {
      case 'p' :
        ioPPS = strtol(optarg, &tail, 0);
        if (ioPPS < 0) {
          std::cerr << "option -p: parameter out of range" << std::endl;
          return 1;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
        break;
      case 't' :
        ioTS = strtol(optarg, &tail, 0);
        if (ioTS < 0) {
          std::cerr << "option -t: parameter out of range" << std::endl;
          return 1;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          return 1;
        } // if *tail
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
    std::cerr << program << " expecting two non-optional arguments: <device name PPS> <device name TS>" << std::endl;
    help();
    return 1;
  }

  // no parameters, no command: just display help and exit
  if ((optind <= 2) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  //printf("optind %d\n", optind);

  deviceNamePPS = argv[optind];
  deviceNameTS  = argv[optind+1];
  if (optind+1 < argc) sprintf(prefix, "b2b_%s", argv[optind+2]);
  else                 sprintf(prefix, "b2b_blabla");

  gethostname(disHostname, 32);
  sprintf(ppsName, "IO%d", ioPPS);
  sprintf(tsName,  "IO%d", ioTS);

  printf("PPS %s, TS %s\n", deviceNamePPS, deviceNameTS);


  printf("%s: starting server using prefix %s\n", program, prefix);

  disAddServices(prefix);
  // uuuuhhhh, mixing c++ and c  
  sprintf(tmp, "%s-jittercheck_cmd_cleardiag", prefix);
  RecvCommand cmdClearDiag(tmp);
  
  sprintf(disName, "%s-jittercheck", prefix);
  dis_start_serving(disName);

  // clear statistics
  clearStats();
  
  try {
    // basic saftd stuff
    std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

    // connect to timing receiver
    map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();

    // PPS
    if (devices.find(deviceNamePPS) == devices.end()) {
      std::cerr << "Device '" << deviceNamePPS << "' does not exist" << std::endl;
      return -1;
    } // find device
    ppsReceiver = TimingReceiver_Proxy::create(devices[deviceNamePPS]);
    ppsOutConfig();
    
    // TS
    if (devices.find(deviceNameTS) == devices.end()) {
      std::cerr << "Device '" << deviceNameTS << "' does not exist" << std::endl;
      return -1;
    } // find device
    tsReceiver = TimingReceiver_Proxy::create(devices[deviceNameTS]);
    tsInConfig();

    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(tsReceiver->NewSoftwareActionSink(""));
    std::shared_ptr<SoftwareCondition_Proxy> condition;

    // configure condition; convention: prefix contains io number
    ioPrefix   = (uint64_t)ioTS << 36;
    evtPrefix |= ioPrefix;            
    snoopID    = evtPrefix + 0x1; // last bit is set: rising edge
    //printf("id %llx\n", snoopID);
    condition  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffffffffffffffff, 0));
    condition->setAcceptLate(true);
    condition->setAcceptEarly(true);
    condition->setAcceptConflict(true);
    condition->setAcceptDelayed(true);
    condition->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), 17));

    // configure input to create an event (with timestamp) when LVTTL input is received
    tsInProxy->setEventEnable(false);
    tsInProxy->setEventPrefix(evtPrefix);
    tsInProxy->setEventEnable(true);

    // let's go
    condition->setActive(true);    

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


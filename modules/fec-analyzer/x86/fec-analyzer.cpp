/*******************************************************************************************
 *  fec-analyzer.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 11-February-2021
 *
 * uses a typical ECA configuration, receives messages and analyzes run-time data
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
#define FEC_ANALYZER_VERSION 0x000100

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

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
#include <termios.h>

// saftlib includes
#include "SAFTd.h"
#include "TimingReceiver.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

// etherbone
#include <etherbone.h>
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"


using namespace std;
using namespace saftlib;
//using namespace std;


// EVTNO
#define BP_START     0x100              // event numbers used 
#define SEQ_START    0x101
#define GAP_START    0x102
#define GAP_END      0x103
#define FG_PREP      0x200
#define FG_START     0x201

#define FID          0x1                // format ID of timing messages

#define MAXCOND      42

enum evtTag{tagBps, tagSqs, tagGps, tagGpe, tagFgp, tagFgs};
typedef enum evtTag evtTag_t;

const char * evtName[] = {"BP_START", "SEQ_START", "GAP_START", "GAP_END", "FG_PREP", "FG_START"};

static const char* program;

//std::shared_ptr<TimingReceiver_Proxy> receiverCB;

// global variables
uint32_t gid;                           // Group ID
uint32_t sid;                           // Sequence ID
uint32_t nCond;                         // # of conditions
uint64_t aveOffset;                     // average system time offset;



uint32_t nMsg[MAXCOND];                 // # of received messages
uint32_t nLate[MAXCOND];                // # of late messages
uint32_t nEarly[MAXCOND];               // # of early messages
uint32_t nConflict[MAXCOND];            // # of conflict messages
uint32_t nDelay[MAXCOND];               // # of delayed messages
uint32_t dMin[MAXCOND];                 // mininum delay
uint32_t dMax[MAXCOND];                 // maximum delay
double   dAve[MAXCOND];                 // average delay
double   dSdev[MAXCOND];                // standard deviation of delay
double   dStreamOld[MAXCOND];           // 'old' stream value of delay
double   dAveOld[MAXCOND];              // 'old' average value of delay
uint32_t sdMin[MAXCOND];                // mininum delivery time
uint32_t sdMax[MAXCOND];                // maximum delivery time
double   sdAve[MAXCOND];                // average delivery time
double   sdSdev[MAXCOND];               // standard deviation of delivery time
double   sdStreamOld[MAXCOND];          // 'old' stream value of delivery time
double   sdAveOld[MAXCOND];             // 'old' average value of delay

// get host system time [ns]
uint64_t getSysTime()
{
  uint64_t tNs;
  
  struct timeval tv;
  gettimeofday(&tv,NULL);
  tNs = (tv.tv_sec*(uint64_t)1000000+tv.tv_usec) * 1000;

  return tNs;
} // small helper function


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


void clearStats(){
  int i;

  for (i=0; i<(int)nCond; i++) {
    nMsg[i]        = 0;       
    nLate[i]       = 0;      
    nEarly[i]      = 0;    
    nConflict[i]   = 0;  
    nDelay[i]      = 0;     
    dMin[i]        = 0x7fffffff;       
    dMax[i]        = 0;       
    dAve[i]        = 0;       
    dSdev[i]       = 0;      
    dStreamOld[i]  = 0;
    dAveOld[i]     = 0;
    sdMin[i]       = 0x7fffffff;      
    sdMax[i]       = 0;      
    sdAve[i]       = 0;      
    sdSdev[i]      = 0;     
    sdStreamOld[i] = 0;    
    sdAveOld[i]    = 0;    
  } // for i
} // clearStats


// this will be called when receiving ECA actions
static void recTimingMessage(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags, uint32_t tag)
{
  uint32_t            recGid;          // received GID
  uint32_t            recSid;          // received SID
  int                 flagLate;
  int                 flagEarly;
  int                 flagConflict;
  int                 flagDelayed;
  uint64_t            delay;
  uint64_t            tHost;

  double    sdev = 0;
  double    aveNew;
  double    streamNew = 0;
  double    dummy;

  tHost        = getSysTime();

  recGid       = ((id    & 0x0fff000000000000) >> 48);
  recSid       = ((id    & 0x00000000fff00000) >> 20);
  flagLate     = (flags     ) & 0x1;
  flagEarly    = (flags >> 1) & 0x1;
  flagConflict = (flags >> 2) & 0x1;
  flagDelayed  = (flags >> 3) & 0x1;

  // check ranges
  if (recSid  != sid)    return;
  if (recGid  != gid)    return;
  if (tag     >= nCond)  return;

  nMsg[tag]++;
  if (flagLate)  nLate[tag]++;      
  if (flagEarly) nEarly[tag]++;
  if (flagConflict) nConflict[tag]++;
  if (flagDelayed) {
    nDelay[tag]++;
    delay = executed.getTAI() - deadline.getTAI();
    
    // statistics
    calcStats(&aveNew, dAveOld[tag], &streamNew, dStreamOld[tag], delay, nDelay[tag] , &dummy, &sdev);
    dAveOld[tag]          = aveNew;
    dStreamOld[tag]       = streamNew;
    
    if (delay < dMin[tag]) dMin[tag] = delay;
    if (delay > dMax[tag]) dMax[tag] = delay;
    dAve[tag]  = aveNew;
    dSdev[tag] = sdev;
  } // if flagDelay

  // saftlib propagation time
  delay = tHost + aveOffset - executed.getTAI(); // printf("%f\n", (double)delay/1000.0);

  // statistics
  calcStats(&aveNew, sdAveOld[tag], &streamNew, sdStreamOld[tag], delay, nMsg[tag] , &dummy, &sdev);
  sdAveOld[tag]          = aveNew;
  sdStreamOld[tag]       = streamNew;
    
  if (delay < sdMin[tag]) sdMin[tag] = delay;
  if (delay > sdMax[tag]) sdMax[tag] = delay;
  sdAve[tag]  = aveNew;
  sdSdev[tag] = sdev;
} // recTimingMessage


void printStats()
{
  int i;
enum evtTag{tagBps, tagSqs, tagGps, tagGpe, tagFgp, tagFgs};
  for (i=0; i<nCond; i++) printf("%10s: n %4d, late %4d, early %4d, conflict %4d, delayed %4d\n", evtName[i], nMsg[i], nLate[i], nEarly[i], nConflict[i], nDelay[i]);
  for (i=0; i<nCond; i++) {
    if (nDelay[i] > 0) printf("%10s - delay    [us]: ave(sdev) %8.3f(%8.3f) min %8.3f max %8.3f\n", evtName[i], (double)dAve[i]/1000.0, (double)dSdev[i]/1000.0, (double)dMin[i]/1000.0, (double)dMax[i]/1000.0);
    else               printf("%10s - delay    [us]: N/A\n", evtName[i]);
  } // for i
  for (i=0; i<nCond; i++) printf("%10s - delivery [us]: ave(sdev) %8.3f(%8.3f) min %8.3f max %8.3f\n", evtName[i], (double)sdAve[i]/1000.0, (double)sdSdev[i]/1000.0, (double)sdMin[i]/1000.0, (double)sdMax[i]/1000.0);
  printf("\n");
} // printStats

                        
// display help
static void help(void) {
  std::cerr << std::endl << "Usage: " << program << " <device name> <gid> <sid> " << std::endl;
  std::cerr << std::endl;
  std::cerr << "  -h                   display this help and exit" << std::endl;
  std::cerr << "  -e                   display version" << std::endl;
  std::cerr << "  -o <value>           offset for FG_PREP" << std::endl;
  std::cerr << "  -g                   use negative value for offset" << std::endl;
  std::cerr << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cerr << std::endl;
  std::cerr << std::endl;
  std::cerr << "This tool analyzes message data on a FEC" << std::endl;
  std::cerr << "Example: '" << program << " 0x112 0x7 subscribes to group SIS18_RING and SID 7" << std::endl;
  std::cerr << std::endl;
  std::cerr << "During runtime, the following keys may be used:" << std::endl;
  std::cerr << " q       quit" << std::endl;
  std::cerr << " c       clear statistics" << std::endl;
  std::cerr << " d       include saftlib delivery time" << std::endl;
  std::cerr << " p       print statistics" << std::endl;
  std::cerr << std::endl;
  std::cerr << "Report bugs to ACO !!!" << std::endl;
  fprintf(stderr, "Version %x. Licensed under the LGPL v3.\n", FEC_ANALYZER_VERSION);
} // help

int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool useFirstDev    = false;
  bool negOffset      = false;
  int32_t offset      = 0;
  char *tail;

#define NREAD       20
  char              devName[32];
  eb_socket_t       socket;
  eb_status_t       status;
  eb_cycle_t        cycle;
  eb_data_t         data1, data2, data3;
  eb_device_t       device;
  eb_address_t      address_hi;
  eb_address_t      address_lo;

  uint64_t          t[NREAD];
  uint64_t          offs[NREAD];
  uint64_t          aveReadT;
  

  struct sdb_device sdbDevice;
  int               nDevices;


  // variables snoop event
  uint64_t snoopID     = 0x0;

  int i;

  // variables inject event
  saftlib::Time eventTime;     // time for next event in PTP time
  saftlib::Time ppsNext;       // time for next PPS 
  saftlib::Time wrTime;        // current WR time  

  // variables attach, remove
  char    *deviceName = NULL;


  int quit       = 0;
  int getVersion = 0;

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "o:hfdg")) != -1) {
    switch (opt) {
      case 'e' :
        getVersion = 1;
        break;
      case 'f' :
        useFirstDev = true;
        break;
      case 'g' :
         negOffset = true;
        break;
      case 'o' :
        offset = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
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

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind
  
  if (optind + 2  >= argc) {
    std::cerr << program << " expecting three non-optional arguments: <device name> <gid> <sid>" << std::endl;
    help();
    return 1;
  } // if optind

  if (getVersion) printf("%s: version %x\n", program, FEC_ANALYZER_VERSION);

  if (negOffset) offset = -offset;
  printf("offset for FG_PREP is %8.3f us\n", (double)offset/1000.0);

  // EB stuff: get wr time quickly
  sprintf(devName, "dev/wbm0");
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) {std::cerr << "can't open socket" << std::endl; return 1;}
  if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK) {std::cerr << "can't open device" << std::endl; return 1;}
  nDevices = 1; //quick and dirty, only use first device
  if ((status = eb_sdb_find_by_identity(device,  ECA_SDB_VENDOR_ID, ECA_SDB_DEVICE_ID, &sdbDevice, &nDevices)) != EB_OK) {std::cerr << "can't find address" << std::endl; return 1;}
  address_lo = sdbDevice.sdb_component.addr_first + ECA_TIME_LO_GET;
  address_hi = sdbDevice.sdb_component.addr_first + ECA_TIME_HI_GET;

  // determine average time to read a timestamp via EB
  for (i=0; i<NREAD; i++) {
    do {
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {std::cerr << "can't open cycle" << std::endl; exit (1);}
      eb_cycle_read(cycle, address_hi, EB_BIG_ENDIAN|EB_DATA32, &data1);
      eb_cycle_read(cycle, address_lo, EB_BIG_ENDIAN|EB_DATA32, &data2);
      eb_cycle_read(cycle, address_hi, EB_BIG_ENDIAN|EB_DATA32, &data3);
      if ((status = eb_cycle_close(cycle)) != EB_OK) {std::cerr << "can't close cycle" << std::endl; exit (1);}
    } while (data1 != data3);
    t[i]  = (uint64_t)data1 << 32;
    t[i] += (uint64_t)data2;
  } // for i
  aveReadT  = (t[NREAD - 1] - t[0]) / (NREAD - 1);

  // determine system time offset
  for (i=0; i<NREAD; i++) {
    do {
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {std::cerr << "can't open cycle" << std::endl; exit (1);}
      eb_cycle_read(cycle, address_hi, EB_BIG_ENDIAN|EB_DATA32, &data1);
      eb_cycle_read(cycle, address_lo, EB_BIG_ENDIAN|EB_DATA32, &data2);
      eb_cycle_read(cycle, address_hi, EB_BIG_ENDIAN|EB_DATA32, &data3);
      if ((status = eb_cycle_close(cycle)) != EB_OK) {std::cerr << "can't close cycle" << std::endl; exit (1);}
    } while (data1 != data3);
    t[i]  = (uint64_t)data1 << 32;
    t[i] += (uint64_t)data2;
    offs[i] = t[i] - getSysTime();
  } // for i

  aveOffset = 0;
  for (i=0; i<NREAD; i++) aveOffset += offs[i];
  aveOffset  = aveOffset / NREAD;
  printf("average TS read time via EB is  %15.3f us \n", (double)aveReadT/1000.0);
  printf("average system time offset is   %15.3f us \n", (double)aveOffset/1000.0);
  aveOffset  = aveOffset - aveReadT/2;
  printf("corrected system time offset is %15.3f us \n", (double)aveOffset/1000.0);

  deviceName = argv[optind];
  gid        = strtol(argv[optind+1], &tail, 0);
  sid        = strtol(argv[optind+2], &tail, 0);
  
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
      receiver   = TimingReceiver_Proxy::create(devices[deviceName]);
      //receiverCB = receiver;
    } //if(useFirstDevice);


    // create software action sink
    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));

    nCond = 6;
    clearStats();

    if (nCond > MAXCOND) {
      std::cerr << "bah! number of conditions exceeds" << MAXCOND << "." << std::endl;
      return 1;
    } // if nCond
    
    std::shared_ptr<SoftwareCondition_Proxy> condition[nCond];
    uint32_t tag[nCond];

    // BP_START
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)BP_START << 36);
    condition[0]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[0]        = tagBps;

    // SEQ_START
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)SEQ_START << 36);
    condition[1]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[1]        = tagSqs;

    // GAP_START
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)GAP_START << 36);
    condition[2]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[2]        = tagGps;

    // GAP_END
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)GAP_END << 36);
    condition[3]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[3]        = tagGpe;

    // FG_PREP
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)FG_PREP << 36);
    condition[4]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, offset));
    tag[4]        = tagFgp;

    // FG_START
    snoopID       = ((uint64_t)FID << 60) | ((uint64_t)gid << 48) | ((uint64_t)FG_START << 36);
    condition[5]  = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfffffff000000000, 0));
    tag[5]        = tagFgs;

    // let's go!
    for (i=0; i<(int)nCond; i++) {
      condition[i]->setAcceptLate(true);
      condition[i]->setAcceptEarly(true);
      condition[i]->setAcceptConflict(true);
      condition[i]->setAcceptDelayed(true);
      condition[i]->SigAction.connect(sigc::bind(sigc::ptr_fun(&recTimingMessage), tag[i]));
      condition[i]->setActive(true);    
    } // for i

    // reconfigure terminal
    static struct termios oldt, newt;
    char ch = 0;
    int  len;
    
    // check for any character....
    // get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    
    // set non canonical mode
    newt = oldt;
    //newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ICANON | ECHO); 
    
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
    
    while(!quit) {
      saftlib::wait_for_signal();
      len = read(STDIN_FILENO, &ch, 1);
      if (len) {
        switch (ch) {
          case 'q' :
            quit = 1;
            break;
          case 'c' :
            clearStats();
            break;
          case 'p' :
            printStats();
            break;
          default :
            ;
        } // switch ch
      } // if len
    } // while !quit

    // reset to old terminal settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  } // try
  catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  } // catch

  eb_device_close(device);
  eb_socket_close(socket);
  
  return 0;
} // main


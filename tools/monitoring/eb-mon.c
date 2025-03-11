///////////////////////////////////////////////////////////////////////////////
//  eb-mon.c (previously wr-mon.c)
//
//  created : 2015
//  author  : Dietrich Beck, GSI-Darmstadt
//  version : 11-mar-2025
//
// Command-line interface for WR monitoring via Etherbone.
//
// -------------------------------------------------------------------------------------------
// License Agreement for this software:
//
// Copyright (C) 2013  Dietrich Beck
// GSI Helmholtzzentrum für Schwerionenforschung GmbH
// Planckstraße 1
// D-64291 Darmstadt
// Germany
//
// Contact: d.beck@gsi.de
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 3 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.
//
// For all questions and ideas contact: d.beck@gsi.de
// Last update: 25-April-2015
//////////////////////////////////////////////////////////////////////////////////////////////
#define EBMON_VERSION "2.2.4"
#define AHEADT       1000000     // data master works ahead of time [ns]
#define EARLYDT   1000000000     // detection limit for early events [ns]

// standard includes
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

// Etherbone
#include <etherbone.h>

// Wishbone api
#include <wb_api.h>
#include <wb_slaves.h>

const char* program;
static int verbose=0;
eb_device_t device;        // needs to be global for 1-wire stuff
eb_device_t deviceOther;   // other EB device for comparing timestamps


static void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} // die


static void help(void)
{
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -a               display gateware 'build type'\n");
  fprintf(stderr, "  -b<busIndex>     display ID (ID of slave on the specified 1-wire bus)\n");
  fprintf(stderr, "  -c<eb-device>    compare timestamp with the one of <eb-device> and display the result\n");
  fprintf(stderr, "  -d               display WR time\n");
  fprintf(stderr, "  -e               display etherbone version\n");
  fprintf(stderr, "  -f<familyCode>   specify family code of 1-wire slave (0x43: EEPROM; 0x28,0x42: temperature)\n");
  fprintf(stderr, "  -g               display WR statistics (lock, time continuity)\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "  -i               display WR IP\n");
  fprintf(stderr, "  -j<cpu>          display lm32 stall info (default: j0) \n");
  fprintf(stderr, "  -k               display 'ECA-Tap' statistics\n");
  fprintf(stderr, "  -l               display WR link status\n");
  fprintf(stderr, "  -m               display WR MAC\n");
  fprintf(stderr, "  -n<NIC index>    specify NIC for selected properties (0: 1st NIC; 1: 2nd NIC; default: n0)\n");
  fprintf(stderr, "  -o               display offset between WR time and system time [ms]\n");
  fprintf(stderr, "  -p               display state of IP\n");
  fprintf(stderr, "  -s<secs> <cpu>   snoop for information continuously (and print warnings. THIS OPTION RESETS ALL STATS!)\n");
  fprintf(stderr, "  -t<busIndex>     display temperature of sensor on the specified 1-wire bus\n");
  fprintf(stderr, "  -u<index>        user 1-wire: specify WB device in case multiple WB devices of the same type exist (default: u0)\n");
  fprintf(stderr, "  -v               display verbose information\n");
  fprintf(stderr, "  -w<index>        WR 1-wire: specify WB device in case multiple WB devices of the same type exist (default: w0)\n");
  fprintf(stderr, "  -x               display the 8b/10b encoding error counter\n");
  fprintf(stderr, "  -y               display WR sync status\n");
  fprintf(stderr, "  -z               display FPGA uptime [h]\n");
  fprintf(stderr, "  -K               display WR time; for dual SFP boards only\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  wrstatreset  <tWrObs> <tStallObs>  command clears WR statistics and sets observation times (default: 8 50000)\n");
  fprintf(stderr, "  ecatapreset  <lateOffset>          command resets ECA-Tap and sets offset for detection of late events (default: 0)\n");
  fprintf(stderr, "  ecatapclear  <clearFlag>           command clears ECA-Tap counters (b3: late count, b2: count/accu, b1: max, b0: min)\n");
  fprintf(stderr, "  ecatapenable                       command enables capture on ECA-Tap\n");
  fprintf(stderr, "  ecatapdisable                      command disables capture on ECA-Tap\n");
  fprintf(stderr, "  encerrclear  <clearFlag>           command clears the enconder error counter (b1: PHY index 1; b0: PHY index 0)\n");
  fprintf(stderr, "\n");  
  fprintf(stderr, "Use this tool to get some info about WR enabled hardware.\n");
  fprintf(stderr, "Example1: '%s -v dev/wbm0' display typical information.\n", program);
  fprintf(stderr, "Example2: '%s -b0 -f0x43 dev/wbm0' read ID of EEPROM connected to 1st (user) 1-wire bus\n", program);
  fprintf(stderr, "Example3: '%s -x0 dev/wbm0' read number of encoder errors for the first PHY\n", program);
  fprintf(stderr, "Example4: '%s dev/wbm0 encerrclear 0x1' clears the encoder error counter for the first PHY\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "eb-mon:    WR [ns]   | CPU stall[%%]|                     [n(Hz)]   ECA                    [us]| # enc err\n");
  fprintf(stderr, "eb-mon:  lock +dt -dt|   max(  act)| nMessages( rate ) early late  min   max  avrge( act) ltncy|          \n");
  fprintf(stderr, "eb-mon:     1  16   0| 32.71(17.87)|      2501(  69.0)     0    0  879   986    935( 935)   121          0\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '      '    '      '          '\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '      '    '      '          '- # of encoding errors\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '      '    '      '- latency\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '      '    '- actual average (dl - ts)\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '      '- average (dl - ts)\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '     '- max (dl - ts) since last 'early event'\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '    '- min (deadline - timestamp) since last 'late event'\n");
  fprintf(stderr, "            '   '   '      '     '           '      '      '    '- # of late messages\n");
  fprintf(stderr, "            '   '   '      '     '           '      '       - # of early messages\n");
  fprintf(stderr, "            '   '   '      '     '           '      ' - actual message rate [Hz]\n");
  fprintf(stderr, "            '   '   '      '     '           '- total # of messages\n");
  fprintf(stderr, "            '   '   '      '     ' - actual rate of eCPU stalls (should be below '50.0')\n");
  fprintf(stderr, "            '   '   '      '- max continous eCPU stall (should be below '50.0')\n");
  fprintf(stderr, "            '   '   '- WR time continuity: maximum negative difference (should be '0')\n");
  fprintf(stderr, "            '   '- WR time continuity: maximum positive difference (should be '8' for a 125 MHz CPU clock)\n");
  fprintf(stderr, "            '- WR lock: '1' signals 'TRACK_PHASE'\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMON_VERSION);
} // help


void printSnoopHeader()
{ 
  fprintf(stdout, "%s:    WR [ns]   | CPU stall[%%]|                      [n(Hz)]   ECA                    [us]| # enc err\n", program);
  fprintf(stdout, "%s:  lock +dt -dt|   max(  act)| nMessages( rate ) early late  min   max  avrge( act) ltncy|          \n", program);
} // printSnoopHeader


void printSnoopData(int snoopInterval, int snoopLockFlag, int64_t contMaxPosDT, int64_t contMaxNegDT, double snoopStallMax, double snoopStallAct, uint64_t ecaNMessage, int64_t ecaMin, int64_t ecaMax, int64_t ecaDtSum, int ecaLate, int ecaEarly, uint32_t nErrEnc)
{
  int average;
  int latency;
  uint64_t nMessageAct;
  int64_t  dtSumAct;
  int      averageAct;
  static uint64_t nMessagePrev = 0;
  static uint64_t dtSumPrev    = 0;

  average      = (int)(ecaDtSum/(1000*ecaNMessage+1)); // ns -> us
  latency      = (AHEADT - ecaMin) / 1000.0;           // ns -> us
  nMessageAct  = ecaNMessage - nMessagePrev;
  dtSumAct     = ecaDtSum - dtSumPrev; 
  averageAct   = (int)(dtSumAct/(1000*nMessageAct+1));

  nMessagePrev = ecaNMessage;
  dtSumPrev    = ecaDtSum;
  
  fprintf(stdout, "%s: ", program);
  fprintf(stdout, "    %1d ", snoopLockFlag);
  fprintf(stdout, "%3d ", (int)contMaxPosDT);
  fprintf(stdout, "%3d|", (int)contMaxNegDT);
  fprintf(stdout, " %5.2f(%5.2f)|", snoopStallMax, snoopStallAct);
  if (ecaNMessage == 0) fprintf(stdout, " %9"PRIu64"(%6.1f)   %3d  %3d  %3d %5d   %4d(%4d)   %3d|   %7d", (uint64_t)0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0);
  else                  fprintf(stdout, " %9"PRIu64"(%6.1f)   %3d  %3d  %3d %5d   %4d(%4d)   %3d|   %7d", ecaNMessage, (double)nMessageAct/(double)snoopInterval, ecaEarly, ecaLate, (int)(ecaMin/1000), (int)(ecaMax/1000), average, averageAct, latency, nErrEnc);
  fprintf(stdout, "\n");
  fflush(stdout); 

} // printSnoopData

int main(int argc, char** argv) {
  #define BUILDTYPELEN   256
  #define STALLTOBS    50000
  #define WRTOBS           8

  eb_status_t       status;
  eb_socket_t       socket;
  int               devIndex=-1;  // 0,1,2... - there may be more than 1 device on the WB bus
  unsigned int      busIndex=-1;  // index of 1-wire bus connected to a controller
  int               nicIndex=0;   // index of WR interface; 0: 1st NIC, 1: 2nd NIC

  int               i;            // counter for comparing WR time with other device
  int               nCompare = 5; // number of compares
  uint64_t          nsecsDiff64;
  int               diffIsPositive;               

  const char* devName;
  const char* devNameOther=NULL;
  const char* command;      

  int         getEBVersion=0;
  int         getWRDate=0;
  int         getWRDNDate=0;
  int         getWROffset=0;
  int         getWRSync=0;
  int         getWRMac=0;
  int         getWRLink=0;
  int         getWRIP=0;
  int         getWRIPState=0;
  int         getWRStats=0;
  int         getBoardID=0;
  int         getBoardTemp=0;
  int         getWRDateOther=0;
  int         getWRUptime=0;
  int         getBuildType=0;
  int         getCPUStall=0;
  int         getECATap=0;
  int         getEncErrCounter=0;
  int         snoopMode=0;
  int         exitCode=0;

  unsigned int family         = 0;    // 1-Wire: familyCode
  unsigned int user1Wire      = 1;    // 1-Wire: 1 - User-1Wire; 0 - WR-Periph-1Wire
  unsigned int ecpu           = 0;    // # of embedded cpu (for 'stall' statistics)
  unsigned int snoopSecs      = 0;    // # of seconds; poll rate for snoop mode
  int          snoopLockFlag  = 0;
  double       snoopStallMax  = 0;
  double       snoopStallAct  = 0;
  uint32_t     nSecs;

  uint64_t    nsecs64, nsecsOther64;
  uint64_t    nsecsSum64, nsecsSumOther64;
  uint64_t    nsecsRound64, nsecsRoundOther64;
  uint64_t    tmpa64, tmpb64;
  uint64_t    usecs64;
  uint64_t    hostusecs64;
  int64_t     offset;
  uint64_t    mac;
  uint64_t    lockLossTS;
  uint64_t    lockAcqTS;
  uint32_t    lockNAcq;
  uint64_t    contObsT;
  int64_t     contMaxPosDT;
  uint64_t    contMaxPosTS;
  int64_t     contMaxNegDT;
  uint64_t    contMaxNegTS;
  uint64_t    stallObsT;
  uint32_t    stallMax;
  uint32_t    stallAct;
  uint64_t    stallTS;
  short       ecaClearFlag;
  short       encErrClearFlag;
  uint64_t    ecaNMessage;
  int64_t     ecaDtSum;
  int64_t     ecaDtMin;
  int64_t     ecaDtMax;
  uint32_t    ecaNLate;
  int32_t     ecaLateOffset;
  int         ecaSumEarly;
  uint32_t    nEncErr;
  uint32_t    nEncErrOld;
  int         flagEncErrOverflow;
  int         flagEncErrExists; 

  
  int         link;
  uint32_t    uptime;
  int         syncState;
  int         ip;
  int         ipState;
  uint64_t    id;
  double      temp;
  char linkStr[64];
  char ipStateStr[64];
  char syncStr[64];
  char timestr[60];
  char encErrStr[64];
  char dummy[64];
  uint32_t dummy32;
  char buildType[BUILDTYPELEN];
  time_t secs;
  const struct tm* tm;
  struct timeval htm;

  int opt, error=0;
  char *tail;

  program = argv[0];

  while ((opt = getopt(argc, argv, "t:u:w:f:b:c:j:s:n:adgopymlievhzkxK")) != -1) {
    switch (opt) {
      case 'a':
        getBuildType=1;
        break;
      case 'b':
        getBoardID=1;
        busIndex = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'c':
        getWRDateOther=1;
        devNameOther = optarg;
        break;
      case 'K':
        getWRDNDate=1;
        break; 
     case 'f':
        family = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'g':
        getWRStats=1;
        break;
      case 'j':
        getCPUStall=1;
        ecpu   = strtol(optarg, &tail, 0);
        if (ecpu > 16) {
          fprintf(stderr, "# of cpu '%d' unreasonable large!\n", ecpu);
          exit(1);
        } // if ecpu
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'k':
        getECATap=1;
        break;
      case 'o':
        getWROffset=1;
        break;
      case 'm':
        getWRMac=1;
        break;
      case 'l':
        getWRLink=1;
        break;
      case 'i':
        getWRIP=1;
        break;
      case 'p':
        getWRIPState=1;
        break;
      case 'n':
        nicIndex = strtol(optarg, &tail, 0);
        if (!(nicIndex == 0 || nicIndex == 1)) {
          fprintf(stderr, "NIC index has to be 0 or 1, not %d!\n", nicIndex);
          exit(1);
        } // if nicIndex
        break;
      case 'x':
        getEncErrCounter=1;
        break;
      case 'y':
        getWRSync=1;
        break;
      case 'z':
        getWRUptime=1;
        break;
      case 's':
        snoopMode=1;
        if (argv[optind-1] != NULL) {                          // need to parse '-s1 0' as well as '-s 1 0'
          sprintf(dummy, "huhu%s", argv[optind-1]);            // add dummy text to be sure we have a non-digit as prefix
          sscanf(dummy,  "%*[^0123456789]%d", &snoopSecs);     // ignore preceeding non-digits
          if (snoopSecs < 1) snoopSecs = 1;
        }
        else                        {fprintf(stderr, "missing '# of seconds'!\n"); exit(1);}
        if (argv[optind+0] != NULL) {ecpu      = strtol(argv[optind+0], &tail, 0); }
        else                        {fprintf(stderr, "missing '# of ecpu'!\n"); exit(1);}
        if ( ecpu > 16) {
          fprintf(stderr, "# of cpu '%d' unreasonable large!\n", ecpu);
          exit(1);
        } // if ecpu
        break;
      case 't':
        getBoardTemp=1;
        busIndex = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'e':
        getEBVersion=1;
        break;
      case 'u':
        user1Wire = 1;
        devIndex  = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'v':
        getWRDate=1;
        getWROffset=1;
        getWRSync=1;
        getWRMac=1;
        getWRLink=1;
        getWRIP=1;
        getWRIPState=1;
        getWRUptime=1;
        getEBVersion=1;
        getBuildType=1;
        getEncErrCounter=1;
        verbose=1;
        break;
      case 'w':
        user1Wire = 0;
        devIndex  = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'h':
        help();
        return 0;
      case ':':
      case '?':
        error = 1;
        break;
      default:
        fprintf(stderr, "%s: bad getopt result\n", program);
        return 1;
    } // switch opt
  } // while opt
  
  if (error) {
    help();
    return 1;
  }
  
  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];
  if (devIndex < 0) devIndex = 0; // default: grab first device of the requested type on the wishbone bus

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;
  
  
  if (getEBVersion) {
    if (verbose) fprintf(stdout, "EB version / EB source: ");
    fprintf(stdout, "%s / %s\n", eb_source_version(), eb_build_info());
  }

  // open Etherbone device and socket
  if ((status = wb_open(devName, &device, &socket)) != EB_OK) {
    fprintf(stderr, "can't open connection to device %s (%s) \n", devName, eb_status(status));
    return (1);
  }
  
  if (snoopMode) { /* chk: mit der heissen Nadel gestrickt */
    // init
    wb_eca_stats_reset(device, devIndex, 0);
    wb_eca_stats_enable(device, devIndex, 0x1);
    wb_wr_stats_reset(device, devIndex, WRTOBS, STALLTOBS);
    wb_wr_reset_enc_err_counter(device, devIndex, nicIndex);
    nSecs            = snoopSecs;
    ecaSumEarly      = 0;
    nEncErrOld       = 0;
    flagEncErrExists = 1;
    printSnoopHeader();
    while(1) {
      // get data
      // wr lock
      wb_wr_get_sync_state(device, nicIndex, &syncState);
      if (syncState == WR_PPS_GEN_ESCR_MASK) snoopLockFlag = 1;
      else                                   snoopLockFlag = 0;
      if (!snoopLockFlag) {
        fprintf(stdout,                         "%s: error - WR not locked (resetting DM-Diagnostics)\n", program);
        wb_wr_stats_reset(device, devIndex, WRTOBS, STALLTOBS); 
      } // if !snoopLockFlag

      // time continuity
      wb_wr_stats_get_continuity(device, devIndex, &contObsT, &contMaxPosDT, &contMaxPosTS, &contMaxNegDT, &contMaxNegTS);
      if (contMaxPosDT > 16)    fprintf(stdout, "%s: error - WR time (posDT) jumps by %d [ns]\n", program, (int)contMaxPosDT);
      if (contMaxNegDT != 0)    fprintf(stdout, "%s: error - WR time (negDT) jumps by %d [ns]\n", program, (int)contMaxNegDT);

      // CPU stalls
      wb_wr_stats_get_stall(device, devIndex, ecpu, &stallObsT, &stallMax, &stallAct, &stallTS);
      snoopStallMax = (double)stallMax / (double)stallObsT * 100.0;
      snoopStallAct = (double)stallAct         / (double)stallObsT * 100.0;
      if (snoopStallMax > 50.0) fprintf(stdout, "%s: error - max CPU stall %f\n", program, snoopStallMax);

      // ECA
      wb_eca_stats_get(device, devIndex, &ecaNMessage, &ecaDtSum, &ecaDtMin, &ecaDtMax, &ecaNLate, &ecaLateOffset);
      if (ecaDtMin < ecaLateOffset) {  
        fprintf(stdout,                         "%s: error - late event %f [us]\n", program, (double)ecaDtMin/1000.0);
        wb_eca_stats_clear(device, devIndex, 0x1);
      }
      if (ecaDtMax > 1000000000) {  /* chk */
        ecaSumEarly++;              /* chk */
        fprintf(stdout,                         "%s: error - early event %f [us]\n", program, (double)ecaDtMax/1000.0);
        wb_eca_stats_clear(device, devIndex, 0x2);
      }

      // error encoder
      if (flagEncErrExists) {
        status = wb_wr_read_enc_err_counter(device, devIndex, nicIndex, &nEncErr, &flagEncErrOverflow);
        if (status != EB_OK) {flagEncErrExists = 0; nEncErr = 9999999;}
        if (nEncErr > nEncErrOld) {
          fprintf(stdout,                       "%s: error - 8b/10b cnt increased from %d to %d\n", program, nEncErrOld, nEncErr);
          nEncErrOld = nEncErr;
        } // if nEncErr
      } // reduce warnings in case gateware does not support error counter
       
      if (nSecs >= snoopSecs) {
        printSnoopData(snoopSecs, snoopLockFlag, contMaxPosDT, contMaxNegDT, snoopStallMax, snoopStallAct,
                       ecaNMessage, ecaDtMin, ecaDtMax, ecaDtSum, ecaNLate, ecaSumEarly, nEncErr);
        nSecs = 1;
      } // if nSecs
      else nSecs++;
      usleep(1000000);  // iterate once per second
    } // while
  } // if snoopMode

  if (getWRDateOther) {
    if ((status = wb_open(devNameOther, &deviceOther, &socket)) != EB_OK) {
      fprintf(stderr, "can't open connection to device %s \n", devNameOther);
      return (1);
    }

    // do one round, to be sure WR network "knows" route to other device
    if ((status = wb_wr_get_time(deviceOther, 0,        &nsecsOther64)) != EB_OK) die("WR get time other", status);
    if ((status = wb_wr_get_time(device,      devIndex, &nsecs64))      != EB_OK) die("WR get time", status);

    // now start
    nsecsSum64      = 0;
    nsecsSumOther64 = 0;

    for (i=0; i < nCompare; i++) {
      if ((status = wb_wr_get_time(device,      devIndex, &tmpa64)) != EB_OK) die("WR get time", status);
      if ((status = wb_wr_get_time(deviceOther, 0,        &tmpb64)) != EB_OK) die("WR get time other", status);

      nsecsSum64      += tmpa64;
      nsecsSumOther64 += tmpb64;
    } 

    nsecs64       = (uint64_t)((double)(nsecsSum64)      / (double)nCompare);
    nsecsOther64  = (uint64_t)((double)(nsecsSumOther64) / (double)nCompare);

    // determine the roundtrip time for device
    wb_wr_get_time(device, 0, &tmpa64);
    for (i=0; i < nCompare; i++) wb_wr_get_time(device, 0, &tmpb64);
    nsecsRound64 = tmpb64 - tmpa64;
    nsecsRound64 = (uint64_t)((double)nsecsRound64/(double)nCompare);

    // determine the roundtrip time for other device
    wb_wr_get_time(deviceOther, 0, &tmpa64);
    for (i=0; i < nCompare; i++) wb_wr_get_time(deviceOther, 0, &tmpb64);
    nsecsRoundOther64 = tmpb64 - tmpa64;
    nsecsRoundOther64 = (uint64_t)((double)nsecsRoundOther64/(double)nCompare);

    // nsecsOther64 has been measured after the nsecs64 has been completed. So we need to subtract that roundtrip time of the first device
    nsecsOther64 = nsecsOther64 - nsecsRound64;

    // the timestamps nsecs64 and nsecsOther64 are measured after the etherbone packet arrived at the FPGA
    // we use the simplified model, that the the transmission times to and from the remote FPGA are identical (symmetry) and that the roundtrip is only due to total transmission time
    // hece, the timestamp is latched after half of the roundtrip time, so we need to subtract that from both values

    nsecsOther64 = nsecsOther64 - (nsecsRoundOther64 >> 1);
    nsecs64      = nsecs64      - (nsecsRound64      >> 1);

    if (nsecs64 > nsecsOther64) {
      nsecsDiff64    = nsecs64 - nsecsOther64;
      diffIsPositive = 1;
    }
    else {
      nsecsDiff64    = nsecsOther64 - nsecs64; 
      diffIsPositive = 0;
    }

      fprintf(stdout, "WR differs by ");
      if (diffIsPositive) fprintf(stdout, "+");
      else                fprintf(stdout, "-");
      fprintf(stdout, "%"PRIu64" us\n", nsecsDiff64 / 1000);
  } 

  if (getWRDate || getWROffset) {
    if ((status = wb_wr_get_time(device, devIndex, &nsecs64)) != EB_OK) die("WR get time", status);
    secs     = (unsigned long)((double)nsecs64 / 1000000000.0);
    usecs64  = nsecs64 / 1000.0;

    if (getWROffset) {
      // get system time
      gettimeofday(&htm, NULL);
      hostusecs64 = htm.tv_sec*1000000 + htm.tv_usec;
      offset      = usecs64 - hostusecs64;
      if (verbose) fprintf(stdout, "WR_time - host_time [ms]: ");
      fprintf(stdout, "%9.3f\n", (double)offset/1000.0);
    }

    if (getWRDate) {
      // Format the date
      tm = gmtime(&secs);
      strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
      
      if (verbose) fprintf(stdout, "Current TAI: ");
      fprintf(stdout, "%s (%lu us)", timestr, usecs64 - secs * 1000000);
      fprintf(stdout, ", %"PRIu64" us\n", nsecs64 / 1000);
    }
  } // if getWRDate

  if (getWRDNDate) {
    if ((status = wb_wr_get_dualnic_time(device, nicIndex, &nsecs64)) != EB_OK) die("WR get dual NIC time", status);
    secs     = (unsigned long)((double)nsecs64 / 1000000000.0);
    usecs64  = nsecs64 / 1000.0;

    // Format the date
    tm = gmtime(&secs);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
    
    if (verbose) fprintf(stdout, "Current TAI: ");
    fprintf(stdout, "%s (%lu us)", timestr, usecs64 - secs * 1000000);
      fprintf(stdout, ", %"PRIu64" us\n", nsecs64 / 1000);
  } // if getWRDNDate

  if (getWRSync) {
    if ((status = wb_wr_get_sync_state(device, nicIndex, &syncState)) != EB_OK) die("WR get sync state", status);
    if ((syncState == WR_PPS_GEN_ESCR_MASK))
      sprintf(syncStr,"TRACKING");
    else if ((syncState == WR_PPS_GEN_ESCR_MASKTS))
      sprintf(syncStr,"TIME");
    else if ((syncState == WR_PPS_GEN_ESCR_MASKPPS))
      sprintf(syncStr, "PPS");
    else
      sprintf(syncStr, "NO SYNC");
    if (verbose) fprintf(stdout, "Sync Status: ");
    fprintf(stdout, "%s\n", syncStr);
  } // getWRSync

  if (getWRMac) {
    if ((status = wb_wr_get_mac(device, nicIndex, &mac)) != EB_OK) die("WR get MAC", status);
    if (verbose) fprintf(stdout, "MAC: ");
    fprintf(stdout, "%012llx\n", (long long unsigned)mac);
  }

  if (getWRLink) {
    if ((status = wb_wr_get_link(device, nicIndex, &link)) != EB_OK) die("WR get link state", status);
    if (link) 
      sprintf(linkStr, "LINK_UP");
    else
      sprintf(linkStr, "LINK_DOWN");
    if (verbose) fprintf(stdout, "Link Status: ");
    fprintf(stdout, "%s\n", linkStr);
  }

  if (getWRIP) {
    if ((status = wb_wr_get_ip(device, nicIndex, &ip)) != EB_OK) die("WR get IP", status);
    if (verbose) fprintf(stdout, "IP: ");
    fprintf(stdout, "%03d.%03d.%03d.%03d\n", (ip & 0xFF000000) >> 24, (ip & 0x00FF0000) >> 16, (ip & 0x0000FF00) >> 8, ip & 0x000000FF);
  }

  if(getWRIPState) {
     if ((status = wb_get_build_type(device, BUILDTYPELEN, buildType, &dummy32)) != EB_OK) die("WB get build type (for IP state)", status);
     if ((status = wb_wr_get_ip_state(device, nicIndex, dummy32, &ipState)) != EB_OK) die("WB get IP state", status);
     switch (ipState) {
       case -1 :
         sprintf(ipStateStr, "unknown");
         break;
       case 0 :
         sprintf(ipStateStr, "invalid");
         break;
       case 1 :
         sprintf(ipStateStr, "valid (BOOTP)");
         break;;
       case 2:
         sprintf(ipStateStr, "valid (static)");
         break;
       default :
         sprintf(ipStateStr, "error");
         break;
     } // switch ipState
     if (verbose)  fprintf(stdout, "IP state: ");
     fprintf(stdout, "%s\n", ipStateStr);
  }
  
  if (getWRUptime) {
    if ((status = wb_wr_get_uptime(device, devIndex, &uptime)) != EB_OK) die("WR get uptime", status);
    if (verbose) fprintf(stdout, "FPGA uptime [h]: ");
    fprintf(stdout, "%013.2f\n", (double)uptime / 3600.0 );
  } 

  if (getBuildType) {
    if ((status = wb_get_build_type(device, BUILDTYPELEN, buildType, &dummy32)) != EB_OK) die("WB get build type", status);
    if (verbose) fprintf(stdout, "FPGA build type: ");
    fprintf(stdout, "%s\n", buildType);
  }

  if (getBoardID) {
    if (!family) die("family code not specified (1-wire)", EB_OOM);
    if ((status = wb_1wire_get_id(device, devIndex, busIndex, family, user1Wire, &id)) != EB_OK) die("WR get board ID", status);
    if (verbose) fprintf(stdout, "ID: ");
    fprintf(stdout, "0x%016"PRIx64"\n", id);
  }

  if (getBoardTemp) {
    if (!family) die("family code not specified (1-wire)", EB_OOM);
    if ((status = wb_1wire_get_temp(device, devIndex, busIndex, family, user1Wire, &temp)) != EB_OK) die("WR get board temperature", status);
    if (verbose) fprintf(stdout, "temp: ");
    fprintf(stdout, "%.4f\n", (float)temp);
  } 

  if (getWRStats || getCPUStall || getECATap) {
    fprintf(stdout, "\n");
    fprintf(stdout, "=============\n");
    fprintf(stdout, "Statistics\n");
    fprintf(stdout, "=============\n");
  } // if getWRStats || ...
  
  if (getWRStats) {
    fprintf(stdout, "= White Rabbit\n");
    
    // WR stats: lock
    fprintf(stdout, "-- locks...\n");
    if ((status = wb_wr_stats_get_lock(device, devIndex, &lockLossTS, &lockAcqTS, &lockNAcq)) != EB_OK) die("WR get lock statistics", status);
    fprintf(stdout, "---# of acquired locks: %d\n", lockNAcq);
    
    if (lockLossTS == 0xffffffffffffffff)
      fprintf(stdout, "---WR lock lost       : N/A\n");
    else {
      secs = (unsigned long)((double)lockLossTS / 1000000000.0);
      tm = gmtime(&secs);
      strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
      fprintf(stdout, "---WR lock lost       : %s\n", timestr);
    }
    
    if (lockAcqTS == 0xffffffffffffffff)
      fprintf(stdout, "---WR lock acquired   : N/A\n");
    else {
      secs = (unsigned long)((double)lockAcqTS / 1000000000.0);
      tm = gmtime(&secs);
      strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
      fprintf(stdout, "---WR lock aquired    : %s\n", timestr);
    }
    
    // WR stats: time continuity
    fprintf(stdout, "-- time continuity [ns]\n");
    if ((status = wb_wr_stats_get_continuity(device, devIndex, &contObsT, &contMaxPosDT, &contMaxPosTS, &contMaxNegDT, &contMaxNegTS)) != EB_OK) die("WR get lock statistics", status);
    fprintf(stdout, "--- observation period: %"PRIu64"\n", contObsT);
    fprintf(stdout, "---- max pos diff     : %"PRId64"\n", contMaxPosDT);
    secs = (unsigned long)((double)contMaxPosTS / 1000000000.0);
    tm = gmtime(&secs);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
    fprintf(stdout, "---- TS of pos  diff  : %s\n", timestr);
    fprintf(stdout, "---- max neg diff     : %"PRId64"\n", contMaxNegDT);
    secs = (unsigned long)((double)contMaxNegTS / 1000000000.0);
    tm = gmtime(&secs);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
    fprintf(stdout, "---- TS of neg diff   : %s\n", timestr);
  } // if getWRStats
  
  if (getCPUStall) {
    fprintf(stdout, "= lm32 stalls for CPU %d [cycles]\n", ecpu);
    if ((status = wb_wr_stats_get_stall(device, devIndex, ecpu, &stallObsT, &stallMax, &stallAct, &stallTS)) != EB_OK) die("WR get CPU stall statistics", status);
    fprintf(stdout, "--- observation period: %"PRIu64"\n", stallObsT);
    fprintf(stdout, "---- stall time max   : %u\n", stallMax);
    fprintf(stdout, "---- stall time act   : %u\n", stallAct);
    secs = (unsigned long)((double)stallTS / 1000000000.0);
    tm = gmtime(&secs);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
    fprintf(stdout, "---- TS of 'time max' : %s\n", timestr); 
  } // if getCPUStall

  if (getECATap) {
    fprintf(stdout, "= ECA-Tap (input) [ns]\n");
    if ((status = wb_eca_stats_get(device, devIndex, &ecaNMessage, &ecaDtSum, &ecaDtMin, &ecaDtMax, &ecaNLate, &ecaLateOffset)) != EB_OK) fprintf(stdout, "warning: can't ECA-Tap statistics (can't find in gateware)\n");
    fprintf(stdout, "--- # of messages     : %"PRIu64"\n", ecaNMessage);
    fprintf(stdout, "--- ave (dl - ts)     : %d\n", (int32_t)((double)ecaDtSum/(double)ecaNMessage));
    fprintf(stdout, "--- min (dl - ts)     : %"PRId64"\n", ecaDtMin);
    fprintf(stdout, "--- max (dl - ts)     : %"PRId64"\n", ecaDtMax);
    fprintf(stdout, "--- calc latency      : %d\n", AHEADT - (int32_t)((double)ecaDtSum/(double)ecaNMessage));
    fprintf(stdout, "--- late offset       : %d\n", ecaLateOffset);
    fprintf(stdout, "--- # of late messages: %u\n", ecaNLate);
  } // getECATap

  if (getEncErrCounter) {
    status = wb_wr_read_enc_err_counter(device, devIndex, nicIndex, &nEncErr, &flagEncErrOverflow);
    // die if not verbose
    if ((status != EB_OK) && !verbose) die("WR get encoder error counter", status);
    // if verbose, continue; this shall keep eb-mon -v useable even with old gateware
    if (status != EB_OK) sprintf(encErrStr, "EB error");
    else                 sprintf(encErrStr, "%u", nEncErr);
    fprintf(stdout, "%s\n", encErrStr);
  } // if getEncerrcounter
  
  if (command) {

    if (!strcasecmp(command, "wrstatreset")) {
      if (optind+3  != argc) {printf("expecting exactly two arguments: wrstatclear  <tWrObs> <tStallObs>\n"); return 1;}
      contObsT = strtoul(argv[optind+1], &tail, 0);
      stallObsT = strtoul(argv[optind+2], &tail, 0);
      wb_wr_stats_reset(device, devIndex, contObsT, stallObsT);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // wrstatreset

    if (!strcasecmp(command, "ecatapreset")) {
      if (optind+2  != argc) {printf("expecting exactly one argument: ecatapreset <lateOffset>\n"); return 1;}
      ecaLateOffset = strtol(argv[optind+1], &tail, 0);
      wb_eca_stats_reset(device, devIndex, ecaLateOffset);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // ecatapreset

    if (!strcasecmp(command, "ecatapclear")) {
      if (optind+2  != argc) {printf("expecting exactly one argument: ecatapclear <clearFlag>\n"); return 1;}
      ecaClearFlag = strtoul(argv[optind+1], &tail, 0);
      wb_eca_stats_clear(device, devIndex, ecaClearFlag);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // ecatapclear

    if (!strcasecmp(command, "ecatapenable")) {
      wb_eca_stats_enable(device, devIndex, 0x1);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // ecatapenable

    if (!strcasecmp(command, "ecatapdisable")) {
      wb_eca_stats_enable(device, devIndex, 0x0);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // ecatapdisable

    if (!strcasecmp(command, "encerrclear")) {
      if (optind+2  != argc)     {printf("expecting exactly one argument: encerrclear <clearFlag>\n"); return 1;}
      encErrClearFlag = strtoul(argv[optind+1], &tail, 0);
      if (encErrClearFlag > 0x3) {printf("clear error encoder counter: parameter out of range\n"); return 1;}
      if (encErrClearFlag & 0x1) wb_wr_reset_enc_err_counter(device, devIndex, 0);
      if (encErrClearFlag & 0x2) wb_wr_reset_enc_err_counter(device, devIndex, 1);
      fprintf(stdout, "eb-mon: %s\n", command);
    } // encerrclear   
    
  } // if command

  wb_close(device, socket);
  wb_close(deviceOther, socket);
  
  return exitCode;
}

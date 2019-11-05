///////////////////////////////////////////////////////////////////////////////
//  eb-mon.c (previously wr-mon.c)
//
//  created : 2015
//  author  : Dietrich Beck, GSI-Darmstadt
//  version : 27-Sep-2019
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
#define EBMON_VERSION "2.0.0"

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


static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
}

static void help(void) {
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
  fprintf(stderr, "  -j<cpu>          display lm32 stall info; # of CPU starts with '0'\n");
  fprintf(stderr, "  -k               display 'ECA-Tap' statistics\n");
  fprintf(stderr, "  -l               display WR link status\n");
  fprintf(stderr, "  -m               display WR MAC\n");
  fprintf(stderr, "  -o               display offset between WR time and system time [ms]\n");
  fprintf(stderr, "  -s<number>       snoop  for information continuously\n");
  fprintf(stderr, "                      0: print all messages (default)\n");
  fprintf(stderr, "                      1..N print after N number of seconds\n");
  fprintf(stderr, "  -t<busIndex>     display temperature of sensor on the specified 1-wire bus\n");
  fprintf(stderr, "  -u<index>        user 1-wire: specify WB device in case multiple WB devices of the same type exist (default: u0)\n");
  fprintf(stderr, "  -v               display verbose information\n");
  fprintf(stderr, "  -w<index>        WR 1-wire: specify WB device in case multiple WB devices of the same type exist (default: u0)\n");
  fprintf(stderr, "  -y               display WR sync status\n");
  fprintf(stderr, "  -z               display FPGA uptime [h]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  wrstatclear  <tWrObs> <tStallObs>  command clears statics and sets observation times for 'WR statistics' and 'CPU stall' [ns]\n");
  fprintf(stderr, "  ecatapclear                        command clears statics on ECA Tap module\n");
  fprintf(stderr, "  ecatapenable                       command enables capture on ECA Tap module\n");
  fprintf(stderr, "  ecatapdisable                      command enables capture on ECA Tap module\n");
  fprintf(stderr, "\n");  
  fprintf(stderr, "Use this tool to get some info about WR enabled hardware.\n");
  fprintf(stderr, "Example1: '%s -v dev/wbm0' display typical information.\n", program);
  fprintf(stderr, "Example2: '%s -b0 -f0x43 dev/wbm0' read ID of EEPROM connected to 1st (user) 1-wire bus\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMON_VERSION);
}


int main(int argc, char** argv) {
  #define BUILDTYPELEN 256

  eb_status_t       status;
  eb_socket_t       socket;
  int               devIndex=-1;  // 0,1,2... - there may be more than 1 device on the WB bus
  unsigned int      busIndex=-1;  // index of 1-wire bus connected to a controller

  int               i;            // counter for comparing WR time with other device
  int               nCompare = 5; // number of compares
  uint64_t          nsecsDiff64;
  int               diffIsPositive;               

  const char* devName;
  const char* devNameOther=NULL;
  const char* command;      

  int         getEBVersion=0;
  int         getWRDate=0;
  int         getWROffset=0;
  int         getWRSync=0;
  int         getWRMac=0;
  int         getWRLink=0;
  int         getWRIP=0;
  int         getWRStats=0;
  int         getBoardID=0;
  int         getBoardTemp=0;
  int         getWRDateOther=0;
  int         getWRUptime=0;
  int         getBuildType=0;
  int         getCPUStall=0;
  int         getECATap=0;
  int         exitCode=0;

  unsigned int family    = 0;    // 1-Wire: familyCode
  unsigned int user1Wire = 1;    // 1-Wire: 1 - User-1Wire; 0 - WR-Periph-1Wire
  unsigned int ecpu      = 0;    // # of embedded cpu (for 'stall' statistics) 

  uint64_t    nsecs64, nsecsOther64;
  uint64_t    nsecsSum64, nsecsSumOther64;
  uint64_t    nsecsRound64, nsecsRoundOther64;
  uint64_t    tmpa64, tmpb64;
  uint64_t    msecs64;
  uint64_t    hostmsecs64;
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
  uint32_t    stallMaxStreak;
  uint32_t    stallN;
  uint64_t    stallTS;
  uint64_t    ecaNMessage;
  int64_t     ecaDtSum;
  int64_t     ecaDtMin;
  int64_t     ecaDtMax;
  
  int         link;
  uint32_t    uptime;
  int         syncState;
  int         ip;
  uint64_t    id;
  double      temp;
  char linkStr[64];
  char syncStr[64];
  char timestr[60];
  char buildType[BUILDTYPELEN];
  time_t secs;
  const struct tm* tm;
  struct timeval htm;

  int opt, error=0;
  char *tail;

  program = argv[0];

  while ((opt = getopt(argc, argv, "t:u:w:f:b:c:j:adgoymlievhzk")) != -1) {
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
      case 'd':
        getWRDate=1;
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
      case 'y':
        getWRSync=1;
        break;
      case 'z':
        getWRUptime=1;
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
        getWRStats=1;
        getWRSync=1;
        getWRMac=1;
        getWRLink=1;
        getWRIP=1;
        getWRUptime=1;
        getEBVersion=1;
        getBuildType=1;
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
    fprintf(stderr, "can't open connection to device %s \n", devName);
    return (1);
  }

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
    msecs64  = nsecs64 / 1000000.0;

    if (getWROffset) {
      // get system time
      gettimeofday(&htm, NULL);
      hostmsecs64 = htm.tv_sec*1000 + htm.tv_usec/1000;
      offset      = msecs64 - hostmsecs64;
      if (verbose) fprintf(stdout, "WR_time - host_time [ms]: ");
      fprintf(stdout, "%ld\n", (long)offset);
    }

    if (getWRDate) {
      // Format the date
      tm = gmtime(&secs);
      strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
      
      if (verbose) fprintf(stdout, "Current TAI: ");
      fprintf(stdout, "%s (%3lu ms)", timestr, msecs64 - secs * 1000);
      fprintf(stdout, ", %"PRIu64" us\n", nsecs64 / 1000);
    }
  } // if getWRDate

  if (getWRSync) {
    if ((status = wb_wr_get_sync_state(device, devIndex, &syncState)) != EB_OK) die("WR get sync state", status);
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
    if ((status = wb_wr_get_mac(device, devIndex, &mac)) != EB_OK) die("WR get MAC", status);
    if (verbose) fprintf(stdout, "MAC: ");
    fprintf(stdout, "%012llx\n", (long long unsigned)mac);
  }

  if (getWRLink) {
    if ((status = wb_wr_get_link(device, devIndex, &link)) != EB_OK) die("WR get link state", status);
    if (link) 
      sprintf(linkStr, "LINK_UP");
    else
      sprintf(linkStr, "LINK_DOWN");
    if (verbose) fprintf(stdout, "Link Status: ");
    fprintf(stdout, "%s\n", linkStr);
  }

  if (getWRIP) {
    if ((status = wb_wr_get_ip(device, devIndex, &ip)) != EB_OK) die("WR get IP", status);
    if (verbose) fprintf(stdout, "IP: ");
    fprintf(stdout, "%03d.%03d.%03d.%03d\n", (ip & 0xFF000000) >> 24, (ip & 0x00FF0000) >> 16, (ip & 0x0000FF00) >> 8, ip & 0x000000FF);
  }
  
  if (getWRUptime) {
    if ((status = wb_wr_get_uptime(device, devIndex, &uptime)) != EB_OK) die("WR get uptime", status);
    if (verbose) fprintf(stdout, "FPGA uptime [h]: ");
    fprintf(stdout, "%013.2f\n", (double)uptime / 3600.0 );
  } 

  if (getBuildType) {
    if ((status = wb_get_build_type(device, BUILDTYPELEN, buildType)) != EB_OK) die("WB get build type", status);
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
    fprintf(stdout, "= lm32 stalls for CPU %d [ns]\n", ecpu);
    if ((status = wb_wr_stats_get_stall(device, devIndex, ecpu, &stallObsT, &stallMaxStreak, &stallN, &stallTS)) != EB_OK) die("WR get CPU stall statistics", status);
    fprintf(stdout, "--- observation period: %"PRIu64"\n", stallObsT);
    fprintf(stdout, "---- stall time max   : %u\n", stallMaxStreak);
    fprintf(stdout, "---- stall time act   : %u\n", stallN);
    secs = (unsigned long)((double)stallTS / 1000000000.0);
    tm = gmtime(&secs);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
    fprintf(stdout, "---- TS of neg diff   : %s\n", timestr); 
  } // if getCPUStall

  if (getECATap) {
    int32_t aheadt = 1000000;
    fprintf(stdout, "= ECA-Tap (input) [ns]\n");
    if ((status = wb_eca_stats_get(device, devIndex, &ecaNMessage, &ecaDtSum, &ecaDtMin, &ecaDtMax)) != EB_OK) fprintf(stdout, "warning: can't ECA-Tap statistics (can't find in gateware)\n");
    fprintf(stdout, "--- # of messages     : %"PRIu64"\n", ecaNMessage);
    fprintf(stdout, "--- ave (dl - ts)     : %d\n", (int32_t)((double)ecaDtSum/(double)ecaNMessage));
    fprintf(stdout, "--- min (dl - ts)     : %"PRId64"\n", ecaDtMin);
    fprintf(stdout, "--- max (dl - ts)     : %"PRId64"\n", ecaDtMax);
    fprintf(stdout, "--- calc latency      : %d\n", aheadt - (int32_t)((double)ecaDtSum/(double)ecaNMessage));
  } // getECATap
  
  if (command) {
    if (!strcasecmp(command, "wrstatclear")) {
      if (optind+3  != argc) {printf("expecting exactly two arguments: wrstatclear  <tWrObs> <tStallObs>\n"); return 1;}
      contObsT = strtoul(argv[optind+1], &tail, 0);
      stallObsT = strtoul(argv[optind+2], &tail, 0);
      wb_wr_stats_reset(device, devIndex, contObsT, stallObsT);
    } // wrstatclear

    if (!strcasecmp(command, "ecatapclear")) {
      wb_eca_stats_reset(device, devIndex);
    } // ecatapclear

    if (!strcasecmp(command, "ecatapenable")) {
      wb_eca_stats_enable(device, devIndex, 0x1);
    } // ecatapenable

    if (!strcasecmp(command, "ecatapdisable")) {
      wb_eca_stats_enable(device, devIndex, 0x0);
    } // ecatapdisable

    
  } // if command
  
  wb_close(device, socket);
  wb_close(deviceOther, socket);
  
  return exitCode;
}

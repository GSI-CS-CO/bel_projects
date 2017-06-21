/******************************************************************************
 *  eb-mon.c (previously wr-mon.c)
 *
 *  created : 2015
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Dec-2016
 *
 * Command-line interface for WR monitoring via Etherbone.
 *
 * -------------------------------------------------------------------------------------------
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
 * Last update: 25-April-2015
 ********************************************************************************************/
#define EBMON_VERSION "1.2.0"

/* standard includes */
#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

/* Etherbone */
#include <etherbone.h>

/* Wishbone api */
#include <wb_api.h>

const char* program;
static int verbose=0;
eb_device_t device;        /* needs to be global for 1-wire stuff */


static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
}

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -b<busIndex>     display ID (ID of slave on the specified 1-wire bus)\n");
  fprintf(stderr, "  -d               display WR time\n");
  fprintf(stderr, "  -e               display etherbone version\n");
  fprintf(stderr, "  -f<familyCode>   specifiy family code of 1-wire slave (0x43: EEPROM; 0x28,0x42: temperature)\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "  -i               display WR IP\n");
  fprintf(stderr, "  -l               display WR link status\n");
  fprintf(stderr, "  -m               display WR MAC\n");
  fprintf(stderr, "  -o               display offset between WR time and system time\n");
  fprintf(stderr, "  -s               display WR sync status\n");
  fprintf(stderr, "  -t<busIndex>     display temperature of sensor on the specified 1-wire bus\n");
  fprintf(stderr, "  -v               display verbose information\n");
  fprintf(stderr, "  -w<index>        specify WB device in case multiple WB devices of the same type exist (default: 0)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to get some info about WR enabled hardware.\n");
  fprintf(stderr, "Example1: '%s -v dev/wbm0' display typical information.\n", program);
  fprintf(stderr, "Example2: '%s -b0 -f0x43 dev/wbm0' read ID of EEPROM connected to 1st 1-wire bus\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMON_VERSION);
}


int main(int argc, char** argv) {
  eb_status_t       status;
  eb_socket_t       socket;
  int               devIndex=-1; /* 0,1,2... - there may be more than 1 device on the WB bus */
  unsigned int      busIndex=-1;  /* index of 1-wire bus connected to a controller*/

  const char* devName;

  int         getEBVersion=0;
  int         getWRDate=0;
  int         getWROffset=0;
  int         getWRSync=0;
  int         getWRMac=0;
  int         getWRLink=0;
  int         getWRIP=0;
  int         getBoardID=0;
  int         getBoardTemp=0;
  int         exitCode=0;

  unsigned int family = 0;

  uint64_t    nsecs64;
  uint64_t    msecs64;
  uint64_t    hostmsecs64;
  int64_t     offset;
  uint64_t    mac;
  int         link;
  int         syncState;
  int         ip;
  uint64_t    id;
  double      temp;
  char linkStr[64];
  char syncStr[64];
  char timestr[60];
  time_t secs;
  const struct tm* tm;
  struct timeval htm;

  int opt, error=0;
  char *tail;

  program = argv[0];

  while ((opt = getopt(argc, argv, "t:w:f:b:dosmlievh")) != -1) {
    switch (opt) {
    case 'b':
      getBoardID=1;
      busIndex = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
      break;
    case 'd':
      getWRDate=1;
      break;
    case 'f':
      family = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
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
    case 's':
      getWRSync=1;
      break;
    case 't':
      getBoardTemp=1;
      busIndex = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
      break;
    case 'e':
      getEBVersion=1;
      break;
    case 'v':
      getWRDate=1;
      getWROffset=1;
      getWRSync=1;
      getWRMac=1;
      getWRLink=1;
      getWRIP=1;
      getEBVersion=1;
      verbose=1;
      break;
    case 'w':
      devIndex = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
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
    } /* switch opt */
  } /* while opt */

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
  if (devIndex < 0) devIndex = 0; /* default: grab first device of the requested type on the wishbone bus */
  
  if (getEBVersion) {
    if (verbose) fprintf(stdout, "EB version / EB source: ");
    fprintf(stdout, "%s / %s\n", eb_source_version(), eb_build_info());
  }

  /* open Etherbone device and socket */
  if ((status = wb_open(devName, &device, &socket)) != EB_OK) {
    fprintf(stderr, "can't open connection to device %s \n", devName);
    return (1);
  }

  if (getWRDate || getWROffset) {
    if ((status = wb_wr_get_time(device, devIndex, &nsecs64)) != EB_OK) die("WR get time", status);
    secs     = (unsigned long)((double)nsecs64 / 1000000000.0);
    msecs64  = nsecs64 / 1000000.0;

    if (getWROffset) {
      /* get system time */
      gettimeofday(&htm, NULL);
      hostmsecs64 = htm.tv_sec*1000 + htm.tv_usec/1000;
      offset      = msecs64 - hostmsecs64;
      if (verbose) fprintf(stdout, "WR_time - host_time [ms]: ");
      fprintf(stdout, "%ld\n", (long)offset);
    }

    if (getWRDate) {
      /* Format the date */
      tm = gmtime(&secs);
      strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %Z", tm);
      
      if (verbose) fprintf(stdout, "Current TAI:");
      fprintf(stdout, "%s\n", timestr);
    }
  }

  if (getWRSync) {
    if ((status = wb_wr_get_sync_state(device, devIndex, &syncState)) != EB_OK) die("WR get sync state", status);
    if ((syncState & 0x8))
      sprintf(syncStr,"TRACKING");
    else if ((syncState & 0x4))
      sprintf(syncStr,"TIME");
    else if ((syncState & 0x2))
      sprintf(syncStr, "PPS");
    else
      sprintf(syncStr, "NO SYNC");
    if (verbose) fprintf(stdout, "Sync Status: ");
    fprintf(stdout, "%s\n", syncStr);
  }

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
    fprintf(stdout, "%d.%d.%d.%d\n", (ip & 0xFF000000) >> 24, (ip & 0x00FF0000) >> 16, (ip & 0x0000FF00) >> 8, ip & 0x000000FF);
  }
  
  if (getBoardID) {
    if (!family) die("family code not specified (1-wire)", EB_OOM);
    if ((status = wb_wr_get_id(device, devIndex, busIndex, family, &id)) != EB_OK) die("WR get board ID", status);
    if (verbose) fprintf(stdout, "ID: ");
    fprintf(stdout, "0x%016"PRIx64"\n", id);
  }

 if (getBoardTemp) {
   if (!family) die("family code not specified (1-wire)", EB_OOM);
   if ((status = wb_wr_get_temp(device, devIndex, busIndex, family, &temp)) != EB_OK) die("WR get board temperature", status);
   if (verbose) fprintf(stdout, "temp: ");
   fprintf(stdout, "%.4f\n", (float)temp);
 } 

  wb_close(device, socket);
  
  return exitCode;
}

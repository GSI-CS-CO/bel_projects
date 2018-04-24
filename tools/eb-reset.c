/******************************************************************************
 *  eb-reset.c 
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Apr-2018
 *
 * Command-line interface for resetting a FPGA. This forces a restart using the image stored
 * in the local flash of the timing receiver.
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
 * Last update: 01-December-2017
 ********************************************************************************************/
#define EBRESET_VERSION "1.0.0"

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
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -e               display etherbone version\n");
  fprintf(stderr, "  -p<t>            after reset, wait for the specified time [s] and probe device\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  cpuhalt <cpu>    halts a user lm32 CPU\n");
  fprintf(stderr, "                   specify a single CPU (0..31) or all CPUs (0xff)\n");
  fprintf(stderr, "  cpureset <cpu>   reset a user lm32 CPU, firmware restarts.\n");
  fprintf(stderr, "                   specify a single CPU (0..31) or all CPUs (0xff)\n");
  fprintf(stderr, "  cpustatus        get the 'halt status' of all user lm32 (rightmost bit: CPU 0)\n");
  fprintf(stderr, "  fpgareset        recycles the entire FPGA (see below)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to reset a FPGA or lm32 user CPU(s).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "The command 'fpgareset' forces a restart of the entire FPGA using the image stored in the \n");
  fprintf(stderr, "flash of the device. Only use this command if the flash contains a valid image (otherwiese\n");
  fprintf(stderr, "your devices becomes bricked).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBRESET_VERSION);
}


int main(int argc, char** argv) {
  eb_status_t       status;
  eb_socket_t       socket;
  int               devIndex=-1;  // 0,1,2... - there may be more than 1 device on the WB bus

  const char* devName;
  const char* command;
  int         cmdExecuted=0;

  int         getEBVersion=0;
  int         probeAfterReset=0;
  int         waitTime=1;
  int         exitCode=0;
  uint32_t    nCPU;
  uint64_t    i;

  int opt, error=0;
  char *tail;

  int         ip;

  program = argv[0];

  while ((opt = getopt(argc, argv, "p:eh")) != -1) {
    switch (opt) {
    case 'p' :
      probeAfterReset=1;
      waitTime = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'e':
      getEBVersion=1;
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
  devIndex = 0; // default: grab first device of the requested type on the wishbone bus

  if (optind+1 < argc)  command = argv[++optind];
  else                  command = NULL;
  
  if (getEBVersion) {
    if (verbose) fprintf(stdout, "EB version / EB source: ");
    fprintf(stdout, "%s / %s\n", eb_source_version(), eb_build_info());
  }


  if (command) {

    // open Etherbone device and socket 
    if ((status = wb_open(devName, &device, &socket)) != EB_OK) {
      fprintf(stderr, "can't open connection to device %s \n", devName);
      return (1);
    }
    
    // prior reset, probe device by gettings its IP
    if ((status = wb_wr_get_ip(device, devIndex, &ip)) != EB_OK) die("eb-reset: can't to connect to device", status);

    // FPGA reset
    if (!strcasecmp(command, "fpgareset")) {
      cmdExecuted = 1;
      
      // depending on the device, the etherbone cycle either completes or times out
      status = wb_wr_reset(device, devIndex, 0xdeadbeef);
      if ((status != EB_TIMEOUT) && (status != EB_OK)) die("RESET FPGA", status);
      
      if (probeAfterReset) {
        //close, wait and reopen socket, try to read a property (here: ip) from the device
        
        if ((status = wb_close(device, socket)) != EB_OK) die ("Probe FPGA", status);
        sleep(waitTime);
        if ((status = wb_open(devName, &device, &socket)) != EB_OK) die("Probe FPGA", status);
        
        if ((status = wb_wr_get_ip(device, devIndex, &ip)) != EB_OK) die("Probe FPGA", status);
      } // probe after reset
    } // fpga reset

    // halt user CPU
    if (!strcasecmp(command, "cpuhalt")) {
      cmdExecuted = 1;
      if (optind+2  != argc) {printf("eb-reset: expecting exactly one argument: cpuhalt <cpu>\n"); return 1;}

      nCPU = strtoul(argv[optind+1], &tail, 0);
      status = wb_cpu_halt(device, devIndex, nCPU);
      if (status == EB_OOM) {printf("eb-reset: illegal value for <cpu>\n"); return 1;}
      if (status != EB_OK)  die("eb-reset: ", status);
    } // halt lm32 CPU

    // reset user CPU
    if (!strcasecmp(command, "cpureset")) {
      cmdExecuted = 1;
      if (optind+2  != argc) {printf("eb-reset: expecting exactly one argument: cpureset <cpu>\n"); return 1;}

      nCPU = strtoul(argv[optind+1], &tail, 0);
      status = wb_cpu_halt(device, devIndex, nCPU);
      if (status == EB_OOM) {printf("eb-reset: illegal value for <cpu>\n"); return 1;}
      if (status != EB_OK)  die("eb-reset: ", status);

      usleep(1000); // probably not required, but its better to be on the safe side
      
      status = wb_cpu_resume(device, devIndex, nCPU);
      if (status == EB_OOM) {printf("eb-reset: illegal value for <cpu>\n"); return 1;}
      if (status != EB_OK)  die("eb-reset: ", status);
    } // resume lm32 CPU

    // get user CPU halt state
    if (!strcasecmp(command, "cpustatus")) {
      cmdExecuted = 1;
      status = wb_cpu_status(device, devIndex, &nCPU);
      if (status != EB_OK)  die("eb-reset: ", status);

      printf("eb-reset: status of user lm32 ");
      for (i = 1 << 16; i > 0; i = i / 2) {if (nCPU & i) printf("1"); else printf("0");}
      printf("\n");
    } // get lm32 CPU status

    wb_close(device, socket);

    if (!cmdExecuted) printf("eb-reset: unknonwn command %s\n", command);
  } // if command
  
  return exitCode;
} // main

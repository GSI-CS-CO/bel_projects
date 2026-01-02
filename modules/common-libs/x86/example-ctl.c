/********************************************************************************************
 *  example-ctl.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-jan-2026
 *
 *  command-line interface for example firmware
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
 * Last update: 17-May-2017
 *********************************************************************************************/
#define EXAMPLE_X86_VERSION  "00.00.03"

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// Etherbone
#include <etherbone.h>

// common example
#include <common-lib.h>                  // common API
#include <common-defs.h>                 // FW
#include <example_shared_mmap.h>         // LM32

const char*  program;
static int   getInfo    = 0;
static int   getConfig  = 0;
static int   getVersion = 0;
static int   snoop      = 0;
static int   logLevel   = 0; 
eb_device_t  device;                     // keep this and below global
eb_address_t lm32_base;                  // base address of lm32

// application specific stuff
eb_address_t example_cmd;                // send commands to fw
// ... add more code here
 

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -c                  display configuration of EXAMPLE\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n");
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailled information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EXAMPLE_X86_VERSION);
} //help


const char* statusText(uint32_t bit) {  
  static char message[256];

  switch (bit) {
    // ... add code here
    // case EXAMPLE_STATUS_TRANSFER         : sprintf(message, "error %d, %s",    bit, "transfer failed"); break;
    default                              : sprintf(message, "%s",  comlib_statusText(bit)) ; break;
  } // switch

  return message;
} // statusText


int main(int argc, char** argv) {
  // eb stuff
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351
  eb_status_t         eb_status;
  eb_socket_t         socket;
  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores


  // CLI
  const char* devName;
  const char* command;
  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  // local variables
  uint64_t sumStatus;                     // status bits
  uint32_t state;                         // state of state machine
  uint32_t nBadStatus;                    // # of bad status incidents
  uint32_t nBadState;                     // # of bad state incidents
  uint32_t version;                       // fw version
  uint64_t tDiag;                         // time when diagnostics was cleared
  uint64_t tS0;                           // time when fw started
  uint64_t mac;                           // mac for config of EB master
  uint32_t ip;                            // ip for config of EB master
  uint32_t nTransfer;                     // # of transfers
  uint32_t nInjection;                    // # of injections
  uint32_t statTrans;                     // status bits of transfer (application specific)
  uint32_t nLate;                         // number of messages that could not be delivered in time
  uint32_t offsDone;                      // offset event deadline to time when we are done [ns]
  uint32_t comLatency;                    // latency for messages received from via ECA (tDeadline - tNow)) [ns]
  uint32_t usedSize;                      // used size of shared memory
                                          // '1' print information to stdout                                                                                                               
  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:ceih")) != -1) {
    switch (opt) {
    case 'c':
      getConfig = 1;
      break;
    case 'e':
      getVersion = 1;
      break;
    case 'i':
      getInfo = 1;
      break;
    case 's':
      snoop = 1;
      logLevel = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } /* if *tail */
      if ((logLevel < COMMON_LOGLEVEL_ALL) || (logLevel > COMMON_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
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

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;
  /* open Etherbone device and socket */
  if ((eb_status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) die("eb_socket_open", eb_status);
  if ((eb_status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK) die("eb_device_open", eb_status);

  /* get device Wishbone address of lm32 */
  nDevices = 1; // quick and dirty, use only first core
  if ((eb_status = eb_sdb_find_by_identity(device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) die("find lm32", eb_status);
  lm32_base =  sdbDevice.sdb_component.addr_first;

  // init addresses to DP RAM
  comlib_initShared(lm32_base, SHARED_OFFS);
  example_cmd = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  // ... add more code here

  if (getVersion) {
    comlib_readDiag(device, &sumStatus, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);
    printf("example: software (firmware) version %s (%06x)\n",  EXAMPLE_X86_VERSION, version);     
  } // if getEBVersion

  if (command) {
    // state required to give proper warnings
    comlib_readDiag(device, &sumStatus, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("example: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP  , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("example: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP   , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("example: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER  , 0, eb_block);
      if (state != COMMON_STATE_ERROR) printf("example: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE     , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("example: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"

    // diagnostics
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, example_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("example: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      comlib_readDiag(device, &sumStatus, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 1);
    } // "diag"

  } //if command

  // close Etherbone device and socket
  if ((eb_status = eb_device_close(device)) != EB_OK) die("eb_device_close", eb_status);
  if ((eb_status = eb_socket_close(socket)) != EB_OK) die("eb_socket_close", eb_status);

  return exitCode;
}

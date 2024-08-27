/*******************************************************************************************
 *  b2b-ctl.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 20-Aug-2024
 *
 * Command-line interface for b2b
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

// b2b
#include <common-lib.h>                  // COMMON
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW
#include <b2bcbu_shared_mmap.h>          // LM32

const char* program;
static int getInfo    = 0;
static int getVersion = 0;
static int ddsValue   = 0;
static int snoop      = 0;
static int logLevel   = 0;

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  uint32_t version;
  
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on B2B\n");
  fprintf(stderr, "  -s<n>               snoop ... for information continuously\n");
  fprintf(stderr, "                      0: print all messages (default)\n");
  fprintf(stderr, "                      1: as 0, once per second\n");
  fprintf(stderr, "                      2: as 1, inform in case of status or state changes\n");
  fprintf(stderr, "                      3: as 2, inform in case of state changes\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n");
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailed information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  submit              force a submit of values within shared memory (written by other programs)\n");
  fprintf(stderr, "  clearconfig         clears configuration data for all (0x0..0xf) Sequence IDs\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Tip: For using negative values with commands such as 'snoop', consider\n");
  fprintf(stderr, "using the special argument '--' to terminate option scanning.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control B2B from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "b2b:        nTrans |                 INFO\n");
  fprintf(stderr, "b2b: STATUS      n |   state      nchng     stat      nchng\n");
  fprintf(stderr, "b2b:    0000065325 | OpReady    (     1), status 0x00000001 (     0)\n");
  fprintf(stderr, "                 ' |       '          '                   '       '\n");
  fprintf(stderr, "                 ' |       '          '                   '       '- # of bad status incidents\n");
  fprintf(stderr, "                 ' |       '          '                   '- status (bitwise ored)\n");
  fprintf(stderr, "                 ' |       '          '- # of bad state incidents\n");
  fprintf(stderr, "                 ' |       '- state\n");
  fprintf(stderr, "                 '- # of transfers\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  b2b_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(version));
} //help


void printTransferHeader()
{
  printf("b2b:        nTrans |      DIAG       |                 INFO                  \n");
  printf("b2b: STATUS      n |  gid  sid  mode |   state      nchng     stat      nchng\n");
} // printTransferHeader


void printTransfer(uint32_t nTransfer, uint32_t sid, uint32_t gid, uint32_t mode)
{
  // diag
  printf("b2b:    %010u | %4x   %2u     %1u |", nTransfer, gid, sid, mode);
} // printTransfer


int main(int argc, char** argv) {
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint64_t statusArray;
  uint64_t actStatusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t nTransfer;
  uint32_t getsid;                             // SID
  uint32_t getgid;                             // GID 
  uint32_t getmode;                            // mode
  uint64_t getTH1Ext;                          // h=1 period [as] of extraction machine
  uint64_t getTH1Inj;                          // h=1 period [as] of injection machine
  uint32_t getnHExt;                           // harmonic number extraction machine
  uint32_t getnHInj;                           // harmonic number injection machine
  double   getcPhase;                          // phase correction
  double   getcTrigExt;                        // trigger correction extraction
  double   getcTrigInj;                        // trigger correction injection
  uint64_t getTBeat;                           // period [as] of frequency beating

  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t actNTransfer;                       // actual number of transfers
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing
  uint32_t verLib;
  uint32_t verFw;

  int      i;

  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;


  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:eihd")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'd':
        ddsValue = 1;
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

  if ((status =  b2b_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);
  
  if (getVersion) {
    b2b_version_library(&verLib);
    printf("b2b: library (firmware) version %s",  b2b_version_text(verLib));     
    b2b_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  b2b_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status
    b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, 0);
    b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    printTransferHeader();
    printTransfer(nTransfer, getsid, getgid, getmode);
    printf(", %s (%6u), ", b2b_state_text(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", b2b_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      b2b_cmd_configure(ebDevice);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("b2b: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      b2b_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      b2b_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      b2b_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("b2b: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      b2b_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      b2b_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", b2b_status_text(i));
      } // for i
      b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, 1);
    } // "diag"

    if (!strcasecmp(command, "submit")) {
      b2b_cmd_submit(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "submit"
    
    if (!strcasecmp(command, "clearconfig")) {
      b2b_cmd_clearConfig(ebDevice);      
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "clearconfig"
  } //if command

if (snoop) {
    printf("b2b: continous monitoring of gateway, loglevel = %d\n", logLevel);

    actNTransfer   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    printTransferHeader();

    while (1) {
      b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);
      switch(state) {
      case COMMON_STATE_OPREADY :
        if (actNTransfer != nTransfer) sleepTime = COMMON_DEFAULT_TIMEOUT * 1000 * 2;        // ongoing transfer: reduce polling rate ...
        else                           sleepTime = COMMON_DEFAULT_TIMEOUT * 1000;            // sleep for default timeout to catch next REQ_TK
        break;
      default:
        sleepTime = COMMON_DEFAULT_TIMEOUT * 1000;                          
      } // switch actState
      
      // determine when to print info
      printFlag = 0;

      if ((actState       != state)        && (logLevel <= COMMON_LOGLEVEL_STATE))   {printFlag = 1; actState       = state;}
      if ((actStatusArray != statusArray)  && (logLevel <= COMMON_LOGLEVEL_STATUS))  {printFlag = 1; actStatusArray = statusArray;}
      if ((actNTransfer   != nTransfer)    && (logLevel <= COMMON_LOGLEVEL_ONCE))    {printFlag = 1; actNTransfer   = nTransfer;}

      if (printFlag) {
        b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, 0);
        printTransfer(nTransfer, getsid, getgid, getmode); 
        printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", b2b_status_text(i));
        } // for i
      } // if printFlag

      fflush(stdout);                                                                         // required for immediate writing (if stdout is piped to syslog)

      //sleep 
      usleep(sleepTime);
    } // while
  } // if snoop

  // close connection to firmware
  if ((status = b2b_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

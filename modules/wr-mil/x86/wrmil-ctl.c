/*******************************************************************************************
 *  wrmil-ctl.c
 *
 *  created : 2014
 *  author  : Dietrich Beck, Michael Reese GSI-Darmstadt
 *  version : 02-Feb-2025
 *
 * Command-line interface for wr-mil
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

// wr-mil
#include <common-lib.h>                  // COMMON
//#include <wrmillib.h>                    // API
#include <wr-mil.h>                      // FW
#include <wrmil_shared_mmap.h>           // LM32

const char* program;
static int getInfo    = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  uint32_t version;
  
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> <wb address MIL> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n"                                );
  fprintf(stderr, "  -e                  display version\n"                                           );
  fprintf(stderr, "  -i                  show gateway information. Repeat the option"                 );
  fprintf(stderr, "                      to get more detailed information, e.g. -iii"                 );
  fprintf(stderr, "  -R                  read register content"                                       );
  fprintf(stderr, "  -m                  start monitoring loop"                                       );
  fprintf(stderr, "  -g                  show received MIL events in monitoring loop"                 );
  fprintf(stderr, "  -b                  bugfix mode: show relevent WR-Events together"               );
  fprintf(stderr, "                                   with events that trigger MIL-generation"        );
  fprintf(stderr, "                                   together with snooped MIL events"               );
  fprintf(stderr, "  -H                  show MIL-event histogram"                                    );
  fprintf(stderr, "  -r                  Pause gateway for 1 s, and reset"                            );
  fprintf(stderr, "  -s <MIL domain>     start WR-MIL gateway. 'MIL domain' can be"                   );
  fprintf(stderr, "                      0: PZU-QR; UNILAC, Source Right"                             );
  fprintf(stderr, "                      1: PZU-QL; UNILAC, Source Left"                              );     
  fprintf(stderr, "                      2: PZU-QN; UNILAC, Source High Charge State Injector (HLI)"  );
  fprintf(stderr, "                      3: PZU-UN; UNILAC, High Charge State Injector (HLI)"         );
  fprintf(stderr, "                      4: PZU-UH; UNILAC, High Current Injector (HSI)"              );
  fprintf(stderr, "                      5: PZU-AT; UNILAC, Alvarez Cavities"                         );
  fprintf(stderr, "                      6: PZU-TK; UNILAC, Transfer Line"                            );
  fprintf(stderr, "                      7: PZ-SIS18"                                                 );
  fprintf(stderr, "                      8: PZ-ESR"                                                   );
  fprintf(stderr, "  -t <trigger>        Set UTC-trigger event [0..255]"                              );
  fprintf(stderr, "  -o <offset>         Set UTC-offset [s] (value is added to WR-time)"              );
  fprintf(stderr, "  -d <delay>          Set Trigger-UTC delay [us]"                                  );
  fprintf(stderr, "  -u <delay>          Set UTC-UTC delay [us]"                                      );
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
  fprintf(stderr, "Tip: For using negative values with commands such as 'snoop', consider\n");
  fprintf(stderr, "using the special argument '--' to terminate option scanning.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control the wr-mil gateway from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 0x4711 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  wrmil_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", wrmil_version_text(version));
} //help

/*
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


void printDiags(uint32_t sid, uint32_t gid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj, uint64_t TBeat, double cPhase, double cTrigExt, double cTrigInj, int32_t comLatency)
{
  printf("\n\n");
  printf("b2b: statistics ...\n\n");

  printf("GID                   : %012u\n"     , gid);
  printf("SID                   : %012u\n"     , sid);
  printf("mode                  : %012u\n"     , mode);
  printf("period h=1 extraction : %012.6f ns\n", (double)TH1Ext/1000000000.0);
  printf("period h=1 injection  : %012.6f ns\n", (double)TH1Inj/1000000000.0);
  printf("harmonic number extr. : %012d\n"     , nHExt);
  printf("harmonic number inj.  : %012d\n"     , nHInj);
  printf("period of beating     : %012.6f us\n", (double)TBeat/1000000000000.0);
  printf("corr. matching        : %012.3f\n"   , cPhase);
  printf("corr. trigger extr    : %012.3f\n"   , cTrigExt);
  printf("corr. trigger inj     : %012.3f\n"   , cTrigInj);
  printf("communication latency : %012.3f us\n", (double)comLatency/1000.0);
} // printDiags
*/

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
  int32_t  getcomLatency;                      // message latency from ECA

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

  int      mil_latency    = -1;
  int      utc_trigger    = -1;
  int      trig_utc_delay = -1;
  
    
  


  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:t:o:d:u:heiRmgbHr")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'R':
        getRegister = 1;
        break;
      case 'm':
        printf("monitoring loop not yet implemented; use saft-ctl instead\n");
        return 0;
      case 'g':
        printf("show received MIL events not yet implemented\n");
        return 0;
      case 'b':
        printf("bugfix mode node yet implemented\n");
        return 0;
      case 'H':
        printf("MIL-event histogram not yet implemented\n");
        return 0;
      case 'r':
        printf("reset not yet implemented\n");
        return 0;
      case 'l':
        mil_latency = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1);
        break;
      case 't':
        utc_trigger = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1);
        break;
      case 'o':
        utc_offset = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1);
        break;
      case 'd':
        trig_utc_delay = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1);
        break;
      case 'u':
        utc_utc_delay = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1);
        break;
        /*      case 's':
        snoop = 1;
        logLevel = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        if ((logLevel < COMMON_LOGLEVEL_ALL) || (logLevel > COMMON_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
        break;*/
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
    b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, &getcomLatency, 0);
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
      b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, &getcomLatency, 0);
      printDiags(getsid, getgid, getmode, getTH1Ext, getnHExt, getTH1Inj, getnHInj, getTBeat, getcPhase, getcTrigExt, getcTrigInj, getcomLatency);
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
        b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, &getcomLatency, 0);
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

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
#include <wrmillib.h>                    // API
#include <wr-mil.h>                      // FW
#include <wrmil_shared_mmap.h>           // LM32

const char* program;
static int getInfo     = 0;
static int getRegister = 0;
static int getVersion  = 0;
static int snoop       = 0;
static int logLevel    = 0;

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  uint32_t version;
  
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n"                                );
  fprintf(stderr, "  -e                  display version\n"                                           );
  fprintf(stderr, "  -i                  show gateway information. Repeat the option\n"               );
  fprintf(stderr, "                      to get more detailed information, e.g. -iii\n"               );
  fprintf(stderr, "  -R                  read register content\n"                                     );
  fprintf(stderr, "  -m                  start monitoring loop\n"                                     );
  fprintf(stderr, "  -g                  show received MIL events in monitoring loop\n"               );
  fprintf(stderr, "  -b                  bugfix mode: show relevent WR-Events together\n"             );
  fprintf(stderr, "                                   with events that trigger MIL-generation\n"      );
  fprintf(stderr, "                                   together with snooped MIL events\n"             );
  fprintf(stderr, "  -H                  show MIL-event histogram\n"                                  );
  fprintf(stderr, "  -r                  Pause gateway for 1 s, and reset\n"                          );
  fprintf(stderr, "\n");
  fprintf(stderr, "The following parameters are to be used with command 'configure' \n"               );
  fprintf(stderr, "with command 'configure', parameters -w and -s are mandatory\n"                    );
  fprintf(stderr, "  -w <MIL WB addr>    wishbone address of MIL device\n"                            );
  fprintf(stderr, "  -s <MIL domain>     MIL domain; this can be\n"                                   );
  fprintf(stderr, "                      0: PZU-QR; UNILAC, Source Right\n"                           );
  fprintf(stderr, "                      1: PZU-QL; UNILAC, Source Left\n"                            );     
  fprintf(stderr, "                      2: PZU-QN; UNILAC, Source High Charge State Injector (HLI)\n");
  fprintf(stderr, "                      3: PZU-UN; UNILAC, High Charge State Injector (HLI)\n"       );
  fprintf(stderr, "                      4: PZU-UH; UNILAC, High Current Injector (HSI)\n"            );
  fprintf(stderr, "                      5: PZU-AT; UNILAC, Alvarez Cavities\n"                       );
  fprintf(stderr, "                      6: PZU-TK; UNILAC, Transfer Line\n"                          );
  fprintf(stderr, "                      7: PZ-SIS18\n"                                               );
  fprintf(stderr, "                      8: PZ-ESR\n"                                                 );
  fprintf(stderr, "  -t <trigger>        evtNo of UTC-trigger event [0..255], default 0xf6\n"         );
  fprintf(stderr, "  -o <offset>         UTC-offset [s]                     , default yr 2008\n"      );
  fprintf(stderr, "  -d <delay>          Set Trigger-UTC delay [us]         , default 0\n"            );
  fprintf(stderr, "  -u <delay>          Set UTC-UTC delay [us]             , default 30\n"           );
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "                      'configure' requires parameters -w, -s\n");
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
  uint32_t getUtcTrigger;                      // the MIL event that triggers the generation of UTC events
  uint32_t getUtcDelay;                        // delay [us] between the 5 generated UTC MIL events
  uint32_t getTrigUtcDelay;                    // delay [us] between the trigger event and the first UTC (and other) generated events
  uint32_t getGid;                             // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
  int32_t  getLatency;                         // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
  uint64_t getUtcOffset;                       // delay [ms] between the TAI and the MIL-UTC, high word   
  uint32_t getRequestFill;                     // if this is written to 1, the gateway will send a fill event as soon as possible
  uint32_t getMilDevAddr;                      // wishbone address of MIL device; MIL device could be a MIL piggy or a SIO
  uint64_t getNumEvts;                         // number of translated events from WR to MIL
  uint32_t getLateEvts;                        // number of translated events that could not be delivered in time
  uint32_t getComLatency;                      // latency for messages received from via ECA (tDeadline - tNow)) [ns]
                     


  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t actNTransfer;                       // actual number of transfers
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing
  uint32_t verLib;
  uint32_t verFw;

  int      i;
  int      tmp;

  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  uint32_t  utc_trigger    = WRMIL_DFLT_UTC_TRIGGER;
  int32_t   utc_utc_delay  = WRMIL_DFLT_UTC_UTC_DELAY;     
  int32_t   trig_utc_delay = WRMIL_DFLT_TRIG_UTC_DELAY;
  uint64_t  utc_offset     = WRMIL_DFLT_UTC_OFFSET;
  uint32_t  mil_latency    = WRMIL_DFLT_LATENCY;
  uint32_t  mil_domain     = -1;
  uint32_t  mil_wb_addr    = -1;

  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:t:o:d:u:w:heiRmgbHr")) != -1) {
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
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 't':
        utc_trigger = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 'o':
        utc_offset = strtoull(optarg, &tail, 0) * 1000; // convert to ms
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 'd':
        trig_utc_delay = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 'u':
        utc_utc_delay = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 'w':
        mil_wb_addr   = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        break;
      case 's':
        tmp           = strtoull(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        switch (tmp) {
          case 0: mil_domain = 0x1c0; break;
          case 1: mil_domain = 0x1c1; break;
          case 2: mil_domain = 0x1c2; break;
          case 3: mil_domain = 0x1c3; break;
          case 4: mil_domain = 0x1c4; break;
          case 5: mil_domain = 0x1c5; break;
          case 6: mil_domain = 0x1c6; break;
          case 7: mil_domain = 0x12c; break;
          case 8: mil_domain = 0x154; break;
          default: fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1; 
        } // switch tmp
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
    fprintf(stderr, "%s: expecting non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;

  if ((status =  wrmil_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);
  
  if (getVersion) {
    wrmil_version_library(&verLib);
    printf("b2b: library (firmware) version %s",  wrmil_version_text(verLib));     
    wrmil_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  wrmil_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status
    wrmil_info_read(ebDevice, &getUtcTrigger, &getUtcDelay, &getTrigUtcDelay, &getGid, &getLatency, &getUtcOffset, &getRequestFill, &getMilDevAddr, &getNumEvts, &getLateEvts, &getComLatency, 0);
    wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrmil_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      if (mil_domain  == -1) {fprintf(stderr, "parameter -s is non-optional\n"); return 1;}
      if (mil_wb_addr == -1) {fprintf(stderr, "parameter -w is non-optional\n"); return 1;}
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("wr-mil: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
      else {
        wrmil_upload(ebDevice, utc_trigger, utc_utc_delay, trig_utc_delay, mil_domain, mil_latency, utc_offset, 0, mil_wb_addr);
        wrmil_cmd_configure(ebDevice);
      } // else state
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      wrmil_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      wrmil_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      wrmil_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("b2b: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      wrmil_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      wrmil_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", wrmil_status_text(i));
      } // for i
      wrmil_info_read(ebDevice, &getUtcTrigger, &getUtcDelay, &getTrigUtcDelay, &getGid, &getLatency, &getUtcOffset, &getRequestFill, &getMilDevAddr, &getNumEvts, &getLateEvts, &getComLatency, 1);
    } // "diag"
  } //if command

if (snoop) {
    printf("wr-mil: continous monitoring of gateway, loglevel = %d\n", logLevel);

    actNTransfer   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    while (1) {
      wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);
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
        wrmil_info_read(ebDevice, &getUtcTrigger, &getUtcDelay, &getTrigUtcDelay, &getGid, &getLatency, &getUtcOffset, &getRequestFill, &getMilDevAddr, &getNumEvts, &getLateEvts, &getComLatency, 0);
        printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrmil_status_text(i));
        } // for i
      } // if printFlag

      fflush(stdout);                                                                         // required for immediate writing (if stdout is piped to syslog)

      //sleep 
      usleep(sleepTime);
    } // while
  } // if snoop

  // close connection to firmware
  if ((status = wrmil_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

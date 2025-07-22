/*******************************************************************************************
 *  wrf50-ctl.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, Michael Reese GSI-Darmstadt
 *  version : 22-Jul-2025
 *
 * Command-line interface for wr-f50
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
#include <wr-f50.h>                      // FW
#include <wrmil_shared_mmap.h>           // LM32

const char* program;
static int getInfo     = 0;
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
  fprintf(stderr, "  -h                  display this help and exit\n"                                 );
  fprintf(stderr, "  -e                  display version\n"                                            );
  fprintf(stderr, "  -i                  show 50 Hz sync information\n"                                );
  fprintf(stderr, "\n");
  fprintf(stderr, "All following parameters are to be used with command 'configure' \n"                );
  fprintf(stderr, "  -o <offset>         offset [us] to zero transition of 50 Hz mains, default 0\n"   );
  fprintf(stderr, "                      'offset': t_cycle_mains - t_cycle_DM and must be positive\n"  );
  fprintf(stderr, "  -m <mode>           mode selection, default 1 \n"                                 );
  fprintf(stderr, "                      0: off\n"                                                     );
  fprintf(stderr, "                      2: hard locking, Data Master simulation mode\n"               );
  fprintf(stderr, "                      4: hard locking of Data Master\n"                             );
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n"   );
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n"                      );
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailed information\n"                  );
  fprintf(stderr, "  cleardiag           command clears FW statistics\n"                               );
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control the UNILAC 50 Hz synchronization from the command line\n"  );
  fprintf(stderr, "Example1: '%s dev/wbm0 -m 3 configure'\n", program                                  );
  fprintf(stderr, "\n");
  fprintf(stderr, "For debugging, some messages are available via the ECA with gid 0x4c0. EvtNo:\n"    );
  fprintf(stderr, "    0xa01: rising and falling edge of HW signal from 50 Hz module\n"                );
  fprintf(stderr, "    0xfc0: start of Data Master cycle\n"                                            );
  fprintf(stderr, "    0xfc1: message to Data Master; set-value of cycle length param field\n"         );
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  wrmil_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", wrmil_version_text(version));
} //help

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

  int32_t  f50Offset        = WR50_DFLT_F50OFFSET;
  uint32_t mode             = WRF50_MODE_LOCK_HARD_SIM;

  uint32_t TMainsAct;
  uint32_t TDmAct;
  uint32_t TDmSet;
  int32_t  offsDmAct;
  int32_t  offsDmMin;
  int32_t  offsDmMax;
  int32_t  dTDMAct;
  int32_t  dTDMMin;
  int32_t  dTDMMax;
  int32_t  offsMainsAct;
  int32_t  offsMainsMin;
  int32_t  offsMainsMax;
  uint32_t lockState;
  uint64_t lockDate;
  uint32_t nLocked;
  uint32_t nCycles;
  uint32_t nSent; 
 
  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:o:m:ghei")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'o':
        f50Offset = strtol(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        // we must make sure the message still arrives in the actual UNILAC cycle; thus the offset must be smaller than
        // - the minimum cycle length
        // - minus the offset for sending the message
        // - minus the critical limit for late messages
        if (f50Offset > (WRF50_CYCLELEN_MIN - WRF50_TUNE_MSG_DELAY - COMMON_LATELIMIT)) {fprintf(stderr, "Parameter o: %d out of range\n", f50Offset); return 1;}
        break;
      case 'm':
        tmp           = strtol(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
        switch (tmp) {
          case 0: mode = WRF50_MODE_OFF;               break;
          case 1: fprintf(stderr, "option not implemented\n"); return 1;
          case 2: mode = WRF50_MODE_LOCK_HARD_SIM;     break;
          case 3: fprintf(stderr, "option not implemented\n"); return 1;
          case 4: mode = WRF50_MODE_LOCK_HARD_DM;      break;
          case 5: fprintf(stderr, "option not implemented\n"); return 1;
          default: fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1; 
        } // switch tmp
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
    fprintf(stderr, "%s: expecting non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;

  if ((status =  wrf50_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  if (getVersion) {
    wrmil_version_library(&verLib);
    printf("wr-f50: library (firmware) version %s",  wrmil_version_text(verLib));     
    wrmil_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  wrmil_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status

    wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);
    wrf50_info_read(ebDevice, &f50Offset, &mode, &TMainsAct, &TDmAct, &TDmSet, &offsDmAct, &offsDmMin, &offsDmMax, &dTDMAct, &dTDMMin, &dTDMMax,
                    &offsMainsAct, &offsMainsMin, &offsMainsMax, &lockState, &lockDate, &nLocked, &nCycles, &nSent, 0);

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
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("wr-f50: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
      else {
        if (wrf50_upload(ebDevice, f50Offset, mode) != COMMON_STATUS_OK) die("wr-f50 upload", status);
        wrmil_cmd_configure(ebDevice);
      } // else state
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      wrmil_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-f50: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      wrmil_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("wr-f50: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      wrmil_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("wr-f50: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      wrmil_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-f50: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      wrmil_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("wr-f50: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      wrmil_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", wrmil_status_text(i));
      } // for i
      wrf50_info_read(ebDevice, &f50Offset, &mode, &TMainsAct, &TDmAct, &TDmSet, &offsDmAct, &offsDmMin, &offsDmMax, &dTDMAct, &dTDMMin, &dTDMMax,
                      &offsMainsAct, &offsMainsMin, &offsMainsMax, &lockState, &lockDate, &nLocked, &nCycles, &nSent, 1);
    } // "diag"    
  } //if command

if (snoop) {
    printf("wr-f50: continous monitoring of gateway, loglevel = %d\n", logLevel);

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
        wrf50_info_read(ebDevice, &f50Offset, &mode, &TMainsAct, &TDmAct, &TDmSet, &offsDmAct, &offsDmMin, &offsDmMax, &dTDMAct, &dTDMMin, &dTDMMax,
                        &offsMainsAct, &offsMainsMin, &offsMainsMax, &lockState, &lockDate, &nLocked, &nCycles, &nSent, 0);
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
  if ((status = wrf50_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

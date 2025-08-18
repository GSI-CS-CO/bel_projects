/*******************************************************************************************
 *  uniblm-ctl.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, Tobias Habermann GSI-Darmstadt
 *  version : 07-Nov-2024
 *
 * Command-line interface for uni-blm
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
#include <time.h>

// Etherbone
#include <etherbone.h>

// uni-blm
#include <common-lib.h>                  // COMMON
#include <uniblmlib.h>                  // API
#include <uni-blm.h>                    // FW
#include <uniblm_shared_mmap.h>         // LM32

const char* program;
static int getInfo     = 0;
static int getVersion  = 0;

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
  fprintf(stderr, "  -i                  show blm information\n"                                       );
  fprintf(stderr, "  -w <eventKey>       set parameter eventKey\n"                                           );
  fprintf(stderr, "                      \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n"   );
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n"                      );
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailed information\n"                  );
  fprintf(stderr, "  cleardiag           command clears FW statistics\n"                               );
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control (some) properties of the UNILAC blm from the command line\n"  );
  fprintf(stderr, "Example:\n"                                                                         );
  fprintf(stderr, "  '%s dev/wbm0 startop'\n",       program                                           );
  fprintf(stderr, "  '%s dev/wbm0 diag'\n",          program                                           );
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <some.one@gsi.de>\n");

  uniblm_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", uniblm_version_text(version));
} //help

int main(int argc, char** argv) {
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint64_t statusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t nTransfer;
  uint32_t nLate;
          
  uint32_t verLib;
  uint32_t verFw;

  int      i;

  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  uint32_t setEventKey;
  uint32_t getReloadCounter;
   
  program = argv[0];
  setEventKey = 42;
  

  while ((opt = getopt(argc, argv, "w:hei")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'w':
        setEventKey = strtol(optarg, &tail, 0);
        if (*tail != 0) {fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg); return 1;}
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

  if ((status =  uniblm_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  if (getVersion) {
    uniblm_version_library(&verLib);
    printf("uni-blm: library (firmware) version %s",  uniblm_version_text(verLib));     
    uniblm_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  uniblm_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status

    uniblm_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, &nLate, 0);

    uniblm_info_read(ebDevice, &getReloadCounter, 0);
    
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", uniblm_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    uniblm_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, &nLate, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("uni-blm: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
      else {
        if (uniblm_upload(ebDevice, setEventKey) != COMMON_STATUS_OK) die("uni-blm upload", status);
        uniblm_cmd_configure(ebDevice);
      } // else state
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      uniblm_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("uni-blm: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      uniblm_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("uni-blm: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      uniblm_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("uni-blm: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      uniblm_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("uni-blm: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      uniblm_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("uni-blm: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      uniblm_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, &nLate, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", uniblm_status_text(i));
      } // for i

      uniblm_info_read(ebDevice, &getReloadCounter, 1);
    } // "diag"    
  } //if command

  // close connection to firmware
  if ((status = uniblm_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

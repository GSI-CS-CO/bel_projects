/*******************************************************************************************
 *  unichop-ctl.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, Tobias Habermann GSI-Darmstadt
 *  version : 12-Sep-2024
 *
 * Command-line interface for uni-chop
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

// uni-chop
#include <common-lib.h>                  // COMMON
#include <unichoplib.h>                  // API
#include <uni-chop.h>                    // FW
#include <unichop_shared_mmap.h>         // LM32

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
  fprintf(stderr, "  -i                  show chopper information\n"                                   );
  fprintf(stderr, "\n");
  fprintf(stderr, "All following parameters are to be used with command 'configure' \n"                );
  fprintf(stderr, "  -w <MIL device>     MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO\n");
  fprintf(stderr, "                      \n"                                                           );
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n"   );
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n"                      );
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailed information\n"                  );
  fprintf(stderr, "  cleardiag           command clears FW statistics\n"                               );
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control (some) properties of the UNILAC chopper from the command line\n"  );
  fprintf(stderr, "Example1: '%s dev/wbm0 -w1 configure'\n", program                                   );
  fprintf(stderr, "\n");
  fprintf(stderr, "For testing and debugging timing messages can be sent / received via saftlib\n"     );
  fprintf(stderr, "GID:\n"                                                                             );
  fprintf(stderr, "      0xff0: group for timing messages exchanged between host and lm32\n"           );
  fprintf(stderr, "      0xff1: group for timing messages carrying diagnostic information\n"           );
  fprintf(stderr, "EvtNo:\n"                                                                           );
  fprintf(stderr, "      0xfa0: host -> lm32: write Strahlweg data\n"                                  );
  fprintf(stderr, "      0xfa1: host -> lm32: read Strahlweg data\n"                                   );
  fprintf(stderr, "      0xfa2: host -> lm32: write RPG data (write only)\n"                           );
  fprintf(stderr, "      0xfa2: host -> lm32: write RPG data (write only)\n"                           );
  fprintf(stderr, "      0xfa6: host -> lm32: general MIL write\n"                                     );
  fprintf(stderr, "      0xfa7: host -> lm32: general MIL read\n"                                      );
  fprintf(stderr, "      0xfa8: lm32 -> ECA (host): diagnostics, MIL write\n"                          );
  fprintf(stderr, "      0xfa9: lm32 -> ECA (host): diagnostics, MIL read\n"                           );
  fprintf(stderr, "      0xfaa: lm32 -> ECA (host): diagnostics, Strahlweg data in chopper unit\n"     );
  fprintf(stderr, "Examles\n"                                                                          );
  fprintf(stderr, "write/read host<->lm32<->MIL<->Chopper unit:\n"                                     );
  fprintf(stderr, "saft-ctl tr0 -p inject 0x1ff0fa0000000000 0x00000000ffff0001 0\n"                   );
  fprintf(stderr, "      write Strahlweg data to HW                        ^^^^: strahlweg_register\n" );
  fprintf(stderr, "                                                    ^^^^    : strahlweg_maske\n"    );
  fprintf(stderr, "saft-ctl tr0 -p inject 0x1ff0fa1000000000 0x0000000000000000 0\n"                   );
  fprintf(stderr, "      read Strahlweg data from HW\n"                                                );
  fprintf(stderr, "\n");
  fprintf(stderr, "saft-ctl tr0 -p inject 0x1ff0fa2000000000 0x0806080608020802 0\n"                   );
  fprintf(stderr, "      write RPG data to HW                                ^^: IRQ start event\n"    );
  fprintf(stderr, "                                                        ^^  : IRQ stop event\n"     );
  fprintf(stderr, "                                                      ^^    : IRL start event\n"    );
  fprintf(stderr, "                                                    ^^      : IQL stop event\n"     );
  fprintf(stderr, "                                                  ^^        : HLI start event\n"    );
  fprintf(stderr, "                                                ^^          : HLI stop event\n"     );
  fprintf(stderr, "                                              ^^            : HSI start event\n"    );
  fprintf(stderr, "                                            ^^              : HSI stop event\n"     );
  fprintf(stderr, "data from lm32:\n"                                                                  );
  fprintf(stderr, "saft-ctl tr0 -x snoop 0x1ff0000000000000 0xffff000000000000 0\n"                    );
  fprintf(stderr, "tDeadline: 0x17f6a44700881490 EvtID: 0x1ff0faa000000000 Param: 0x0000000000ff0001\n");
  fprintf(stderr, "     Strahlweg data from HW                strahlweg_register:               ^^^^\n");
  fprintf(stderr, "                                           strahlweg_maske   :           ^^^^\n"    );
  fprintf(stderr, "MIL bus diagnostic:\n"                                                              );
  fprintf(stderr, "saft-ctl tr0 -x snoop 0x1ff1000000000000 0xffff000000000000 0\n"                    );
  fprintf(stderr, "tDeadline: 0x17f6a4794b1c5228 EvtID: 0x1ff1fa8000000000 Param: 0x0001016009600001\n");
  fprintf(stderr, "     MIL diagnostic (strahlweg reg  write)  MIL error code   :   ^^^^\n"            );
  fprintf(stderr, "                                            SIO slot         :       ^^\n"          );
  fprintf(stderr, "                                            MIL ifb addr     :         ^^\n"        );
  fprintf(stderr, "                                            module addr      :           ^^\n"      );
  fprintf(stderr, "                                            submodule addr   :             ^^\n"    );
  fprintf(stderr, "                                            data             :               ^^^^\n");
  fprintf(stderr, "tDeadline: 0x17f6a4794b1c7278 EvtID: 0x1ff1fa8000000000 Param: 0x000101600962ffff\n");
  fprintf(stderr, "     MIL diagnostic (strahlweg mask write)  MIL error code   :   ^^^^\n"            );
  fprintf(stderr, "                                            SIO slot         :       ^^\n"          );
  fprintf(stderr, "                                            MIL ifb addr     :         ^^\n"        );
  fprintf(stderr, "                                            module addr      :           ^^\n"      );
  fprintf(stderr, "                                            submodule addr   :             ^^\n"    );
  fprintf(stderr, "                                            data             :               ^^^^\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  unichop_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", unichop_version_text(version));
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
          
  uint32_t verLib;
  uint32_t verFw;

  int      i;

  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  uint32_t milDev;
  uint64_t nMilSend;
  uint32_t nMilError;
  uint64_t nEvtsReceived;
  
  program = argv[0];    

  while ((opt = getopt(argc, argv, "w:ghei")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'w':
        milDev = strtol(optarg, &tail, 0);
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

  if ((status =  unichop_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  if (getVersion) {
    unichop_version_library(&verLib);
    printf("uni-chop: library (firmware) version %s",  unichop_version_text(verLib));     
    unichop_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  unichop_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status

    unichop_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    unichop_info_read(ebDevice, &milDev, &nMilSend, &nMilError, &nEvtsReceived, 0);
    
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", unichop_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    unichop_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("uni-chop: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
      else {
        if (unichop_upload(ebDevice, milDev) != COMMON_STATUS_OK) die("uni-chop upload", status);
        unichop_cmd_configure(ebDevice);
      } // else state
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      unichop_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("uni-chop: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      unichop_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("uni-chop: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      unichop_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("uni-chop: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      unichop_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("uni-chop: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      unichop_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("uni-chop: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      unichop_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", unichop_status_text(i));
      } // for i

      unichop_info_read(ebDevice, &milDev, &nMilSend, &nMilError, &nEvtsReceived, 1);
    } // "diag"    
  } //if command

  // close connection to firmware
  if ((status = unichop_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

/******************************************************************************
 *  eb-reset.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Mar-2025
 *
 * Command-line interface for resetting a FPGA. This forces a restart using the image stored
 * in the local flash of the timing receiver.
 * This tool also helps in configuring the watchdog
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
#define EBRESET_VERSION "01.05.00"

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
  fprintf(stderr, "  -p<t>            after FPGA reset, wait for the specified time [s] and probe device\n");
  fprintf(stderr, "  -f               force 'fpgareset' of FPGA with incompatible FPGA gateware\n");
  fprintf(stderr, "                   use the 'force' option at your own risk: this might brick your device\n");
  fprintf(stderr, "  -n<NIC index>    specify NIC when using dual SFP boards (0: 1st NIC; 1: 2nd NIC; default: n0)\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  wddisable        disables the watchdog (preventing automated FPGA reset permanently)\n");
  fprintf(stderr, "  wdenable         enables the watchdog (automated FPGA reset after 'some time')\n");
  fprintf(stderr, "  wdretrigger      retriggers an enabled watchdog (preventing automated FPGA reset for 'some time')\n");
  fprintf(stderr, "  wdstatus         gets the status of the watchdog; '1': enabled, '0': disabled\n");
  fprintf(stderr, "  phyreset         resets the PHY (recommended for experts and developers); use option '-n' for dual NIC boards\n");
  fprintf(stderr, "  sfpreset         resets the SFP (recommended for users); use option '-n' for dual NIC boards\n");
  fprintf(stderr, "  cpuhalt  <cpu>   halts a user lm32 CPU\n");
  fprintf(stderr, "                   specify a single CPU (0..31) or all CPUs (0xff)\n");
  fprintf(stderr, "  cpureset <cpu>   resets a user lm32 CPU, firmware restarts.\n");
  fprintf(stderr, "                   specify a single CPU (0..31) or all CPUs (0xff)\n");
  fprintf(stderr, "  cpustatus        get the 'halt status' of all user lm32 (rightmost bit: CPU 0)\n");
  fprintf(stderr, "  fpgareset        resets the entire FPGA (see below)\n");
  fprintf(stderr, "  comxpcyc         performs a power cycle of the SCUs COM Express module (see below)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to reset a FPGA or lm32 user CPU(s) or to power cycle a COM Express module.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "The command 'fpgareset' forces a restart of the entire FPGA using the image stored in the \n");
  fprintf(stderr, "flash of the device. Don't use this command unless the flash contains a valid image (otherwise\n");
  fprintf(stderr, "your devices becomes bricked).\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "The command 'comxpcyc' performs a power cycle of the SCUs COM Express board.\n");
  fprintf(stderr, "Warning: Don't use the command from the COM Express board (or via socat) itself; if you ignore\n");
  fprintf(stderr, "this warning, your COM Express board will power off leaving the 'power button' in an\n");
  fprintf(stderr, "undefined state. Only use this command remotely (USB, timing network ...).\n");
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
  int         cmdExecuted     = 0;

  int         getEBVersion    = 0;
  int         flagForce       = 0;
  int         probeAfterReset = 0;
  int         waitTime        = 1;
  int         nicIndex        = 0;
  int         exitCode        = 0;
  uint32_t    nCPU;
  uint64_t    i;

  int opt, error=0;
  char *tail;

  int         ip;
  int         flagEnabled;

  program = argv[0];

  while ((opt = getopt(argc, argv, "p:n:feh")) != -1) {
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
        getEBVersion = 1;
        break;
      case 'f':
        flagForce = 1;
        break;
      case 'n':
        nicIndex = strtol(optarg, &tail, 0);
        if (!(nicIndex == 0 || nicIndex == 1)) {
          fprintf(stderr, "NIC index has to be 0 or 1, not %d!\n", nicIndex);
          exit(1);
        } // if nicIndex
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
      status = wb_wr_reset(device, devIndex, 0xdeadbeef, flagForce);
      if ((status != EB_TIMEOUT) && (status != EB_OK) && (status != EB_ABI)) die("RESET FPGA", status);

      if (probeAfterReset) {
        //close, wait and reopen socket, try to read a property (here: ip) from the device

        if ((status = wb_close(device, socket)) != EB_OK) die ("Probe FPGA", status);
        sleep(waitTime);
        if ((status = wb_open(devName, &device, &socket)) != EB_OK) die("Probe FPGA", status);

        if ((status = wb_wr_get_ip(device, devIndex, &ip)) != EB_OK) die("Probe FPGA", status);
      } // probe after reset
    } // fpga reset

    // watchdog disable
    if (!strcasecmp(command, "wddisable")) {
      cmdExecuted = 1;

      status = wb_wr_watchdog(device, devIndex, 0);
      if (status != EB_OK)  die("eb-reset: ", status);
    } // watchdog disable

    // watchdog enable
    if (!strcasecmp(command, "wdenable")) {
      cmdExecuted = 1;

      status = wb_wr_watchdog(device, devIndex, 1);
      if (status != EB_OK)  die("eb-reset: ", status);
    } // watchdog enable

    // watchdog retrigger
    if (!strcasecmp(command, "wdretrigger")) {
      cmdExecuted = 1;

      status = wb_wr_watchdog_retrigger(device, devIndex);
      if (status != EB_OK)  die("eb-reset: ", status);
    } // watchdog retrigger

    // watchdog status
    if (!strcasecmp(command, "wdstatus")) {
      cmdExecuted = 1;
      status = wb_wr_watchdog_status(device, devIndex, &flagEnabled);
      if (status != EB_OK)  die("eb-reset: ", status);
      printf("%d\n", flagEnabled);
    } // get watchdog 'enabled' status

    // reset PHY
    if (!strcasecmp(command, "phyreset")) {
      cmdExecuted = 1;

      status = wb_wr_phy_reset(device, devIndex, nicIndex);
      if (status != EB_OK)  die("eb-reset: ", status);
    } // reset PHY

    // reset SFP
    if (!strcasecmp(command, "sfpreset")) {
      cmdExecuted = 1;

      status = wb_wr_sfp_reset(device, devIndex, nicIndex);
      if (status != EB_OK)  die("eb-reset: ", status);
    } // reset SFP

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

    // power cycle com x board
    if (!strcasecmp(command, "comxpcyc")) {
      cmdExecuted = 1;

      if (strstr(devName, "dev/wbm") != 0) die("eb-reset: refusing to power cycle myself", EB_OOM);

      // perform power off-on sequence
      printf("eb-reset: powering off com express\n");
      status = wb_comx_power(device, devIndex, 0);
      printf("... done\n");

      printf("eb-reset: just a moment ...\n");
      sleep(2);

      printf("eb-reset: powering on com express\n");
      status = wb_comx_power(device, devIndex, 1);
      printf("... done\n");

      printf("eb-reset: com express should be booting now; check host in a minute or so\n");
      if (status != EB_OK)  die("eb-reset: ", status);
    } // power cycle com x board
    
    wb_close(device, socket);

    if (!cmdExecuted) printf("eb-reset: unknonwn command %s\n", command);
  } // if command
  else printf("eb-reset: no action on FPGA or lm32, use option '-h' to learn about commands\n");

  return exitCode;
} // main

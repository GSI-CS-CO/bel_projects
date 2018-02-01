/******************************************************************************
 *  eb-fec.c
 *
 *  created : 2018
 *  author  : Cesar Prados, GSI-Darmstadt
 *  version : Jan 2018
 *
 * Command-line interface for WR monitoring via Etherbone.
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2013 Cesar Prados
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: c.prados@gsi.de - bradomyn@mailfence.com
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
 * For all questions and ideas contact: c.prados@gsi.de
 ********************************************************************************************/
#define EBMON_VERSION "1.3.0"

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

///* Wishbone api */
#include <wb_api.h>
#include <wb_slaves.h>

const char* program;
static int verbose=0;
eb_device_t       device;

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
}

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -t<polling interval> \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <c.pradosk@gsi.de> <bradomyn@mailfence.com>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMON_VERSION);
}


int main(int argc, char** argv) {
  eb_status_t       status;
  eb_socket_t       socket;
  struct fec_info   info;
  int               devIndex=-1;  /* 0,1,2... - there may be more than 1 device on the WB bus */

  const char* devName;

  int         getEBVersion=0;
  int         exitCode=0;
  int         polling_rate=1;

  program = argv[0];

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

  while(1) {
    printf("\e[1;1H\e[2J");

    if ((status = wb_fec_info(device, devIndex, &info)) != EB_OK) die("------FEC Info ----- \n", status);

    printf("------- FEC Info %s ------\n", devName);

    printf("# FEC Configuration \n");
    if (info.fec_type == 0)
      printf("Fixed Rate Code\n");
    else if (info.fec_type == 1)
      printf("Fixed Rate Code + Golay\n");
    else if (info.fec_type == 2)
      printf("LDPC\n");
    else if (info.fec_type == 3)
    printf("LDPC + Golay\n");

    if (info.config == 0)
      printf("Encoder: OFF -- Decoder: OFF\n");
    else if (info.config == 1)
      printf("Encoder: ON -- Decoder: OFF\n");
    else if (info.config == 2)
      printf("Encoder: OFF -- Decoder: ON\n");
    else if (info.config == 3)
      printf("Encoder: ON -- Decoder: ON\n");

    printf("FEC Ethertype: 0x%x \n", info.fec_ethtype);
    printf("Etherbone Ethertype: 0x%x \n", info.eth_ethtype);

    printf("# FEC Stats \n");
    printf("Encoder Packets: %d\n", info.enc_cnt);
    printf("Decoder Packets: %d\n", info.dec_cnt);
    printf("Error Decoder: %d\n", info.err_dec);
    printf("Error Encoder: %d\n", info.err_enc);
    printf("Jumbo Packet Rx: %d\n", info.jumbo_rx);

    sleep(polling_rate);

  }

  wb_close(device, socket);

  return exitCode;
}

/********************************************************************************************
 *  wrunipz-load.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 16-May-2019
 *
 * command-line example for wrunipz; how-to load event data for a virt acc
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

// wr-unipz
#include <wrunipz-api.h>                 // API
#include <wr-unipz.h>                    // FW
#include <wrunipz_shared_mmap.h>         // LM32

const char* program;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32

eb_address_t wrunipz_cmd;          // command, write
eb_address_t wrunipz_confVacc;     // virtAcc of config, write
eb_address_t wrunipz_confStat;     // status of config transaction, read
eb_address_t wrunipz_confPz;       // bit field (indicates, which PZ is submitted), write
eb_address_t wrunipz_confFlag;     // flags of config, write
eb_address_t wrunipz_confData;     // data of config, write

eb_data_t   data1;
 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  test <vacc>      loads a dummy table from file for virt acc <vacc> to pulszentrale <pz> 0..6 from <file>\n");
  fprintf(stderr, "  cleartables      clears all event tables of all PZs\n");
  fprintf(stderr, "  kill             kills possibly ongoing transactions\n");  
  fprintf(stderr, "\n");
  fprintf(stderr, "This tool is just a programming example on how-to load event tables to WR-UNIPZ.\n");
  //fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", WRUNIPZ_X86_VERSION);
} //help


int main(int argc, char** argv) {

  // etherbone stuff (could be pushed to the api too ...)
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351
  
  eb_status_t         eb_status;
  eb_socket_t         socket;
  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores
  const char*         devName;
  const char*         command;

  // opt arg ...
  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint32_t status;

  // command test
  uint32_t    dataChn0[WRUNIPZ_NEVT];
  uint32_t    nDataChn0;
  uint32_t    dataChn1[WRUNIPZ_NEVT];
  uint32_t    nDataChn1;
  uint32_t    vacc;
  uint32_t    pz;

  program = argv[0];    

  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
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

  wrunipz_cmd          = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  wrunipz_confVacc     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_VACC;
  wrunipz_confStat     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_STAT;
  wrunipz_confPz       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_PZ;
  wrunipz_confData     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_DATA;
  wrunipz_confFlag     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_FLAG;

  if (command) {

    // test with data
    if (!strcasecmp(command, "test")) {

      // analyze command line options
      if (optind+2  != argc)                     {printf("wr-unipz: expecting exactly three arguments: test <vacc>\n"); return 1;}

      vacc   = strtoul(argv[optind+1], &tail, 0);
      if ((vacc < 0) || (vacc >= WRUNIPZ_NVACC)) {printf("wr-unipz: invalid virtual accelerator -- %s\n", argv[optind+2]); return 1;}

      // initialize transaction: tell wr-unipz which virtual accelerator will get new data
      if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, vacc)) !=  COMMON_STATUS_OK) {
        printf("wr-unipz: transaction init - %s\n", wrunipz_status_text(status));
      }
      else {
        // load data into all seven PZs 
        for (pz=0; pz < WRUNIPZ_NPZ; pz++) {
          // generate dummy data (identical for channel 0 and channel 1)
          dataChn0[0] = dataChn1[0] = (    0 << 16) |  16;
          dataChn0[1] = dataChn1[1] = (   30 << 16) | 255;
          dataChn0[2] = dataChn1[2] = (  100 << 16) |  19;
          dataChn0[3] = dataChn1[3] = (  130 << 16) |   3;
          dataChn0[4] = dataChn1[4] = (  250 << 16) |   2;
          dataChn0[5] = dataChn1[5] = (  450 << 16) |   6;
          dataChn0[6] = dataChn1[6] = (10960 << 16) |  10;
          dataChn0[7] = dataChn1[7] = (11000 << 16) |   8;
          dataChn0[8] = dataChn1[8] = (11060 << 16) |  29;
          nDataChn0   = nDataChn1   = 9;

          // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
          // in principle, uploading data is the only thing that needs to be done from the host system
          // upload data to wr-unipz
          // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
          if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, pz, dataChn0, nDataChn0, dataChn1, nDataChn1)) != COMMON_STATUS_OK)
            printf("wr-unipz: transaction upload - %s\n", wrunipz_status_text(status));
        }  // for i 
        
        // so far data have only by uploaded; now they need to be commited.
        // tere, it is demonstrated how to do that 'manually'. In real life, this will be done by Super-PZ
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);
      } // else 
    } // "test"

    // this allows to kill an ongoing transaction (just in case we messed it up)
    if (!strcasecmp(command, "kill")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFKILL, 0, eb_block);
    } // "kill"

    // this allows to clear ALL event tables
    if (!strcasecmp(command, "cleartables")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFCLEAR, 0, eb_block);
    } // "cleartables"

  } //if command
  
  // close Etherbone device and socket
  if ((eb_status = eb_device_close(device)) != EB_OK) die("eb_device_close", eb_status);
  if ((eb_status = eb_socket_close(socket)) != EB_OK) die("eb_socket_close", eb_status);

  return exitCode;
}

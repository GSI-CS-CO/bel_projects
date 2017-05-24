/******************************************************************************
 *  dmunipz-ctl.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 17-May-2017
 *
 * Command-line interface for dmunipz
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
 * Last update: 17-May-2017
 ********************************************************************************************/
#define DMUNIPZ_X86_VERSION "0.0.2"

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

// dm-unipz
#include <dm-unipz.h>
#include <dm-unipz_smmap.h>

const char* program;
static int getInfo = 0;
static int snoop = 0;
static int logLevel=0;

eb_device_t  device;             // keep this and below global
eb_address_t lm32_base;          // base address of lm32
eb_address_t dmunipz_status;     // status of dmunipz, read
eb_address_t dmunipz_state;      // state, read
eb_address_t dmunipz_iterations; // number of iterations of main loop, read
eb_address_t dmunipz_transfers;  // number of transfers from UNILAC to SIS, read
eb_address_t dmunipz_injections; // number of injections in ongoing transfer
eb_address_t dmunipz_virtAcc;    // number of virtual accelerator of ongoing or last transfer, read
eb_address_t dmunipz_statTrans;  // status of ongoing or last transfer, read
eb_address_t dmunipz_cmd;        // command, write
eb_address_t dmunipz_version;    // version, read



static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


const char* dmunipz_status_text(uint32_t code) {
  switch (code) {
  case DMUNIPZ_STATUS_UNKNOWN          : return "unknown status";
  case DMUNIPZ_STATUS_OK               : return "OK";
  case DMUNIPZ_STATUS_ERROR            : return "an error occured";
  case DMUNIPZ_STATUS_TIMEDOUT         : return "a timeout occured";
  case DMUNIPZ_STATUS_OUTOFRANGE       : return "some value is out of range";
  case DMUNIPZ_STATUS_REQTKFAILED      : return "UNILAC refuses TK request";
  case DMUNIPZ_STATUS_REQTKTIMEOUT     : return "UNILAC TK request timed out";
  case DMUNIPZ_STATUS_REQBEAMFAILED    : return "UNILAC refuses beam request";
  case DMUNIPZ_STATUS_RELTKFAILED      : return "UNILAC refuses to release TK request";
  case DMUNIPZ_STATUS_RELBEAMFAILED    : return "UNILAC refuses to release beam request";
  case DMUNIPZ_STATUS_DEVBUSERROR      : return "something went wrong with write/read on the MIL devicebus";
  case DMUNIPZ_STATUS_REQNOTOK         : return "UNILAC signals 'request not ok'";
  case DMUNIPZ_STATUS_REQBEAMTIMEDOUT  : return "UNILAC beam request timed out";
  default                              : return "dm-unipz: undefined error code";
  }
}

const char* dmunipz_state_text(uint32_t code) {
  switch (code) {
  case DMUNIPZ_STATE_UNKNOWN      : return "UNKNOWN   ";
  case DMUNIPZ_STATE_S0           : return "S0        ";
  case DMUNIPZ_STATE_IDLE         : return "IDLE      ";                                       
  case DMUNIPZ_STATE_CONFIGURED   : return "CONFIGURED";
  case DMUNIPZ_STATE_OPERATION    : return "OPERATION ";
  case DMUNIPZ_STATE_STOPPING     : return "STOPPING  ";
  case DMUNIPZ_STATE_ERROR        : return "ERROR     ";
  case DMUNIPZ_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined  ";
  }
}

const char* dmunipz_transferStatus_text(uint32_t code) {
  switch (code) {
  case DMUNIPZ_TRANS_UNKNOWN   : return "unknown status";
  case DMUNIPZ_TRANS_REQTK     : return "TK requested";
  case DMUNIPZ_TRANS_REQTKOK   : return "TK request succeeded";
  case DMUNIPZ_TRANS_RELTK     : return "TK released";
  case DMUNIPZ_TRANS_REQBEAM   : return "beam requested";
  case DMUNIPZ_TRANS_REQBEAMOK : return "beam request succeeded";
  case DMUNIPZ_TRANS_RELBEAM   : return "beam released";
  default                      : return "undefined transfer status";
  }
}


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "  -i               display information on gateway\n");
  fprintf(stderr, "  -s<n>            snoop gateway for information continuously\n");
  fprintf(stderr, "                   0: print all messages (default)\n");
  fprintf(stderr, "                   1: as 0, but without info on ongoing transfers\n");
  fprintf(stderr, "                   2: as 1, but without info on transfers\n");
  fprintf(stderr, "                   3: as 2, but without info on status\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure        command requests state change to CONFIGURED\n");
  fprintf(stderr, "  startop          command requests state change to OPERATION\n");
  fprintf(stderr, "  stopop           command requests state change to STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover          command tries to recover from ERROR state and transit to IDLE\n");
  fprintf(stderr, "  idle             command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control the DM-UNIPZ gateway from the command line.\n");
  fprintf(stderr, "Example1: '%s -i dev/wbm0' display typical information.\n", program);
  fprintf(stderr, "Example2: '%s -s0 dev/wbm0 | logger -t TIMING -sp local0.info' monitor firmware and print to screen and to diagnostic logging", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "dm-unipz: transfer - 00000074, 01, 002, 1 1 1 1 1 1, OPERATION , OK\n");
  fprintf(stderr, "                            |   |    |  | | | | | |  |           |\n");
  fprintf(stderr, "                            |   |    |  | | | | | |  |           - status\n");
  fprintf(stderr, "                            |   |    |  | | | | | |   - state\n");
  fprintf(stderr, "                            |   |    |  | | | | | - beam (request) released\n");
  fprintf(stderr, "                            |   |    |  | | | | - beam request succeeded\n");
  fprintf(stderr, "                            |   |    |  | | | - beam requested\n");
  fprintf(stderr, "                            |   |    |  | | - TK (request) released -> transfer completed\n");
  fprintf(stderr, "                            |   |    |  | - TK request succeeded\n");
  fprintf(stderr, "                            |   |    |  - TK requested\n");
  fprintf(stderr, "                            |   |    - number of virtual accelerator\n");
  fprintf(stderr, "                            |   |- number of injections in current transfer\n");
  fprintf(stderr, "                            - number of transfers\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", DMUNIPZ_X86_VERSION);
} //help


int readTransfers(uint32_t *transfers)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("EP eb_cycle_open", eb_status);
  eb_cycle_read(cycle, dmunipz_transfers,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)transfers);
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", eb_status);

  return eb_status;
} // getInfo


int readInfo(uint32_t *status, uint32_t *state, uint32_t *iterations, uint32_t *transfers, uint32_t *injections, uint32_t *virtAcc, uint32_t *statTrans)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("EP eb_cycle_open", eb_status);

  eb_cycle_read(cycle, dmunipz_status,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)status);
  eb_cycle_read(cycle, dmunipz_state,       EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)state);
  eb_cycle_read(cycle, dmunipz_iterations,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)iterations);
  eb_cycle_read(cycle, dmunipz_transfers,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)transfers);
  eb_cycle_read(cycle, dmunipz_injections,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)injections);
  eb_cycle_read(cycle, dmunipz_virtAcc,     EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)virtAcc);
  eb_cycle_read(cycle, dmunipz_statTrans,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)statTrans);
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("EP eb_cycle_close", eb_status);

  return eb_status;
} // getInfo


void printTransfer(uint32_t transfers, uint32_t injections, uint32_t virtAcc, uint32_t statTrans)
{
  printf("%08d, %02d, %03d, %d %d %d %d %d %d", transfers, injections, virtAcc, 
         ((statTrans & DMUNIPZ_TRANS_REQTK    ) > 0),  
         ((statTrans & DMUNIPZ_TRANS_REQTKOK  ) > 0), 
         ((statTrans & DMUNIPZ_TRANS_RELTK    ) > 0),
         ((statTrans & DMUNIPZ_TRANS_REQBEAM  ) > 0),
         ((statTrans & DMUNIPZ_TRANS_REQBEAMOK) > 0),
         ((statTrans & DMUNIPZ_TRANS_RELBEAM  ) > 0)
         );
  
} // printTransfer

int main(int argc, char** argv) {
  eb_status_t       eb_status;
  eb_socket_t       socket;

  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint32_t status;    
  uint32_t state;     
  uint32_t iterations;
  uint32_t transfers; 
  uint32_t injections;
  uint32_t virtAcc;   
  uint32_t statTrans; 
  uint32_t version; 

  uint32_t actTransfers;   // actual number of transfers
  uint32_t actState;       // actual state of gateway
  uint32_t actStatus;      // actual status of gateway
  uint32_t actStatTrans;   // actual status of ongoing transfer
  uint32_t sleepTime;      // time to sleep [us]
  uint32_t printFlag;      // flag for printing
  

  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:ih")) != -1) {
    switch (opt) {
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
      if ((logLevel < DMUNIPZ_LOGLEVEL_ALL) || (logLevel > DMUNIPZ_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
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
  lm32_base          = 0x20090000;  //chk, need to do this properly
  dmunipz_status     = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_STATUS;
  dmunipz_state      = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_STATE;;
  dmunipz_iterations = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_NITERMAIN;
  dmunipz_transfers  = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TRANSN;
  dmunipz_injections = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_INJECTN;
  dmunipz_virtAcc    = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TRANSVIRTACC;
  dmunipz_statTrans  = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_TRANSSTATUS;
  dmunipz_cmd        = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_CMD;
  dmunipz_version    = lm32_base + SHARED_OFFS + DMUNIPZ_SHARED_VERSION;

  if (getInfo) {
    // version info
    eb_device_read(device, dmunipz_version, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t *)(&version), 0, eb_block);
    printf("dm-unipz: software (firmware) version %s (%06x)\n",  DMUNIPZ_X86_VERSION, version); 

    // status
    readInfo(&status, &state, &iterations, &transfers, &injections, &virtAcc, &statTrans);
    printf("dm-unipz: state %s, status %s, iterations %d\n",dmunipz_state_text(state),  dmunipz_status_text(status), iterations);
    printf("dm-unipz: "); printTransfer(transfers, injections, virtAcc, statTrans); printf("\n");
    printf("          # of transfers, # of injections, virtAcc, status transfer\n");
  } // if getInfo

  if (command) {
    if (!strcasecmp(command, "configure")) eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_CONFIGURE, 0, eb_block);
    if (!strcasecmp(command, "startop"))   eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_STARTOP  , 0, eb_block);
    if (!strcasecmp(command, "stopop"))    eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_STOPOP   , 0, eb_block);
    if (!strcasecmp(command, "recover"))   eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_RECOVER  , 0, eb_block);
    if (!strcasecmp(command, "idle"))      eb_device_write(device, dmunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)DMUNIPZ_CMD_IDLE     , 0, eb_block);
  } //if command

  if (snoop) {
    printf("dm-unipz: continous monitoring of gateway...\n");
    
    actTransfers = 0;
    actState     = DMUNIPZ_STATE_UNKNOWN;
    actStatus    = DMUNIPZ_STATUS_UNKNOWN;
    actStatTrans = DMUNIPZ_TRANS_UNKNOWN;

    while (1) {
      readInfo(&status, &state, &iterations, &transfers, &injections, &virtAcc, &statTrans);  // read info from lm32

      switch(state) {
      case DMUNIPZ_STATE_OPERATION :
        if (actTransfers != transfers) sleepTime = DMUNIPZ_DEFAULT_TIMEOUT * 1000 * 2;        // ongoing transfer: reduce polling rate ...
        else                           sleepTime = DMUNIPZ_DEFAULT_TIMEOUT * 1000;            // sleep for default timeout to catch next REQ_TK
        break;
      default:
        sleepTime = DMUNIPZ_DEFAULT_TIMEOUT * 1000;                          
      } // switch actState
      
      // if required, print status change
      if  ((actState != state) && (logLevel <= DMUNIPZ_LOGLEVEL_STATE)) printFlag = 1;

      // determine when to print info
      printFlag = 0;

      if ((actState     != state)     && (logLevel <= DMUNIPZ_LOGLEVEL_STATE))                                         {printFlag = 1; actState = state;}
      if ((actStatus    != status)    && (logLevel <= DMUNIPZ_LOGLEVEL_STATUS))                                        {printFlag = 1; actStatus = status;}
      if ((actTransfers != transfers) && (logLevel <= DMUNIPZ_LOGLEVEL_COMPLETE) && (statTrans & DMUNIPZ_TRANS_RELTK)) {printFlag = 1; actTransfers = transfers;}
      if ((actStatTrans != statTrans) && (logLevel <= DMUNIPZ_LOGLEVEL_ALL))                                           {printFlag = 1; actStatTrans = statTrans;}

      if (printFlag) {
        printf("dm-unipz: transfer - "); 
        printTransfer(transfers, injections, virtAcc, statTrans); 
        printf(", %s, %s\n", dmunipz_state_text(state), dmunipz_status_text(status));
      } // if printFlag

      fflush(stdout);                                                                         // required for immediate writing (if stdout is piped to syslog)

      //sleep 
      usleep(sleepTime);
    } // while
  } // if snoop

  // close Etherbone device and socket
  if ((eb_status = eb_device_close(device)) != EB_OK) die("eb_device_close", eb_status);
  if ((eb_status = eb_socket_close(socket)) != EB_OK) die("eb_socket_close", eb_status);

  return exitCode;
}

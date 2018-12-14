/******************************************************************************
 *  wrunipz-ctl.cpp
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-December-2018
 *
 * Command-line interface for wrunipz
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
#define WRUNIPZ_X86_VERSION "0.0.2"

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

// Etherbone
#include <etherbone.h>

// wr-unipz
#include <wr-unipz.h>
#include <wr-unipz_smmap.h>

const char* program;
static int getInfo    = 0;
static int getConfig  = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32
eb_address_t wrunipz_status;       // status of wrunipz, read
eb_address_t wrunipz_state;        // state, read
eb_address_t wrunipz_iterations;   // # of iterations of main loop, read
eb_address_t wrunipz_cycles;       // # of UNILAC cycles
eb_address_t wrunipz_cmd;          // command, write
eb_address_t wrunipz_version;      // version, read
eb_address_t wrunipz_macHi;        // ebm src mac, read
eb_address_t wrunipz_macLo;        // ebm src mac, read
eb_address_t wrunipz_ip;           // ebm src ip, read
eb_address_t wrunipz_nBadStatus;   // # of bad status ("ERROR") incidents, read
eb_address_t wrunipz_nBadState;    // # of bad state ("not in operation") incidents, read
eb_address_t wrunipz_tCycleAvg;    // period of cycle [us] (average over one second), read
eb_address_t wrunipz_nMessageLo;   // number of messages, read
eb_address_t wrunipz_nMessageHi;   // number of messages, read
eb_address_t wrunipz_msgFreqAvg;   // message rate (average over one second), read
eb_address_t wrunipz_dtMax;        // delta T (max) between message time of dispatching and deadline
eb_address_t wrunipz_dtMin;        // delta T (min) between message time of dispatching and deadline

eb_data_t   data1;

 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


const char* wrunipz_status_text(uint32_t code) {  
  static char message[256];

  switch (code) {
  case WRUNIPZ_STATUS_UNKNOWN          : sprintf(message, "error %d, %s",    code, "unknown status"); break;
  case WRUNIPZ_STATUS_OK               : sprintf(message, "OK"); break;
  case WRUNIPZ_STATUS_ERROR            : sprintf(message, "error %d, %s",    code, "an error occured"); break;
  case WRUNIPZ_STATUS_TIMEDOUT         : sprintf(message, "error %d, %s",    code, "a timeout occured"); break;
  case WRUNIPZ_STATUS_OUTOFRANGE       : sprintf(message, "error %d, %s",    code, "some value is out of range"); break;
  case WRUNIPZ_STATUS_LATE             : sprintf(message, "error %d, %s",    code, "a timing messages is not dispatched in time"); break;
  case WRUNIPZ_STATUS_EARLY            : sprintf(message, "error %d, %s",    code, "a timing messages is dispatched unreasonably early (dt > UNILAC period)"); break;
  case WRUNIPZ_STATUS_NOIP             : sprintf(message, "error %d, %s",    code, "DHCP request via WR network failed"); break;
  case WRUNIPZ_STATUS_EBREADTIMEDOUT   : sprintf(message, "error %d, %s",    code, "EB read via WR network timed out"); break;
  case WRUNIPZ_STATUS_WRONGVIRTACC     : sprintf(message, "error %d, %s",    code, "mismatching virtual accelerator for EVT_READY_TO_SIS from UNIPZ"); break;
  case WRUNIPZ_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    code, "violation of safety margin for data master and timing network"); break;
  case WRUNIPZ_STATUS_NOTIMESTAMP      : sprintf(message, "error %d, %s",    code, "received EVT_READY_TO_SIS in MIL FIFO but no TS via TLU -> ECA"); break;
  case WRUNIPZ_STATUS_BADTIMESTAMP     : sprintf(message, "error %d, %s",    code, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break;
  case WRUNIPZ_STATUS_WAIT4UNIEVENT    : sprintf(message, "error %d, %s",    code, "timeout while waiting for EVT_READY_TO_SIS"); break;
  case WRUNIPZ_STATUS_WRBADSYNC        : sprintf(message, "error %d, %s",    code, "White Rabbit: not in 'TRACK_PHASE'"); break;
  case WRUNIPZ_STATUS_AUTORECOVERY     : sprintf(message, "errorFix %d, %s", code, "attempting auto-recovery from state ERROR"); break;
  default                              : sprintf(message, "error %d, %s",    code, "wr-unipz: undefined error code"); break;
  }

  return message;
} // wrunipz_status_text


const char* wrunipz_state_text(uint32_t code) {
  switch (code) {
  case WRUNIPZ_STATE_UNKNOWN      : return "UNKNOWN   ";
  case WRUNIPZ_STATE_S0           : return "S0        ";
  case WRUNIPZ_STATE_IDLE         : return "IDLE      ";                                       
  case WRUNIPZ_STATE_CONFIGURED   : return "CONFIGURED";
  case WRUNIPZ_STATE_OPREADY      : return "OpReady   ";
  case WRUNIPZ_STATE_STOPPING     : return "STOPPING  ";
  case WRUNIPZ_STATE_ERROR        : return "ERROR     ";
  case WRUNIPZ_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined ";
  }
} // wrunipz_state_text


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -c                  display configuration of WR-UNIPZ\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on WR-UNIPZ\n");
  fprintf(stderr, "  -s<n>               snoop ... for information continuously\n");
  fprintf(stderr, "                      0: print all messages (default)\n");
  fprintf(stderr, "                      1: as 0, once per second\n");
  fprintf(stderr, "                      2: as 1, inform in case of status or state changes\n");
  fprintf(stderr, "                      3: as 2, inform in case of state changes\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED to CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED to OPREADY\n");
  fprintf(stderr, "  stopop              command requests state change from OPREADY to STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  cleardiag           clear statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control WR-UNIPZ from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "wr-unipz:                  TRANSFERS                |                   INJECTION                     | DIAGNOSIS  |                    INFO   \n");
  fprintf(stderr, "wr-unipz:              n    sum(tkr)  set(get)/noBm | n(r2s/sumr2s)   sum( prep/bmrq/r2sis->mbtrig)   | DIAG margn | status         state      nchng stat   nchng\n");
  fprintf(stderr, "wr-unipz: TRANS 00057399,  5967( 13)ms, va 10(10)/0 | INJ 06(06/06),  964(0.146/   0/ 954 -> 9.979)ms | DG 1.453ms | 1 1 1 1 1 1, OpReady    (     0), OK (     4)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |      '  '  '      '     '    '    '        '    |        '   | ' ' ' ' ' '        '          '    '       ' \n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |      '  '  '      '     '    '    '        '    |        '   | ' ' ' ' ' '        '          '    '       ' - # of 'bad status' incidents\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", WRUNIPZ_X86_VERSION);
} //help


int readTransfers(uint32_t *transfers)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data;
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, wrunipz_cycles,   EB_BIG_ENDIAN|EB_DATA32, &data);
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

  *transfers = data;

  return eb_status;
} // getInfo


int readInfo(uint32_t *status, uint32_t *state, uint32_t *iterations, uint32_t *cycles, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *tCycleAvg, uint64_t *msgFreqAvg, int32_t *dtMax, int32_t *dtMin)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);

  eb_cycle_read(cycle, wrunipz_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_iterations,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, wrunipz_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, wrunipz_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, wrunipz_tCycleAvg,     EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, wrunipz_msgFreqAvg,    EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, wrunipz_dtMax,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, wrunipz_dtMin,         EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

  *status        = data[0];
  *state         = data[1];
  *iterations    = data[2];
  *nBadStatus    = data[3];
  *nBadState     = data[4];
  *cycles        = data[5];
  *tCycleAvg     = data[6];
  *msgFreqAvg    = data[7];
  *dtMax         = data[8];
  *dtMin         = data[9];
  
  return eb_status;
} // readInfo

int readConfig(uint64_t *mac, uint32_t *ip)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[10];

  uint32_t macHi, macLo;

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);

  eb_cycle_read(cycle, wrunipz_macHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_macLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_ip,         EB_BIG_ENDIAN|EB_DATA32, &(data[2]));

  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

  macHi   = data[0];
  macLo   = data[1];  
  *ip     = data[2];
  
  *mac    = macHi;
  *mac    = (*mac << 32);
  *mac    = *mac + macLo;

  return eb_status;
} //readConfig


void printCycleHeader()
{
  printf("wr-unipz:         CYCLES |          DIAGNOSIS        |                 INFO           \n");
  printf("wr-unipz:              n |    fUni  fMsg dtMax dtMin |   state      nchng stat   nchng\n");
} // printCycleHeader


void printCycle(uint32_t cycles, uint32_t tCycleAvg, uint32_t msgFreqAvg, int32_t dtMax, int32_t dtMin)
{
  // cycle
  printf("wr-unipz: CYC %010d |", cycles);

  // diag
  printf("DG %5.2f %05d %05d %05d |", 1000000.0/(double)tCycleAvg, msgFreqAvg, dtMax / 1000, dtMin / 1000);

} // printCycle

int main(int argc, char** argv) {
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

  
  eb_status_t         eb_status;
  eb_socket_t         socket;
  eb_data_t           data;

  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores

  
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint32_t status;    
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t iterations;
  uint32_t cycles;
  uint64_t messages;
  uint32_t tCycle;
  uint32_t version;
  int32_t  dtMax;
  int32_t  dtMin;

  uint32_t actCycles;                          // actual number of cycles
  uint32_t actState = WRUNIPZ_STATE_UNKNOWN;   // actual state of gateway
  uint32_t actStatus;                          // actual status of gateway
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing

  uint64_t mac;                                // mac for config of EB master
  uint32_t ip;                                 // ip for config of EB master
    
  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:ceih")) != -1) {
    switch (opt) {
    case 'c':
      getConfig = 1;
      break;
    case 'e':
      getVersion = 1;
      break;
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
      if ((logLevel < WRUNIPZ_LOGLEVEL_ALL) || (logLevel > WRUNIPZ_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
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
  nDevices = 1; // quick and dirty, use only first core
  if ((eb_status = eb_sdb_find_by_identity(device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) die("find lm32", eb_status);
  lm32_base =  sdbDevice.sdb_component.addr_first;

  wrunipz_status       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_STATUS;
  wrunipz_state        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_STATE;;
  wrunipz_cycles       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NCYCLE;
  wrunipz_cmd          = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CMD;
  wrunipz_version      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_VERSION;
  wrunipz_macHi        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MACHI;
  wrunipz_macLo        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MACLO;
  wrunipz_ip           = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_IP;
  wrunipz_nBadStatus   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NBADSTATUS;
  wrunipz_nBadState    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NBADSTATE;
  wrunipz_tCycleAvg    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TCYCLEAVG;
  wrunipz_nMessageHi   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGEHI;
  wrunipz_nMessageLo   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGELO;
  wrunipz_msgFreqAvg   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MSGFREQAVG;
  wrunipz_dtMax        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMAX;
  wrunipz_dtMin        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMIN;

  // printf("wr-unipz: lm32_base 0x%08x, 0x%08x\n", lm32_base, wrunipz_iterations);

  if (getConfig) {
    readConfig(&mac, &ip);
    printf("wr-unipz: EB Master: mac 0x%012"PRIx64", ip %03d.%03d.%03d.%03d\n", mac, (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  } // if getConfig

  if (getVersion) {
    eb_device_read(device, wrunipz_version, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    version = data;
    printf("wr-unipz: software (firmware) version %s (%06x)\n",  WRUNIPZ_X86_VERSION, version);     
  } // if getEBVersion

  if (getInfo) {
    // status
    readInfo(&status, &state, &iterations, &cycles, &nBadStatus, &nBadState, &tCycle, &messages, &dtMax, &dtMin);
    printCycleHeader();
    printCycle(cycles, tCycle, messages, dtMax, dtMin);
    printf(" %s (%6u), %s (%6u)\n", wrunipz_state_text(state), nBadState, wrunipz_status_text(status), nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    eb_device_read(device, wrunipz_state, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    state = data;

    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFIGURE, 0, eb_block);
      if ((state != WRUNIPZ_STATE_CONFIGURED) && (state != WRUNIPZ_STATE_IDLE)) printf("wr-unipz: WARNING command has not effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_STARTOP  , 0, eb_block);
      if (state != WRUNIPZ_STATE_CONFIGURED) printf("wr-unipz: WARNING command has not effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_STOPOP   , 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has not effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_RECOVER  , 0, eb_block);
      if (state != WRUNIPZ_STATE_ERROR) printf("wr-unipz: WARNING command has not effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_IDLE     , 0, eb_block);
      if (state != WRUNIPZ_STATE_CONFIGURED) printf("wr-unipz: WARNING command has not effect (not in state CONFIGURED)\n");
    } // "idle"
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CLEARDIAG , 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has not effect (not in state OPREADY)\n");
    } // "cleardiag"
  } //if command
  

  if (snoop) {
    printf("wr-unipz: continous monitoring of gateway, loglevel = %d\n", logLevel);
    
    actCycles  = 0;
    actState   = WRUNIPZ_STATE_UNKNOWN;
    actStatus  = WRUNIPZ_STATUS_UNKNOWN;

    printCycleHeader();

    while (1) {
      readInfo(&status, &state, &iterations, &cycles, &nBadStatus, &nBadState, &tCycle, &messages, &dtMax, &dtMin); // read info from lm32

      switch(state) {
      case WRUNIPZ_STATE_OPREADY :
        if (actCycles != cycles) sleepTime = WRUNIPZ_DEFAULT_TIMEOUT * 1000 * 2;              // ongoing cycle: reduce polling rate ...
        else                     sleepTime = WRUNIPZ_DEFAULT_TIMEOUT * 1000;                  // sleep for default timeout to catch next cycle
        break;
      default:
        sleepTime = WRUNIPZ_DEFAULT_TIMEOUT * 1000;                          
      } // switch actState
      
      // if required, print status change
      if  ((actState != state) && (logLevel <= WRUNIPZ_LOGLEVEL_STATE)) printFlag = 1;

      // determine when to print info
      printFlag = 0;

      if ((actState     != state)     && (logLevel <= WRUNIPZ_LOGLEVEL_STATE))                                         {printFlag = 1; actState = state;}
      if ((actStatus    != status)    && (logLevel <= WRUNIPZ_LOGLEVEL_STATUS))                                        {printFlag = 1; actStatus = status;}

      if (printFlag) {
        printCycle(cycles, tCycle, messages, dtMax, dtMin); 
        printf(" %s (%6u), %s (%6u)\n", wrunipz_state_text(state), nBadState, wrunipz_status_text(status), nBadStatus);
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

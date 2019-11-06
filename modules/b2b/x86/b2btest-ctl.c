/*******************************************************************************************
 *  b2btest-ctl.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 05-November-2019
 *
 * Command-line interface for b2btest
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

// b2b-test
#include <b2btest-api.h>                 // API
#include <b2b-test.h>                    // FW
#include <b2bcbu_shared_mmap.h>          // LM32

const char* program;
static int getInfo    = 0;
static int getConfig  = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32

// common stuff
eb_address_t b2btest_status;       // status of b2btest, read
eb_address_t b2btest_state;        // state, read
eb_address_t b2btest_cmd;          // command, write
eb_address_t b2btest_version;      // version, read
eb_address_t b2btest_macHi;        // ebm src mac, read
eb_address_t b2btest_macLo;        // ebm src mac, read
eb_address_t b2btest_ip;           // ebm src ip, read
eb_address_t b2btest_nBadStatus;   // # of bad status ("ERROR") incidents, read
eb_address_t b2btest_nBadState;    // # of bad state ("not in operation") incidents, read
eb_address_t b2btest_tDiagHi;      // time when diagnostics was cleared, high bits
eb_address_t b2btest_tDiagLo;      // time when diagnostics was cleared, low bits
eb_address_t b2btest_tS0Hi;        // time when FW was in S0 state (start of FW), high bits
eb_address_t b2btest_tS0Lo;        // time when FW was in S0 state (start of FW), low bits

// application specific stuff
eb_address_t b2btest_nTransfer;    // # of transfers
eb_address_t b2btest_transStat;    // status of transfer
eb_address_t b2btest_TH1ExtHi;     // period of h=1 extraction, high bits
eb_address_t b2btest_TH1ExtLo;     // period of h=1 extraction, low bits
eb_address_t b2btest_nHExt;        // harmonic number of extraction RF
eb_address_t b2btest_TH1InjHi;     // period of h=1 injection, high bits
eb_address_t b2btest_TH1InjLo;     // period of h=1 injection, low bits
eb_address_t b2btest_nHInj;        // harmonic number of injection RF

eb_data_t   data1;
 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -c                  display configuration of B2B-TEST\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on B2B-TEST\n");
  fprintf(stderr, "  -s<n>               snoop ... for information continuously\n");
  fprintf(stderr, "                      0: print all messages (default)\n");
  fprintf(stderr, "                      1: as 0, once per second\n");
  fprintf(stderr, "                      2: as 1, inform in case of status or state changes\n");
  fprintf(stderr, "                      3: as 2, inform in case of state changes\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  configure           command requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop             command requests state change from CONFIGURED -> OPREADY\n");
  fprintf(stderr, "  stopop              command requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover             command tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                command requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailled information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  seth1inj <freq> <h> set h=1 frequency [Hz] and harmonic number of injection machine\n");
  fprintf(stderr, "  seth1ext <freq> <h> set h=1 frequency [Hz] and harmonic number of extraction machine\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control B2B-TEST from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "b2b-test:        nTrans |                 INFO\n");
  fprintf(stderr, "b2b-test: STATUS      n |   state      nchng     stat      nchng\n");
  fprintf(stderr, "b2b-test:    0000065325 | OpReady    (     1), status 0x00000001 (     0)\n");
  fprintf(stderr, "                      ' |       '          '                   '       '\n");
  fprintf(stderr, "                      ' |       '          '                   '       '- # of bad status incidents\n");
  fprintf(stderr, "                      ' |       '          '                   '- status (bitwise ored)\n");
  fprintf(stderr, "                      ' |       '          '- # of bad state incidents\n");
  fprintf(stderr, "                      ' |       '- state\n");
  fprintf(stderr, "                      '- # of transfers\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", B2BTEST_X86_VERSION);
} //help


const char* statusText(uint32_t bit) {  
  static char message[256];

  switch (bit) {
    case B2BTEST_STATUS_PHASEFAILED      : sprintf(message, "error %d, %s",    bit, "phase measurement failed"); break;
    case B2BTEST_STATUS_TRANSFER         : sprintf(message, "error %d, %s",    bit, "transfer failed"); break;
    case B2BTEST_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
    default                              : sprintf(message, "%s",  api_statusText(bit)) ; break;
  }

  return message;
} // b2btest_status_text


int readInfo(uint32_t *sumStatus, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *nTransfer)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b-test: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2btest_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2btest_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2btest_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2btest_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2btest_nTransfer,     EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b-test: eb_cycle_close", eb_status);

  *sumStatus     = data[0];
  *state         = data[1];
  *nBadStatus    = data[2];
  *nBadState     = data[3];
  *nTransfer     = data[4];

  return eb_status;
} // readInfo


int readDiags(uint32_t *sumStatus, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint64_t *tDiag, uint64_t *tS0, uint32_t *nTransfer, uint32_t *transStat, uint64_t *TH1Ext, uint32_t *nHExt, uint64_t *TH1Inj, uint32_t *nHInj)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b-test: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2btest_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2btest_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2btest_tDiagHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, b2btest_tDiagLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, b2btest_tS0Hi,         EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, b2btest_tS0Lo,         EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, b2btest_nTransfer,     EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, b2btest_transStat,     EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, b2btest_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, b2btest_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, b2btest_nHExt,         EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, b2btest_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, b2btest_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  eb_cycle_read(cycle, b2btest_nHInj,         EB_BIG_ENDIAN|EB_DATA32, &(data[15]));

  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b-test: eb_cycle_close", eb_status);

  *sumStatus     = data[0];
  *state         = data[1];
  *nBadStatus    = data[2];
  *nBadState     = data[3];
  *tDiag         = (uint64_t)(data[4]) << 32;
  *tDiag        += data[5];
  *tS0           = (uint64_t)(data[6]) << 32;
  *tS0          += data[7];
  *nTransfer     = data[8];
  *transStat     = data[9];
  *TH1Ext        = (uint64_t)(data[10]) << 32;
  *TH1Ext       += data[11];
  *nHExt         = data[12];
  *TH1Inj        = (uint64_t)(data[13]) << 32;
  *TH1Inj       += data[14];
  *nHInj         = data[15];
 
  return eb_status;
} // readDiags


int readConfig(uint64_t *mac, uint32_t *ip)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[10];

  uint32_t macHi, macLo;

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b-test: eb_cycle_open", eb_status);

  eb_cycle_read(cycle, b2btest_macHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2btest_macLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2btest_ip,         EB_BIG_ENDIAN|EB_DATA32, &(data[2]));

  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b-test: eb_cycle_close", eb_status);

  macHi   = data[0];
  macLo   = data[1];  
  *ip     = data[2];
  
  *mac    = macHi;
  *mac    = (*mac << 32);
  *mac    = *mac + macLo;

  return eb_status;
} //readConfig


void printTransferHeader()
{
  printf("b2b-test:        nTrans |                 INFO                  \n");
  printf("b2b-test: STATUS      n |   state      nchng     stat      nchng\n");
} // printTransferHeader


void printTransfer(uint32_t nTransfer)
{
  // diag
  printf("b2b-test:    %010u |", nTransfer);

} // printTransfer


/*
void printDiags(uint32_t sumStatus, uint32_t state, uint32_t nBadStatus, uint32_t nBadState, uint64_t tDiag, uint64_t tS0, uint32_t nTransfers, uint32_t transStat, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;
  int              i;

  printf("b2b-test: statistics ...\n\n");

  secs     = (unsigned long)((double)tS0 / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("firmware boot at      : %s\n", timestr);

  secs     = (unsigned long)((double)tDiag / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("diagnostics reset at  : %s\n", timestr);
  
  printf("state (# of changes)  : %s (%u)\n", api_stateText(state), nBadState);
  printf("sum status (# changes): 0x%08x (%u)\n", sumStatus, nBadStatus);
  if ((sumStatus >> COMMON_STATUS_OK) & 0x1)
    printf("overall status        : OK\n");
  else
    printf("overall status        : NOT OK\n");  
  for (i= COMMON_STATUS_OK + 1; i<(sizeof(sumStatus)*8); i++) {
    if ((sumStatus >> i) & 0x1)
      printf("sum status bit is set : %s\n", b2btest_status_text(i));
  } // for i

  printf("# of transfers        : %010u\n", nTransfers);
  printf("status of act transfer: %010x\n", transStat);
  printf("period h=1 extraction : %012.6f ns\n", (double)TH1Ext/1000000000.0);
  printf("period h=1 injection  : %012.6f ns\n", (double)TH1Inj/1000000000.0);
  printf("harmonic number extr. : %012d\n"     , nHExt);
  printf("harmonic number inj.  : %012d\n"     , nHInj);
} // printDiags
*/


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

  uint64_t statusArray;
  uint64_t actStatusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t version;
  uint64_t tDiag;
  uint64_t tS0;
  uint32_t nTransfer;
  uint32_t nInjection;
  uint32_t statTrans;
  uint64_t TH1Ext;                             // h=1 period [as] of extraction machine
  uint64_t TH1Inj;                             // h=1 period [as] of injection machine
  uint32_t nHExt;                              // harmonic number extraction machine
  uint32_t nHInj;                              // harmonic number injection machine
  uint32_t fH1Ext;                             // h=1 frequency [Hz] of extraction machine
  uint32_t fH1Inj;                             // h=1 frequency [Hz] of injection machine
  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t actNTransfer;                       // actual number of transfers
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing

  uint64_t mac;                                // mac for config of EB master
  uint32_t ip;                                 // ip for config of EB master

  int      i;

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
      if ((logLevel < COMMON_LOGLEVEL_ALL) || (logLevel > COMMON_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
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

  api_initShared(lm32_base, SHARED_OFFS);
  b2btest_TH1ExtHi     = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TH1EXTHI;
  b2btest_TH1ExtLo     = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TH1EXTLO;
  b2btest_nHExt        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_NHEXT;
  b2btest_TH1InjHi     = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TH1INJHI;
  b2btest_TH1InjLo     = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TH1INJLO;
  b2btest_nHInj        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_NHINJ;
  

  if (getConfig) {
    readConfig(&mac, &ip);
    printf("b2b-test: EB Master: mac 0x%012"PRIx64", ip %03d.%03d.%03d.%03d\n", mac, (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  } // if getConfig

  if (getVersion) {
    eb_device_read(device, b2btest_version, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    version = data;
    printf("b2b-test: software (firmware) version %s (%06x)\n",  B2BTEST_X86_VERSION, version);     
  } // if getEBVersion

  if (getInfo) {
    // status
    api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, 0);
    printTransferHeader();
    printTransfer(nTransfer);
    printf(", %s (%6u), ",  api_stateText(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    eb_device_read(device, b2btest_state, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    state = data;

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP  , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP   , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b-test: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER  , 0, eb_block);
      if (state != COMMON_STATE_ERROR) printf("b2b-test: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE     , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"

    // diagnostics
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b-test: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, 1);
      // readDiags(&sumStatus, &state, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &transStat, &TH1Ext, &nHExt, &TH1Inj, &nHInj);
      //printDiags(sumStatus, state, nBadStatus, nBadState, tDiag, tS0, nTransfer, transStat, TH1Ext, nHExt, TH1Inj, nHInj);
    } // "diag"

    if (!strcasecmp(command, "seth1inj")) {
      if (optind+3  != argc) {printf("b2b-test: expecting exactly two arguments: seth1inj <freq> <h>\n"); return 1;}

      fH1Inj = strtoul(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b-test: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      TH1Inj = (double)1000000000000000000.0 / (double)fH1Inj;  // period in attoseconds

      nHInj  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b-test: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}

      /* chk, consider using one cycle */
      eb_device_write(device, b2btest_TH1InjHi, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj >> 32)       , 0, eb_block);
      eb_device_write(device, b2btest_TH1InjLo, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj & 0xffffffff), 0, eb_block);
      eb_device_write(device, b2btest_nHInj,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHInj                , 0, eb_block);
    } // "seth1inj"
    
    if (!strcasecmp(command, "seth1ext")) {
      if (optind+3  != argc) {printf("b2b-test: expecting exactly two arguments: seth1ext <freq> <h> \n"); return 1;}

      fH1Ext = strtoul(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b-test: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      TH1Ext = (double)1000000000000000000.0 / (double)fH1Ext;  // period in attoseconds

      nHExt  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b-test: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}
      
      /* chk, consider using one cycle */      
      eb_device_write(device, b2btest_TH1ExtHi, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext >> 32)       , 0, eb_block);
      eb_device_write(device, b2btest_TH1ExtLo, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext & 0xffffffff), 0, eb_block);
      eb_device_write(device, b2btest_nHExt,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHExt                , 0, eb_block);
    } // "seth1ext"

  } //if command

  if (snoop) {
    printf("b2b-test: continous monitoring of gateway, loglevel = %d\n", logLevel);

    actNTransfer   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    printTransferHeader();

    while (1) {
      api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, 0);

      switch(state) {
      case COMMON_STATE_OPREADY :
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
        printTransfer(nTransfer); 
        printf(", %s (%6u), ",  api_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", api_statusText(i));
        } // for i
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

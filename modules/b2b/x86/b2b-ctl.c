/*******************************************************************************************
 *  b2b-ctl.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 25-November-2020
 *
 * Command-line interface for b2b
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

// b2b
#include <common-lib.h>                  // COMMON
#include <b2b-api.h>                     // API
#include <b2b.h>                         // FW
#include <b2bcbu_shared_mmap.h>          // LM32

const char* program;
static int getInfo    = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32


// application specific stuff
// set values
eb_address_t b2b_set_gid;      // GID for transfer
eb_address_t b2b_set_sid;      // SID for transfer    
eb_address_t b2b_set_mode;     // mode of B2B transfer
eb_address_t b2b_set_TH1ExtHi; // period of h=1 extraction, high bits
eb_address_t b2b_set_TH1ExtLo; // period of h=1 extraction, low bits
eb_address_t b2b_set_nHExt;    // harmonic number of extraction RF
eb_address_t b2b_set_TH1InjHi; // period of h=1 injection, high bits
eb_address_t b2b_set_TH1InjLo; // period of h=1 injection, low bits
eb_address_t b2b_set_nHInj;    // harmonic number of injection RF
eb_address_t b2b_set_cPhase;   // phase correction
eb_address_t b2b_set_cTrigExt; // kicker correction extraction
eb_address_t b2b_set_cTrigInj; // kicker correction injection
eb_address_t b2b_cmd;          // command, write


// get values
eb_address_t b2b_gid;          // GID for transfer
eb_address_t b2b_sid;          // SID for transfer    
eb_address_t b2b_mode;         // mode of B2B transfer
eb_address_t b2b_TH1ExtHi;     // period of h=1 extraction, high bits
eb_address_t b2b_TH1ExtLo;     // period of h=1 extraction, low bits
eb_address_t b2b_nHExt;        // harmonic number of extraction RF
eb_address_t b2b_TH1InjHi;     // period of h=1 injection, high bits
eb_address_t b2b_TH1InjLo;     // period of h=1 injection, low bits
eb_address_t b2b_nHInj;        // harmonic number of injection RF
eb_address_t b2b_TBeatHi;      // period of beating, high bits
eb_address_t b2b_TBeatLo;      // period of beating, low bits
eb_address_t b2b_cPhase;       // phase correction
eb_address_t b2b_cTrigExt;     // kicker correction extraction
eb_address_t b2b_cTrigInj;     // kicker correction injection
eb_address_t b2b_comLatency;    // latency for message transfer via ECA
 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  /*fprintf(stderr, "  -c                  display configuration of B2B\n");*/
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on B2B\n");
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
  fprintf(stderr, "  diag                shows statistics and detailed information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  seth1inj <freq> <h> set h=1 frequency [Hz] and harmonic number of injection machine\n");
  fprintf(stderr, "  seth1ext <freq> <h> set h=1 frequency [Hz] and harmonic number of extraction machine\n");
  fprintf(stderr, "  setgid      <SID>   set Group ID of B2B transfer ('0x3a1')\n");
  fprintf(stderr, "  setsid      <SID>   set Sequence ID of schedule in extraction machine; allowed range is 0x0..0xf\n");
  fprintf(stderr, "  setmode     <SID>   set mode (0: Off, 1: EVT_KICK_START, 2: B2Extraction, 3: B2Coasting, 4: B2Bucket\n");
  fprintf(stderr, "  setcphase   <offs>  set correction for phase matching [ns]\n");
  fprintf(stderr, "  setctrigext <offs>  set correction for trigger kicker extraction [ns]\n");
  fprintf(stderr, "  setctriginj <offs>  set correction for trigger kicker injection [ns]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  submit              submits values that have been set\n");
  fprintf(stderr, "  clearconfig         clears configuration data for all (0x0..0xf) Sequence IDs\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Tip: For using negative values with commands such as 'snoop', consider\n");
  fprintf(stderr, "using the special argument '--' to terminate option scanning.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control B2B from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "b2b:        nTrans |                 INFO\n");
  fprintf(stderr, "b2b: STATUS      n |   state      nchng     stat      nchng\n");
  fprintf(stderr, "b2b:    0000065325 | OpReady    (     1), status 0x00000001 (     0)\n");
  fprintf(stderr, "                 ' |       '          '                   '       '\n");
  fprintf(stderr, "                 ' |       '          '                   '       '- # of bad status incidents\n");
  fprintf(stderr, "                 ' |       '          '                   '- status (bitwise ored)\n");
  fprintf(stderr, "                 ' |       '          '- # of bad state incidents\n");
  fprintf(stderr, "                 ' |       '- state\n");
  fprintf(stderr, "                 '- # of transfers\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", B2B_X86_VERSION);
} //help


const char* statusText(uint32_t bit) {  
  static char message[256];

  switch (bit) {
    case B2B_STATUS_PHASEFAILED      : sprintf(message, "error %d, %s",    bit, "phase measurement failed"); break;
    case B2B_STATUS_TRANSFER         : sprintf(message, "error %d, %s",    bit, "transfer failed"); break;
    case B2B_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
    default                          : sprintf(message, "%s",  comlib_statusText(bit)) ; break;
  }

  return message;
} // status_text


int readDiags(uint32_t *gid, uint32_t *sid, uint32_t *mode, uint64_t *TH1Ext, uint32_t *nHExt, uint64_t *TH1Inj, uint32_t *nHInj, uint64_t *TBeat, int32_t *cPhase, int32_t *cTrigExt, int32_t *cTrigInj, int32_t *comLatency)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2b_gid,           EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2b_sid,           EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2b_mode,          EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2b_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2b_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, b2b_nHExt,         EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, b2b_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, b2b_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, b2b_nHInj,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, b2b_TBeatHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, b2b_TBeatLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, b2b_cPhase,        EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, b2b_cTrigExt,      EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, b2b_cTrigInj,      EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, b2b_comLatency,    EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b: eb_cycle_close", eb_status);

  *gid           = data[0];
  *sid           = data[1];
  *mode          = data[2];
  *TH1Ext        = (uint64_t)(data[3]) << 32;
  *TH1Ext       += data[4];
  *nHExt         = data[5];
  *TH1Inj        = (uint64_t)(data[6]) << 32;
  *TH1Inj       += data[7];
  *nHInj         = data[8];
  *TBeat         = (uint64_t)(data[9]) << 32;
  *TBeat        += data[10];
  *cPhase        = data[11];
  *cTrigExt      = data[12];
  *cTrigInj      = data[13];
  *comLatency    = data[14];
 
  return eb_status;
} // readDiags


void printTransferHeader()
{
  printf("b2b:        nTrans |                 INFO                  \n");
  printf("b2b: STATUS      n |   state      nchng     stat      nchng\n");
} // printTransferHeader


void printTransfer(uint32_t nTransfer)
{
  // diag
  printf("b2b:    %010u |", nTransfer);

} // printTransfer


void printDiags(uint32_t gid, uint32_t sid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj, uint64_t TBeat, int32_t cPhase, int32_t cTrigExt, int32_t cTrigInj, int32_t comLatency)
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
  printf("corr. matching        : %012d\n"     , cPhase);
  printf("corr. trigger extr    : %012d\n"     , cTrigExt);
  printf("corr. trigger inj     : %012d\n"     , cTrigInj);
  printf("communication latency : %012.3f us\n", (double)comLatency/1000.0);
} // printDiags


int main(int argc, char** argv) {
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

  
  eb_status_t         eb_status;
  eb_socket_t         socket;

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
  uint32_t usedSize;
  uint64_t tDiag;
  uint64_t tS0;
  uint32_t nTransfer;
  uint32_t nInjection;
  uint32_t statTrans;
  uint32_t gid;                                // GID 
  uint32_t sid;                                // SID
  uint32_t mode;                               // mode
  uint64_t TH1Ext;                             // h=1 period [as] of extraction machine
  uint64_t TH1Inj;                             // h=1 period [as] of injection machine
  uint32_t nHExt;                              // harmonic number extraction machine
  uint32_t nHInj;                              // harmonic number injection machine
  uint64_t TBeat;                              // period [as] of frequency beating
  int32_t  cPhase;                             // phase correction
  int32_t  cTrigExt;                           // trigger correction extraction
  int32_t  cTrigInj;                           // trigger correction injection
  int32_t  comLatency;                         // message latency from ECA
  double   fH1Ext;                             // h=1 frequency [Hz] of extraction machine
  double   fH1Inj;                             // h=1 frequency [Hz] of injection machine
  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t actNTransfer;                       // actual number of transfers
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing

  uint64_t mac;                                // mac for config of EB master
  uint32_t ip;                                 // ip for config of EB master

  int      i;

  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:eih")) != -1) {
    switch (opt) {
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
  if ((eb_status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDRX|EB_DATAX, &socket)) != EB_OK) die("eb_socket_open", eb_status);
  if ((eb_status = eb_device_open(socket, devName, EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK) die("eb_device_open", eb_status);

  /* get device Wishbone address of lm32 */
  nDevices = 1; // quick and dirty, use only first core
  if ((eb_status = eb_sdb_find_by_identity(device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) die("find lm32", eb_status);
  lm32_base =  sdbDevice.sdb_component.addr_first;

  comlib_initShared(lm32_base, SHARED_OFFS);
  b2b_cmd          = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  b2b_set_gid      = lm32_base + SHARED_OFFS + B2B_SHARED_SET_GID;     
  b2b_set_sid      = lm32_base + SHARED_OFFS + B2B_SHARED_SET_SID;     
  b2b_set_mode     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_MODE;;   
  b2b_set_TH1ExtHi = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTHI;
  b2b_set_TH1ExtLo = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTLO;
  b2b_set_nHExt    = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHEXT;   
  b2b_set_TH1InjHi = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJHI;
  b2b_set_TH1InjLo = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJLO;
  b2b_set_nHInj    = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHINJ;
  b2b_set_cPhase   = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CPHASE;  
  b2b_set_cTrigExt = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGEXT;
  b2b_set_cTrigInj = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGINJ;                   
  b2b_gid          = lm32_base + SHARED_OFFS + B2B_SHARED_GID;
  b2b_sid          = lm32_base + SHARED_OFFS + B2B_SHARED_SID;
  b2b_mode         = lm32_base + SHARED_OFFS + B2B_SHARED_MODE;;
  b2b_TH1ExtHi     = lm32_base + SHARED_OFFS + B2B_SHARED_TH1EXTHI;
  b2b_TH1ExtLo     = lm32_base + SHARED_OFFS + B2B_SHARED_TH1EXTLO;
  b2b_nHExt        = lm32_base + SHARED_OFFS + B2B_SHARED_NHEXT;
  b2b_TH1InjHi     = lm32_base + SHARED_OFFS + B2B_SHARED_TH1INJHI;
  b2b_TH1InjLo     = lm32_base + SHARED_OFFS + B2B_SHARED_TH1INJLO;
  b2b_nHInj        = lm32_base + SHARED_OFFS + B2B_SHARED_NHINJ;
  b2b_TBeatHi      = lm32_base + SHARED_OFFS + B2B_SHARED_TBEATHI;
  b2b_TBeatLo      = lm32_base + SHARED_OFFS + B2B_SHARED_TBEATLO;
  b2b_cPhase       = lm32_base + SHARED_OFFS + B2B_SHARED_CPHASE;
  b2b_cTrigExt     = lm32_base + SHARED_OFFS + B2B_SHARED_CTRIGEXT;
  b2b_cTrigInj     = lm32_base + SHARED_OFFS + B2B_SHARED_CTRIGINJ;
  b2b_comLatency   = lm32_base + SHARED_OFFS + B2B_SHARED_COMLATENCY;
  
  if (getVersion) {
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &usedSize, 0);
    printf("b2b: software (firmware) version %s (%06x)\n",  B2B_X86_VERSION, version);     
  } // if getEBVersion

  if (getInfo) {
    // status
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &usedSize, 0);
    printTransferHeader();
    printTransfer(nTransfer);
    printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &usedSize, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("b2b: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP  , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP   , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER  , 0, eb_block);
      if (state != COMMON_STATE_ERROR) printf("b2b: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE     , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "submit")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFSUBMIT, 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "clearconfig")) {
      eb_device_write(device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFCLEAR, 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &usedSize, 1);
      readDiags(&gid, &sid, &mode, &TH1Ext, &nHExt, &TH1Inj, &nHInj, &TBeat, &cPhase, &cTrigExt, &cTrigInj, &comLatency);
      printDiags(gid, sid, mode, TH1Ext, nHExt, TH1Inj, nHInj, TBeat, cPhase, cTrigExt, cTrigInj, comLatency);
    } // "diag"

    if (!strcasecmp(command, "seth1inj")) {
      if (optind+3  != argc) {printf("b2b: expecting exactly two arguments: seth1inj <freq> <h>\n"); return 1;}

      fH1Inj = strtod(argv[optind+1], &tail);
      if (*tail != 0)        {printf("b2b: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      printf("b2b: lsa %f [Hz], dds %f [Hz]\n", fH1Inj, api_flsa2fdds(fH1Inj));
      TH1Inj = (double)1000000000000000000.0 / api_flsa2fdds(fH1Inj);  // period in attoseconds

      nHInj  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}

      /* chk, consider using one cycle */
      eb_device_write(device, b2b_set_TH1InjHi, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj >> 32)       , 0, eb_block);
      eb_device_write(device, b2b_set_TH1InjLo, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj & 0xffffffff), 0, eb_block);
      eb_device_write(device, b2b_set_nHInj,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHInj                , 0, eb_block);
    } // "seth1inj"
    
    if (!strcasecmp(command, "seth1ext")) {
      if (optind+3  != argc) {printf("b2b: expecting exactly two arguments: seth1ext <freq> <h> \n"); return 1;}

      fH1Ext = strtod(argv[optind+1], &tail);
      if (*tail != 0)        {printf("b2b: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      printf("b2b: lsa %f [Hz], dds %f [Hz]\n", fH1Ext, api_flsa2fdds(fH1Ext));
      TH1Ext = (double)1000000000000000000.0 / api_flsa2fdds(fH1Ext);  // period in attoseconds

      nHExt  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}
      
      /* chk, consider using one cycle */      
      eb_device_write(device, b2b_set_TH1ExtHi, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext >> 32)       , 0, eb_block);
      eb_device_write(device, b2b_set_TH1ExtLo, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext & 0xffffffff), 0, eb_block);
      eb_device_write(device, b2b_set_nHExt,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHExt                , 0, eb_block);
    } // "seth1ext"

   if (!strcasecmp(command, "setgid")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setgid <value>\n"); return 1;}

      gid = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid group ID -- %s\n", argv[optind+2]); return 1;}

      printf("gidExt %d\n", gid);
            
      eb_device_write(device, b2b_set_gid,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)gid        , 0, eb_block);
   } // "setgid"  

   if (!strcasecmp(command, "setsid")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setsid <value>\n"); return 1;}

      sid = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid sequence ID -- %s\n", argv[optind+2]); return 1;}

      printf("sid %d\n", sid);
            
      eb_device_write(device, b2b_set_sid,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)sid        , 0, eb_block);
   } // "setsid"  

   if (!strcasecmp(command, "setmode")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setmode <value>\n"); return 1;}

      mode  = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid mode -- %s\n", argv[optind+2]); return 1;}

      printf("mode %d\n", mode);
            
      eb_device_write(device, b2b_set_mode,     EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)mode       , 0, eb_block);
   } // "setmode"  

   if (!strcasecmp(command, "setcphase")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setcphase <value>\n"); return 1;}

      cPhase = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cPhase %d\n", cPhase);
            
      eb_device_write(device, b2b_set_cPhase,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)cPhase      , 0, eb_block);
   } // "setcphase"  

   if (!strcasecmp(command, "setctrigext")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setctrigext <value>\n"); return 1;}

      cTrigExt = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cTrigExt %d\n", cTrigExt);
            
      eb_device_write(device, b2b_set_cTrigExt, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)cTrigExt    , 0, eb_block);
   } // "setctrigext"  

   if (!strcasecmp(command, "setctriginj")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setctriginj <value>\n"); return 1;}

      cTrigInj = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cTrigInj %d\n", cTrigInj);
            
      eb_device_write(device, b2b_set_cTrigInj, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)cTrigInj    , 0, eb_block);
   } // "setctriginj"  

  } //if command

if (snoop) {
    printf("b2b: continous monitoring of gateway, loglevel = %d\n", logLevel);

    actNTransfer   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    printTransferHeader();

    while (1) {
      comlib_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &usedSize, 0);

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
        printf(", %s (%6u), ",  comlib_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", comlib_statusText(i));
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

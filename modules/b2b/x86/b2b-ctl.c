/*******************************************************************************************
 *  b2b-ctl.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 10-December-2020
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
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW
#include <b2bcbu_shared_mmap.h>          // LM32

const char* program;
static int getInfo    = 0;
static int getVersion = 0;
static int ddsValue   = 0;
static int snoop      = 0;
static int logLevel   = 0;

/*
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

*/
 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  uint32_t version;
  
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

  b2b_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(version));
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

/*
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
*/

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


void printDiags(uint32_t sid, uint32_t gid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj, uint64_t TBeat, int32_t cPhase, int32_t cTrigExt, int32_t cTrigInj, int32_t comLatency)
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

void initSetValues(uint32_t *setsid, uint32_t *setgid, uint32_t *setmode, uint64_t *setTH1Ext, uint64_t *setTH1Inj, uint32_t *setnHExt, uint32_t *setnHInj, int32_t  *setcPhase, int32_t  *setcTrigExt, int32_t  *setcTrigInj)
{
  *setsid      = 0;
  *setgid      = 0;
  *setmode     = 0;
  *setTH1Ext   = 0;
  *setTH1Inj   = 0;
  *setnHExt    = 1;
  *setnHInj    = 1;
  *setcPhase   = 0;
  *setcTrigExt = 0;
  *setcTrigInj = 0;
} // iniSetValues


int main(int argc, char** argv) {
  /*
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351
  */
  
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
  double   fH1Ext;                             // h=1 frequency [Hz] of extraction machine
  double   fH1Inj;                             // h=1 frequency [Hz] of injection machine
  uint32_t setsid;                             // SID
  uint32_t setgid;                             // GID 
  uint32_t setmode;                            // mode
  uint64_t setTH1Ext;                          // h=1 period [as] of extraction machine
  uint64_t setTH1Inj;                          // h=1 period [as] of injection machine
  uint32_t setnHExt;                           // harmonic number extraction machine
  uint32_t setnHInj;                           // harmonic number injection machine
  int32_t  setcPhase;                          // phase correction
  int32_t  setcTrigExt;                        // trigger correction extraction
  int32_t  setcTrigInj;                        // trigger correction injection
  uint32_t getsid;                             // SID
  uint32_t getgid;                             // GID 
  uint32_t getmode;                            // mode
  uint64_t getTH1Ext;                          // h=1 period [as] of extraction machine
  uint64_t getTH1Inj;                          // h=1 period [as] of injection machine
  uint32_t getnHExt;                           // harmonic number extraction machine
  uint32_t getnHInj;                           // harmonic number injection machine
  int32_t  getcPhase;                          // phase correction
  int32_t  getcTrigExt;                        // trigger correction extraction
  int32_t  getcTrigInj;                        // trigger correction injection
  uint64_t getTBeat;                           // period [as] of frequency beating
  int32_t  getcomLatency;                      // message latency from ECA

  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t actNTransfer;                       // actual number of transfers
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing
  uint32_t verLib;
  uint32_t verFw;

  int      i;

  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;


  program = argv[0];    

  while ((opt = getopt(argc, argv, "s:eihd")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'i':
        getInfo = 1;
        break;
      case 'd':
        ddsValue = 1;
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

  if ((status =  b2b_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);
  initSetValues(&setsid, &setgid, &setmode, &setTH1Ext, &setTH1Inj, &setnHExt, &setnHInj, &setcPhase, &setcTrigExt, &setcTrigInj);
  
  if (getVersion) {
    b2b_version_library(&verLib);
    printf("b2b: library (firmware) version %s",  b2b_version_text(verLib));     
    b2b_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  b2b_version_text(verFw));     
  } // if getVersion

  if (getInfo) {
    // status
    b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, &getcomLatency);
    b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    printTransferHeader();
    printTransfer(nTransfer);
    printf(", %s (%6u), ", b2b_state_text(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", b2b_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      b2b_cmd_configure(ebDevice);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("b2b: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      b2b_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      b2b_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      b2b_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("b2b: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      b2b_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("b2b: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    // diagnostics

    if (!strcasecmp(command, "cleardiag")) {
      b2b_cmd_cleardiag(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 1);
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", b2b_status_text(i));
      } // for i
      b2b_info_read(ebDevice, &getsid, &getgid, &getmode, &getTH1Ext, &getnHExt, &getTH1Inj, &getnHInj, &getTBeat, &getcPhase, &getcTrigExt, &getcTrigInj, &getcomLatency);
      printDiags(getsid, getgid, getmode, getTH1Ext, getnHExt, getTH1Inj, getnHInj, getTBeat, getcPhase, getcTrigExt, getcTrigInj, getcomLatency);
    } // "diag"

    if (!strcasecmp(command, "submit")) {
      b2b_context_upload(ebDevice, setsid, setgid, setmode, setTH1Ext, setnHExt, setTH1Inj, setnHInj, setcPhase, setcTrigExt, setcTrigInj);
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "submit"
    
    if (!strcasecmp(command, "clearconfig")) {
      b2b_cmd_clearConfig(ebDevice);      
      if (state != COMMON_STATE_OPREADY) printf("b2b: WARNING command has no effect (not in state OPREADY)\n");
    } // "clearconfig"

    if (!strcasecmp(command, "seth1inj")) {
      if (optind+3  != argc) {printf("b2b: expecting exactly two arguments: seth1inj <freq> <h>\n"); return 1;}

      fH1Inj = strtod(argv[optind+1], &tail);
      if (*tail != 0)        {printf("b2b: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      if (ddsValue) {
        printf("b2b: dds %f [Hz]\n", fH1Inj);
        setTH1Inj = (double)1000000000000000000.0 / (double)fH1Inj;         // period in attoseconds       
      } // if ddsValue     
      else {
        printf("b2b: lsa %f [Hz], dds %f [Hz]\n", fH1Inj, b2b_flsa2fdds(fH1Inj));
        setTH1Inj = (double)1000000000000000000.0 / b2b_flsa2fdds(fH1Inj);  // period in attoseconds
      } // else ddsValue

      setnHInj  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}

      // values get written to FW on command 'submit'
    } // "seth1inj"
    
    if (!strcasecmp(command, "seth1ext")) {
      if (optind+3  != argc) {printf("b2b: expecting exactly two arguments: seth1ext <freq> <h> \n"); return 1;}

      fH1Ext = strtod(argv[optind+1], &tail);
      if (*tail != 0)        {printf("b2b: invalid frequency -- %s\n", argv[optind+2]); return 1;}
      if (ddsValue) {
        printf("b2b: dds %f [Hz]\n", fH1Ext);
        setTH1Ext = (double)1000000000000000000.0 / (double)fH1Ext;         // period in attoseconds
      } // if ddsValue     
      else {
        printf("b2b: lsa %f [Hz], dds %f [Hz]\n", fH1Ext, b2b_flsa2fdds(fH1Ext));
        setTH1Ext = (double)1000000000000000000.0 / b2b_flsa2fdds(fH1Ext);  // period in attoseconds
      } // else ddsValue

      setnHExt  = strtoul(argv[optind+2], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid harmonic number -- %s\n", argv[optind+3]); return 1;}
      // values get written to FW on command 'submit'
    } // "seth1ext"

   if (!strcasecmp(command, "setgid")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setgid <value>\n"); return 1;}

      setgid = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid group ID -- %s\n", argv[optind+2]); return 1;}

      printf("gidExt %d\n", setgid);
      // values get written to FW on command 'submit'            
   } // "setgid"  

   if (!strcasecmp(command, "setsid")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setsid <value>\n"); return 1;}

      setsid = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid sequence ID -- %s\n", argv[optind+2]); return 1;}

      printf("sid %d\n", setsid);
      // values get written to FW on command 'submit'            
   } // "setsid"  

   if (!strcasecmp(command, "setmode")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setmode <value>\n"); return 1;}

      setmode  = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid mode -- %s\n", argv[optind+2]); return 1;}

      printf("mode %d\n", setmode);
      // values get written to FW on command 'submit'            
   } // "setmode"  

   if (!strcasecmp(command, "setcphase")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setcphase <value>\n"); return 1;}

      setcPhase = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cPhase %d\n", setcPhase);
      // values get written to FW on command 'submit'            
   } // "setcphase"  

   if (!strcasecmp(command, "setctrigext")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setctrigext <value>\n"); return 1;}

      setcTrigExt = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cTrigExt %d\n", setcTrigExt);
      // values get written to FW on command 'submit'            
   } // "setctrigext"  

   if (!strcasecmp(command, "setctriginj")) {
      if (optind+2  != argc) {printf("b2b: expecting exactly one argument: setctriginj <value>\n"); return 1;}

      setcTrigInj = strtol(argv[optind+1], &tail, 0);
      if (*tail != 0)        {printf("b2b: invalid calibration value -- %s\n", argv[optind+2]); return 1;}

      printf("cTrigInj %d\n", setcTrigInj);
      // values get written to FW on command 'submit'            
   } // "setctriginj"  

  } //if command

if (snoop) {
    printf("b2b: continous monitoring of gateway, loglevel = %d\n", logLevel);

    actNTransfer   = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    printTransferHeader();

    while (1) {
      b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);

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

  // close connection to firmware
  if ((status = b2b_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

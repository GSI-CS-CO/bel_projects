
/********************************************************************************************
 *  b2btest-ctl.c
 *
 *  created : 2019
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 15-April-2019
 *
 * Command-line interface for wrunipz
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
#include <b2btest_shared_mmap.h>         // LM32

const char* program;
static int getInfo    = 0;
static int getConfig  = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

eb_device_t  device;               // keep this and below global
eb_address_t lm32_base;            // base address of lm32
eb_address_t b2btest_status;       // status of b2btest, read
eb_address_t b2btest_state;        // state, read
eb_address_t b2btest_iterations;   // # of iterations of main loop, read
eb_address_t b2btest_cycles;       // # of UNILAC cycles
eb_address_t b2btest_cmd;          // command, write
eb_address_t b2btest_version;      // version, read
eb_address_t b2btest_macHi;        // ebm src mac, read
eb_address_t b2btest_macLo;        // ebm src mac, read
eb_address_t b2btest_ip;           // ebm src ip, read
eb_address_t b2btest_nBadStatus;   // # of bad status ("ERROR") incidents, read
eb_address_t b2btest_nBadState;    // # of bad state ("not in operation") incidents, read
eb_address_t b2btest_tCycleAvg;    // period of cycle [us] (average over one second), read
eb_address_t b2btest_nMessageLo;   // number of messages, read
eb_address_t b2btest_nMessageHi;   // number of messages, read
eb_address_t b2btest_msgFreqAvg;   // message rate (average over one second), read
eb_address_t b2btest_dtMax;        // delta T (max) between message time of dispatching and deadline, read
eb_address_t b2btest_dtMin;        // delta T (min) between message time of dispatching and deadline, read
eb_address_t b2btest_nLate;        // # of late messages, read
eb_address_t b2btest_vaccAvg;      // virtual accelerators played over the past second, read
eb_address_t b2btest_pzAvg;        // PZs used over the past second, read
eb_address_t b2btest_mode;         // mode, see B2BTEST_MODE_...
eb_address_t b2btest_tDiagHi;      // time when diagnostics was cleared, high bits
eb_address_t b2btest_tDiagLo;      // time when diagnostics was cleared, low bits
eb_address_t b2btest_tS0Hi;        // time when FW was in S0 state (start of FW), high bits
eb_address_t b2btest_tS0Lo;        // time when FW was in S0 state (start of FW), low bits
eb_address_t b2btest_confVacc;     // virtAcc of config, write
eb_address_t b2btest_confStat;     // status of config transaction, read
eb_address_t b2btest_confPz;       // bit field (indicates, which PZ is submitted), write
eb_address_t b2btest_confFlag;     // flags of config, write
eb_address_t b2btest_confData;     // data of config, write

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
  fprintf(stderr, "  modespz             command sets to PZ mode (listen to SuperPZ)\n");
  fprintf(stderr, "  modetest            command sets to test mode (listen to internal 50 Hz trigger)\n");
  fprintf(stderr, "  test <vacc> <pz>    command loads dummy event table for virtual accelerator <vacc> to pulszentrale <pz>\n");
  fprintf(stderr, "  testfull            command loads dummy event tables for ALL virt accs (except virt acc 0xf) and all PZs\n");
  fprintf(stderr, "  cleartables         command clears all event tables of all PZs\n");
  fprintf(stderr, "  kill                command kills possibly ongoing transactions\n");  
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                shows statistics and detailled information\n");
  fprintf(stderr, "  cleardiag           command clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control B2B-TEST from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>', the following information is displayed\n");
  fprintf(stderr, "b2b-test:                  TRANSFERS                |                   INJECTION                     | DIAGNOSIS  |                    INFO   \n");
  fprintf(stderr, "b2b-test:              n    sum(tkr)  set(get)/noBm | n(r2s/sumr2s)   sum( prep/bmrq/r2sis->mbtrig)   | DIAG margn | status         state      nchng stat   nchng\n");
  fprintf(stderr, "b2b-test: TRANS 00057399,  5967( 13)ms, va 10(10)/0 | INJ 06(06/06),  964(0.146/   0/ 954 -> 9.979)ms | DG 1.453ms | 1 1 1 1 1 1, OpReady    (     0), OK (     4)\n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |      '  '  '      '     '    '    '        '    |        '   | ' ' ' ' ' '        '          '    '       ' \n");
  fprintf(stderr, "          |            '      '   '         '  '  ' |      '  '  '      '     '    '    '        '    |        '   | ' ' ' ' ' '        '          '    '       ' - # of 'bad status' incidents\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", B2BTEST_X86_VERSION);
} //help


int readInfo(uint32_t *sumStatus, uint32_t *state, uint32_t *cycles, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *tCycleAvg, uint32_t *msgFreqAvg, uint32_t *confStat, uint32_t *nLate, uint32_t *vaccAvg, uint32_t *pzAvg, uint32_t *mode)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b-test: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2btest_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2btest_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2btest_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2btest_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2btest_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, b2btest_tCycleAvg,     EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, b2btest_msgFreqAvg,    EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, b2btest_confStat,      EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, b2btest_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, b2btest_vaccAvg,       EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, b2btest_pzAvg,         EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, b2btest_mode,          EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b-test: eb_cycle_close", eb_status);

  *sumStatus     = data[0];
  *state         = data[1];
  *nBadStatus    = data[2];
  *nBadState     = data[3];
  *cycles        = data[4];
  *tCycleAvg     = data[5];
  *msgFreqAvg    = data[6];
  *confStat      = data[7];
  *nLate         = data[8];
  *vaccAvg       = data[9];
  *pzAvg         = data[10];
  *mode          = data[11];

  return eb_status;
} // readInfo


int readDiags(uint32_t *sumStatus, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *nCycles, uint64_t *nMessages, int32_t *dtMax, int32_t *dtMin, uint32_t *nLate, uint64_t *tDiag, uint64_t *tS0)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2b-test: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2btest_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2btest_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2btest_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2btest_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2btest_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, b2btest_nMessageHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, b2btest_nMessageLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, b2btest_dtMax,         EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, b2btest_dtMin,         EB_BIG_ENDIAN|EB_DATA32, &(data[8])); 
  eb_cycle_read(cycle, b2btest_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, b2btest_tDiagHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, b2btest_tDiagLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, b2btest_tS0Hi,         EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, b2btest_tS0Lo,         EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("b2b-test: eb_cycle_close", eb_status);

  *sumStatus     = data[0];
  *state         = data[1];
  *nBadStatus    = data[2];
  *nBadState     = data[3];
  *nCycles       = data[4];
  *nMessages     = (uint64_t)(data[5]) << 32;
  *nMessages    += data[6];
  *dtMax         = data[7];
  *dtMin         = data[8];
  *nLate         = data[9];
  *tDiag         = (uint64_t)(data[10]) << 32;
  *tDiag        += data[11];
  *tS0           = (uint64_t)(data[12]) << 32;
  *tS0          += data[13];
 
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


void printCycleHeader()
{
  printf("b2b-test:        cycles      virtAcc        PZ   |         DIAGNOSIS        |                 INFO           \n");
  printf("b2b-test: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate T M |   state      nchng stat   nchng\n");
} // printCycleHeader


void printCycle(uint32_t cycles, uint32_t tCycleAvg, uint32_t msgFreqAvg, uint32_t confStat, uint32_t nLate, uint32_t vaccAvg, uint32_t pzAvg, uint32_t mode)
{
  // past cycles
  printf("b2b-test: ST %010d ", cycles);

  // diag
  printf("DG %6.3f %05d %05d %1d %1d |", 1000000000.0/(double)tCycleAvg, msgFreqAvg, nLate, confStat, mode);

} // printCycle


void printDiags(uint32_t sumStatus, uint32_t state, uint32_t nBadStatus, uint32_t nBadState, uint32_t nCycles, uint64_t nMessages, int32_t dtMax, int32_t dtMin, uint32_t nLate, uint64_t tDiag, uint64_t tS0)
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
  
  printf("state (# of changes)  : %s (%u)\n", b2btest_state_text(state), nBadState);
  printf("sum status (# changes): 0x%08x (%u)\n", sumStatus, nBadStatus);
  if ((sumStatus >> B2BTEST_STATUS_OK) & 0x1)
    printf("overall status        : OK\n");
  else
    printf("overall status        : NOT OK\n");  
  for (i= B2BTEST_STATUS_OK + 1; i<(sizeof(sumStatus)*8); i++) {
    if ((sumStatus >> i) & 0x1)
      printf("sum status bit ist set: %s\n", b2btest_status_text(i));
  } // for i
  printf("# of cycles           : %010u\n",   nCycles);
  printf("# of messages         : %010lu\n",  nMessages);
  printf("# of late messages    : %010u\n",   nLate);
  printf("dt min [us]           : %08.1f\n",  (double)dtMin / 1000.0);
  printf("dt max [us]           : %08.1f\n",  (double)dtMax / 1000.0);
} // printDiags




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

  uint32_t sumStatus;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t cycles;
  uint64_t messages;
  uint32_t fMessages;
  uint32_t tCycle;
  uint32_t version;
  int32_t  dtMax;
  int32_t  dtMin;
  uint32_t nLate;
  uint32_t vaccAvg;
  uint32_t pzAvg;
  uint32_t mode;
  uint32_t confStat;
  uint64_t tDiag;
  uint64_t tS0;

  uint32_t actCycles;                          // actual number of cycles
  uint32_t actState = B2BTEST_STATE_UNKNOWN;   // actual state of gateway
  uint32_t actSumStatus;                       // actual sum status of gateway
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
      if ((logLevel < B2BTEST_LOGLEVEL_ALL) || (logLevel > B2BTEST_LOGLEVEL_STATE)) fprintf(stderr, "log level out of range\n");
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

  b2btest_status       = lm32_base + SHARED_OFFS + B2BTEST_SHARED_SUMSTATUS;
  b2btest_cmd          = lm32_base + SHARED_OFFS + B2BTEST_SHARED_CMD;
  b2btest_state        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_STATE;;
  b2btest_version      = lm32_base + SHARED_OFFS + B2BTEST_SHARED_VERSION;
  b2btest_macHi        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_MACHI;
  b2btest_macLo        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_MACLO;
  b2btest_ip           = lm32_base + SHARED_OFFS + B2BTEST_SHARED_IP;
  b2btest_nBadStatus   = lm32_base + SHARED_OFFS + B2BTEST_SHARED_NBADSTATUS;
  b2btest_nBadState    = lm32_base + SHARED_OFFS + B2BTEST_SHARED_NBADSTATE;
  b2btest_tDiagHi      = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TDIAGHI;
  b2btest_tDiagLo      = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TDIAGLO;
  b2btest_tS0Hi        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TS0HI;
  b2btest_tS0Lo        = lm32_base + SHARED_OFFS + B2BTEST_SHARED_TS0LO;

  // printf("b2b-test: lm32_base 0x%08x, 0x%08x\n", lm32_base, b2btest_iterations);

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
    readInfo(&sumStatus, &state, &cycles, &nBadStatus, &nBadState, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg, &mode);
    printCycleHeader();
    printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg, mode);
    printf(" %s (%6u), status 0x%08x (%6u)\n", b2btest_state_text(state), nBadState, sumStatus, nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    eb_device_read(device, b2btest_state, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    state = data;

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_CONFIGURE, 0, eb_block);
      if ((state != B2BTEST_STATE_CONFIGURED) && (state != B2BTEST_STATE_IDLE)) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_STARTOP  , 0, eb_block);
      if (state != B2BTEST_STATE_CONFIGURED) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_STOPOP   , 0, eb_block);
      if (state != B2BTEST_STATE_OPREADY) printf("b2b-test: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_RECOVER  , 0, eb_block);
      if (state != B2BTEST_STATE_ERROR) printf("b2b-test: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_IDLE     , 0, eb_block);
      if (state != B2BTEST_STATE_CONFIGURED) printf("b2b-test: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"

    // diagnostics
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, b2btest_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2BTEST_CMD_CLEARDIAG , 0, eb_block);
      if (state != B2BTEST_STATE_OPREADY) printf("b2b-test: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      readDiags(&sumStatus, &state, &nBadStatus, &nBadState, &cycles, &messages, &dtMax, &dtMin, &nLate, &tDiag, &tS0);
      printDiags(sumStatus, state, nBadStatus, nBadState, cycles, messages, dtMax, dtMin, nLate, tDiag, tS0);
    } // "diag"

  } //if command
  

  if (snoop) {
    printf("b2b-test: continous monitoring of gateway, loglevel = %d\n", logLevel);
    
    actCycles    = 0;
    actState     = B2BTEST_STATE_UNKNOWN;
    actSumStatus = 0;

    printCycleHeader();

    while (1) {
      readInfo(&sumStatus, &state, &cycles, &nBadStatus, &nBadState, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg, &mode); // read info from lm32

      switch(state) {
      case B2BTEST_STATE_OPREADY :
        if (actCycles != cycles) sleepTime = B2BTEST_DEFAULT_TIMEOUT * 1000 * 2;              // ongoing cycle: reduce polling rate ...
        else                     sleepTime = B2BTEST_DEFAULT_TIMEOUT * 1000;                  // sleep for default timeout to catch next cycle
        break;
      default:
        sleepTime = B2BTEST_DEFAULT_TIMEOUT * 1000;                          
      } // switch actState
      
      // if required, print status change
      if  ((actState != state) && (logLevel <= B2BTEST_LOGLEVEL_STATE)) printFlag = 1;

      // determine when to print info
      printFlag = 0;

      if ((actState     != state)        && (logLevel <= B2BTEST_LOGLEVEL_STATE))                                         {printFlag = 1; actState  = state;}
      if ((actSumStatus != sumStatus)    && (logLevel <= B2BTEST_LOGLEVEL_STATUS))                                        {printFlag = 1; actSumStatus = sumStatus;}

      if (printFlag) {
        printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg, mode); 
        printf(" %s (%6u), status 0x%08x (%d)\n", b2btest_state_text(state), nBadState, sumStatus, nBadStatus);
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

/********************************************************************************************
 *  wrunipz-ctl.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-January-2019
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
eb_address_t wrunipz_dtMax;        // delta T (max) between message time of dispatching and deadline, read
eb_address_t wrunipz_dtMin;        // delta T (min) between message time of dispatching and deadline, read
eb_address_t wrunipz_nLate;        // # of late messages, read
eb_address_t wrunipz_vaccAvg;      // virtual accelerators played over the past second, read
eb_address_t wrunipz_pzAvg;        // PZs used over the past second, read
eb_address_t wrunipz_mode;         // mode, see WRUNIPZ_MODE_...
eb_address_t wrunipz_tDiagHi;      // time when diagnostics was cleared, high bits
eb_address_t wrunipz_tDiagLo;      // time when diagnostics was cleared, low bits
eb_address_t wrunipz_tS0Hi;        // time when FW was in S0 state (start of FW), high bits
eb_address_t wrunipz_tS0Lo;        // time when FW was in S0 state (start of FW), low bits
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


int readInfo(uint32_t *sumStatus, uint32_t *state, uint32_t *cycles, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *tCycleAvg, uint32_t *msgFreqAvg, uint32_t *confStat, uint32_t *nLate, uint32_t *vaccAvg, uint32_t *pzAvg, uint32_t *mode)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, wrunipz_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, wrunipz_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, wrunipz_tCycleAvg,     EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, wrunipz_msgFreqAvg,    EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, wrunipz_confStat,      EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, wrunipz_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, wrunipz_vaccAvg,       EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, wrunipz_pzAvg,         EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, wrunipz_mode,          EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

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

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, wrunipz_status,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_state,         EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_nBadStatus,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, wrunipz_nBadState,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, wrunipz_nMessageHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, wrunipz_nMessageLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, wrunipz_dtMax,         EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, wrunipz_dtMin,         EB_BIG_ENDIAN|EB_DATA32, &(data[8])); 
  eb_cycle_read(cycle, wrunipz_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, wrunipz_tDiagHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, wrunipz_tDiagLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, wrunipz_tS0Hi,         EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, wrunipz_tS0Lo,         EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

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
  printf("wr-unipz:        cycles      virtAcc        PZ   |         DIAGNOSIS        |                 INFO           \n");
  printf("wr-unipz: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate T M |   state      nchng stat   nchng\n");
} // printCycleHeader


void printCycle(uint32_t cycles, uint32_t tCycleAvg, uint32_t msgFreqAvg, uint32_t confStat, uint32_t nLate, uint32_t vaccAvg, uint32_t pzAvg, uint32_t mode)
{
  int i;
  
  // past cycles
  printf("wr-unipz: ST %010d ", cycles);
  for (i=0; i < WRUNIPZ_NVACC; i++) printf("%d", (((1 << i) & vaccAvg) > 0));
  printf(" ");
  for (i=0; i < WRUNIPZ_NPZ;   i++) printf("%d", (((1 << i) & pzAvg) > 0));
  printf(" |");

  // diag
  printf("DG %6.3f %05d %05d %1d %1d |", 1000000000.0/(double)tCycleAvg, msgFreqAvg, nLate, confStat, mode);

} // printCycle


void printDiags(uint32_t sumStatus, uint32_t state, uint32_t nBadStatus, uint32_t nBadState, uint32_t nCycles, uint64_t nMessages, int32_t dtMax, int32_t dtMin, uint32_t nLate, uint64_t tDiag, uint64_t tS0)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;
  int              i;

  printf("wr-unipz: statistics ...\n\n");

  secs     = (unsigned long)((double)tS0 / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("firmware boot at      : %s\n", timestr);

  secs     = (unsigned long)((double)tDiag / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("diagnostics reset at  : %s\n", timestr);
  
  printf("state (# of changes)  : %s (%u)\n", wrunipz_state_text(state), nBadState);
  printf("sum status (# changes): 0x%08x (%u)\n", sumStatus, nBadStatus);
  if ((sumStatus >> WRUNIPZ_STATUS_OK) & 0x1)
    printf("overall status        : OK\n");
  else
    printf("overall status        : NOT OK\n");  
  for (i= WRUNIPZ_STATUS_OK + 1; i<(sizeof(sumStatus)*8); i++) {
    if ((sumStatus >> i) & 0x1)
      printf("sum status bit ist set: %s\n", wrunipz_status_text(i));
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
  int                 i,j,k;

  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores

  
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint32_t status;
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
  uint32_t actState = WRUNIPZ_STATE_UNKNOWN;   // actual state of gateway
  uint32_t actSumStatus;                       // actual sum status of gateway
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing
  uint64_t t1, t2;

  uint64_t mac;                                // mac for config of EB master
  uint32_t ip;                                 // ip for config of EB master

  // command test
  uint32_t    dataChn0[WRUNIPZ_NEVT];
  uint32_t    nDataChn0;
  uint32_t    dataChn1[WRUNIPZ_NEVT];
  uint32_t    nDataChn1;
  uint32_t    vacc;
  uint32_t    pz;

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

  wrunipz_status       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_SUMSTATUS;
  wrunipz_cmd          = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CMD;
  wrunipz_state        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_STATE;;
  wrunipz_tCycleAvg    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TCYCLEAVG;
  wrunipz_version      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_VERSION;
  wrunipz_macHi        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MACHI;
  wrunipz_macLo        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MACLO;
  wrunipz_ip           = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_IP;
  wrunipz_nBadStatus   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NBADSTATUS;
  wrunipz_nBadState    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NBADSTATE;
  wrunipz_cycles       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NCYCLE;
  wrunipz_nMessageHi   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGEHI;
  wrunipz_nMessageLo   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGELO;
  wrunipz_msgFreqAvg   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MSGFREQAVG;
  wrunipz_dtMax        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMAX;
  wrunipz_dtMin        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMIN;
  wrunipz_nLate        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NLATE;
  wrunipz_vaccAvg      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_VACCAVG;
  wrunipz_pzAvg        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_PZAVG;
  wrunipz_mode         = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MODE;
  wrunipz_tDiagHi      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TDIAGHI;
  wrunipz_tDiagLo      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TDIAGLO;
  wrunipz_tS0Hi        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TS0HI;
  wrunipz_tS0Lo        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TS0LO;
  wrunipz_confVacc     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_VACC;
  wrunipz_confStat     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_STAT;
  wrunipz_confPz       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_PZ;
  wrunipz_confData     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_DATA;
  wrunipz_confFlag     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_FLAG;

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
    readInfo(&sumStatus, &state, &cycles, &nBadStatus, &nBadState, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg, &mode);
    printCycleHeader();
    printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg, mode);
    printf(" %s (%6u), status 0x%08x (%6u)\n", wrunipz_state_text(state), nBadState, sumStatus, nBadStatus);
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    eb_device_read(device, wrunipz_state, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block);
    state = data;

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFIGURE, 0, eb_block);
      if ((state != WRUNIPZ_STATE_CONFIGURED) && (state != WRUNIPZ_STATE_IDLE)) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_STARTOP  , 0, eb_block);
      if (state != WRUNIPZ_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_STOPOP   , 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_RECOVER  , 0, eb_block);
      if (state != WRUNIPZ_STATE_ERROR) printf("wr-unipz: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_IDLE     , 0, eb_block);
      if (state != WRUNIPZ_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"

    // diagnostics
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CLEARDIAG , 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      readDiags(&sumStatus, &state, &nBadStatus, &nBadState, &cycles, &messages, &dtMax, &dtMin, &nLate, &tDiag, &tS0);
      printDiags(sumStatus, state, nBadStatus, nBadState, cycles, messages, dtMax, dtMin, nLate, tDiag, tS0);
    } // "diag"

    // ...
    if (!strcasecmp(command, "test")) {
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+3  != argc) {printf("wr-unipz: expecting exactly two arguments: test <vacc> <pz>\n"); return 1;}
      vacc = strtoul(argv[optind+1], &tail, 0);
      if ((vacc < 0) || (vacc >= WRUNIPZ_NVACC)) {printf("wr-unipz: invalid virtual accelerator -- %s\n", argv[optind+2]); return 1;}
      pz = strtoul(argv[optind+2], &tail, 0);
      if ((pz < 0) || (pz >= WRUNIPZ_NPZ)) {printf("wr-unipz: invalid PZ -- %s\n", argv[optind+2]); return 1;}

      t1 = getSysTime();

      if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, vacc)) !=  WRUNIPZ_STATUS_OK) {
        printf("wr-unipz: transaction init - %s\n", wrunipz_status_text(status));
      }
      else {
        // upload
        nDataChn0 = WRUNIPZ_NEVT;
        nDataChn1 = 0;
        for (i=0; i < (nDataChn0 -1); i++) dataChn0[i] = ((uint16_t)(i + 100 * pz + 2001) << 16) + i; // 1501 or 2001 
        dataChn0[nDataChn0 -1] = ((uint16_t)2000 << 16) + 64;
        if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, pz, dataChn0, nDataChn0, dataChn1, nDataChn1)) != WRUNIPZ_STATUS_OK)
          printf("wr-unipz: transaction upload - %s\n", wrunipz_status_text(status));
        
        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);

        t2 = getSysTime();
        printf("wr-unipz: transaction took %u us\n", (uint32_t)(t2 -t1));
      }
    } // "testfull"
    if (!strcasecmp(command, "testfull")) {
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");

      t1 = getSysTime();

      for (j=0; j < WRUNIPZ_NVACC - 1; j++) {  // only virt acc 0..14

        if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, j)) !=  WRUNIPZ_STATUS_OK) {
          printf("wr-unipz: transaction init (virt acc %d) - %s\n", j, wrunipz_status_text(status));
        } // if status
        else {
          // upload
          for (k=0; k < WRUNIPZ_NPZ; k++) {
            nDataChn0 = WRUNIPZ_NEVT;
            nDataChn1 = 0;
            for (i=0; i < (nDataChn0 -1); i++) dataChn0[i] = ((uint16_t)(i + 100 * k + 2001) << 16) + i; // 1501 or 2001
            dataChn0[nDataChn0 -1] = ((uint16_t)2000 << 16) + 64;
            if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, k, dataChn0, nDataChn0, dataChn1, nDataChn1)) != WRUNIPZ_STATUS_OK)
              printf("wr-unipz: transaction upload (virt acc %d, pz %d) - %s\n", j, k, wrunipz_status_text(status));
          } // for k
        } // else

        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);

      } // for j

      t2 = getSysTime();
      
      printf("wr-unipz: transaction took %u us per virtAcc\n", (uint32_t)(t2 -t1) / (WRUNIPZ_NVACC - 1));

    } // "testfull"
    if (!strcasecmp(command, "kill")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFKILL, 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "kill"
    if (!strcasecmp(command, "cleartables")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFCLEAR, 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleartables"
    if (!strcasecmp(command, "modespz")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_MODESPZ, 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "modespz"
    if (!strcasecmp(command, "modetest")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_MODETEST, 0, eb_block);
      if (state != WRUNIPZ_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "modetest"
    
      
  } //if command
  

  if (snoop) {
    printf("wr-unipz: continous monitoring of gateway, loglevel = %d\n", logLevel);
    
    actCycles    = 0;
    actState     = WRUNIPZ_STATE_UNKNOWN;
    actSumStatus = 0;

    printCycleHeader();

    while (1) {
      readInfo(&sumStatus, &state, &cycles, &nBadStatus, &nBadState, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg, &mode); // read info from lm32

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

      if ((actState     != state)        && (logLevel <= WRUNIPZ_LOGLEVEL_STATE))                                         {printFlag = 1; actState  = state;}
      if ((actSumStatus != sumStatus)    && (logLevel <= WRUNIPZ_LOGLEVEL_STATUS))                                        {printFlag = 1; actSumStatus = sumStatus;}

      if (printFlag) {
        printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg, mode); 
        printf(" %s (%6u), status 0x%08x (%d)\n", wrunipz_state_text(state), nBadState, sumStatus, nBadStatus);
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

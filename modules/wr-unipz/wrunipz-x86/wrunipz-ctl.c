/********************************************************************************************
 *  wrunipz-ctl.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 30-August-2019
 *
 *  command-line interface for wrunipz
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
#include <wrunipz-api.h>                 // API wrunipz
#include <b2btest-api.h>                 // API B2B
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
eb_address_t wrunipz_iterations;   // # of iterations of main loop, read
eb_address_t wrunipz_cycles;       // # of UNILAC cycles
eb_address_t wrunipz_cmd;          // command, write
eb_address_t wrunipz_tCycleAvg;    // period of cycle [us] (average over one second), read
eb_address_t wrunipz_nMessageLo;   // number of messages, read
eb_address_t wrunipz_nMessageHi;   // number of messages, read
eb_address_t wrunipz_msgFreqAvg;   // message rate (average over one second), read
eb_address_t wrunipz_dtMax;        // delta T (max) between message time of dispatching and deadline, read
eb_address_t wrunipz_dtMin;        // delta T (min) between message time of dispatching and deadline, read
eb_address_t wrunipz_cycJmpMax;    // delta T (max) between expected and actual start of UNILAC cycle, read
eb_address_t wrunipz_cycJmpMin;    // delta T (min) between expected and actual start of UNILAC cycle, read
eb_address_t wrunipz_nLate;        // # of late messages, read
eb_address_t wrunipz_vaccAvg;      // virtual accelerators played over the past second, read
eb_address_t wrunipz_pzAvg;        // PZs used over the past second, read
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


const char* wrunipz_statusText(uint32_t bit) {  
  static char message[256];

  switch (bit) {
    printf("bit %d\n", bit);
    case WRUNIPZ_STATUS_LATE            : sprintf(message, "error %d, %s",    bit, "a timing messages is not dispatched in time"); break;
    case WRUNIPZ_STATUS_EARLY           : sprintf(message, "error %d, %s",    bit, "a timing messages is dispatched unreasonably early (dt > UNILACPERIOD)"); break;
    case WRUNIPZ_STATUS_TRANSACTION     : sprintf(message, "error %d, %s",    bit, "transaction failed"); break;
    case WRUNIPZ_STATUS_MIL             : sprintf(message, "error %d, %s",    bit, "an error on MIL hardware occured (MIL piggy etc...)"); break;
    case WRUNIPZ_STATUS_NOMILEVENTS     : sprintf(message, "error %d, %s",    bit, "no MIL events from UNIPZ"); break;
    case WRUNIPZ_STATUS_WRONGVIRTACC    : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS with wrong virt acc number"); break;
    case WRUNIPZ_STATUS_SAFETYMARGIN    : sprintf(message, "error %d, %s",    bit, "violation of safety margin for sending data to the timing network"); break;
    case WRUNIPZ_STATUS_NOTIMESTAMP     : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA"); break;
    case WRUNIPZ_STATUS_BADTIMESTAMP    : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break;                     
    case WRUNIPZ_STATUS_ORDERTIMESTAMP  : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA and MIL Events are out of order"); break;
    default                             : return api_statusText(bit); break;
  }

  return message;
} // dmunipz_statusText

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -c                  display configuration of WR-UNIPZ\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on WR-UNIPZ\n");
  fprintf(stderr, "  -s<n>               snoop for information continuously\n");
  fprintf(stderr, "                      0: print info every second (default)\n");
  fprintf(stderr, "                      1: print every 10 seconds\n");
  fprintf(stderr, "                      2: as 1, inform in case of status or state changes\n");
  fprintf(stderr, "                      3: as 2, inform in case of state changes\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  commands:\n");
  fprintf(stderr, "  ---------\n");
  fprintf(stderr, "  configure                      requests state change from IDLE or CONFIGURED -> CONFIGURED\n");
  fprintf(stderr, "  startop                        requests state change from CONFIGURED -> OPREADY\n");
  fprintf(stderr, "  stopop                         requests state change from OPREADY -> STOPPING -> CONFIGURED\n");
  fprintf(stderr, "  recover                        tries to recover from state ERROR and transit to state IDLE\n");
  fprintf(stderr, "  idle                           requests state change to IDLE\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  test      <offset> <vacc> <pz> loads dummy table with starting at <offset> (us) for virt acc <vacc> to pulszentrale <pz> 0..6\n");
  fprintf(stderr, "  testfull  <offset>             loads dummy tables with starting at <offset> (us) for ALL virt accs (except 0xf) and all PZs\n");
  fprintf(stderr, "  ftest     <file> <vacc> <pz>   loads table from file for virt acc <vacc> to pulszentrale <pz> 0..6 from <file>\n");
  fprintf(stderr, "  ftestfull <file>               loads table from file for ALL virt accs and all PZs from <file>\n");
  fprintf(stderr, "  cleartables                    clears all event tables of all PZs\n");
  fprintf(stderr, "  kill                           kills possibly ongoing transactions\n");  
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                           shows statistics and detailled information\n");
  fprintf(stderr, "  cleardiag                      clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control WR-UNIPZ from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>' or '-i', the following information is displayed\n");
  fprintf(stderr, "wr-unipz:        cycles      virtAcc        PZ   |         DIAGNOSIS      |                 INFO           \n");
  fprintf(stderr, "wr-unipz: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate T |   state      nchng stat   nchng\n");
  fprintf(stderr, "wr-unipz: ST 0054834398 0010010111000011 1111111 |DG 49.972 02335 00000 0 | OpReady    (     0), status 0x00000001 (     0)\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' ' |       '          '                   '       ' \n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' ' |       '          '                   '       '- # of 'bad status' incidents\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' ' |       '          '                   '- status value ('1' is OK)\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' ' |       '          '- # of state changes\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' ' |       '- state\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' '- '1': transaction in progress\n");
  fprintf(stderr, "                      '                '       ' |        '     '     '- # of late messages\n");
  fprintf(stderr, "                      '                '       ' |        '     '- average message rate [Hz]\n");
  fprintf(stderr, "                      '                '       ' |        '- average UNILAC cycle rate [Hz]\n");
  fprintf(stderr, "                      '                '       '- '1': PZ is active\n");
  fprintf(stderr, "                      '                '- '1': vacc is played\n");
  fprintf(stderr, "                      '- # of UNILAC cycles\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", WRUNIPZ_X86_VERSION);
} //help


int readInfo(uint32_t *cycles, uint32_t *tCycleAvg, uint32_t *msgFreqAvg, uint32_t *confStat, uint32_t *nLate, uint32_t *vaccAvg, uint32_t *pzAvg)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_tCycleAvg,     EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_msgFreqAvg,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, wrunipz_confStat,      EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, wrunipz_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, wrunipz_vaccAvg,       EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, wrunipz_pzAvg,         EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

  *cycles        = data[0];
  *tCycleAvg     = data[1];
  *msgFreqAvg    = data[2];
  *confStat      = data[3];
  *nLate         = data[4];
  *vaccAvg       = data[5];
  *pzAvg         = data[6];

  return eb_status;
} // readInfo


int readDiags(uint32_t *nCycles, uint64_t *nMessages, int32_t *dtMax, int32_t *dtMin, int32_t *cycJmpMax, int32_t *cycJmpMin, uint32_t *nLate)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];

  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("wr-unipz: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, wrunipz_nMessageHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, wrunipz_nMessageLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, wrunipz_dtMax,         EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, wrunipz_dtMin,         EB_BIG_ENDIAN|EB_DATA32, &(data[4])); 
  eb_cycle_read(cycle, wrunipz_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, wrunipz_cycJmpMax,     EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, wrunipz_cycJmpMin,     EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("wr-unipz: eb_cycle_close", eb_status);

  *nCycles       = data[0];
  *nMessages     = (uint64_t)(data[1]) << 32;
  *nMessages    += data[2];
  *dtMax         = data[3];
  *dtMin         = data[4];
  *nLate         = data[5];
  *cycJmpMax     = data[6]; 
  *cycJmpMin     = data[7];
 
  return eb_status;
} // readDiags


/*
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
*/

void printCycleHeader()
{
  printf("wr-unipz:        cycles      virtAcc        PZ   |         DIAGNOSIS      |                 INFO           \n");
  printf("wr-unipz: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate T |   state      nchng stat   nchng\n");
} // printCycleHeader


void printCycle(uint32_t cycles, uint32_t tCycleAvg, uint32_t msgFreqAvg, uint32_t confStat, uint32_t nLate, uint32_t vaccAvg, uint32_t pzAvg)
{
  int i;
  
  // past cycles
  printf("wr-unipz: ST %010d ", cycles);
  for (i=0; i < WRUNIPZ_NVACC; i++) printf("%d", (((1 << i) & vaccAvg) > 0));
  printf(" ");
  for (i=0; i < WRUNIPZ_NPZ;   i++) printf("%d", (((1 << i) & pzAvg) > 0));
  printf(" |");

  // diag
  printf("DG %6.3f %05d %05d %1d |", 1000000000.0/(double)tCycleAvg, msgFreqAvg, nLate, confStat);

} // printCycle


void printDiags(uint32_t nCycles, uint64_t nMessages, int32_t dtMax, int32_t dtMin, int32_t cycJmpMax, int32_t cycJmpMin, uint32_t nLate)
{
  printf("\nwr-unipz: statistics ...\n");
  printf("# of cycles           : %010u\n",   nCycles);
  printf("# of messages         : %010lu\n",  nMessages);
  printf("# of late messages    : %010u\n",   nLate);
  printf("dt min [us]           : %08.1f\n",  (double)dtMin / 1000.0);
  printf("dt max [us]           : %08.1f\n",  (double)dtMax / 1000.0);
  printf("cycle jump min [us]   : %08.1f\n",  (double)cycJmpMin / 1000.0);
  printf("cycle jump max [us]   : %08.1f\n",  (double)cycJmpMax / 1000.0);
} // printDiags




int main(int argc, char** argv) {
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

  
  eb_status_t         eb_status;
  eb_socket_t         socket;
  int                 i,j,k;

  struct sdb_device   sdbDevice;          // instantiated lm32 core
  int                 nDevices;           // number of instantiated cores

  
  const char* devName;
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint32_t status;
  uint64_t statusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t nDummyT;
  uint32_t nDummyI;
  uint32_t statDummy;
  uint32_t cycles;
  uint64_t messages;
  uint32_t fMessages;
  uint32_t tCycle;
  uint32_t version;
  int32_t  dtMax;
  int32_t  dtMin;
  int32_t  cycJmpMax;
  int32_t  cycJmpMin;
  uint32_t nLate;
  uint32_t vaccAvg;
  uint32_t pzAvg;
  uint32_t confStat;
  uint64_t tDiag;
  uint64_t tS0;

  //  uint32_t actCycles;                          // actual number of cycles
  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint64_t actStatusArray;                     // actual sum status of gateway
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
  uint32_t    offset;
  char        *filename;

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
  wrunipz_cmd          = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  wrunipz_tCycleAvg    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TCYCLEAVG;
  wrunipz_cycles       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NCYCLE;
  wrunipz_nMessageHi   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGEHI;
  wrunipz_nMessageLo   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGELO;
  wrunipz_msgFreqAvg   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MSGFREQAVG;
  wrunipz_dtMax        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMAX;
  wrunipz_dtMin        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMIN;
  wrunipz_cycJmpMax    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CYCJMPMAX;
  wrunipz_cycJmpMin    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CYCJMPMIN;
  wrunipz_nLate        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NLATE;
  wrunipz_vaccAvg      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_VACCAVG;
  wrunipz_pzAvg        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_PZAVG;
  wrunipz_confVacc     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_VACC;
  wrunipz_confStat     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_STAT;
  wrunipz_confPz       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_PZ;
  wrunipz_confData     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_DATA;
  wrunipz_confFlag     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CONF_FLAG;

  // printf("wr-unipz: lm32_base 0x%08x, 0x%08x\n", lm32_base, wrunipz_iterations);

  if (getConfig) {
    api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nDummyT, &nDummyI, &statDummy, 0);
    printf("wr-unipz: EB Master: mac 0x%012"PRIx64", ip %03d.%03d.%03d.%03d\n", mac, (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  } // if getConfig

  if (getVersion) {
    api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nDummyT, &nDummyI, &statDummy, 0);
    printf("wr-unipz: software (firmware) version %s (%06x)\n",  WRUNIPZ_X86_VERSION, version);     
  } // if getEBVersion

  if (getInfo) {
    // status
    api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nDummyT, &nDummyI, &statDummy, 0);
    readInfo(&cycles, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg);
    printCycleHeader();
    printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg);
    /*    printf(" %s (%6u), status 0x%08x (%6u)\n", common_state_text(state), nBadState, statusArray, nBadStatus); */
    printf(", %s (%6u), ",  api_stateText(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrunipz_statusText(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    readInfo(&cycles, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"
    if (!strcasecmp(command, "startop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP  , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"
    if (!strcasecmp(command, "stopop")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP   , 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"
    if (!strcasecmp(command, "recover")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER  , 0, eb_block);
      if (state != COMMON_STATE_ERROR) printf("wr-unipz: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"
    if (!strcasecmp(command, "idle")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE     , 0, eb_block);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"
    if (!strcasecmp(command, "cleardiag")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG , 0, eb_block);
    } // "cleardiag"
    if (!strcasecmp(command, "diag")) {
      api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nDummyT, &nDummyI, &statDummy, 1);
      // print set status bits (except OK)
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", wrunipz_statusText(i));
      } // for i
      readDiags(&cycles, &messages, &dtMax, &dtMin, &cycJmpMax, &cycJmpMin, &nLate);
      printDiags(cycles, messages, dtMax, dtMin, cycJmpMax, cycJmpMin, nLate);
    } // "diag"

    // test with data
    if (!strcasecmp(command, "test")) {
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+4  != argc)                     {printf("wr-unipz: expecting exactly three arguments: test <offset> <vacc> <pz>\n"); return 1;}
      offset = strtoul(argv[optind+1], &tail, 0);
      if (offset > 10000)                        {printf("wr-unipz: offset must be smaller than 10000 us -- %s\n", argv[optind+1]); return 1;}
      vacc   = strtoul(argv[optind+2], &tail, 0);
      if ((vacc < 0) || (vacc >= WRUNIPZ_NVACC)) {printf("wr-unipz: invalid virtual accelerator -- %s\n", argv[optind+2]); return 1;}
      pz     = strtoul(argv[optind+3], &tail, 0);
      if ((pz < 0) || (pz >= WRUNIPZ_NPZ))       {printf("wr-unipz: invalid PZ -- %s\n", argv[optind+3]); return 1;}

      t1 = getSysTime();

      if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, vacc)) !=  COMMON_STATUS_OK) {
        printf("wr-unipz: transaction init - %s\n", wrunipz_status_text(status));
      }
      else {
        // load dummy channel data
	wrunipz_fill_channel_dummy(offset, pz, vacc, dataChn0, &nDataChn0, dataChn1, &nDataChn1);

        // upload
        if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, pz, dataChn0, nDataChn0, dataChn1, nDataChn1)) != COMMON_STATUS_OK)
          printf("wr-unipz: transaction upload - %s\n", wrunipz_status_text(status));
        
        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);

        t2 = getSysTime();
        printf("wr-unipz: transaction took %u us\n", (uint32_t)(t2 -t1));
      }
    } // "test"
    if (!strcasecmp(command, "testfull")) {
      if (state != COMMON_STATE_OPREADY)  printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+2  != argc)              {printf("wr-unipz: expecting exactly one argument: testfull <offset>\n"); return 1;}
      offset = strtoul(argv[optind+1], &tail, 0);
      if (offset > 10000)                 {printf("wr-unipz: offset must be smaller than 10000 us -- %s\n", argv[optind+1]); return 1;}

      t1 = getSysTime();

      for (j=0; j < WRUNIPZ_NVACC - 1; j++) {  // only virt acc 0..14
        if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, j)) !=  COMMON_STATUS_OK) {
          printf("wr-unipz: transaction init (virt acc %d) - %s\n", j, wrunipz_status_text(status));
        } // if status
        else {
	  // load data and upload table
          for (k=0; k < WRUNIPZ_NPZ; k++) {
	    wrunipz_fill_channel_dummy(offset, k, j, dataChn0, &nDataChn0, dataChn1, &nDataChn1);
            if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, k, dataChn0, nDataChn0, dataChn1, nDataChn1)) != COMMON_STATUS_OK)
              printf("wr-unipz: transaction upload (virt acc %d, pz %d) - %s\n", j, k, wrunipz_status_text(status));
          } // for k
        } // else

        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);
      } // for j

      t2 = getSysTime();
      
      printf("wr-unipz: transaction took %u us per virtAcc\n", (uint32_t)(t2 -t1) / (WRUNIPZ_NVACC - 1));

    } // "testfull"
    if (!strcasecmp(command, "ftest")) {
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+4  != argc)             {printf("wr-unipz: expecting exactly three arguments: test <file> <vacc> <pz>\n"); return 1;}
      filename = argv[optind+1];
      vacc   = strtoul(argv[optind+2], &tail, 0);
      if ((vacc < 0) || (vacc >= WRUNIPZ_NVACC)) {printf("wr-unipz: invalid virtual accelerator -- %s\n", argv[optind+2]); return 1;}
      pz     = strtoul(argv[optind+3], &tail, 0);
      if ((pz < 0) || (pz >= WRUNIPZ_NPZ))       {printf("wr-unipz: invalid PZ -- %s\n", argv[optind+3]); return 1;}

      t1 = getSysTime();

      if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, vacc)) !=  COMMON_STATUS_OK) {
        printf("wr-unipz: transaction init - %s\n", wrunipz_status_text(status));
      }
      else {
        // load channel data from filename
	wrunipz_fill_channel_file(filename, pz, vacc, dataChn0, &nDataChn0, dataChn1, &nDataChn1);

	// upload table
	if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, pz, dataChn0, nDataChn0, dataChn1, nDataChn1)) != COMMON_STATUS_OK)
          printf("wr-unipz: transaction upload - %s\n", wrunipz_status_text(status));
        
        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);

        t2 = getSysTime();
        printf("wr-unipz: transaction took %u us\n", (uint32_t)(t2 -t1));
      }
    } // "ftest"
    if (!strcasecmp(command, "ftestfull")) {
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+2  != argc)             {printf("wr-unipz: expecting exactly one argument: ftestfull <file>\n"); return 1;}
      filename = argv[optind+1];

      t1 = getSysTime();

      for (j=0; j < WRUNIPZ_NVACC; j++) {  // only virt acc 0..14

        if ((status = wrunipz_transaction_init(device, wrunipz_cmd, wrunipz_confVacc, wrunipz_confStat, j)) !=  COMMON_STATUS_OK) {
          printf("wr-unipz: transaction init (virt acc %d) - %s\n", j, wrunipz_status_text(status));
        } // if status
        else {
	  // load data and upload table
          for (k=0; k < WRUNIPZ_NPZ; k++) {
	    wrunipz_fill_channel_file(filename, k, j, dataChn0, &nDataChn0, dataChn1, &nDataChn1);
            if ((status = wrunipz_transaction_upload(device, wrunipz_confStat, wrunipz_confPz, wrunipz_confData, wrunipz_confFlag, k, dataChn0, nDataChn0, dataChn1, nDataChn1)) != COMMON_STATUS_OK)
              printf("wr-unipz: transaction upload (virt acc %d, pz %d) - %s\n", j, k, wrunipz_status_text(status));
          } // for k
        } // else

        // submit
        wrunipz_transaction_submit(device, wrunipz_cmd, wrunipz_confStat);

      } // for j

      t2 = getSysTime();
      
      printf("wr-unipz: transaction took %u us per virtAcc\n", (uint32_t)(t2 -t1) / (WRUNIPZ_NVACC - 1));

    } // "ftestfull"
    if (!strcasecmp(command, "kill")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFKILL, 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "kill"
    if (!strcasecmp(command, "cleartables")) {
      eb_device_write(device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFCLEAR, 0, eb_block);
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "cleartables"
  } //if command
  

  if (snoop) {
    printf("wr-unipz: continous monitoring of 'UNIPZ (WR)', loglevel = %d\n", logLevel);
    
    //    actCycles    = 0;
    actState       = COMMON_STATE_UNKNOWN;
    actStatusArray = 0x1 << COMMON_STATUS_OK;

    printCycleHeader();

    while (1) {
      api_readDiag(device, &statusArray, &state, &version, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nDummyT, &nDummyI, &statDummy, 0);
      readInfo(&cycles, &tCycle, &fMessages, &confStat, &nLate, &vaccAvg, &pzAvg); // read info from lm32
      
      if (logLevel == 1) sleepTime = COMMON_DEFAULT_TIMEOUT * 100000;
      else               sleepTime = COMMON_DEFAULT_TIMEOUT * 10000;
      
      // if required, print status change
      if  ((actState != state) && (logLevel <= COMMON_LOGLEVEL_STATE)) printFlag = 1;

      // determine when to print info
      printFlag = 0;

      if (logLevel <= 1) printFlag = 1;

      if ((actState       != state)       && (logLevel <= COMMON_LOGLEVEL_STATE))   {printFlag = 1; actState       = state;}
      if ((actStatusArray != statusArray) && (logLevel <= COMMON_LOGLEVEL_STATUS))  {printFlag = 1; actStatusArray = actStatusArray;}

      if (printFlag) {
        printCycle(cycles, tCycle, fMessages, confStat, nLate, vaccAvg, pzAvg);
        printf(", %s (%6u), ",  api_stateText(state), nBadState);
        if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
        else printf("NOTOK(%6u)\n", nBadStatus);
        // print set status bits (except OK)
        for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
          if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrunipz_status_text(i));
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

/********************************************************************************************
 *  wrunipz-ctl.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 11-Jul-2024
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

// wr-unipz
#include <wrunipzlib.h>                  // LIB wrunipz
#include <wr-unipz.h>                    // FW
#include <wrunipz_shared_mmap.h>         // LM32

const char* program;
static int getInfo    = 0;
static int getVersion = 0;
static int snoop      = 0;
static int logLevel   = 0;

static void die(const char* where, uint32_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, wrunipz_status_text(status));
  exit(1);
} //die


static void help(void)
{
  uint32_t version;
  
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
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
  fprintf(stderr, "  ftestfull <file>               load (and submit) tables from <file> for ALL virt accs and all PZs\n");
  fprintf(stderr, "  readtables                     downloads all tables from PZs and prints some statistics\n");
  fprintf(stderr, "  cleartables                    clears all event tables of all PZs\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  diag                           shows statistics and detailled information\n");
  fprintf(stderr, "  cleardiag                      clears FW statistics\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to control WR-UNIPZ from the command line\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 bla bla bla\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "When using option '-s<n>' or '-i', the following information is displayed\n");
  fprintf(stderr, "wr-unipz:        cycles      virtAcc        PZ   |         DIAGNOSIS    |                 INFO           \n");
  fprintf(stderr, "wr-unipz: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate |   state      nchng stat   nchng\n");
  fprintf(stderr, "wr-unipz: ST 0054834398 0010010111000011 1111111 |DG 49.972 02335 00000 | OpReady    (     0), status 0x00000001 (     0)\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' |       '          '                   '       ' \n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' |       '          '                   '       '- # of 'bad status' incidents\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' |       '          '                   '- status value ('1' is OK)\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' |       '          '- # of state changes\n");
  fprintf(stderr, "                      '                '       ' |        '     '     ' |       '- state\n");
  fprintf(stderr, "                      '                '       ' |        '     '     '- # of late messages\n");
  fprintf(stderr, "                      '                '       ' |        '     '- average message rate [Hz]\n");
  fprintf(stderr, "                      '                '       ' |        '- average UNILAC cycle rate [Hz]\n");
  fprintf(stderr, "                      '                '       '- '1': PZ is active\n");
  fprintf(stderr, "                      '                '- '1': messages for this vacc are played\n");
  fprintf(stderr, "                      '- # of UNILAC cycles\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "for debugging purposes, the firmware writes all events received (via internal MIL bus) from the Superpulszentrale to its own ECA:\n");
  fprintf(stderr, "- FID  : 0xc\n");
  fprintf(stderr, "- GID  : 0xafe\n");
  fprintf(stderr, "- EvtNo: evtno\n");
  fprintf(stderr, "- SID  : vacc\n");
  fprintf(stderr, "- param: evtdata\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  wrunipz_version_library(&version);

  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", wrunipz_version_text(version));
} //help


void printCycleHeader()
{
  printf("wr-unipz:        cycles      virtAcc        PZ   |         DIAGNOSIS    |                 INFO           \n");
  printf("wr-unipz: STATUS      n 0....5....A....F 0.....6 |     fUni  fMsg nLate |   state      nchng stat   nchng\n");
} // printCycleHeader


void printCycle(uint32_t cycles, uint32_t tCycleAvg, uint32_t msgFreqAvg, uint32_t nLate, uint32_t vaccAvg, uint32_t pzAvg)
{
  int i;
  
  // past cycles
  printf("wr-unipz: ST %010d ", cycles);
  for (i=0; i < WRUNIPZ_NVACC; i++) printf("%d", (((1 << i) & vaccAvg) > 0));
  printf(" ");
  for (i=0; i < WRUNIPZ_NPZ;   i++) printf("%d", (((1 << i) & pzAvg) > 0));
  printf(" |");

  // diag
  printf("DG %6.3f %05d %05d |", 1000000000.0/(double)tCycleAvg, msgFreqAvg, nLate);

} // printCycle


void printDiags(uint32_t nCycles, uint64_t nMessages, int32_t dtMax, int32_t dtMin, int32_t cycJmpMax, int32_t cycJmpMin)
{
  printf("\nwr-unipz: statistics ...\n");
  printf("# of cycles           : %010u\n",   nCycles);
  printf("# of messages         : %010lu\n",  nMessages);
  printf("dt min [us]           : %08.1f\n",  (double)dtMin / 1000.0);
  printf("dt max [us]           : %08.1f\n",  (double)dtMax / 1000.0);
  printf("cycle jump min [us]   : %08.1f\n",  (double)cycJmpMin / 1000.0);
  printf("cycle jump max [us]   : %08.1f\n",  (double)cycJmpMax / 1000.0);
} // printDiags


void readDataFromFile(char *filename, uint32_t pz, uint32_t vAcc, uint32_t chn, uint32_t *data, uint32_t *nData)
{
#define  MAXLEN 4096

  int     i,j,k;
  char    charData[MAXLEN];
  FILE    *fp;
  char    *line = NULL;
  size_t  len = 0;
  ssize_t read;

  char     *tmp;       
  int      dataOffset;
  uint32_t nEvt;
  uint32_t evtCode, offset;
  
  // init
  for (i=0; i<WRUNIPZ_NEVT; i++) data[i]     = 0x0;
  for (i=0; i<MAXLEN; i++)       charData[i] = '\0';
  *nData = 0;

  // read data from file
  fp = fopen(filename, "r"); 
  if (fp == NULL) {
    printf("wr-unipz: can't open file with event table\n");
    exit(1);
  } // if fp

  for (i=0; i<WRUNIPZ_NPZ; i++) {
    for (j=0; j<WRUNIPZ_NVACC; j++) {
      for (k=0; k<WRUNIPZ_NCHN; k++) {
        if((read = getline(&line, &len, fp)) != -1) {
          // printf("pz %d, vacc %d, ch %d, line %s\n", i, j, k, line);
          if ((i==pz) && (j==vAcc) && (k == chn)) strcpy(charData, (line+1)); // ommit leading '['
        } // while
      } // for k
    } // for j
  } // for i
        
  fclose(fp);
  if (line) free (line);

  // printf("charData %s\n", charData);
  
  // extract data for pz, vacc, chn
  tmp     = charData;
  *nData  = 0;
  nEvt    = 0;
  if (sscanf(tmp, "%u%n", &nEvt, &dataOffset) == 1) tmp += dataOffset;
  for (i=0; i<nEvt; i++) {
    if (sscanf(tmp, ", %u%n", &evtCode, &dataOffset) == 1) tmp += dataOffset;
    if (sscanf(tmp, ", %uL%n", &offset, &dataOffset) == 1) tmp += dataOffset;
    data[*nData] = (offset << 16) | evtCode;
    (*nData)++;
  } // for i
} // readDataFromFile


int main(int argc, char** argv) {
  int         i, j, k, l;
  
  const char* devName; 
  const char* command;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  uint64_t ebDevice;
  uint32_t status;
  uint64_t statusArray;
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
  int32_t  cycJmpMax;
  int32_t  cycJmpMin;
  uint32_t nLate;
  uint32_t vaccAvg;
  uint32_t pzAvg;
  uint32_t verLib;
  uint32_t verFw;

  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint64_t actStatusArray;                     // actual sum status of gateway
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing

  uint32_t cpu;                                // lm32 address

  uint64_t t1, t2;

  // command test
  uint32_t    evtData[WRUNIPZ_NEVT];
  uint32_t    nEvtData;
  char        *filename;

  program = argv[0];

  while ((opt = getopt(argc, argv, "s:ceih")) != -1) {
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
      } // if *tail
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
    } // switch opt 
  } // while opt 

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

  if ((status =  wrunipz_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  if (getVersion) {
    wrunipz_version_library(&verLib);
    printf("wr-unipz: library (firmware) version %s",  wrunipz_version_text(verLib));     
    wrunipz_version_firmware(ebDevice, &verFw);
    printf(" (%s)\n",  wrunipz_version_text(verFw));     
  } // if getEBVersion

  if (getInfo) {
    wrunipz_info_read(ebDevice, &cycles, &tCycle, &fMessages, &nLate, &vaccAvg, &pzAvg, &messages, &dtMax, &dtMin, &cycJmpMax, &cycJmpMin);
    wrunipz_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &version, 0);
    
    printCycleHeader();
    printCycle(cycles, tCycle, fMessages, nLate, vaccAvg, pzAvg);
    printf(" %s (%6u), ",  wrunipz_state_text(state), nBadState);
    if ((statusArray >> COMMON_STATUS_OK) & 0x1) printf("OK   (%6u)\n", nBadStatus);
    else                                         printf("NOTOK(%6u)\n", nBadStatus);
    // print set status bits (except OK)
    for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
      if ((statusArray >> i) & 0x1)  printf("  ------ status bit is set : %s\n", wrunipz_status_text(i));
    } // for i
  } // if getInfo

  if (command) {
    // state required to give proper warnings
    wrunipz_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &version, 0);

    // request state changes
    if (!strcasecmp(command, "configure")) {
      wrunipz_cmd_configure(ebDevice);
      if ((state != COMMON_STATE_CONFIGURED) && (state != COMMON_STATE_IDLE)) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED or IDLE)\n");
    } // "configure"

    if (!strcasecmp(command, "startop")) {
      wrunipz_cmd_startop(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "startop"

    if (!strcasecmp(command, "stopop")) {
      wrunipz_cmd_stopop(ebDevice);
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
    } // "startop"

    if (!strcasecmp(command, "recover")) {
      wrunipz_cmd_recover(ebDevice);
      if (state != COMMON_STATE_ERROR) printf("wr-unipz: WARNING command has no effect (not in state ERROR)\n");
    } // "recover"

    if (!strcasecmp(command, "idle")) {
      wrunipz_cmd_idle(ebDevice);
      if (state != COMMON_STATE_CONFIGURED) printf("wr-unipz: WARNING command has no effect (not in state CONFIGURED)\n");
    } // "idle"

    if (!strcasecmp(command, "cleardiag")) {
      wrunipz_cmd_cleardiag(ebDevice);
    } // "cleardiag"

    if (!strcasecmp(command, "diag")) {
      wrunipz_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &version, 1);
      // print set status bits (except OK)
      for (i = COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
        if ((statusArray >> i) & 0x1)  printf("    status bit is set : %s\n", wrunipz_status_text(i));
      } // for i
      wrunipz_info_read(ebDevice, &cycles, &tCycle, &fMessages, &nLate, &vaccAvg, &pzAvg, &messages, &dtMax, &dtMin, &cycJmpMax, &cycJmpMin);
      printDiags(cycles, messages, dtMax, dtMin, cycJmpMax, cycJmpMin);
    } // "diag"

    if (!strcasecmp(command, "ftestfull")) {
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");
      if (optind+2  != argc)             {printf("wr-unipz: expecting exactly one argument: ftestfull <file>\n"); return 1;}
      filename = argv[optind+1];

      t1 = wrunipz_getSysTime();

      for (j=0; j < WRUNIPZ_NPZ; j++) {
        for(k=0; k < WRUNIPZ_NVACC; k++) {
          for (l=0; l < WRUNIPZ_NCHN; l++) {
            readDataFromFile(filename, j, k, l, evtData, &nEvtData);
            wrunipz_table_upload(ebDevice, j, k, l, evtData, nEvtData);
          } // for l
        } // for k
      } // for j

      wrunipz_cmd_submit(ebDevice);  // submit changes
      
      t2 = wrunipz_getSysTime();
      
      printf("wr-unipz: transaction took %u us per virtAcc\n", (uint32_t)(t2 -t1) / (WRUNIPZ_NVACC - 1));
    } // "ftestfull"

    if (!strcasecmp(command, "readtables")) {
      if (state != COMMON_STATE_OPREADY) printf("wr-unipz: WARNING command has no effect (not in state OPREADY)\n");

      t1 = wrunipz_getSysTime();

      printf("wr-unipz: downloading event tables from firmware...\n");
      for (j=0; j < WRUNIPZ_NPZ; j++) {
        for(k=0; k < WRUNIPZ_NVACC; k++) {
          for (l=0; l < WRUNIPZ_NCHN; l++) {
            wrunipz_table_download(ebDevice, j, k, l, evtData, &nEvtData);
            if (nEvtData > 0) {
              printf("          PZ %d, vacc %2d, chn %d: %2d events\n", j, k, l, nEvtData);
            } // if nEvtData
          } // for l
        } // for k
      } // for j

      t2 = wrunipz_getSysTime();
      
      printf("wr-unipz: transaction took %u us per virtAcc\n", (uint32_t)(t2 -t1) / (WRUNIPZ_NVACC - 1));
    } // "readtables"

    if (!strcasecmp(command, "cleartables")) {
      wrunipz_cmd_clearTables(ebDevice);
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
      wrunipz_info_read(ebDevice, &cycles, &tCycle, &fMessages, &nLate, &vaccAvg, &pzAvg, &messages, &dtMax, &dtMin, &cycJmpMax, &cycJmpMin);
      wrunipz_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &version, 0);
      
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
        printCycle(cycles, tCycle, fMessages, nLate, vaccAvg, pzAvg);
        printf(", %s (%6u), ",  wrunipz_state_text(state), nBadState);
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

  // close connection to firmware
  if ((status = wrunipz_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

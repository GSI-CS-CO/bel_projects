/*******************************************************************************************
 *  ecpu-mon.c
 *
 *  created : 2026
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 09-jan-2026
 *
 * subscribes to and displays status of a ecpu systems based on common lib
 * (requires a server such as 'b2b-serv-sys' on each local host)
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
#define ECPU_MON_VERSION 0x000812

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// dim
#include <dic.h>

// common
#include <common-defs.h>                 // COMMON  
#include <common-lib.h>
//#include <b2blib.h>                      // API
//#include <b2b.h>                         // FW

const char* program;

#define ECPUNSYS     36                   // number of ECPU systems

#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names
#define SCREENWIDTH 1024                 // width of screen

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

char    disEcpuEnv[DIMMAXSIZE];

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     header0[SCREENWIDTH+1];                            // header line to be printed
char     header1[SCREENWIDTH+1];                            // header line to be printed
char     empty[SCREENWIDTH+1];                              // an empty line

uint32_t flagPrintOther;                                    // option 'd'

const char * sysClearKeys[] = {
  "0",
  "1",
  "2",
  "3",
  "4",
  "5",
  "6",
  "7",
  "8",
  "9",
  "A",
  "B",
  "C",
  "D",
  "E",
  "F",
  "G",
  "H",
  "I",
  "J",
  "K",
  "L",
  "M",
  "N",
  "O",
  "P",
  "Q",
  "R",
  "S",
  "T",
  "U",
  "V",
  "W",
  "X",
  "Y",
  "Z"
};

const char * projNames[] = {
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "b2b",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
  "wrmil",
};

const char * sysShortNames[] = {
  "sis18-cbu",
  "sis18-pm",
  "sis18-psm",
  "sis18-kde",
  "sis18-raw",
  "sis18-cal",
  "esr-cbu",
  "esr-pm",
  "esr-psm",
  "esr-kdx",
  "esr-raw",
  "esr-cal",
  "yr-cbu",
  "yr-pm",
  "yr-psm",
  "yr-kdi",
  "yr-kde",
  "yr-raw",
  "yr-cal",
  "sis100-cbu",
  "sis100-pm",
  "sis100-psm",
  "sis100-kdi",
  "sis100-kde",
  "sis100-raw",
  "sis100-cal",
  "pzu_qr-mon",
  "pzu_ql-mon",
  "pzu_qn-mon",
  "pzu_un-mon",
  "pzu_uh-mon",
  "pzu_at-mon",
  "pzu_tk-mon",
  "pzu_f50-mon",
  "sis18_ring-mon",
  "esr_ring-mon"
};

const char * ringNames[] = {
  " SIS18",
  " SIS18",
  " SIS18",
  " SIS18",
  " SIS18",
  " SIS18",
  "   ESR",
  "   ESR",
  "   ESR",
  "   ESR",
  "   ESR",
  "   ESR",
  "    YR",
  "    YR",
  "    YR",
  "    YR",
  "    YR",
  "    YR",
  "    YR",
  "SIS100",
  "SIS100",
  "SIS100",
  "SIS100",
  "SIS100",
  "SIS100",
  "SIS100",
  "unilac",
  "unilac",
  "unilac",
  "unilac",
  "unilac",
  "unilac",
  "unilac",
  "unilac",
  " SIS18",
  "  ESR"
};

const char * typeNames[] = {
  "CBU",
  " PM",
  "PSM",
  "KDE",
  "DAQ",
  "CAL",
  "CBU",
  " PM",
  "PSM",
  "KDX",
  "DAQ",
  "CAL",
  "CBU",
  " PM",
  "PSM",
  "KDI",
  "KDE",
  "DAQ",
  "CAL",
  "CBU",
  " PM",
  "PSM",
  "KDE",
  "KDI",
  "DAQ",
  "CAL",
  " QR",
  " QL",
  " QN",
  " UN",
  " UH",
  " AT",
  " TK",
  "F50",
  "SIS",
  "ESR"
};

struct ecpuSystem_t {
  char          version[DIMCHARSIZE];
  char          state[DIMCHARSIZE];
  char          hostname[DIMCHARSIZE];
  uint64_t      status;
  comlib_diag_t diagData;
  
  uint32_t      versionId;
  uint32_t      stateId;
  uint32_t      hostnameId;
  uint32_t      statusId;
  uint32_t      diagDataId;
}; // struct ecpuSystem

struct ecpuSystem_t dicSystem[ECPUNSYS];


// help
static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] [ENVIRONMENT]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -o                  print info only once and exit (useful with '-s')\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display system information on the ECPU system\n");
  fprintf(stderr, "Example1: '%s pro'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", comlib_version_text(ECPU_MON_VERSION));
} //help


void buildHeader(char * environment)
{
  sprintf(title, "\033[7m ECPU System Status %3s ------------------------------------------------------------------------------------------------ (units [us] unless explicitly given) - v%8s\033[0m", environment, comlib_version_text(ECPU_MON_VERSION));
  sprintf(header0, "  #   ring sys  version     state  #badState        status #badStats      fwBootTime     fwDiagReset  fwSize    #fwLate   #fwEarly #fwCnflict #fwDelayed             node");
  sprintf(header1, "  #   ring sys    #fwSlow  offsSlow  offsSMax  offsSMin  comLtncy  ltncyMax  ltncyMin  offsDone  offsDMax  offsDMin  #transfer                                       node");
  sprintf(empty ,  "                                                                                                                                                                         ");
  //       printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// add all dim services
void dicSubscribeServices(char *environment)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<ECPUNSYS; i++) {
    sprintf(name, "%s_%s_%s_version_fw",    projNames[i],  environment, sysShortNames[i]);
    dicSystem[i].versionId       = dic_info_service(name, MONITORED, 0, (dicSystem[i].version),        8,                                  0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_state",         projNames[i], environment, sysShortNames[i]);
    dicSystem[i].stateId         = dic_info_service(name, MONITORED, 0, (dicSystem[i].state),          10,                                 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_hostname",      projNames[i], environment, sysShortNames[i]);
    dicSystem[i].hostnameId      = dic_info_service(name, MONITORED, 0, (dicSystem[i].hostname),       DIMCHARSIZE,                        0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_status",        projNames[i], environment, sysShortNames[i]);
    dicSystem[i].statusId        = dic_info_service(name, MONITORED, 0, &(dicSystem[i].status),        sizeof(dicSystem[i].status),        0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s_%s_comlib_diag",   projNames[i], environment, sysShortNames[i]);
    dicSystem[i].diagDataId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].diagData),      sizeof(dicSystem[i].diagData),      0, 0, &no_link_32, sizeof(no_link_32));
  } // for i
} // dicSubscribeServices


// send 'clear diag' command to server
void dicCmdClearDiag(char *environment, uint32_t indexServer)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s_%s_%s_cmd_cleardiag", projNames[indexServer], environment, sysShortNames[indexServer]);
  dic_cmnd_service(name, 0, 0);
} // dicCmdClearDiag


// print services to screen
void printServices(int flagOnce)
{
  int i;

  char     cVersion[9];
  char     cState[11];
  char     cNBadState[10];
  char     cStatus[17];
  char     cNBadStatus[10];
  char     cTBoot[16];
  char     cTDiag[16];
  char     cUsedSize[8];
  char     cNLate[11];
  char     cNEarly[11];
  char     cNConflict[11];
  char     cNDelayed[11];
  char     cNSlow[11];
  char     cOffsSlow[10];
  char     cOffsSlowMax[10];
  char     cOffsSlowMin[10];
  char     cComLatency[10];
  char     cComLatencyMax[10];
  char     cComLatencyMin[10];
  char     cOffsDone[10];
  char     cOffsDoneMax[10];
  char     cOffsDoneMin[10];
  char     cNTransfer[11];

  char     cHost[19];

  char     buff[100];
  time_t   time_date;
  uint32_t *tmp;
  uint32_t itmp;
  int      flagNolink;

  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");

  // footer with date and time
  time_date = time(0);
  strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
  sprintf(footer, "\033[7m exit <q> | toggle data <d> | clear status <digit> | print status <s> | help <h>                                                                          %s\033[0m", buff);
  
  comlib_term_curpos(1,1);
  
  if (!flagOnce) printf("%s\n", title);
  if (!flagPrintOther) printf("%s\n", header0);
  else                 printf("%s\n", header1);

  
  for (i=0; i<ECPUNSYS; i++) {
    // print hostname always
    tmp = (uint32_t *)(&(dicSystem[i].hostname));
    if (*tmp == no_link_32)                          sprintf(cHost,         "%16s",         no_link_str);
    else                                             sprintf(cHost,         "%16s",         dicSystem[i].hostname);

    // check link for commonlib diagnostic data
    tmp =  (uint32_t *)(&(dicSystem[i].diagData));
    if (*tmp == no_link_32)  flagNolink = 1;
    else                     flagNolink = 0;
   
    // standard data
    if (!flagPrintOther) {
      if (flagNolink)                                sprintf(buff,          "%15s",         no_link_str);
      else {
        time_date = (time_t)(dicSystem[i].diagData.tDiag / 1000000000);
        strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
      } // else: valid
                                                     sprintf(cTDiag,         "%15s",         buff);
      if (flagNolink)                                sprintf(buff,           "%15s",         no_link_str);
      else {
        time_date = (time_t)(dicSystem[i].diagData.tS0   / 1000000000);
        strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
      } // else: valid
                                                     sprintf(cTBoot,         "%15s",         buff);
      if (flagNolink)                                sprintf(cNLate,         "%10s",         no_link_str);
      else                                           sprintf(cNLate,         "%10u",         dicSystem[i].diagData.nLate);
      if (flagNolink)                                sprintf(cNEarly,        "%10s",         no_link_str);
      else                                           sprintf(cNEarly,        "%10u",         dicSystem[i].diagData.nEarly);
      if (flagNolink)                                sprintf(cNConflict,     "%10s",         no_link_str);
      else                                           sprintf(cNConflict,     "%10u",         dicSystem[i].diagData.nConflict);
      if (flagNolink)                                sprintf(cNDelayed,      "%10s",         no_link_str);
      else                                           sprintf(cNDelayed,      "%10u",         dicSystem[i].diagData.nDelayed);
      if (flagNolink)                                sprintf(cUsedSize,       "%7s",         no_link_str);
      else                                           sprintf(cUsedSize,       "%7u",         dicSystem[i].diagData.usedSize);
      if (flagNolink)                                sprintf(cNBadStatus,     "%9s",         no_link_str);
      else                                           sprintf(cNBadStatus,     "%9u",         dicSystem[i].diagData.nBadStatus);
      if (flagNolink)                                sprintf(cNBadState,      "%9s",         no_link_str);
      else                                           sprintf(cNBadState,      "%9u",         dicSystem[i].diagData.nBadState);
      if (dicSystem[i].status       == no_link_64)   sprintf(cStatus,        "%13s",         no_link_str);
      else                                           sprintf(cStatus, "%13"PRIx64"",         dicSystem[i].status);
      tmp = (uint32_t *)(&(dicSystem[i].state));
      if (*tmp == no_link_32)                        sprintf(cState,         "%10s",         no_link_str);
      else                                           sprintf(cState,         "%10s",         dicSystem[i].state);
      tmp = (uint32_t *)(&(dicSystem[i].version));
      if (*tmp == no_link_32)                        sprintf(cVersion,        "%8s",         no_link_str);
      else                                           sprintf(cVersion,        "%8s",         dicSystem[i].version); 
      
      printf(" %2s %6s %3s %8s %10s %9s %13s %9s %15s %15s %7s %10s %10s %10s %10s %16s\n", sysClearKeys[i], ringNames[i], typeNames[i], cVersion, cState, cNBadState, cStatus,
             cNBadStatus, cTBoot, cTDiag, cUsedSize, cNLate, cNEarly, cNConflict, cNDelayed, cHost);
    } // if !flagPrintOther
    // other data
    else { 
      if (flagNolink)                                sprintf(cNSlow,          "%10s",        no_link_str);
      else                                           sprintf(cNSlow,          "%10u",        dicSystem[i].diagData.nSlow);
      if (flagNolink)                                sprintf(cOffsSlow,        "%9s",        no_link_str);
      else                                           sprintf(cOffsSlow,      "%9.3f",        (double)(dicSystem[i].diagData.offsSlow)/1000.0);
      if (flagNolink)                                sprintf(cOffsSlowMax,     "%9s",        no_link_str);
      else                                           sprintf(cOffsSlowMax,   "%9.3f",        (double)(dicSystem[i].diagData.offsSlowMax)/1000.0);
      if (flagNolink)                                sprintf(cOffsSlowMin,     "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].diagData.offsSlowMin;
        if (itmp == 0xffffffff) itmp = 0;
                                                     sprintf(cOffsSlowMin,   "%9.3f",        (double)itmp/1000.0);
      } // data valid

      if (flagNolink)                                sprintf(cComLatency,      "%9s",        no_link_str);
      else                                           sprintf(cComLatency,    "%9.3f",        (double)(dicSystem[i].diagData.comLatency)/1000.0);
      if (flagNolink)                                sprintf(cComLatencyMax,   "%9s",        no_link_str);
      else                                           sprintf(cComLatencyMax, "%9.3f",        (double)(dicSystem[i].diagData.comLatencyMax)/1000.0);
      if (flagNolink)                                sprintf(cComLatencyMin,   "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].diagData.comLatencyMin;
        if (itmp == 0xffffffff) itmp = 0;
                                                     sprintf(cComLatencyMin, "%9.3f",        (double)itmp/1000.0);
      } // data valid

      if (flagNolink)                                sprintf(cOffsDone,        "%9s",        no_link_str);
      else                                           sprintf(cOffsDone,      "%9.3f",        (double)(dicSystem[i].diagData.offsDone)/1000.0);
      if (flagNolink)                                sprintf(cOffsDoneMax,     "%9s",        no_link_str);
      else                                           sprintf(cOffsDoneMax,   "%9.3f",        (double)(dicSystem[i].diagData.offsDoneMax)/1000.0);
      if (flagNolink)                                sprintf(cOffsDoneMin,     "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].diagData.offsDoneMin;
        if (itmp == 0xffffffff) itmp = 0;
                                                     sprintf(cOffsDoneMin,   "%9.3f",        (double)itmp/1000.0);
      } // data valid
      if (flagNolink)                                sprintf(cNTransfer,      "%10s",        no_link_str);
      else                                           sprintf(cNTransfer,      "%10u",        dicSystem[i].diagData.nTransfer);


      printf(" %2s %6s %3s %10s %9s %9s %9s %9s %9s %9s %9s %9s %9s %10s                           %16s\n", sysClearKeys[i], ringNames[i], typeNames[i], cNSlow, cOffsSlow, cOffsSlowMax, cOffsSlowMin,
             cComLatency, cComLatencyMax, cComLatencyMin, cOffsDone, cOffsDoneMax, cOffsDoneMin, cNTransfer, cHost);
    }  // flagPrintOther
  } // for i

  //for (i=0; i<2; i++) printf("%s\n", empty);
  if (!flagOnce) printf("%s\n", footer);
} // printServices


// print status text to screen
void printStatusText()
{
  int i,j;
  uint64_t status;

  for (i=0; i<ECPUNSYS; i++) {
    status = dicSystem[i].status;
    if ((status != 0x1) && (status != no_link_64)) {
      printf(" %6s %3s:\n", ringNames[i], typeNames[i]);
      for (j = COMMON_STATUS_OK + 1; j<(int)(sizeof(status)*8); j++) {
        if ((status >> j) & 0x1)  printf("  ---------- status bit is set : %s\n", comlib_statusText(j));
      } // for j
    } // if status
  } // for i
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printStatusText


// print help text to screen
void printHelpText()
{
  int i;

  comlib_term_curpos(1,1);
  printf("%s\n", title);
  
  for (i=0; i<ECPUNSYS; i++) printf("%s\n", empty);
  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("please visit the following URL                                                  \n");
  printf("https://wiki.gsi.de/TOS/BunchBucket/BunchBucketHowCLI#ECPU_System_Status         \n");
  printf("%s\n", empty);
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printHelpText


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;
  //  char *tail;

  int      getVersion;
  int      once;

  char     userInput;
  int      sysId;
  int      quit;

  char     environment[132];

  program        = argv[0];
  getVersion     = 0;
  once           = 0;
  quit           = 0;
  flagPrintOther = 0;

  while ((opt = getopt(argc, argv, "eho")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'o':
        once = 1;
        break;
      case 'h':
        help();
        return 0;
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

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  if (optind< argc) sprintf(environment, "%s", argv[optind]);
  else              sprintf(environment, "%s", "na");

  comlib_term_clear();
  buildHeader(environment);
  if (getVersion) printf("%s: version %s\n", program, comlib_version_text(ECPU_MON_VERSION));

  printf("ecpu-mon: starting using environment %s\n", environment);
  sleep(1);
  dicSubscribeServices(environment);
  sleep(1);

  while (!quit) {
    if (once) {sleep(1); quit=1;}                 // wait a bit to get the values
    printServices(once);
    if (!quit) {
      sysId = 0xffff;
      userInput = comlib_term_getChar();
      switch (userInput) {
        case 'A' ... 'Z' :
          sysId = userInput - 55;                 // no break on purpose
        case '0' ... '9' :
          if (sysId == 0xffff) sysId = userInput - 48; // ugly
          dicCmdClearDiag(environment, sysId);
          break;
        case 'h'         :
          printHelpText();
          break;
        case 'q'         :
          quit = 1;
          break;
        case 'd' :
          // toggle printing of data
          flagPrintOther = !flagPrintOther;
          break;
        case 's'         :
          printStatusText();
          break;
        default          :
          usleep(1000000);
      } // switch
    } // if !quit
  } // while

  return exitCode;
}

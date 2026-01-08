/*******************************************************************************************
 *  ecpu-mon.c
 *
 *  created : 2026
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 08-jan-2026
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
#define ECPU_MON_VERSION 0x000811

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
  char      version[DIMCHARSIZE];
  char      state[DIMCHARSIZE];
  char      hostname[DIMCHARSIZE];
  uint64_t  status;
  uint32_t  nBadState;
  uint32_t  nBadStatus;
  uint64_t  tS0;
  uint64_t  tDiag;
  uint32_t  nTransfer;
  uint32_t  usedSize;
  uint32_t  nLate;
  uint32_t  nEarly;
  uint32_t  nConflict;
  uint32_t  nDelayed;
  uint32_t  nSlow;
  uint32_t  offsSlow;
  uint32_t  offsSlowMax;
  uint32_t  offsSlowMin;
  uint32_t  comLatency;
  uint32_t  comLatencyMax;
  uint32_t  comLatencyMin;
  uint32_t  offsDone;
  uint32_t  offsDoneMax;
  uint32_t  offsDoneMin;

  uint32_t  versionId;
  uint32_t  stateId;
  uint32_t  hostnameId;
  uint32_t  statusId;
  uint32_t  nBadStateId;
  uint32_t  nBadStatusId;
  uint32_t  tS0Id;
  uint32_t  tDiagId;
  uint32_t  nTransferId;
  uint32_t  usedSizeId;
  uint32_t  nLateId;
  uint32_t  nEarlyId;
  uint32_t  nConflictId;
  uint32_t  nDelayedId;
  uint32_t  nSlowId;
  uint32_t  offsSlowId;
  uint32_t  offsSlowMaxId;
  uint32_t  offsSlowMinId;
  uint32_t  comLatencyId;
  uint32_t  comLatencyMaxId;
  uint32_t  comLatencyMinId;  
  uint32_t  offsDoneId;
  uint32_t  offsDoneMaxId;
  uint32_t  offsDoneMinId;
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
    dicSystem[i].statusId        = dic_info_service(name, MONITORED, 0, &(dicSystem[i].status),        sizeof(uint64_t),                   0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s_%s_nbadstate",     projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nBadStateId     = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nBadState),     sizeof(dicSystem[i].nBadState),     0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_nbadstatus",    projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nBadStatusId    = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nBadStatus),    sizeof(dicSystem[i].nBadStatus),    0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_ts0",           projNames[i], environment, sysShortNames[i]);
    dicSystem[i].tS0Id           = dic_info_service(name, MONITORED, 0, &(dicSystem[i].tS0),           sizeof(dicSystem[i].tS0),           0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s_%s_ntransfer",     projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nTransferId     = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nTransfer),     sizeof(dicSystem[i].nTransfer),     0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_tdiag",         projNames[i], environment, sysShortNames[i]);
    dicSystem[i].tDiagId         = dic_info_service(name, MONITORED, 0, &(dicSystem[i].tDiag),         sizeof(dicSystem[i].tDiag),         0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s_%s_usedsize",      projNames[i], environment, sysShortNames[i]);
    dicSystem[i].usedSizeId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].usedSize),      sizeof(dicSystem[i].usedSize),      0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_nlate",         projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nLateId         = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nLate),         sizeof(dicSystem[i].nLate),         0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_nearly",        projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nEarlyId        = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nEarly),        sizeof(dicSystem[i].nEarly),        0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_nconflict",     projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nConflictId     = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nConflict),     sizeof(dicSystem[i].nConflict),     0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_ndelayed",      projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nDelayedId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nDelayed),      sizeof(dicSystem[i].nDelayed),      0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_nslow",         projNames[i], environment, sysShortNames[i]);
    dicSystem[i].nSlowId         = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nSlow),         sizeof(dicSystem[i].nSlow),         0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsslow",      projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsSlowId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsSlow),      sizeof(dicSystem[i].offsSlow),      0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsslowmax",   projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsSlowMaxId   = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsSlowMax),   sizeof(dicSystem[i].offsSlowMax),   0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsslowmin",   projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsSlowMinId   = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsSlowMin),   sizeof(dicSystem[i].offsSlowMin),   0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_comlatency",    projNames[i], environment, sysShortNames[i]);
    dicSystem[i].comLatencyId    = dic_info_service(name, MONITORED, 0, &(dicSystem[i].comLatency),    sizeof(dicSystem[i].comLatency),    0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_comlatencymax", projNames[i], environment, sysShortNames[i]);
    dicSystem[i].comLatencyMaxId = dic_info_service(name, MONITORED, 0, &(dicSystem[i].comLatencyMax), sizeof(dicSystem[i].comLatencyMax), 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_comlatencymin", projNames[i], environment, sysShortNames[i]);
    dicSystem[i].comLatencyMinId = dic_info_service(name, MONITORED, 0, &(dicSystem[i].comLatencyMin), sizeof(dicSystem[i].comLatencyMin), 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsdone",      projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsDoneId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsDone),      sizeof(dicSystem[i].offsDone),      0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsdonemax",   projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsDoneMaxId   = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsDoneMax),   sizeof(dicSystem[i].offsDoneMax),   0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s_%s_offsdonemin",   projNames[i], environment, sysShortNames[i]);
    dicSystem[i].offsDoneId      = dic_info_service(name, MONITORED, 0, &(dicSystem[i].offsDoneMin),   sizeof(dicSystem[i].offsDoneMin),   0, 0, &no_link_32, sizeof(no_link_32));
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
    if (*tmp == no_link_32)                      sprintf(cHost,        "%16s",         no_link_str);
    else                                         sprintf(cHost,        "%16s",         dicSystem[i].hostname);

    // standard data
    if (!flagPrintOther) {
      if (dicSystem[i].tDiag      == no_link_64)   sprintf(buff,       "%15s",         no_link_str);
      else {
        time_date = (time_t)(dicSystem[i].tDiag / 1000000000);
        strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
      } // else: valid
      sprintf(cTDiag,     "%15s",         buff);
      if (dicSystem[i].tS0        == no_link_64)   sprintf(buff,       "%15s",         no_link_str);
      else {
        time_date = (time_t)(dicSystem[i].tS0   / 1000000000);
        strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
      } // else: valid
      sprintf(cTBoot,     "%15s",         buff);
      if (dicSystem[i].nLate         == no_link_32)   sprintf(cNLate,         "%10s",         no_link_str);
      else                                            sprintf(cNLate,         "%10u",         dicSystem[i].nLate);
      if (dicSystem[i].nEarly        == no_link_32)   sprintf(cNEarly,        "%10s",         no_link_str);
      else                                            sprintf(cNEarly,        "%10u",         dicSystem[i].nEarly);
      if (dicSystem[i].nConflict     == no_link_32)   sprintf(cNConflict,     "%10s",         no_link_str);
      else                                            sprintf(cNConflict,     "%10u",         dicSystem[i].nConflict);
      if (dicSystem[i].nDelayed      == no_link_32)   sprintf(cNDelayed,      "%10s",         no_link_str);
      else                                            sprintf(cNDelayed,      "%10u",         dicSystem[i].nDelayed);
      if (dicSystem[i].usedSize      == no_link_32)   sprintf(cUsedSize,       "%7s",         no_link_str);
      else                                            sprintf(cUsedSize,       "%7u",         dicSystem[i].usedSize);
      if (dicSystem[i].nBadStatus    == no_link_32)   sprintf(cNBadStatus,     "%9s",         no_link_str);
      else                                            sprintf(cNBadStatus,     "%9u",         dicSystem[i].nBadStatus);
      if (dicSystem[i].nBadState     == no_link_32)   sprintf(cNBadState,      "%9s",         no_link_str);
      else                                            sprintf(cNBadState,      "%9u",         dicSystem[i].nBadState);
      if (dicSystem[i].status        == no_link_64)   sprintf(cStatus,        "%13s",         no_link_str);
      else                                            sprintf(cStatus, "%13"PRIx64"",         dicSystem[i].status);
      tmp = (uint32_t *)(&(dicSystem[i].state));
      if (*tmp == no_link_32)                         sprintf(cState,         "%10s",         no_link_str);
      else                                            sprintf(cState,         "%10s",         dicSystem[i].state); 
      tmp = (uint32_t *)(&(dicSystem[i].version));
      if (*tmp == no_link_32)                         sprintf(cVersion,        "%8s",         no_link_str);
      else                                            sprintf(cVersion,        "%8s",         dicSystem[i].version); 
      
      printf(" %2s %6s %3s %8s %10s %9s %13s %9s %15s %15s %7s %10s %10s %10s %10s %16s\n", sysClearKeys[i], ringNames[i], typeNames[i], cVersion, cState, cNBadState, cStatus,
             cNBadStatus, cTBoot, cTDiag, cUsedSize, cNLate, cNEarly, cNConflict, cNDelayed, cHost);
    } // if !flagPrintOther
    // other data
    else { 
      if (dicSystem[i].nSlow         == no_link_32)   sprintf(cNSlow,          "%10s",        no_link_str);
      else                                            sprintf(cNSlow,          "%10u",        dicSystem[i].nSlow);
      if (dicSystem[i].offsSlow      == no_link_32)   sprintf(cOffsSlow,        "%9s",        no_link_str);
      else                                            sprintf(cOffsSlow,      "%9.3f",        (double)(dicSystem[i].offsSlow)/1000.0);
      if (dicSystem[i].offsSlowMax   == no_link_32)   sprintf(cOffsSlowMax,     "%9s",        no_link_str);
      else                                            sprintf(cOffsSlowMax,   "%9.3f",        (double)(dicSystem[i].offsSlowMax)/1000.0);
      if (dicSystem[i].offsSlowMin   == no_link_32)   sprintf(cOffsSlowMin,     "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].offsSlowMin;
        if (itmp == 0xffffffff) itmp = 0;             sprintf(cOffsSlowMin,   "%9.3f",        (double)itmp/1000.0);
      } // data valid

      if (dicSystem[i].comLatency    == no_link_32)   sprintf(cComLatency,      "%9s",        no_link_str);
      else                                            sprintf(cComLatency,    "%9.3f",        (double)(dicSystem[i].comLatency)/1000.0);
      if (dicSystem[i].comLatencyMax == no_link_32)   sprintf(cComLatencyMax,   "%9s",        no_link_str);
      else                                            sprintf(cComLatencyMax, "%9.3f",        (double)(dicSystem[i].comLatencyMax)/1000.0);
      if (dicSystem[i].comLatencyMin == no_link_32)   sprintf(cComLatencyMin,   "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].comLatencyMin;
        if (itmp == 0xffffffff) itmp = 0;             sprintf(cComLatencyMin, "%9.3f",        (double)itmp/1000.0);
      } // data valid

      if (dicSystem[i].offsDone      == no_link_32)   sprintf(cOffsDone,        "%9s",        no_link_str);
      else                                            sprintf(cOffsDone,      "%9.3f",        (double)(dicSystem[i].offsDone)/1000.0);
      if (dicSystem[i].offsDoneMax   == no_link_32)   sprintf(cOffsDoneMax,     "%9s",        no_link_str);
      else                                            sprintf(cOffsDoneMax,   "%9.3f",        (double)(dicSystem[i].offsDoneMax)/1000.0);
      if (dicSystem[i].offsDoneMin   == no_link_32)   sprintf(cOffsDoneMin,     "%9s",        no_link_str);
      else { // display 0xffffffff (no min value) as '0'
        itmp = dicSystem[i].offsDoneMin;
        if (itmp == 0xffffffff) itmp = 0;             sprintf(cOffsDoneMin,   "%9.3f",        (double)itmp/1000.0);
      } // data valid
      if (dicSystem[i].nTransfer     == no_link_32)   sprintf(cNTransfer,     "%10s",         no_link_str);
      else                                            sprintf(cNTransfer,     "%10u",         dicSystem[i].nTransfer);


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
        case 'A' ... 'O' :
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

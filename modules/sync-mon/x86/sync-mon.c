/*******************************************************************************************
 *  sync-mon.c
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 3-jul-2025
 *
 * subscribes to and displays status of tansfers between machines
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
#define SYNC_MON_VERSION 0x000007

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

// sync-mon
#include <common-lib.h>                  // COMMON
#include <syncmonlib.h>                  // API

const char* program;

// dim stuff
#define  DIMCHARSIZE 32                   // standard size for char services
#define  DIMMAXSIZE  1024                 // max size for service names
#define  SCREENWIDTH 1024                 // width of screen
#define  NALLSID     48                   // number of all SIDs observed; 16 each for SIS18, ESR and CRYRING
#define  MAXEVTS     3                    // number of all events monitored
#define  TINACTIVE   3600                 // [s]; if the previous data is more in the past than this value, the transfer data is considered inactive
#define  TOLD        3600 * 24            // [s]; if the previous data is more in the past than this value, the transfer data is considered out of date
#define  EXT         0                    // index of extraction data
#define  INJA        1                    // index of 1st injection data
#define  INJB        2                    // index of 2nd injection data 

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
double   no_link_dbl   = NAN;
char     no_link_str[] = "NO_LINK";

monval_t  dicUnilacE0;                    // UNILAC 'extraction'
monval_t  dicSis18I0;                     // SIS18 main thread
monval_t  dicSis18I1;                     // SIS18 injection thread
monval_t  dicSis18E0;                     // SIS18 extraction
monval_t  dicEsrI0;                       // ESR injection (schedule)
monval_t  dicEsrI1;                       // ESR injection (b2b)
monval_t  dicEsrE0;                       // ESR extraction
monval_t  dicYrI0;                        // CRYRING injection (schedule)
monval_t  dicYrI1;                        // CRYRING injection (b2b)

uint32_t  dicUnilacE0Id;                  // DIM service IDs
uint32_t  dicSis18I0Id;
uint32_t  dicSis18I1Id;
uint32_t  dicSis18E0Id;
uint32_t  dicEsrI0Id;
uint32_t  dicEsrI1Id;
uint32_t  dicEsrE0Id;
uint32_t  dicYrI0Id;
uint32_t  dicYrI1Id;

#define  TXTNA       "  N/A"
#define  TXTUNKWN    "UNKWN"
#define  TXTERROR    "ERROR"
#define  TXTNODATA   "no data"


// get values
monval_t monData[NALLSID][MAXEVTS];                         // [idx][0: extraction machine; 1 (main thread),2 (injection thread): injection machine]
uint32_t flagSetValid[NALLSID];                             // flag: data of this set are valid
//uint32_t flagSetUpdate[NALLSID];                            // flag: update displayed data of this set
int      set_msecs[NALLSID];                                // CBS deadline, fraction [ms]
time_t   set_secs[NALLSID];                                 // CBS deadline, time [s]

time_t   secsOffset;                                        // offset between timestamp and system time

// other
int      flagPrintIdx[NALLSID];                             // flag: print line with given index
int      flagPrintNow;                                      // flag: print stuff to screen NOW

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     headerK[SCREENWIDTH+1];                            // header line to be printed
char     emptyK[SCREENWIDTH+1];                             // empty line to be printed
char     printLineK[NALLSID][SCREENWIDTH+1];                // lines to be printed

double   one_ns_as = 1000000000.0;


// help
static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <name>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display information on transfers\n");
  fprintf(stderr, "Example1: '%s pro'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %6x. Licensed under the LPL v3.\n", SYNC_MON_VERSION);
} //help


// convert index to extraction machine and sid
void idx2MachineSid(uint32_t idx, machine_t *machine, uint32_t *sid)
{
  if (idx >= NALLSID) {
    *machine = NOMACHINE;
    *sid     = 0;
    return;
  } // if idx
  
  switch (idx) {
    case 0 ... 15 :
      *machine = UNILAC;
      *sid     = idx;
      break;
    case 16 ... 31 :
      *machine = SIS18;
      *sid     = idx - 16;
      break;
    case 32 ... 47 :
      *machine = ESR;
      *sid     = idx - 32;
      break;
    default :
      *machine = NOMACHINE;
      *sid     = 0;
      break;
  } // switch idx
} // idx2MachineSid


void t2secs(uint64_t ts, uint32_t *secs, uint32_t *msecs)
{
  *msecs = (uint32_t)(ts % 1000000000) / 1000000.0;
  *secs  = (uint32_t)(ts / 1000000000);
} // t2secs


void buildHeader()
{
  sprintf(headerK, "| t_last [UTC] | origin | sid|            event ||  destn | sid|          event a |      diff a |          event b |      diff b |");
  sprintf(emptyK,  "|              |        |    |                  ||        |    |                  |             |                  |             |");
  //        printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// build strings for printing
void buildPrintLine(uint32_t idx)
{
  char      origin[32];
  char      dest[32];
  int       flagTCBS;
  char      tCBS[64];
  char      extSid[32];
  char      injSid[32];
  char      extEvt[32];
  char      injEvtA[32];
  char      injEvtB[32];
  char      diffA[32];                 // time difference of injEvtA - ExtEvt [us]
  char      diffB[32];                 // time difference of injEvtB - ExtEvt [us]

  char      tmp1[32];
  double    dtmp;

  uint32_t  sid;
  machine_t extRing;

  uint64_t  actNsecs;
  time_t    actT;

  actNsecs  = comlib_getSysTime();
  actT      = (time_t)(actNsecs / 1000000000);
  dtmp      = NAN;

  if (idx > NALLSID) return;

  idx2MachineSid(idx, &extRing, &sid);

  sprintf(tCBS   , "---");
  sprintf(extSid , " ");
  sprintf(extEvt , " ");
  sprintf(dest   , " ");
  sprintf(injSid , " ");
  sprintf(injEvtA, " ");
  sprintf(diffA  , " ");
  sprintf(injEvtB, " ");
  sprintf(diffB  , " ");

  // extraction ring
  switch (extRing) {
    case UNILAC : sprintf(dest, "---"), sprintf(origin, "UNILAC"); flagTCBS = 1; break;
    case SIS18  : sprintf(dest, "---"), sprintf(origin, "SIS18") ; flagTCBS = 1; break;
    case ESR    : sprintf(dest, "---"), sprintf(origin, "ESR")   ; flagTCBS = 1; break;
    default     : sprintf(dest, "---"), sprintf(origin, " ")     ; flagTCBS = 1; break;
  } // switch ringExt

  // ignore ancient timestamps
  if (set_secs[idx] <= 1) flagTCBS = 0;
  
  if (flagTCBS) {
    if ((actT - set_secs[idx]/* - secsOffset*/) > (time_t)TOLD) sprintf(tCBS, "> 24h"); 
    else {
      strftime(tmp1, 10, "%H:%M:%S", gmtime(&(set_secs[idx])));
      sprintf(tCBS, "%8s.%03d", tmp1, set_msecs[idx]);
    } // else actT
  } // if flagTCBS
  
  // SID of extraction machine and data valid
  if (monData[idx][EXT].fid != 0) {
    smGetEvtString(monData[idx][EXT].evtNo, tmp1);
    sprintf(extEvt, "%16s", tmp1);
    sprintf(extSid, "%2d" , monData[idx][EXT].sid);
  } // if data of extraction machine valid

  // injection trigger data valid
  if (monData[idx][INJB].fid != 0) {
    switch (monData[idx][INJB].gid) {
      case GID_SIS18 : sprintf(dest, "SIS18") ; break;
      case GID_ESR   : sprintf(dest, "ESR")   ; break;
      case GID_YR    : sprintf(dest, "YR")    ; break;
      default        : sprintf(dest, TXTUNKWN); break;
    } // switch sid

    smGetEvtString(monData[idx][INJB].evtNo, tmp1);
    sprintf(injEvtB, "%16s", tmp1);
    sprintf(injSid , "%2d" , monData[idx][INJB].sid);

    // extraction: data valid
    if (monData[idx][EXT].fid != 0) {
      smGetEvtString(monData[idx][EXT].evtNo, tmp1);
      sprintf(extEvt, "%16s", tmp1);
      sprintf(extSid, "%2d" , monData[idx][EXT].sid);
      dtmp = (int64_t)(monData[idx][INJB].deadline - monData[idx][EXT].deadline) / 1000.0;
      sprintf(diffB, "%11.3f", dtmp);
    } // if extraction data
  } // if trigger data

  // injection other data valid
  if (monData[idx][INJA].fid != 0) {
    smGetEvtString(monData[idx][INJA].evtNo, tmp1);
    sprintf(injEvtA, "%16s", tmp1);

    // extraction: data valid
    if (monData[idx][EXT].fid != 0) {
      dtmp = (int64_t)(monData[idx][INJA].deadline - monData[idx][EXT].deadline) / 1000.0;
      sprintf(diffB, "%11.3f", dtmp);
    } // if extraction data
  } // if injection other data
  
  
  sprintf(printLineK[idx], "| %12s | %6s | %2s | %16s || %6s | %2s | %16s | %11s | %16s | %11s |", tCBS, origin, extSid, extEvt, dest, injSid, injEvtA, diffA, injEvtB, diffB);
  //                printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  

} //buildPrintLine

// receive trigger events for transfer
void recTrigger(long *tag, monval_t *address, int *size)
{
  monval_t *tmp;
  uint32_t secs;
  uint32_t msecs;
  uint32_t idxOffset;
  uint32_t idx;
  int      flagValid;
  uint64_t deadline;
  uint32_t fid;

  uint64_t actNsecs;
  time_t   actT;

  /* printf("tag %lx\n", *tag); */

  switch (*tag) {
    case sis18Inj : idxOffset =  0; break;
    case esrInj   : idxOffset = 16; break;
    case yrInj    : idxOffset = 32; break;
    default           : return;
  } // switch tag

  flagValid = (*size != sizeof(uint32_t));
  if (!flagValid) return;

  tmp      = address;
  idx      = (*tmp).sid + idxOffset;
  deadline = (*tmp).deadline;
  fid      = (*tmp).fid;

  monData[idx][EXT]  = smEmptyMonData();
  monData[idx][INJA] = smEmptyMonData();
  monData[idx][INJB] = smEmptyMonData();

  if (!fid) return;

  // here we need to find out what belongs together
  // we trigger in the 'injection' thread (INJB) and need to copy data of main thread (INJA) and extraction (EXT)
  // the decision depends on the difference of deadlines (must be smaller than DTLIMIT)
  
  switch (*tag) {
    case sis18Inj :
      // copy received value
      monData[idx][INJB] = *tmp;
        
      // copy other values; only copy values if the difference of the deadlines is below DTLIMIT
      if (abs((*tmp).deadline - dicUnilacE0.deadline ) < DTLIMIT) monData[idx][EXT]  = dicUnilacE0;
      if (abs((*tmp).deadline - dicSis18I0.deadline  ) < DTLIMIT) monData[idx][INJA] = dicSis18I0;
      break;
    case esrInj :
      monData[idx][INJB] = *tmp;
      // copy other values; only copy values if the difference of the deadlines is below DTLIMIT
      /* chk handling SIS100 */
      if (abs((*tmp).deadline - dicSis18E0.deadline  ) < DTLIMIT) monData[idx][EXT]  = dicSis18E0;
      if (abs((*tmp).deadline - dicEsrI0.deadline    ) < DTLIMIT) monData[idx][INJA] = dicEsrI0;
      
      break;
    case yrInj :
      // copy received value
      monData[idx][INJB] = *tmp;
        
      // copy other values; only copy values if the difference of the deadlines is below DTLIMIT
      if (abs((*tmp).deadline - dicSis18E0.deadline  ) < DTLIMIT) monData[idx][EXT]  = dicEsrE0;
      if (abs((*tmp).deadline - dicYrI0.deadline     ) < DTLIMIT) monData[idx][INJA] = dicYrI0;

      break;
    default :
      break;
  } // switch tag

  //printf("sid %d, evtno%d\n", (*tmp).sid, (*tmp).evtNo);

  // get timestamp
  t2secs(deadline, &secs, &msecs);
  set_secs[idx]      = (time_t)(secs);
  set_msecs[idx]     = msecs;

  // calibrate offset between THIS system time and time of set_values
  actNsecs           = comlib_getSysTime();
  actT               = (time_t)(actNsecs / 1000000000);
  secsOffset         = actT - set_secs[idx];

  //  flagSetUpdate[idx] = 1;
  flagPrintNow       = 1;
  buildPrintLine(idx);
} // recTrigger


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char     name[DIMMAXSIZE];

  // UNILAC 'extraction'
  sprintf(name, "%s_unilac-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicUnilacE0Id     = dic_info_service(name, MONITORED, 0, &dicUnilacE0, sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // SIS18 injection, main thread
  sprintf(name, "%s_sis18-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicSis18I0Id       = dic_info_service(name, MONITORED, 0, &dicSis18I0, sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // SIS18 injection, injection thread
  sprintf(name, "%s_sis18-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicSis18I1Id       = dic_info_service(name, MONITORED, 0, &dicSis18I1, sizeof(monval_t), recTrigger, (long)sis18Inj, &no_link_32, sizeof(uint32_t));

  // SIS18 extraction
  sprintf(name, "%s_sis18-ext-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicSis18E0Id       = dic_info_service(name, MONITORED, 0, &dicSis18E0, sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // ESR injection, schedule
  sprintf(name, "%s_esr-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicEsrI0Id         = dic_info_service(name, MONITORED, 0, &dicEsrI0  , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // ESR injection, b2b
  sprintf(name, "%s_esr-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicEsrI1Id         = dic_info_service(name, MONITORED, 0, &dicEsrI1  , sizeof(monval_t), recTrigger, (long)esrInj, &no_link_32, sizeof(uint32_t));

  // ESR extraction
  sprintf(name, "%s_esr-ext-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicEsrE0Id         = dic_info_service(name, MONITORED, 0, &dicEsrE0  , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // CRYRING injection, schedule
  sprintf(name, "%s_yr-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicYrI0Id          = dic_info_service(name, MONITORED, 0, &dicYrI0   , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // CRYRING injection, b2b
  sprintf(name, "%s_yr-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicYrI1Id          = dic_info_service(name, MONITORED, 0, &dicYrI1   , sizeof(monval_t), recTrigger, (long)yrInj, &no_link_32, sizeof(uint32_t));

} // dicSubscribeServices


// calc flags for printing
uint32_t calcFlagPrint()
{
  int       i;
  uint32_t  nLines;

  uint64_t actNsecs;
  time_t   actT;

  nLines   = 0;
  actNsecs = comlib_getSysTime();
  actT     = (time_t)(actNsecs / 1000000000);

  for (i=0; i<NALLSID; i++) {
    flagPrintIdx[i] = 1;

    if ((actT - set_secs[i] - secsOffset) > (time_t)TINACTIVE) flagPrintIdx[i] = 0; // ignore old timestamps
    if (set_secs[i] <= 1)                                      flagPrintIdx[i] = 0; // ignore ancient timestamps

    if (flagPrintIdx[i]) nLines++;
  } // for i

  return nLines;
} // calcFlagPrint;


// print data to screen
void printData(char *name)
{
  char     buff[100];
  char     unitInfo[100];
  time_t   time_date;
  uint32_t nLines;
  uint32_t minLines = 20;
  int      i;

  nLines = calcFlagPrint();

  time_date = time(0);
  strftime(buff,53,"%d-%b-%y %H:%M:%S",localtime(&time_date));
  sprintf(unitInfo, "(units [us] unless explicitly given)");
  sprintf(title,  "\033[7m Sync Monitor %3s ----------------------------------------------------------------- %s - v%06x\033[0m", name, unitInfo, SYNC_MON_VERSION);
  sprintf(footer, "\033[7m exit <q> | help <h>                                                                                            %s\033[0m", buff);

  comlib_term_curpos(1,1);

  // printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("%s\n", title);
  printf("%s\n", headerK);
  for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLineK[i]);
  if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", emptyK);      
  printf("%s\n", footer);
  
  // printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
    
  flagPrintNow = 0;
} // printServices
  

// print help text to screen
void printHelpText()
{
  int i;

  comlib_term_curpos(1,1);
  printf("%s\n", title);
  
  for (i=0; i<15; i++) printf("%s\n", emptyK);
  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("please visit the following URL                                                    \n");
  printf("https://wiki.gsi.de/TOS/....                                                      \n");
  printf("%s\n", emptyK);
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printHelpText


int main(int argc, char** argv)
{
  int opt, error = 0;
  int exitCode   = 0;
  //  char *tail;

  int      getVersion;

  char     userInput;
  int      quit;

  char     prefix[DIMMAXSIZE];
  char     name[DIMMAXSIZE];
  int      i;

  program           = argv[0];

  getVersion        = 0;
  quit              = 0;

  while ((opt = getopt(argc, argv, "eh")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'h':
        help();
        return 0;
        error = 1;
        break;
      default:
        fprintf(stderr, "%s: bad getopt result\n", program);
        return 1;
    } // switch opt
  } //  while opt

  if (error) {
    help();
    return 1;
  } // if error

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  if (optind< argc) {
    sprintf(prefix, "syncserv_%s", argv[optind]);
    sprintf(name, "%s",  argv[optind]);
  } // if optindex
  else {
    sprintf(prefix, "syncser");
    sprintf(name, "none");
  } // else optindex

  if (getVersion) printf("%s: version %06x\n", program, SYNC_MON_VERSION);

  comlib_term_clear();
  printf("%s: starting client using prefix %s\n", program, prefix);

  for (i=0; i<NALLSID; i++) {
    //    flagSetUpdate[i] = 0;
    sprintf(printLineK[i], "not initialized");
  } // for i
  
  
  dicSubscribeServices(prefix);
    
  buildHeader();
  flagPrintNow = 1;

  // wait a bit, then rebuild all indices
  usleep(1000000);
  
  while (!quit) {

    // check for new data and update text for later printing
    /*
    for (i=0; i<NALLSID; i++) {
      if (flagSetUpdate[i]) {
        flagSetUpdate[i] = 0;
        flagPrintNow     = 1;
        buildPrintLine(i);
      } // if flagSetUpdate
    } // for i
    */
    
    if (flagPrintNow) printData(name);
    if (!quit) {
      userInput = comlib_term_getChar();
      switch (userInput) {
        case 'h'         :
          printHelpText();
          flagPrintNow = 1;
          break;
        case 'q'         :
          quit = 1;
          break;
        default          :
          usleep(500000);
          break;
      } // switch
    } // if !quit
  } // while

  return exitCode;
} // main

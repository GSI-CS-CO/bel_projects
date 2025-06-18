/*******************************************************************************************
 *  sync-mon.c
 *
 *  created : 2025
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-jun-2025
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
#define SYNC_MON_VERSION 0x000004

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


// get values
monval_t monData[NALLSID][MAXEVTS];                         // [idx][0: extraction machine, 1,2: injection machine]
uint32_t flagSetValid[NALLSID];                             // flag: data of this set are valid
uint32_t flagSetUpdate[NALLSID];                            // flag: update displayed data of this set
int      flagInj[NALLSID];                                  // flag: injection from preceeding machine
int      set_msecs[NALLSID];                                // CBS deadline, fraction [ms]
time_t   set_secs[NALLSID];                                 // CBS deadline, time [s]

time_t   secsOffset;                                        // offset between timestamp and system time

// other
int      flagPrintIdx[NALLSID];                             // flag: print line with given index
int      flagPrintInactive;                                 // flag: print inactive SIDs too
int      flagPrintOther;                                    // flag: print alternative data set
int      flagPrintUnilac;
int      flagPrintSis18;                                    // flag: print SIDs for SIS18
int      flagPrintEsr;                                      // flag: print SIDs for ESR
int      flagPrintYr;                                       // flag: print SIDs for CRYRINg
int      flagPrintNow;                                      // flag: print stuff to screen NOW
int      flagPrintNs;                                       // flag: in units of nanoseconds

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


// convert index to ring and sid
void idx2RingSid(uint32_t idx, ring_t *ring, uint32_t *sid)
{
  if (idx >= NALLSID) {
    *ring = NORING;
    *sid  = 0;
    return;
  } // if idx
  
  switch (idx) {
    case 0 ... 15 :
      *ring = SIS18;
      *sid  = idx;
      break;
    case 16 ... 31 :
      *ring = ESR;
      *sid  = idx - 16;
      break;
    case 32 ... 47 :
      *ring = CRYRING;
      *sid  = idx - 32;
      break;
    default :
      *ring = NORING;
      *sid  = 0;
      break;
  } // switch idx
} // idx2RingSid


void buildHeader()
{
  sprintf(headerK, "| t_last [UTC] | origin | sid|            event ||  destn | sid|          event a |      diff a |          event b |      diff b |");
  sprintf(emptyK,  "|              |        |    |                  ||        |    |                  |             |                  |             |");
  //        printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// build strings for printing
void buildPrintLine(uint32_t idx)
{
  char     origin[32];
  char     dest[32];
  int      flagTCBS;
  int      flagDest2;
  char     tCBS[64];
  char     extSid[32];
  char     injSid[32];
  char     extEvt[32];
  char     injEvtA[32];
  char     injEvtB[32];
  char     diffA[32];                 // time difference of injEvtA - ExtEvt [us]
  char     diffB[32];                 // time difference of injEvtB - ExtEvt [us]

  char     tmp1[32];
  double   dtmp;

  uint32_t sid;
  ring_t   ringInj;

  uint64_t actNsecs;
  time_t   actT;

  actNsecs  = comlib_getSysTime();
  actT      = (time_t)(actNsecs / 1000000000);
  dtmp      = NAN;

  if (idx > NALLSID) return;

  idx2RingSid(idx, &ringInj, &sid);

  // destination
  switch (ringInj) {
    case SIS18   : sprintf(dest, "SIS18") ; sprintf(origin, "UNILAC"); flagDest2 = 1; flagTCBS = 1; break;
    case ESR     : sprintf(dest, "ESR")   ; sprintf(origin, "SIS18") ; flagDest2 = 1; flagTCBS = 1; break;
    case CRYRING : sprintf(dest, "YR")    ; sprintf(origin, "ESR")   ; flagDest2 = 1; flagTCBS = 1; break;
    default      : sprintf(dest, TXTUNKWN); sprintf(origin, " ")     ; flagDest2 = 0; flagTCBS = 1; break;
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
  else sprintf(tCBS, "---");

  // SID of injection machine
  sprintf(injSid, "%2d", monData[idx][1].sid);

  flagInj[idx] = 1;
 
  // injection: beam came from extraction machine
  if (flagInj[idx]) {
    smGetEvtString(monData[idx][0].evtNo, tmp1);
    sprintf(extSid , "%2d" , monData[idx][0].sid);
    sprintf(extEvt , "%16s", tmp1);

    smGetEvtString(monData[idx][1].evtNo, tmp1);
    sprintf(injEvtA, "%16s", tmp1);
    dtmp = (int64_t)(monData[idx][1].deadline - monData[idx][0].deadline) / 1000.0;
    if (fabs(dtmp) > 1000000.0) dtmp = NAN;

    if (isnan(dtmp)) sprintf(diffA, "%11s"    , "");
    else             sprintf(diffA, "%11.3f", dtmp);

    // two injection events
    if (flagDest2) {
      smGetEvtString(monData[idx][2].evtNo, tmp1);
      sprintf(injEvtB, "%16s", tmp1);
      dtmp = (int64_t)(monData[idx][2].deadline - monData[idx][0].deadline) / 1000.0;
      if (fabs(dtmp) > 1000000.0) dtmp = NAN;

      if (isnan(dtmp)) sprintf(diffB, "%11s"    , "");
      else             sprintf(diffB, "%11.3f", dtmp);
    } // if flagDest2
    else {
      sprintf(injEvtB, "%16s", "");
      sprintf(diffB  , "%11s", "");
    } // else flagDest2
  } // if flagInj
  else {
    sprintf(extSid , "%2s" , "");
    sprintf(extEvt , "%16s", "");
    sprintf(injSid , "%2s" , "");
    sprintf(injEvtA, "%16s", "");
    sprintf(diffA  , "%11s", "");
    sprintf(injEvtB, "%16s", "");
    sprintf(diffB  , "%11s", "");
  } // else flagInj
  
  sprintf(printLineK[idx], "| %12s | %6s | %2s | %16s || %6s | %2s | %16s | %11s | %16s | %11s |", tCBS, origin, extSid, extEvt, dest, injSid, injEvtA, diffA, injEvtB, diffB);
  //                printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  

} //buildPrintLine

// receive set values
void recSetvalue(long *tag, monval_t *address, int *size)
{
  monval_t *tmp;
  int      secs;
  uint32_t idxOffset;
  uint32_t idx;
  int      flagValid;
  uint64_t deadline;

  uint64_t actNsecs;
  time_t   actT;

  /* printf("tag %lx\n", *tag); */

  switch (*tag) {
    case tagSis18i : idxOffset =  0; break;
    case tagEsri   : idxOffset = 16; break;
    default           : return;
  } // switch tag

  flagValid = (*size != sizeof(uint32_t));
  if (!flagValid) return;

  tmp      = address;
  idx      = (*tmp).sid + idxOffset;
  deadline = (*tmp).deadline;

  switch (*tag) {
    case tagSis18i :
      // copy values; only copy values if the separation of deadlines is below DTLIMIT
      if ((deadline - dicUnilacE0.deadline) < DTLIMIT) monData[idx][0] = dicUnilacE0;
      else                                             monData[idx][0] = smEmptyMonData();
      monData[idx][1] = *tmp;
      if ((deadline - dicSis18I1.deadline)  < DTLIMIT) monData[idx][2] = dicSis18I1;
      else                                             monData[idx][2] = smEmptyMonData();
      break;
    case tagEsri :
      // copy values; only copy values if the separation of deadlines is below DTLIMIT
      if ((deadline - dicSis18E0.deadline)  < DTLIMIT) monData[idx][0] = dicSis18E0;
      else                                             monData[idx][0] = smEmptyMonData();
      monData[idx][1] = *tmp;
      if ((deadline - dicEsrI1.deadline)    < DTLIMIT) monData[idx][2] = dicEsrI1;
      else                                             monData[idx][2] = smEmptyMonData();
      break;
    default :
      break;
  } // switch tag

  //printf("sid %d, evtno%d\n", (*tmp).sid, (*tmp).evtNo);

  // get timestamp
  dic_get_timestamp(0, &secs, &(set_msecs[idx]));
  set_secs[idx]      = (time_t)(secs);

  // calibrate offset between THIS system time and time of set_values
  actNsecs           = comlib_getSysTime();
  actT               = (time_t)(actNsecs / 1000000000);
  secsOffset         = actT - set_secs[idx];

  flagSetUpdate[idx] = 1;
  flagPrintNow       = 1;
} // recSetValue


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char     name[DIMMAXSIZE];

  // UNILAC 'extraction'
  sprintf(name, "%s_unilac-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicUnilacE0Id     = dic_info_service_stamped(name, MONITORED, 0, &dicUnilacE0, sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // SIS18 injection, main thread
  sprintf(name, "%s_sis18-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicSis18I0Id       = dic_info_service_stamped(name, MONITORED, 0, &dicSis18I0 , sizeof(monval_t), recSetvalue, (long)tagSis18i, &no_link_32, sizeof(uint32_t));

  // SIS18 injection, injection thread
  sprintf(name, "%s_sis18-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicSis18I1Id       = dic_info_service_stamped(name, MONITORED, 0, &dicSis18I1 , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // SIS18 extraction
  sprintf(name, "%s_sis18-ext-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicSis18E0Id       = dic_info_service_stamped(name, MONITORED, 0, &dicSis18E0 , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // ESR injection, schedule
  sprintf(name, "%s_esr-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicEsrI0Id         = dic_info_service_stamped(name, MONITORED, 0, &dicEsrI0   , sizeof(monval_t), recSetvalue, (long)tagEsri, &no_link_32, sizeof(uint32_t));

  // ESR injection, b2b
  sprintf(name, "%s_esr-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicEsrI1Id         = dic_info_service_stamped(name, MONITORED, 0, &dicEsrI1   , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // ESR extraction
  sprintf(name, "%s_esr-ext-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicEsrE0Id         = dic_info_service_stamped(name, MONITORED, 0, &dicEsrE0   , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

  // CRYRING injection, schedule
  sprintf(name, "%s_yr-inj-mon_data00", prefix);
  /* printf("name %s\n", name); */
  dicYrI0Id          = dic_info_service_stamped(name, MONITORED, 0, &dicYrI0    , sizeof(monval_t), recSetvalue, (long)tagEsri, &no_link_32, sizeof(uint32_t));

  // CRYRING injection, b2b
  sprintf(name, "%s_esr-inj-mon_data01", prefix);
  /* printf("name %s\n", name); */
  dicYrI1Id          = dic_info_service_stamped(name, MONITORED, 0, &dicYrI1    , sizeof(monval_t), 0, 0, &no_link_32, sizeof(uint32_t));

} // dicSubscribeServices


// clear status
void clearStatus()
{
  printf("not yet implemented, press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // clearStatus


// calc flags for printing
uint32_t calcFlagPrint()
{
  int      i;
  ring_t   ring = NORING;
  uint32_t nLines;

  uint64_t actNsecs;
  time_t   actT;

  nLines   = 0;
  actNsecs = comlib_getSysTime();
  actT     = (time_t)(actNsecs / 1000000000);

  for (i=0; i<NALLSID; i++) {
    flagPrintIdx[i] = 1;
    //idx2RingSid(i, &ring, &sid);
        
    if (!flagPrintInactive) {
      if ((actT - set_secs[i] - secsOffset) > (time_t)TINACTIVE) flagPrintIdx[i] = 0; // ignore old timestamps
      if (set_secs[i] <= 1)                                      flagPrintIdx[i] = 0; // ignore ancient timestamps
    } // if !flagPrintActive
    if (!flagPrintSis18)    if (ring == SIS18)                   flagPrintIdx[i] = 0;
    if (!flagPrintEsr)      if (ring == ESR)                     flagPrintIdx[i] = 0;
    if (!flagPrintYr)       if (ring == CRYRING)                 flagPrintIdx[i] = 0;
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
  sprintf(footer, "\033[7m exit <q> | toggle inactive <i>, SIS18 <0>, ESR <1>, YR <2> | help <h>                                          %s\033[0m", buff);

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
  flagPrintInactive = 0;
  flagPrintSis18    = 1;
  flagPrintEsr      = 1;
  flagPrintYr       = 1;
  flagPrintOther    = 0;
  flagPrintNs       = 0;

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
    flagSetUpdate[i] = 0;
    sprintf(printLineK[i], "not initialized");
  } // for i
  dicSubscribeServices(prefix);
    
  buildHeader();
  flagPrintNow = 1;

  // wait a bit, then rebuild all indices
  usleep(1000000);
  
  while (!quit) {

    // check for new data and update text for later printing
    for (i=0; i<NALLSID; i++) {
      if (flagSetUpdate[i]) {
        flagSetUpdate[i] = 0;
        flagPrintNow     = 1;
        buildPrintLine(i);
      } // if flagSetUpdate
    } // for i
    
    if (flagPrintNow) printData(name);
    if (!quit) {
      userInput = comlib_term_getChar();
      switch (userInput) {
        case 'c' :
          clearStatus();
          break;
        case 'i' :
          // toggle printing of inactive patterns
          comlib_term_clear();
          flagPrintInactive = !flagPrintInactive;
          flagPrintNow = 1;
          break;
        case '0' :
          // toggle printint of SIS18 patterns
          flagPrintSis18 = !flagPrintSis18;
          flagPrintNow = 1;
          break;
        case '1' :
          // toggle printing of ESR patterns
          flagPrintEsr = !flagPrintEsr;
          flagPrintNow = 1;
          break;
        case '2' :
          // toggle printing of CRYRING patterns
          flagPrintYr = !flagPrintYr;
          flagPrintNow = 1;
          break;
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

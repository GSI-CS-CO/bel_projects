/*******************************************************************************************
 *  b2b-mon.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Apr-2022
 *
 * subscribes to and displays status of many b2b transfers
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
#define B2B_MON_VERSION 0x000400

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

// b2b
#include <common-lib.h>                  // COMMON
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW

const char* program;

// dim stuff
#define  DIMCHARSIZE 32                   // standard size for char services
#define  DIMMAXSIZE  1024                 // max size for service names
#define  SCREENWIDTH 1024                 // width of screen
#define  NALLSID     48                   // number of all SIDs observed; SIS18 (16), ESR (16), CRYRING (16)
#define  TINACTIVE   120                  // [s]; if the previous data is more in the past than this value, the transfer data is considered inactive

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

setval_t   dicSetval[NALLSID];
getval_t   dicGetval[NALLSID];
diagval_t  dicDiagval[NALLSID];
diagstat_t dicDiagstat[NALLSID];
nueMeas_t  dicNueMeasExt[NALLSID];
char       dicPName[NALLSID][DIMMAXSIZE];

uint32_t  dicSetvalId[NALLSID];
uint32_t  dicGetvalId[NALLSID];
uint32_t  dicDiagvalId[NALLSID];
uint32_t  dicDiagstatId[NALLSID];
uint32_t  dicNueMeasExtId[NALLSID];
uint32_t  dicPNameId[NALLSID];

#define  TXTNA       "  N/A"
#define  TXTUNKWN    "UNKWN"
#define  TXTERROR    "ERROR"


// set values
uint32_t flagSetValid[NALLSID];                             // flag: data of this set are valid
uint32_t flagSetUpdate[NALLSID];                            // flag: update displayed data of this set
uint32_t set_mode[NALLSID];                                 // b2b mode
double   set_extT[NALLSID];                                 // extraction, h=1 period [ns]
double   set_extNue[NALLSID];                               // extraction, h=1 frequency [Hz]
uint32_t set_extH[NALLSID];                                 // extraction, harmonic number
double   set_extCTrig[NALLSID];                             // extraction, kick trigger correction
double   set_injT[NALLSID];                                 // injection ...
double   set_injNue[NALLSID];
uint32_t set_injH[NALLSID];
double   set_injCTrig[NALLSID];
double   set_cPhase[NALLSID];                               // b2b: phase correction [ns]
double   set_cPhaseD[NALLSID];                              // b2b: phase correction [degree]
uint32_t set_msecs[NALLSID];                                // CBS deadline, fraction [ms]
time_t   set_secs[NALLSID];                                 // CBS deadline, time [s]

time_t   secsOffset;                                        // offset between timestamp and system time

#define MSKRECMODE0 0x0                 // mask defining events that should be received for the different modes, mode off
#define MSKRECMODE1 0x050               // ... mode CBS
#define MSKRECMODE2 0x155               // ... mode B2E
#define MSKRECMODE3 0x1f5               // ... mode B2C
#define MSKRECMODE4 0x3ff               // ... mode B2B


// other
int      flagPrintIdx[NALLSID];                             // flag: print line with given index
int      flagPrintInactive;                                 // flag: print inactive SIDs too
int      flagPrintNue;                                      // flag: print frequency data
int      flagPrintSis18;                                    // flag: print SIDs for SIS18
int      flagPrintEsr;                                      // flag: print SIDs for ESR
int      flagPrintYr;                                       // flag: print SIDs for CRYRINg
int      flagPrintNow;                                      // flag: print stuff to screen NOW

int      modeMask;                                          // mask: marks events used in actual mode

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     headerK[SCREENWIDTH+1];                            // header line to be printed; header for kicker info
char     headerN[SCREENWIDTH+1];                            // header line to be printed; header for frequency info
char     emptyK[SCREENWIDTH+1];                             // empty line to be printed; kicker info
char     emptyN[SCREENWIDTH+1];                             // empty line to be printed; frequency info
char     printLineK[NALLSID][SCREENWIDTH+1];                // lines to be printed; line for kicker info
char     printLineN[NALLSID][SCREENWIDTH+1];                // lines to be printed; line for frequency info


// help
static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <name>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display information on all transfers of the B2B system\n");
  fprintf(stderr, "Example1: '%s pro'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(B2B_MON_VERSION));
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
  sprintf(headerK, "|        pattern name | t_last [UTC] | orign | sid| kick  set       trg offst probR | destn |    phase | kick  set       trg offst probR dOffst 'ToF'|");
  sprintf(emptyK,  "|                     |              |       |    |                                 |       |          |                                             |");
  sprintf(headerN, "|        pattern name | t_last [UTC] | orign | sid| h1gDDS  set         get(stdev)diff[Hz] |     destn | kick  set       trg offst probR dOffst 'ToF'|");
  sprintf(emptyN,  "|                     |              |       |    |                                        |           |                                             |");
  //        printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// build string for printing
void buildPrintLine(uint32_t idx)
{
  char     origin[10];
  char     dest[10];
  char     pattern[64];
  int      flagTCBS;
  int      flagExtNue;
  int      flagB2b;
  int      flagExtTrig;
  int      flagInjTrig;
  char     tCBS[64];
  char     extNue[64];
  char     b2b[64];
  char     extTrig[64];
  char     injTrig[64];
  char     nueMeasExt[128];
  char     tmp1[32];
  char     tmp2[32];
  char     tmp3[32];
  char     tmp4[32];
  char     tmp5[32];
  uint32_t utmp1;
  uint32_t utmp2;
  double   dtmp1;

  uint32_t sid;
  ring_t   ring;

  uint64_t actUsecs;
  time_t   actT;

  actUsecs = comlib_getSysTime();
  actT     = (time_t)(actUsecs / 1000000);

  if (idx > NALLSID) return;

  idx2RingSid(idx, &ring, &sid);

  // extraction ring name
  switch (ring) {
    case SIS18   : sprintf(origin, "SIS18");  sprintf(tmp1, "ESR");  break;
    case ESR     : sprintf(origin, "ESR");    sprintf(tmp1, "YR");   break;
    case CRYRING : sprintf(origin, "YR");     sprintf(tmp1, " ");    break;
    default      : sprintf(origin, TXTUNKWN); sprintf(tmp1, " ");    break;
  } // switch ring

  // pattern name
  if (strlen(dicPName[idx]) == 0) sprintf(pattern, "%s", TXTUNKWN);
  else {
    if ((actT - set_secs[idx] - secsOffset) < (time_t)TINACTIVE)
                                  sprintf(pattern, "%.20s", dicPName[idx]);
    else                          sprintf(pattern, "%s", TXTUNKWN);
    if (set_secs[idx] <= 1)       sprintf(pattern, "%s", TXTUNKWN);
  }
  if (!flagSetValid[idx])         sprintf(pattern, "NO_LINK (DATA)");

  // destination
  switch (set_mode[idx]) {
    case 0 : sprintf(dest, "---");      flagTCBS = 1; flagExtNue = 0; flagB2b = 0; flagExtTrig = 0; flagInjTrig = 0; break;
    case 1 : sprintf(dest, "kicker");   flagTCBS = 1; flagExtNue = 0; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case 2 : sprintf(dest, "target");   flagTCBS = 1; flagExtNue = 1; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case 3 : sprintf(dest, "%s", tmp1); flagTCBS = 1; flagExtNue = 1; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 1; break;
    case 4 : sprintf(dest, "%s", tmp1); flagTCBS = 1; flagExtNue = 1; flagB2b = 1; flagExtTrig = 1; flagInjTrig = 1; break;
    default: sprintf(dest, TXTUNKWN);   flagTCBS = 0; flagExtNue = 0; flagB2b = 0; flagExtTrig = 0; flagInjTrig = 0; break;
  } // switch set_mode

  // ignore ancient timestamps
  if (set_secs[idx] <= 1) flagTCBS = 0;
  
  if (flagTCBS)     {strftime(tmp1, 10, "%H:%M:%S", gmtime(&(set_secs[idx]))); sprintf(tCBS, "%8s.%03d", tmp1, set_msecs[idx]);}
  else               sprintf(tCBS, "---");

  if (flagExtNue) {
    if ((dicGetval[idx].flagEvtErr >> 2) & 0x1) sprintf(extNue, "%s",    TXTERROR);
    else                                        sprintf(extNue, "%11.3f", set_extNue[idx]);
    if (*(uint32_t *)&(dicNueMeasExt[idx]) == no_link_32) sprintf(nueMeasExt, "NOLINK");
    else {
      if (dicNueMeasExt[idx].nTS > 2) {
        if (dicNueMeasExt[idx].nueErr > 10.0)     sprintf(tmp1, " > 10");
        else                                      sprintf(tmp1, "%5.3f", dicNueMeasExt[idx].nueErr);
        if (fabs(dicNueMeasExt[idx].nueDiff)>100) sprintf(tmp2, "  > 100");
        else                                      sprintf(tmp2, "%7.3f", dicNueMeasExt[idx].nueDiff);
        sprintf(nueMeasExt, "%11.3f %11.3f(%5s) %s", dicNueMeasExt[idx].nueSet, dicNueMeasExt[idx].nueGet, tmp1, tmp2);
      } // if nTS
    else                                          sprintf(nueMeasExt, "ERROR: no RF signal detected %x ", *(uint32_t *)&(dicNueMeasExt[idx]));
    } // else NOLINK
  } // if flagExtNue
  else {
    sprintf(extNue, "---");
    sprintf(nueMeasExt, "---");
  }
  if (flagB2b) {
    if ((dicGetval[idx].flagEvtErr >> 3) & 0x1) sprintf(b2b, "%s",  TXTERROR);
    else                                        sprintf(b2b, "%9.3f", dicDiagval[idx].phaseOffAct);
  } // if flagB2B
  else {
    if (flagInjTrig) sprintf(b2b, "coastg");
    else             sprintf(b2b, "---");
  } // else flagb2b
  
  if (flagExtTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> 4) & 0x1) sprintf(tmp1, "%9.3f", set_extCTrig[idx] + dicDiagval[idx].ext_ddsOffAct);
    else sprintf(tmp1, "%s", TXTERROR);
    // signal from output of kicker electronics
    if ((dicGetval[idx].flag_nok >> 1) & 0x1) {
      sprintf(tmp2, "%s", TXTERROR);
      sprintf(tmp3, "%s", TXTUNKWN);
    } // if not ok
    else {
      sprintf(tmp2, "%5d", dicGetval[idx].ext_dKickMon);
      // signal from magnet probe
      if ((dicGetval[idx].flag_nok >> 2) & 0x1) sprintf(tmp3, "%s",  TXTUNKWN);
      else                                      sprintf(tmp3, "%5d", dicGetval[idx].ext_dKickProb);
    } //else not ok
    sprintf(extTrig, "%9.3f %9s %5s %5s", set_extCTrig[idx], tmp1, tmp2, tmp3);
  } // if flagExtTrig
  else sprintf(extTrig, "---");

  if (flagInjTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> 5) & 0x1) {
      if (flagB2b) dtmp1 = set_injCTrig[idx] + dicDiagval[idx].inj_ddsOffAct;  //b2b : diff to DDS of injection ring
      else         dtmp1 = set_injCTrig[idx] + dicDiagval[idx].ext_ddsOffAct;  //else: diff to DDS of extraction ring
      sprintf(tmp1, "%9.3f", dtmp1);
    }
    else sprintf(tmp1, "%s", TXTERROR);
    // signal from output of kicker electronics    
    if ((dicGetval[idx].flag_nok >> 6) & 0x1) {
      sprintf(tmp2, "%s", TXTERROR);
      sprintf(tmp3, "%s", TXTUNKWN);
      sprintf(tmp4, "%s", TXTUNKWN);
      sprintf(tmp5, "%s", TXTUNKWN);     
    } // if not ok
    else {
      sprintf(tmp2, "%5d", dicGetval[idx].inj_dKickMon);
      utmp1   = set_injCTrig[idx] - set_extCTrig[idx] + dicGetval[idx].inj_dKickMon - dicGetval[idx].ext_dKickMon;
      if ((dicGetval[idx].flag_nok >> 1) & 0x1)  sprintf(tmp4, "%5s", TXTERROR);
      else                                       sprintf(tmp4, "%5d", utmp1);
      if ((dicGetval[idx].flag_nok >> 7) & 0x1) {sprintf(tmp3, "%5s", TXTUNKWN); sprintf(tmp5, "%5s", TXTUNKWN);}
      else {
        sprintf(tmp3, "%5d", dicGetval[idx].inj_dKickProb);
        utmp2 = set_injCTrig[idx] - set_extCTrig[idx] + dicGetval[idx].inj_dKickProb - dicGetval[idx].ext_dKickProb;
        if ((dicGetval[idx].flag_nok >> 2) & 0x1) sprintf(tmp5, "%5s", TXTUNKWN);
        else                                      sprintf(tmp5, "%5d", utmp2);
      } // if not nok
    } //else not ok
    sprintf(injTrig, "%9.3f %9s %5s %5s %5s %5s", set_injCTrig[idx], tmp1, tmp2, tmp3, tmp4, tmp5);
  } // if flagExtTrig
  else sprintf(injTrig, "---");

  sprintf(printLineK[idx], "|%20s | %12s |%6s | %2d | %31s |%6s |%9s | %43s |", pattern, tCBS, origin, sid, extTrig, dest, b2b, injTrig);
  sprintf(printLineN[idx], "|%20s | %12s |%6s | %2d | %38s |%10s | %43s |", pattern, tCBS, origin, sid, nueMeasExt, dest, injTrig);
  //                printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  

} //buildPrintLine

// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  setval_t *tmp;
  uint32_t secs;
  uint32_t nok;
  uint32_t idx;

  uint64_t actUsecs;
  time_t   actT;

  /* printf("tag %lx\n", *tag); */
  if ((*tag < 0) || (*tag >= NALLSID)) return;
  idx = (uint32_t)(*tag);
  if (idx >= NALLSID) return;
  
  flagSetValid[idx] = (*size != sizeof(uint32_t));

  if (flagSetValid[idx]) {
    tmp = address;

    nok                = (*tmp).flag_nok;
    set_mode[idx]      = (*tmp).mode;
    if ((nok >> 1) & 0x1) {
      set_extT[idx]    = 0.0;
      set_extNue[idx]  = 0.0;
    } // if not valid
    else {
      set_extT[idx]    = (double)((*tmp).ext_T)/1000000000.0;
      set_extNue[idx]  = 1000000000.0 / set_extT[idx];
      set_cPhaseD[idx] = (double)((*tmp).cPhase) / (double)(set_extT[idx]) * 360.0; 
    } // valid
    set_extH[idx]      = (*tmp).ext_h;
    set_extCTrig[idx]  = (*tmp).ext_cTrig;
    if ((nok >> 4) & 0x1) {
      set_injT[idx]    = 0.0;
      set_injNue[idx]  = 0.0;
    } // if not valid
    else {
      set_injT[idx]    = (double)((*tmp).inj_T)/1000000000.0;
      set_injNue[idx]  = 1000000000.0 / set_injT[idx];
    } // valid
    set_injH[idx]      = (*tmp).inj_h;
    set_injCTrig[idx]  = (*tmp).inj_cTrig;
    set_cPhase[idx]    = (*tmp).cPhase;

    dic_get_timestamp(0, &secs, &(set_msecs[idx]));
    set_secs[idx]      = (time_t)(secs);

    // calibrate offset between THIS system time and time of set_values
    actUsecs           = comlib_getSysTime();
    actT               = (time_t)(actUsecs / 1000000);
    secsOffset         = actT - set_secs[idx];
  } // if flagSetValid
  else set_mode[idx] = 0;

  flagSetUpdate[idx] = 1;
  flagPrintNow = 1;
} // recSetValue


// add all dim services
void dicSubscribeServices(char *prefix, uint32_t idx)
{
  char     name[DIMMAXSIZE];
  char     ringName[32];
  ring_t   ring;
  uint32_t sid;

  idx2RingSid(idx, &ring, &sid);
  switch (ring) {
    case SIS18   : sprintf(ringName, "sis18"); break;
    case ESR     : sprintf(ringName, "esr");   break;
    case CRYRING : sprintf(ringName, "yr");    break;
    default : break;
  } // switch ring

  sprintf(name, "%s_%s-raw_sid%02d_setval", prefix, ringName, sid);
  /* printf("name %s\n", name); */
  dicSetvalId[idx]     = dic_info_service_stamped(name, MONITORED, 0, &(dicSetval[idx]), sizeof(setval_t), recSetvalue, (long)idx, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-raw_sid%02d_getval", prefix, ringName, sid);
  /* printf("name %s\n", name); */
  dicGetvalId[idx]     = dic_info_service_stamped(name, MONITORED, 0, &(dicGetval[idx]), sizeof(getval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_diag_sid%02d", prefix, ringName, sid);
  /* printf("name %s\n", name); */
  dicDiagvalId[idx]    = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagval[idx]), sizeof(diagval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_stat_sid%02d", prefix, ringName,  sid);
  /* printf("name %s\n", name); */
  dicDiagstatId[idx]   = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagstat[idx]), sizeof(diagstat_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-other-rf_sid%02d_ext", prefix, ringName,  sid);
  /* printf("name %s\n", name); */
  dicNueMeasExtId[idx] = dic_info_service_stamped(name, MONITORED, 0, &(dicNueMeasExt[idx]), sizeof(nueMeas_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name,"%s_%s-pname_sid%02d", prefix, ringName, sid);
  /* printf("name %s\n", name);*/
  dicPNameId[idx]      = dic_info_service_stamped(name, MONITORED, 0, &(dicPName[idx]), DIMMAXSIZE, 0 , 0, &no_link_str, sizeof(no_link_str));
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
  ring_t   ring;
  uint32_t sid;
  uint32_t nLines;

  uint64_t actUsecs;
  time_t   actT;

  nLines   = 0;
  actUsecs = comlib_getSysTime();
  actT     = (time_t)(actUsecs / 1000000);

  for (i=0; i<NALLSID; i++) {
    flagPrintIdx[i] = 1;
    idx2RingSid(i, &ring, &sid);
        
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
  time_t   time_date;
  uint32_t nLines;
  uint32_t minLines = 20;
  int      i;

  nLines = calcFlagPrint();

  time_date = time(0);
  strftime(buff,53,"%d-%b-%y %H:%M:%S",localtime(&time_date));
  sprintf(title,  "\033[7m B2B Monitor %3s ------------------------------------------------------------------------------------ (units [ns] unless explicitly given) - v%8s\033[0m", name, b2b_version_text(B2B_MON_VERSION));
  sprintf(footer, "\033[7m exit <q> | toggle inactive <i>, SIS18 <0>, ESR <1>, YR <2> | toggle data <d> | help <h>                                            %s\033[0m", buff);

  comlib_term_curpos(1,1);

  printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("%s\n", title);
  if (flagPrintNue) {
    printf("%s\n", headerN);
    for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLineN[i]);
    if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", emptyN);
  } // if printNue
  else {
    printf("%s\n", headerK);
    for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLineK[i]);
    if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", emptyK);      
  } // else printNue
  printf("%s\n", footer);
  
  printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
    
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
  printf("https://www-acc.gsi.de/wiki/BunchBucket/BunchBucketHowCLI#B2B_Monitor             \n");
  printf("%s\n", emptyK);
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printHelpText


int main(int argc, char** argv)
{
  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

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
  flagPrintNue      = 0;

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
    sprintf(prefix, "b2b_%s", argv[optind]);
    sprintf(name, "%s",  argv[optind]);
  } // if optindex
  else {
    sprintf(prefix, "b2b");
    sprintf(name, "none");
  } // else optindex

  if (getVersion) printf("%s: version %s\n", program, b2b_version_text(B2B_MON_VERSION));

  comlib_term_clear();
  printf("%s: starting client using prefix %s\n", program, prefix);
  
  for (i=0; i<NALLSID; i++) {
    flagSetUpdate[i] = 0;
    sprintf(printLineK[i], "not initialized");
    sprintf(printLineN[i], "not initialized");
    dicSubscribeServices(prefix, i);
  } // for i
  buildHeader();
  flagPrintNow = 1;

  // wait a bit, then rebuild all indices
  usleep(1000000);
  /*for (i=0; i<NALLSID; i++) buildPrintLine(i);*/

  
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
          comlib_term_clear();
          flagPrintInactive = !flagPrintInactive;
          flagPrintNow = 1;
          break;
        case '0' :
          flagPrintSis18 = !flagPrintSis18;
          flagPrintNow = 1;
          break;
        case '1' :
          flagPrintEsr = !flagPrintEsr;
          flagPrintNow = 1;
          break;
        case '2' :
          flagPrintYr = !flagPrintYr;
          flagPrintNow = 1;
          break;
        case 'd' :
          flagPrintNue = !flagPrintNue;
          flagPrintNow = 1;
          break;
        case 'h'         :
          printHelpText();
          //          flagSetUpdate[0] = 1; // this is a hack to force an update
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

/*******************************************************************************************
 *  b2b-mon.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-Nov-2021
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
#define B2B_MON_VERSION 0x000310

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

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

setval_t   dicSetval[NALLSID];
getval_t   dicGetval[NALLSID];
diagval_t  dicDiagval[NALLSID];
diagstat_t dicDiagstat[NALLSID];

uint32_t  dicSetvalId[NALLSID];
uint32_t  dicGetvalId[NALLSID];
uint32_t  dicDiagvalId[NALLSID];
uint32_t  dicDiagstatId[NALLSID];

#define  TXTNA       "  N/A"
#define  TXTUNKWN    "UNKWN"
#define  TXTERROR    "ERROR"


// set values
uint32_t flagSetValid[NALLSID];                             // flag set data are valid 
uint32_t set_mode[NALLSID];                                 // b2b mode
double   set_extT[NALLSID];                                 // extraction, h=1 period [ns]
double   set_extNue[NALLSID];                               // extraction, h=1 frequency [Hz]
uint32_t set_extH[NALLSID];                                 // extraction, harmonic number
int32_t  set_extCTrig[NALLSID];                             // extraction, kick trigger correction
double   set_injT[NALLSID];                                 // injection ...
double   set_injNue[NALLSID];
uint32_t set_injH[NALLSID];
int32_t  set_injCTrig[NALLSID];
int32_t  set_cPhase[NALLSID];                               // b2b: phase correction [ns]
double   set_cPhaseD[NALLSID];                              // b2b: phase correction [degree]
uint32_t set_msecs[NALLSID];                                // CBS deadline, fraction [ms]
time_t   set_secs[NALLSID];                                 // CBS deadline, time [s]

/*
// b2b values
double   flagB2bValid;                                      // flag b2b data are valid
double   b2b_extNue;                                        // extraction, rf frequency [Hz]
double   b2b_extT;                                          // extraction, rf period [ns]
double   b2b_extN;                                          // extraction, number of rf periods within beat period
double   b2b_injNue;                                        // injeciton ...
double   b2b_injT;
double   b2b_injN;
double   b2b_diff;                                          // difference of rf periods [ns]
double   b2b_diffD;                                         // difference of rf periods [degree]
double   b2b_beatNue;                                       // beat frequency
double   b2b_beatT;                                         // beat period
*/


#define MSKRECMODE0 0x0                 // mask defining events that should be received for the different modes, mode off
#define MSKRECMODE1 0x050               // ... mode CBS
#define MSKRECMODE2 0x155               // ... mode B2E
#define MSKRECMODE3 0x1f5               // ... mode B2C
#define MSKRECMODE4 0x3ff               // ... mode B2B


// other
int      flagPrintIdx[NALLSID];                             // flag: print line with given index
int      flagPrintInactive;                                 // flag: print inactive SIDs too
int      flagPrintSis18;                                    // flag: print SIDs for SIS18
int      flagPrintEsr;                                      // flag: print SIDs for ESR
int      flagPrintYr;                                       // flag: print SIDs for CRYRINg
int      flagPrintNow;                                      // flag: print stuff to screen NOW

int      modeMask;                                          // mask: marks events used in actual mode

char     header[SCREENWIDTH+1];                             // header line to be printed
char     empty[SCREENWIDTH+1];                              // empty line to be printed
char     printLine[NALLSID][SCREENWIDTH+1];                 // lines to be printed

void term_clear(void)
{
  printf("\033[2J\033[1;1H");
}

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

/*
// convert nanoseconds to degree
double ns2Degree(double phase,          // phase [ns]
                 uint64_t T             // period [as]
                 )
{
  double period;                        // [ns]
  double degree;

  period = (double)T / 1000000000.0;
  degree = phase / period * 360.0;

  return degree;
}  // ns2Degree
*/

void buildHeader()
{
  sprintf(header, "|        pattern name | t_last [UTC] | origin | sid| h1gDDS [Hz] | kick set trg offst probR |  destn |  phase | kick set trg offst probF dOffst 'ToF'|");
  sprintf(empty , "|                     |              |        |    |             |                          |        |        |                                      |");
  //       printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeaderLine


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
  char     tmp1[32];
  char     tmp2[32];
  char     tmp3[32];
  char     tmp4[32];
  char     tmp5[32];
  uint32_t utmp1;
  uint32_t utmp2;
  int32_t  itmp1;

  uint32_t sid;
  ring_t   ring;

  if (idx > NALLSID) return;

  idx2RingSid(idx, &ring, &sid);

  // extraction ring name
  switch (ring) {
    case SIS18   : sprintf(origin, "SIS18");  sprintf(tmp1, "ESR");  break;
    case ESR     : sprintf(origin, "ESR");    sprintf(tmp1, "YR");   break;
    case CRYRING : sprintf(origin, "YR");     sprintf(tmp1, " ");    break;
    default      : sprintf(origin, "UNKNWN"); sprintf(tmp1, " ");    break;
  } // switch ring

  // pattern name
  sprintf(pattern, "coming soon");
  if (!set_mode[idx])     sprintf(pattern, "---");
  if (!flagSetValid[idx]) sprintf(pattern, "NO_LINK");

  // destination
  switch (set_mode[idx]) {
    case 0 : sprintf(dest, "---");      flagTCBS = 0; flagExtNue = 0; flagB2b = 0; flagExtTrig = 0; flagInjTrig = 0; break;
    case 1 : sprintf(dest, "kicker");   flagTCBS = 1; flagExtNue = 0; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case 2 : sprintf(dest, "target");   flagTCBS = 1; flagExtNue = 1; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case 3 : sprintf(dest, "%s", tmp1); flagTCBS = 1; flagExtNue = 1; flagB2b = 0; flagExtTrig = 1; flagInjTrig = 1; break;
    case 4 : sprintf(dest, "%s", tmp1); flagTCBS = 1; flagExtNue = 1; flagB2b = 1; flagExtTrig = 1; flagInjTrig = 1; break;
    default: sprintf(dest, "UNKNWN");   flagTCBS = 0; flagExtNue = 0; flagB2b = 0; flagExtTrig = 0; flagInjTrig = 0; break;
  } // switch set_mode

  if (flagTCBS)     {strftime(tmp1, 10, "%H:%M:%S", gmtime(&(set_secs[idx]))); sprintf(tCBS, "%8s.%03d", tmp1, set_msecs[idx]);}
  else               sprintf(tCBS, "---");

  if (flagExtNue) {
    if ((dicGetval[idx].flagEvtErr >> 2) & 0x1) sprintf(extNue, "%s",    TXTERROR);
    else                                        sprintf(extNue, "%7.3f", set_extNue[idx]);
  } // if flagExtNue
  else               sprintf(extNue, "---");

  if (flagB2b) {
    if ((dicGetval[idx].flagEvtErr >> 3) & 0x1) sprintf(b2b, "%s",  TXTERROR);
    else                                        sprintf(b2b, "%6d", dicDiagval[idx].phaseOffAct);
  } // if flagB2B
  else {
    if (flagInjTrig) sprintf(b2b, "coastg");
    else             sprintf(b2b, "---");
  } // else flagb2b
  
  if (flagExtTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> 4) & 0x1) sprintf(tmp1, "%5d",set_extCTrig[idx] + dicDiagval[idx].ext_ddsOffAct );
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
    sprintf(extTrig, "%5d %5s %5s %5s", set_extCTrig[idx], tmp1, tmp2, tmp3);
  } // if flagExtTrig
  else sprintf(extTrig, "---");

  if (flagInjTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> 5) & 0x1) {
      if (flagB2b) itmp1 = set_injCTrig[idx] + dicDiagval[idx].inj_ddsOffAct;  //b2b : diff to DDS of injection ring
      else         itmp1 = set_injCTrig[idx] + dicDiagval[idx].ext_ddsOffAct;  //else: diff to DDS of extraction ring
      sprintf(tmp1, "%5d", itmp1);
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
      if ((dicGetval[idx].flag_nok >> 1) & 0x1) sprintf(tmp4, "%5s", TXTERROR);
      else                                      sprintf(tmp4, "%5d", utmp1);
      if ((dicGetval[idx].flag_nok >> 7) & 0x1) sprintf(tmp3, "%5s", TXTUNKWN);
      else {
        sprintf(tmp2, "%5d", dicGetval[idx].inj_dKickProb);
        utmp2 = set_injCTrig[idx] - set_extCTrig[idx] + dicGetval[idx].inj_dKickProb - dicGetval[idx].ext_dKickProb;
        if ((dicGetval[idx].flag_nok >> 2) & 0x1) sprintf(tmp5, "%5s", TXTUNKWN);
        else                                      sprintf(tmp5, "%5d", utmp2);
      } // if not nok
    } //else not ok
    sprintf(injTrig, "%5d %5s %5s %5s %5s %5s", set_injCTrig[idx], tmp1, tmp2, tmp3, tmp4, tmp5);
  } // if flagExtTrig
  else sprintf(injTrig, "---");

  sprintf(printLine[idx], "|%20s | %12s | %6s | %2d | %11s | %24s | %6s | %6s | %36s |", pattern, tCBS, origin, sid, extNue, extTrig, dest, b2b, injTrig);
  //               printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  

} //buildPrintLine

// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  setval_t *tmp;
  uint32_t secs;
  uint32_t nok;
  uint32_t idx;

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
  } // if flagSetValid
  else set_mode[idx] = 0;

  buildPrintLine(idx);
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
  dicSetvalId[idx] = dic_info_service_stamped(name, MONITORED, 0, &(dicSetval[idx]), sizeof(setval_t), recSetvalue, idx, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-raw_sid%02d_getval", prefix, ringName, sid);
  dicGetvalId[idx] = dic_info_service_stamped(name, MONITORED, 0, &(dicGetval[idx]), sizeof(getval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_diag_sid%02d", prefix, ringName, sid);
  dicDiagvalId[idx] = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagval[idx]), sizeof(diagval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_stat_sid%02d", prefix, ringName,  sid);
  dicDiagstatId[idx] = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagstat[idx]), sizeof(diagstat_t), 0 , 0, &no_link_32, sizeof(uint32_t));
} // dicSubscribeServices

/*
// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t sid)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s-cal_cmd_cleardiag", prefix);
  dic_cmnd_service(name, &sid, sizeof(sid));
} // dicCmdClearDiag
*/
 /*
// calculate set values for frequency beating
void calcBeatValues()
{
  double   TFast;
  double   TSlow;
  uint32_t nPeriods;

  flagB2bValid = 0;
  b2b_extNue   = 0;    
  b2b_extT     = 0;      
  b2b_extN     = 0;      
  b2b_injNue   = 0;    
  b2b_injT     = 0;      
  b2b_injN     = 0;        
  b2b_diff     = 0;      
  b2b_diffD    = 0;     
  b2b_beatNue  = 0;   
  b2b_beatT    = 0;

  if (!flagSetValid)                      return;
  if (set_mode != 4)                      return;
  if ((set_extH == 0) || (set_injH == 0)) return;
  if ((set_extT == 0) || (set_injT == 0)) return;
  if (set_extT == set_injT)               return;
  flagB2bValid = 1;
  
  b2b_extNue = set_extNue * set_extH;
  b2b_extT   = set_extT / (double)set_extH;
  b2b_injNue = set_injNue * set_injH;
  b2b_injT   = set_injT / (double)set_injH;

  if (b2b_extT > b2b_injT) {
    TFast = b2b_injT;
    TSlow = b2b_extT;
  } // extraction has slower frequency
  else {
    TFast = b2b_extT;
    TSlow = b2b_injT;
  } // injection has slower frequency
  b2b_diff   = TSlow - TFast;
  b2b_diffD  = b2b_diff / TSlow * 360.0;
  nPeriods   = TSlow / b2b_diff;
  b2b_beatT  = nPeriods * TSlow;
  if (b2b_beatT != 0) b2b_beatNue = 1000000000.0 / b2b_beatT;
  else                b2b_beatNue = 0;
  b2b_extN   = b2b_beatT / b2b_extT;
  b2b_injN   = b2b_beatT / b2b_injT; 
} // calcBeatValues
 */
  /*
// print beat values
int printBeat()
{
  calcBeatValues(); 
  if (flagB2bValid) {
      printf("     ext                      %15.6f Hz, %15.6f ns\n", b2b_extNue, b2b_extT);
      printf("     inj                      %15.6f Hz, %15.6f ns\n", b2b_injNue, b2b_injT);
      printf("     diff                         %8.3f°              %8.6f ns\n", b2b_diffD, b2b_diff);
      printf("     beating                    %13.6f Hz, %15.6f ns\n", b2b_beatNue, b2b_beatT); 
      printf("     ext                                          %15.6f periods\n", b2b_extN);
      printf("     inj                                          %15.6f periods\n", b2b_injN);
  } // if beat valid
  else printf("     no beating: check set values\n\n\n\n\n\n");
  return 6;                                                 // 6 lines
} // printBeat
  */
   /*
// print set values
int printSet(uint32_t sid)
{
  char   modeStr[50];
  char   tCBS[100];
  
  switch (set_mode[idx]) {
    case 0 :
      sprintf(modeStr, "'off'");
      modeMask = 0;
      break;
    case 1 :
      sprintf(modeStr, "'CMD_B2B_START'");
      modeMask = MSKRECMODE1;
      break;
    case 2 :
      sprintf(modeStr, "'bunch 2 fast extraction'");
      modeMask = MSKRECMODE2;
      break;
    case 3 :
      sprintf(modeStr, "'bunch 2 coasting beam'");
      modeMask = MSKRECMODE3;      
      break;
    case 4 :
      sprintf(modeStr, "'bunch 2 bucket'");
      modeMask = MSKRECMODE4;
      break;
    default :
      sprintf(modeStr, "'unknonwn'");
  } // switch mode

  strftime(tCBS, 19, "%H:%M:%S", gmtime(&(set_secs[idx])));
  printf("--- set values ---                                                     v%8s\n", b2b_version_text(B2B_VIEWER_VERSION));
  switch (set_mode) {
    case 0 :
      printf("ext: %s\n", TXTNA);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 1 :
      printf("ext: kick  corr %4d ns\n", set_extCTrig);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 2 : 
      printf("ext: kick  corr %4d ns; gDDS %15.6f Hz, %15.6f ns, h =%2d\n", set_extCTrig, set_extNue, set_extT, set_extH);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 3 :
      printf("ext: kick  corr %4d ns; gDDS %15.6f Hz, %15.6f ns, h =%2d\n", set_extCTrig, set_extNue, set_extT, set_extH);
      printf("inj: kick  corr %4d ns; gDDS %15.6f Hz, %15.6f ns, h =%2d\n", set_injCTrig, set_injNue, set_injT, set_injH);
      printf("b2b: %s\n", TXTNA);
      break;
    case 4 :
      printf("ext: kick  corr %4d ns; gDDS %15.6f Hz, %15.6f ns, h =%2d\n", set_extCTrig, set_extNue, set_extT, set_extH);
      printf("inj: kick  corr %4d ns; gDDS %15.6f Hz, %15.6f ns, h =%2d\n", set_injCTrig, set_injNue, set_injT, set_injH);
      printf("b2b: phase corr %4d ns       %12.3f °\n", set_cPhase, set_cPhaseD);
      break;
    default :
      ;
  } // switch set_mode

  return 4;                                                 // 4 lines
} // printSet
*/
/*
// print diagnostic values
int printDiag(uint32_t sid)
{
  printf("--- diag ---                                 #b2b %5u, #ext %5u, #inj %5u\n", dicDiagval.phaseOffN, dicDiagval.ext_ddsOffN, dicDiagval.inj_ddsOffN);
  switch(set_mode) {
    case 0 ... 1 :
      printf("ext: %s\n", TXTNA);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 2 ... 3 :
      if (dicDiagval.ext_ddsOffN == 0) printf("ext: %s\n", TXTNA);
      else  printf("ext: 'diff DDS [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                   dicDiagval.ext_ddsOffAct, dicDiagval.ext_ddsOffAve, dicDiagval.ext_ddsOffSdev, dicDiagval.ext_ddsOffMin, dicDiagval.ext_ddsOffMax);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 4      :
      if (dicDiagval.ext_ddsOffN == 0) printf("ext: %s\n", TXTNA);
      else  printf("ext: 'diff gDDS  [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                   dicDiagval.ext_ddsOffAct, dicDiagval.ext_ddsOffAve, dicDiagval.ext_ddsOffSdev, dicDiagval.ext_ddsOffMin, dicDiagval.ext_ddsOffMax);
      if (dicDiagval.inj_ddsOffN == 0) printf("inj: %s\n", TXTNA);
      else  printf("inj: 'diff gDDS  [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                   dicDiagval.inj_ddsOffAct, dicDiagval.inj_ddsOffAve, dicDiagval.inj_ddsOffSdev, dicDiagval.inj_ddsOffMin, dicDiagval.inj_ddsOffMax);
      if (dicDiagval.phaseOffN == 0) printf("inj: %s\n", TXTNA);
      else  printf("b2b: 'diff phase [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                   dicDiagval.phaseOffAct, dicDiagval.phaseOffAve, dicDiagval.phaseOffSdev, dicDiagval.phaseOffMin, dicDiagval.phaseOffMax);
      break;
    default :
      ;
  } // switch set mode
  return 4;                                                 // 4 lines
} // printDiag
*/
 /*
// print kicker info
int printKick(uint32_t sid)
{
  printf("--- kicker ---                                           #ext %5u, #inj %5u\n", dicDiagstat.ext_monRemN, dicDiagstat.inj_monRemN);

  // extraction kicker
  if (set_mode == 0) printf("ext: %s\n\n", TXTNA);
  else {
    if ((dicGetval.flag_nok >> 1) & 0x1)  printf("ext: %s\n\n", TXTERROR);
    else {
      printf("ext: monitor delay [ns] %5d", dicGetval.ext_dKickMon);
      if ((dicGetval.flag_nok >> 2) & 0x1)  printf(", probe delay [ns] %s\n", TXTUNKWN);
      else                                  printf(", probe delay [ns] %5d\n", dicGetval.ext_dKickProb);
      printf("     mon h=1 ph [ns] act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n", dicDiagstat.ext_monRemAct, dicDiagstat.ext_monRemAve, dicDiagstat.ext_monRemSdev,
             dicDiagstat.ext_monRemMin, dicDiagstat.ext_monRemMax);
    } // else flag_nok
  } // else mode == 0

  // injection kicker
  if (set_mode < 3) printf("inj: %s\n\n", TXTNA);
  else {
    if ((dicGetval.flag_nok >> 6) & 0x1)  printf("inj: %s\n\n", TXTERROR);
    else {
      printf("inj: monitor delay [ns] %5d", dicGetval.inj_dKickMon);
      if ((dicGetval.flag_nok >> 7) & 0x1)  printf(", probe delay [ns] %5s", TXTUNKWN);
      else                                  printf(", probe delay [ns] %5d", dicGetval.inj_dKickProb);
      printf(", diff mon. [ns] %d\n", dicGetval.inj_dKickMon - dicGetval.ext_dKickMon);
      printf("     mon h=1 ph [ns] act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n", dicDiagstat.inj_monRemAct, dicDiagstat.inj_monRemAve, dicDiagstat.inj_monRemSdev,
             dicDiagstat.inj_monRemMin, dicDiagstat.inj_monRemMax);
    } // else flag_nok
  } // else mode < 3

  return 5;                                                 // 5 lines
} // printKick
 */
/*
// print status info
int printStatus(uint32_t sid)
{
  int i;
  uint32_t flagEvtErr;
  double   sdevKteFin;

  flagEvtErr  = dicGetval.flagEvtErr | (modeMask   & ~(dicGetval.flagEvtRec));

  printf("--- status (expert) ---                      #b2b %5u, #ext %5u, #inj %5u\n", dicDiagstat.eks_priOffN, dicDiagstat.eks_kteOffN, dicDiagstat.eks_ktiOffN);

  printf("events  :   PME  PMI  PRE  PRI  KTE  KTI  KDE  KDI  PDE  PDI\n");

  printf("required:");
  for (i=0; i<10; i++) if ((modeMask    >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");

  printf("received:");
  for (i=0; i<10; i++) if ((dicGetval.flagEvtRec  >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");

  printf("late    :");
  for (i=0; i<10; i++) if ((dicGetval.flagEvtLate >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");
  
  printf("error   :");
  for (i=0; i<10; i++) if ((flagEvtErr  >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");

  if (set_mode == 0) {
    printf("fin-CBS [us]: %s\n", TXTNA);
    printf("KTE-CBS [us]: %s\n", TXTNA);
    printf("KTE-fin [us]: %s\n", TXTNA);
  }
  else {
    sdevKteFin = sqrt(pow(dicDiagstat.eks_doneOffSdev, 2)+pow(dicDiagstat.eks_kteOffSdev, 2));
    printf("fin-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.eks_doneOffAct/1000.0, dicDiagstat.eks_doneOffAve/1000.0, dicDiagstat.eks_doneOffSdev/1000.0,
           (double)dicDiagstat.eks_doneOffMin/1000.0, (double)dicDiagstat.eks_doneOffMax/1000.0);
    printf("KTE-fin [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)(dicDiagstat.eks_kteOffAct-dicDiagstat.eks_doneOffAct)/1000.0, (dicDiagstat.eks_kteOffAve-dicDiagstat.eks_doneOffAve)/1000.0, sdevKteFin/1000.0,
           (double)(dicDiagstat.eks_kteOffMin-dicDiagstat.eks_doneOffMax)/1000.0, (double)(dicDiagstat.eks_kteOffMax-dicDiagstat.eks_doneOffMin)/1000.0);
    printf("KTE-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.eks_kteOffAct/1000.0, dicDiagstat.eks_kteOffAve/1000.0, dicDiagstat.eks_kteOffSdev/1000.0,
           (double)dicDiagstat.eks_kteOffMin/1000.0, (double)dicDiagstat.eks_kteOffMax/1000.0);
  }
  if (set_mode < 3)
    printf("KTI-CBS [us]: %s\n", TXTNA);
  else
    printf("KTI-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.eks_ktiOffAct/1000.0, dicDiagstat.eks_ktiOffAve/1000.0, dicDiagstat.eks_ktiOffSdev/1000.0,
           (double)dicDiagstat.eks_ktiOffMin/1000.0, (double)dicDiagstat.eks_ktiOffMax/1000.0);
  if (set_mode <  2)
    printf("t0E-CBS [us]: %s\n", TXTNA);
  else
    printf("t0E-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.eks_preOffAct/1000.0, dicDiagstat.eks_preOffAve/1000.0, dicDiagstat.eks_preOffSdev/1000.0,
           (double)dicDiagstat.eks_preOffMin/1000.0, (double)dicDiagstat.eks_preOffMax/1000.0);
  if (set_mode < 4)
    printf("t0I-CBS [us]: %s\n", TXTNA);
  else
    printf("t0I-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.eks_priOffAct/1000.0, dicDiagstat.eks_priOffAve/1000.0, dicDiagstat.eks_priOffSdev/1000.0,
           (double)dicDiagstat.eks_priOffMin/1000.0, (double)dicDiagstat.eks_priOffMax/1000.0);
  return 12;                                                // 12 lines
} // printStatus
*/
 /*
// print rf values
int printRf(uint32_t sid)
{
  printf("--- rf ---                                               #ext %5u, #inj %5u\n", dicDiagval.ext_rfOffN, dicDiagval.inj_rfOffN);
  switch(set_mode) {
    case 0 ... 1 :
      printf("ext: %s\n", TXTNA);
      printf("inj: %s\n", TXTNA);
      break;
    case 2 ... 3 :
      if (dicDiagval.ext_rfOffN == 0) printf("ext: %s\n", TXTNA);
      else printf("ext: 'raw gDDS [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                  dicDiagval.ext_rfOffAct, dicDiagval.ext_rfOffAve, dicDiagval.ext_rfOffSdev, dicDiagval.ext_rfOffMin, dicDiagval.ext_rfOffMax);
      printf("inj: %s\n", TXTNA);
      if (dicDiagval.ext_rfNueN == 0) printf("ext: %s\n\n", TXTNA);
      else {
           printf("ext:  '   gDDS [Hz]' ave(sdev) %13.6f(%8.6f), diff %9.6f\n", dicDiagval.ext_rfNueAve, dicDiagval.ext_rfNueSdev, dicDiagval.ext_rfNueDiff);
           printf("      '   gDDS [Hz]' estimate  %13.6f,        stepsize 0.046566\n", dicDiagval.ext_rfNueEst);
      } // else
      printf("inj: %s\n\n", TXTNA);
      break;
    case 4      :
      if (dicDiagval.ext_rfOffN == 0) printf("ext: %s\n", TXTNA);
      else printf("ext: 'raw gDDS [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                  dicDiagval.ext_rfOffAct, dicDiagval.ext_rfOffAve, dicDiagval.ext_rfOffSdev, dicDiagval.ext_rfOffMin, dicDiagval.ext_rfOffMax);
      if (dicDiagval.inj_rfOffN == 0) printf("inj: %s\n", TXTNA);
      else printf("inj: 'raw gDDS [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n",
                  dicDiagval.inj_rfOffAct, dicDiagval.inj_rfOffAve, dicDiagval.inj_rfOffSdev, dicDiagval.inj_rfOffMin, dicDiagval.inj_rfOffMax);
      if (dicDiagval.ext_rfNueN == 0) printf("ext: %s\n\n", TXTNA);
      else {
           printf("ext:  '   gDDS [Hz]' ave(sdev) %13.6f(%8.6f), diff %9.6f\n", dicDiagval.ext_rfNueAve, dicDiagval.ext_rfNueSdev, dicDiagval.ext_rfNueDiff);
           printf("      '   gDDS [Hz]' estimate  %13.6f,        stepsize 0.046566\n", dicDiagval.ext_rfNueEst);
      } // else
      if (dicDiagval.inj_rfNueN == 0) printf("inj: %s\n\n", TXTNA);
      else {
           printf("inj:  '   gDDS [Hz]' ave(sdev) %13.6f(%8.6f), diff %9.6f\n", dicDiagval.inj_rfNueAve, dicDiagval.inj_rfNueSdev, dicDiagval.inj_rfNueDiff);
           printf("      '   gDDS [Hz]' estimate  %13.6f,        stepsize 0.046566\n", dicDiagval.inj_rfNueEst);
      } // else
      break;
    default :
      ;
  } // switch set mode
  return 7;                                                 // 7 lines
} // printRf
 */
  /*
// print data to screen
void printData(int flagOnce, uint32_t sid, char *name)
{
  int i;
  int nLines = 0;
  char   tLocal[100];
  time_t time_date;
  char   modeStr[50];
  char   tCBS[100];
  
  switch (set_mode) {
    case 0 :
      sprintf(modeStr, "'off'");
      break;
    case 1 :
      sprintf(modeStr, "'CMD_B2B_START'");
      break;
    case 2 :
      sprintf(modeStr, "'bunch 2 extraction'");
      break;
    case 3 :
      sprintf(modeStr, "'bunch 2 coasting'");
      break;
    case 4 :
      sprintf(modeStr, "'bunch 2 bucket'");
      break;
    default :
      sprintf(modeStr, "'unknonwn'");
  } // switch mode

  strftime(tCBS, 19, "%H:%M:%S", gmtime(&set_secs));
  
  if (!flagOnce) {
    for (i=0;i<60;i++) printf("\n");
    time_date = time(0);
    strftime(tLocal,50,"%d-%b-%y %H:%M",localtime(&time_date));
    printf("\033[7m--- b2b viewer (%9s) ---   SID %02d %21s CBS @ %s.%03d\033[0m\n", name, sid, modeStr, tCBS, set_msecs);
    //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  } // if not once

  if (flagPrintSet)  nLines += printSet(sid);
  if (flagPrintBeat) nLines += printBeat();
  if (flagPrintDiag) nLines += printDiag(sid);
  if (flagPrintRf)   nLines += printRf(sid);
  if (flagPrintKick) nLines += printKick(sid);
  if (flagPrintStat) nLines += printStatus(sid);
  
  if (!flagOnce) {
    if (nLines < 21) for (i=0; i < (21 - nLines); i++) printf("\n");
    //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
    printf("\033[7m <q>uit <c>lear <b>eat <d>diag <r>f <k>ick <s>tatus              %s\033[0m\n", tLocal);
  } // if not once
} // printServices
  */


// clear status
void clearStatus()
{
  printf("not yet implemented, press any key to continue\n");
  while (!comlib_getTermChar()) {usleep(200000);}
} // clearStatus


// calc flags for printing
uint32_t calcFlagPrint()
{
  int      i;
  ring_t   ring;
  uint32_t sid;
  uint32_t nLines;

  nLines = 0;
  
  for (i=0; i<NALLSID; i++) {
    flagPrintIdx[i] = 1;
    idx2RingSid(i, &ring, &sid);
    
    if (!flagPrintInactive) if (set_mode[i] == 0) flagPrintIdx[i] = 0;
    if (!flagPrintSis18)    if (ring == SIS18)    flagPrintIdx[i] = 0;
    if (!flagPrintEsr)      if (ring == ESR)      flagPrintIdx[i] = 0;
    if (!flagPrintYr)       if (ring == CRYRING)  flagPrintIdx[i] = 0;
    if (flagPrintIdx[i]) nLines++;
  } // for i

  return nLines;
} // calcFlagPrint;


// print data to screen
void printData(char *name)
{
  char     buff[100];
  time_t   time_date;
  int      i;
  uint32_t nLines;
  uint32_t minLines = 20;

  //for (i=0;i<60;i++) printf("\n");

  nLines = calcFlagPrint();

  time_date = time(0);
  strftime(buff,53,"%d-%b-%y %H:%M:%S",localtime(&time_date));
  term_clear();
  printf("\033[7m B2B Monitor %3s ------------------------------------------------------------------------------------ (units [ns] unless explicitly given) - v%8s\033[0m\n", name, b2b_version_text(B2B_MON_VERSION));
  //printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("%s\n", header);
  for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLine[i]);
  if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", empty);

  //printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("\033[7m exit <q> | toggle inactive <i>, SIS18 <0>, ESR <1>, YR <2>                                                                         %s\033[0m\n", buff);
  fflush(stdout);
  flagPrintNow = 0;
} // printServices
  
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

  printf("%s: starting client using prefix %s\n", program, prefix);
  
  for (i=0; i<NALLSID; i++) {
    sprintf(printLine[i], "not initialized");
    dicSubscribeServices(prefix, i);
  } // for i
  buildHeader();

  // wait a bit, then rebuild all indices
  usleep(1000000);
  for (i=0; i<NALLSID; i++) buildPrintLine(i);
  flagPrintNow = 1;
  
  while (!quit) {
    if (flagPrintNow) printData(name);
    if (!quit) {
      userInput = comlib_getTermChar();
      switch (userInput) {
        case 'c' :
          clearStatus();
          break;
        case 'i' :
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

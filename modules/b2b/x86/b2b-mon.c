/*******************************************************************************************
 *  b2b-mon.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-feb-2025
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
#define B2B_MON_VERSION 0x000807

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
#define  TINACTIVE   3600                 // [s]; if the previous data is more in the past than this value, the transfer data is considered inactive
#define  TOLD        3600 * 24            // [s]; if the previous data is more in the past than this value, the transfer data is considered out of date
#define  CLIGHT      299792458.0          // speed-of-light in vacuum

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
double   no_link_dbl   = NAN;
char     no_link_str[] = "NO_LINK";

setval_t   dicSetval[NALLSID];            // b2b set-values
getval_t   dicGetval[NALLSID];            // b2b get-values
diagval_t  dicDiagval[NALLSID];           // diagnostic (analyzed values)
diagstat_t dicDiagstat[NALLSID];          // additional status data
char       dicPName[NALLSID][DIMMAXSIZE]; // pattern names

uint32_t  dicSetvalId[NALLSID];
uint32_t  dicGetvalId[NALLSID];
uint32_t  dicDiagvalId[NALLSID];
uint32_t  dicDiagstatId[NALLSID];
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

// other
int      flagPrintIdx[NALLSID];                             // flag: print line with given index
int      flagPrintInactive;                                 // flag: print inactive SIDs too
int      flagPrintOther;                                    // flag: print alternative data set
int      flagPrintSis18;                                    // flag: print SIDs for SIS18
int      flagPrintEsr;                                      // flag: print SIDs for ESR
int      flagPrintYr;                                       // flag: print SIDs for CRYRINg
int      flagPrintNow;                                      // flag: print stuff to screen NOW
int      flagPrintNs;                                       // flag: in units of nanoseconds

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     headerK[SCREENWIDTH+1];                            // header line to be printed; header for kicker info
char     headerN[SCREENWIDTH+1];                            // header line to be printed; header for frequency info
char     emptyK[SCREENWIDTH+1];                             // empty line to be printed; kicker info
char     emptyN[SCREENWIDTH+1];                             // empty line to be printed; frequency info
char     printLineK[NALLSID][SCREENWIDTH+1];               // lines to be printed; line for kicker info
char     printLineN[NALLSID][SCREENWIDTH+1];                // lines to be printed; line for frequency info

double   one_ns_as = 1000000000.0;


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
  fprintf(stderr, "Version %s. Licensed under the LPL v3.\n", b2b_version_text(B2B_MON_VERSION));
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


// convert to appripriate units; nanoseconds or degree
double convertUnit(double value_ns, uint64_t TH1)
{
  double value;
  
  if (flagPrintNs) value = value_ns;
  else {
    if (TH1 == 0) value = NAN;
    else          value = 360.0 * value_ns / ((double)(TH1) / one_ns_as);
  } // else flagPrintNS

  return value;
} // convertUnit


void buildHeader()
{
  sprintf(headerK, "|        pattern name | t_last [UTC] | orign | sid|  mode   | kick set    trg offst start fltop | destn | sid| phase set get  | kick  set    trg offst start fltop Doffst  TOF |");
  sprintf(emptyK,  "|                     |              |       |    |         |                                   |       |    |                |                                                |");
  sprintf(headerN, "|        pattern name | t_last [UTC] | orign | sid|  mode   | h1gDDS ext set      get(stdev)diff[Hz]  v/c | h1gDDS inj set      get(stdev)diff[Hz]  v/c | prob ext inj [%%]     |");
  sprintf(emptyN,  "|                     |              |       |    |         |                                             |                                             |                      |");
  //        printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// build strings for printing
void buildPrintLine(uint32_t idx)
{
  char     origin[10];
  char     dest[32];
  char     mode[32];
  char     pattern[64];
  int      flagTCBS;
  int      flagOther;
  int      flagCphase;
  int      flagB2b;
  int      flagExtTrig;
  int      flagInjTrig;
  char     tCBS[64];
  char     extNue[64];
  char     b2b[64];
  char     injSid[32];
  char     extTrig[512];
  char     injTrig[521];
  char     nueMeasExt[164];
  char     nueMeasInj[164];
  char     setLevelExt[32];
  char     setLevelInj[32];

  char     tmp1[32];
  char     tmp2[32];
  char     tmp3[32];
  char     tmp4[32];
  char     tmp5[32];
  char     tmp6[32];
  double   dtmp1;
  double   nueDiff;

  uint32_t sid;
  ring_t   ringExt;
  double   cRingExt;
  double   cRingInj; 

  uint64_t actNsecs;
  time_t   actT;

  actNsecs  = comlib_getSysTime();
  actT      = (time_t)(actNsecs / 1000000000);
  dtmp1     = NAN;

  if (idx > NALLSID) return;

  idx2RingSid(idx, &ringExt, &sid);

  // extraction ring name
  switch (ringExt) {
    case SIS18   : sprintf(origin, "SIS18");  sprintf(tmp1, "ESR"); cRingExt = 216; cRingInj = 108; break;
    case ESR     : sprintf(origin, "ESR");    sprintf(tmp1, "YR");  cRingExt = 108; cRingInj =  54; break;
    case CRYRING : sprintf(origin, "YR");     sprintf(tmp1, " ");   cRingExt =  54; cRingInj = NAN; break;
    default      : sprintf(origin, TXTUNKWN); sprintf(tmp1, " ");   cRingExt = NAN; cRingInj = NAN; break;
  } // switch ringExt

  // pattern name
  if (strlen(dicPName[idx]) == 0) sprintf(pattern, "%s", TXTUNKWN);               // invalid
  else {
    if ((actT - set_secs[idx] /*- secsOffset*/) < (time_t)TINACTIVE)              
                                  sprintf(pattern, "%.20s",   dicPName[idx]);
    else                          sprintf(pattern, "?%.18s?", dicPName[idx]);     // very old, assignment might be wrong
    if (set_secs[idx] <= 1)       sprintf(pattern, "%s", TXTUNKWN);               // timestamp (probably) invalid
  }
  if (!flagSetValid[idx])         sprintf(pattern, "NO_LINK (DATA)");

  // destination
  switch (set_mode[idx]) {
    case B2B_MODE_OFF       : sprintf(mode, "   OFF   "); sprintf(dest, "---");      flagTCBS = 1; flagOther = 1; flagB2b = 0; flagCphase = 0; flagExtTrig = 0; flagInjTrig = 0; break;
    case B2B_MODE_BSE       : sprintf(mode, " test 1  "); sprintf(dest, "kicker");   flagTCBS = 1; flagOther = 1; flagB2b = 0; flagCphase = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case B2B_MODE_B2E       : sprintf(mode, "extr fast"); sprintf(dest, "target");   flagTCBS = 1; flagOther = 1; flagB2b = 0; flagCphase = 0; flagExtTrig = 1; flagInjTrig = 0; break;
    case B2B_MODE_B2C       : sprintf(mode, "b2b coast"); sprintf(dest, "%s", tmp1); flagTCBS = 1; flagOther = 1; flagB2b = 0; flagCphase = 0; flagExtTrig = 1; flagInjTrig = 1; break;
    case B2B_MODE_B2BFBEAT  : sprintf(mode, "b2b fbeat"); sprintf(dest, "%s", tmp1); flagTCBS = 1; flagOther = 1; flagB2b = 1; flagCphase = 1; flagExtTrig = 1; flagInjTrig = 1; break;
    case B2B_MODE_B2EPSHIFT : sprintf(mode, " test 2  "); sprintf(dest, "target");   flagTCBS = 1; flagOther = 1; flagB2b = 0; flagCphase = 1; flagExtTrig = 1; flagInjTrig = 0; break;
    case B2B_MODE_B2BPSHIFTE: sprintf(mode, "b2b psext"); sprintf(dest, "%s", tmp1); flagTCBS = 1; flagOther = 1; flagB2b = 1; flagCphase = 1; flagExtTrig = 1; flagInjTrig = 1; break;
    case B2B_MODE_B2BPSHIFTI: sprintf(mode, "b2b psinj"); sprintf(dest, "%s", tmp1); flagTCBS = 1; flagOther = 1; flagB2b = 1; flagCphase = 1; flagExtTrig = 1; flagInjTrig = 1; break;
    default:                  sprintf(mode, "   ---   "); sprintf(dest, TXTUNKWN);   flagTCBS = 0; flagOther = 0; flagB2b = 0; flagCphase = 0; flagExtTrig = 0; flagInjTrig = 0; break;
  } // switch set_mode

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

  if (flagOther) {
    // frequency data, extraction
    if (*(uint32_t *)&(dicDiagval[idx]) == no_link_32) sprintf(nueMeasExt, "NOLINK");
    else {
      nueDiff = dicDiagval[idx].ext_rfNueAct - set_extNue[idx];
      if ((dicGetval[idx].flagEvtErr >> tagPre) & 0x1)  sprintf(nueMeasExt, "ERROR: no RF signal detected");
      else {
        if (fabs(dicDiagval[idx].ext_ddsOffAct) > set_extT[idx] / 10) {  // this is hack to possibly detect wrong set-values
          sprintf(tmp1, "check DDS");
          sprintf(tmp2, "value");
          sprintf(tmp3, "n/a");
          sprintf(tmp4, "n/a");
        } // if fabs
        else {
          sprintf(tmp1, "%11.3f", dicDiagval[idx].ext_rfNueAct);
          sprintf(tmp2, "%5.3f",  dicDiagval[idx].ext_rfNueActErr);
          sprintf(tmp3, "%7.3f",  nueDiff);
          sprintf(tmp4, "%4.2f",  set_extNue[idx] * cRingExt / CLIGHT);
        } // else fabs
        sprintf(nueMeasExt, "%11.3f %11s(%5s) %7s %4s",  set_extNue[idx], tmp1, tmp2, tmp3, tmp4);
      } // else flagEvtErr
    } // else NOLINK

    // frequency data, injection
    if ((set_mode[idx] > B2B_MODE_B2E) && (set_mode[idx] != B2B_MODE_B2EPSHIFT)) {
      if (*(uint32_t *)&(dicDiagval[idx]) == no_link_32) sprintf(nueMeasInj, "NOLINK");
      else {
        nueDiff = dicDiagval[idx].inj_rfNueAct - set_injNue[idx];
        if ((dicGetval[idx].flagEvtErr >> tagPri) & 0x1)   sprintf(nueMeasInj, "ERROR: no RF signal detected");
        else {
          if (fabs(dicDiagval[idx].inj_ddsOffAct) > set_injT[idx] / 10) {  // this is hack to possibly detect wrong set-values
            sprintf(tmp1, "check DDS");
            sprintf(tmp2, "value");
            sprintf(tmp3, "n/a");
            sprintf(tmp4, "n/a");  
          } // if fabs
          else {
            sprintf(tmp1, "%11.3f", dicDiagval[idx].inj_rfNueAct);
            sprintf(tmp2, "%5.3f",  dicDiagval[idx].inj_rfNueActErr);
            sprintf(tmp3, "%7.3f",  nueDiff);
            sprintf(tmp4, "%4.2f",  set_injNue[idx] * cRingInj / CLIGHT);
          } // else fabs
          sprintf(nueMeasInj, "%11.3f %11s(%5s) %7s %4s", set_injNue[idx], tmp1, tmp2, tmp3, tmp4);
        } // else flagEvtErr
      } // else NOLINK
    } // if set_mode
    else                                            sprintf(nueMeasInj, "---");
    
    // comparator level for kicker probe signal
    sprintf(setLevelExt, "%6.2f", dicGetval[idx].ext_dKickProbLevel);
    sprintf(setLevelInj, "%6.2f", dicGetval[idx].inj_dKickProbLevel);
  } // if flagOther
  else {
    sprintf(extNue, "---");
    sprintf(nueMeasExt, "---");
    sprintf(nueMeasInj, "---");
    sprintf(setLevelExt, "---");
    sprintf(setLevelInj, "---");
  } // else flagOther
  
  if (flagCphase) {
    if ((dicGetval[idx].flagEvtErr >> tagKte) & 0x1) sprintf(b2b, "%s",  TXTERROR);
    else { if (flagB2b)                              sprintf(b2b, "%7.1f %7.1f", convertUnit(dicSetval[idx].cPhase, dicSetval[idx].ext_T), convertUnit(dicDiagval[idx].phaseOffAct, dicSetval[idx].ext_T));
           else                                      sprintf(b2b, "%7.1f %7.1f", convertUnit(dicSetval[idx].cPhase, dicSetval[idx].ext_T), convertUnit(dicDiagval[idx].ext_rfOffAct, dicSetval[idx].ext_T));
    } // else flagCphase
  } // if flagCphase
  else {
    if (flagInjTrig) sprintf(b2b, "coastg");
    else             sprintf(b2b, "---");
  } // else flagCphase
  
  if (flagExtTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> tagKte) & 0x1) sprintf(tmp1, "%7.1f", convertUnit(set_extCTrig[idx] - dicDiagval[idx].ext_ddsOffAct, dicSetval[idx].ext_T));
    else sprintf(tmp1, "%s", TXTERROR);
    // signal from output of kicker electronics
    if (isnan(dicGetval[idx].ext_dKickMon))     sprintf(tmp2, "%s", TXTERROR);
    else                                        sprintf(tmp2, "%5.0f", convertUnit(dicGetval[idx].ext_dKickMon, dicSetval[idx].ext_T));
    // signal from magent probes
    if (isnan(dicGetval[idx].ext_dKickProb))    sprintf(tmp3, "%s", TXTUNKWN);
    else                                        sprintf(tmp3, "%5.0f", convertUnit(dicGetval[idx].ext_dKickProb,dicSetval[idx].ext_T));
    if (isnan(dicGetval[idx].ext_dKickProbLen)) sprintf(tmp4, "%s", TXTUNKWN);
    else                                        sprintf(tmp4, "%5.0f", convertUnit(dicGetval[idx].ext_dKickProbLen,dicSetval[idx].ext_T));

    sprintf(extTrig, "%7.1f %7s %5s %5s %5s", convertUnit(set_extCTrig[idx], dicSetval[idx].ext_T), tmp1, tmp2, tmp3, tmp4);
  } // if flagExtTrig
  else sprintf(extTrig, "---");

  if (flagInjTrig) {
    // trigger event received
    if ((dicGetval[idx].flagEvtRec >> tagKti) & 0x1) {
      if (flagB2b) {
        if (isnan(dicDiagval[idx].inj_ddsOffAct)) sprintf(tmp1, "%s", TXTUNKWN);
        else                                      sprintf(tmp1, "%7.1f", convertUnit(set_injCTrig[idx] - dicDiagval[idx].inj_ddsOffAct, dicSetval[idx].inj_T)); //b2b : diff to DDS of injection ring
      } // if flagB2B
      else {
        if (isnan(dicDiagval[idx].ext_ddsOffAct)) sprintf(tmp1, "%s", TXTUNKWN);
        else                                      sprintf(tmp1, "%7.1f", convertUnit(set_injCTrig[idx] - dicDiagval[idx].ext_ddsOffAct, dicSetval[idx].inj_T)); //else: diff to DDS of extraction ring
      } // else flagB2B
    } // if flagEvtRec
    else sprintf(tmp1, "%s", TXTERROR);
    
    // signal from output of kicker electronics
    if (isnan(dicGetval[idx].inj_dKickMon))     sprintf(tmp2, "%s", TXTERROR);
    else                                        sprintf(tmp2, "%5.0f", convertUnit(dicGetval[idx].inj_dKickMon,dicSetval[idx].inj_T));
    // signal from magnet probes
    if (isnan(dicGetval[idx].inj_dKickProb))    sprintf(tmp3, "%s", TXTUNKWN);
    else                                        sprintf(tmp3, "%5.0f", convertUnit(dicGetval[idx].inj_dKickProb,dicSetval[idx].inj_T));
    if (isnan(dicGetval[idx].inj_dKickProbLen)) sprintf(tmp6, "%s", TXTUNKWN);
    else                                        sprintf(tmp6, "%5.0f", convertUnit(dicGetval[idx].inj_dKickProbLen,dicSetval[idx].inj_T));

      // difference to kicker electronics extraction
    if (isnan(dicGetval[idx].ext_dKickMon) || isnan(dicGetval[idx].inj_dKickMon))
      sprintf(tmp4, "%s", TXTUNKWN);
    else {
      dtmp1 = set_injCTrig[idx] - set_extCTrig[idx] + dicGetval[idx].inj_dKickMon - dicGetval[idx].ext_dKickMon;
      sprintf(tmp4, "%5.0f", convertUnit(dtmp1, dicSetval[idx].inj_T));
    } // else isnan
    // difference to magnet probe extraction
    if (isnan(dicGetval[idx].ext_dKickProb) || isnan(dicGetval[idx].inj_dKickProb) || isnan(dicGetval[idx].inj_dKickProbLen))
      sprintf(tmp5, "%s", TXTUNKWN);
    else {
      dtmp1 = set_injCTrig[idx] - set_extCTrig[idx] + dicGetval[idx].inj_dKickProb - dicGetval[idx].ext_dKickProb + dicGetval[idx].inj_dKickProbLen;
      sprintf(tmp5, "%5.0f", convertUnit(dtmp1, dicSetval[idx].inj_T));
    } // else isnan

    sprintf(injTrig, "%7.1f %7s %5s %5s %5s %5s %5s", convertUnit(set_injCTrig[idx], dicSetval[idx].inj_T), tmp1, tmp2, tmp3, tmp6, tmp4, tmp5);
  } // if flagInjTrig
  else sprintf(injTrig, "---");

  // SID of injection machine
  if (flagInjTrig) sprintf(injSid, "%2d", dicSetval[idx].inj_sid);
  else             sprintf(injSid, "%s", "--");

  sprintf(printLineK[idx], "|%20s | %12s |%6s | %2d |%9s| %33s |%6s | %2s |%15s | %46s |", pattern, tCBS, origin, sid, mode, extTrig, dest, injSid, b2b, injTrig);
  sprintf(printLineN[idx], "|%20s | %12s |%6s | %2d |%9s| %43s | %43s | %6s  %6s       |", pattern, tCBS, origin, sid, mode, nueMeasExt, nueMeasInj, setLevelExt, setLevelInj);
  //                printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  

} //buildPrintLine

// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  setval_t *tmp;
  uint32_t secs;
  uint32_t idx;

  uint64_t actNsecs;
  time_t   actT;

  /* printf("tag %lx\n", *tag); */
  if ((*tag < 0) || (*tag >= NALLSID)) return;
  idx = (uint32_t)(*tag);
  if (idx >= NALLSID) return;
  
  flagSetValid[idx] = (*size != sizeof(uint32_t));

  if (flagSetValid[idx]) {
    tmp = address;

    set_mode[idx]      = (*tmp).mode;
    set_cPhase[idx]    = (*tmp).cPhase;

    if ((*tmp).ext_T == -1) {
      set_extT[idx]    = 0.0;
      set_extNue[idx]  = 0.0;
      set_cPhaseD[idx] = 0.0;
    } // if extT
    else {
      set_extT[idx]    = (double)((*tmp).ext_T)/1000000000.0;
      set_extNue[idx]  = 1000000000.0 / set_extT[idx];
      set_cPhaseD[idx] = (double)((*tmp).cPhase) / (double)(set_extT[idx]) * 360.0; 
    } // else extT

    if ((*tmp).ext_h == -1) set_extH[idx] = 0;
    else                    set_extH[idx] = (*tmp).ext_h;
    
    set_extCTrig[idx]  = (*tmp).ext_cTrig;

    if ((*tmp).inj_T == -1) {
      set_injT[idx]    = 0.0;
      set_injNue[idx]  = 0.0;
    } // if injT
    else {
      set_injT[idx]    = (double)((*tmp).inj_T)/1000000000.0;
      set_injNue[idx]  = 1000000000.0 / set_injT[idx];
    } // else injT

    if ((*tmp).inj_h == -1) set_injH[idx] = 0;
    else                    set_injH[idx] = (*tmp).inj_h;
    
    set_injCTrig[idx]  = (*tmp).inj_cTrig;

    dic_get_timestamp(0, &secs, &(set_msecs[idx]));
    set_secs[idx]      = (time_t)(secs);

    // calibrate offset between THIS system time and time of set_values
    actNsecs           = comlib_getSysTime();
    actT               = (time_t)(actNsecs / 1000000000);
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
  dicSetvalId[idx]       = dic_info_service_stamped(name, MONITORED, 0, &(dicSetval[idx]), sizeof(setval_t), recSetvalue, (long)idx, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-raw_sid%02d_getval", prefix, ringName, sid);
  /* printf("name %s\n", name); */
  dicGetvalId[idx]       = dic_info_service_stamped(name, MONITORED, 0, &(dicGetval[idx]), sizeof(getval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_diag_sid%02d", prefix, ringName, sid);
  /* printf("name %s\n", name); */
  dicDiagvalId[idx]      = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagval[idx]), sizeof(diagval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s_%s-cal_stat_sid%02d", prefix, ringName,  sid);
  /* printf("name %s\n", name); */
  dicDiagstatId[idx]     = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagstat[idx]), sizeof(diagstat_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name,"%s_%s-pname_sid%02d", prefix, ringName, sid);
  /* printf("name %s\n", name);*/
  dicPNameId[idx]        = dic_info_service_stamped(name, MONITORED, 0, &(dicPName[idx]), DIMMAXSIZE, 0 , 0, &no_link_str, sizeof(no_link_str));  
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

  uint64_t actNsecs;
  time_t   actT;

  nLines   = 0;
  actNsecs = comlib_getSysTime();
  actT     = (time_t)(actNsecs / 1000000000);

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
  char     unitInfo[100];
  time_t   time_date;
  uint32_t nLines;
  uint32_t minLines = 20;
  int      i;

  nLines = calcFlagPrint();

  time_date = time(0);
  strftime(buff,53,"%d-%b-%y %H:%M:%S",localtime(&time_date));
  if (flagPrintNs) sprintf(unitInfo, "(units [ns] unless explicitly given)");
  else             sprintf(unitInfo, " (units [°] unless explicitly given)");
  sprintf(title,  "\033[7m B2B Monitor %3s -------------------------------------------------------------------------------------------------------------- %s - v%8s\033[0m", name, unitInfo, b2b_version_text(B2B_MON_VERSION));
  sprintf(footer, "\033[7m exit <q> | toggle data <d>, units <u> | toggle inactive <i>, SIS18 <0>, ESR <1>, YR <2> | help <h>                                                           %s\033[0m", buff);

  comlib_term_curpos(1,1);

  // printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("%s\n", title);
  if (flagPrintOther) {
    printf("%s\n", headerN);
    for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLineN[i]);
    if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", emptyN);
  } // if printOther
  else {
    printf("%s\n", headerK);
    for (i=0; i<NALLSID; i++ ) if (flagPrintIdx[i]) printf("%s\n", printLineK[i]);
    if (nLines < minLines) for (i=0; i<(minLines-nLines); i++) printf("%s\n", emptyK);      
  } // else printOther
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
  printf("https://wiki.gsi.de/TOS/BunchBucket/BunchBucketHowCLI#B2B_Monitor                 \n");
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
        case 'd' :
          // toggle printing of data (kicker data or other data)
          flagPrintOther = !flagPrintOther;
          flagPrintNow = 1;
          break;
        case 'u' :
          // toggle printing of units (nanoseconds or degree), 'secret' option
          flagPrintNs  = !flagPrintNs;
          for (i=0; i<NALLSID; i++) buildPrintLine(i);
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

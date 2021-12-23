/*******************************************************************************************
 *  b2b-analyzer.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 9-Nov-2021
 *
 * analyzes and publishes get values
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
#define B2B_ANALYZER_VERSION 0x000315

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
#include <dis.h>

// b2b
#include <common-lib.h>                    // COMMON
#include <b2blib.h>                        // API
#include <b2b.h>                           // FW

const char* program;

#define DDSSTEP     0.046566129            // min frequency step of gDDS

// dim stuff
#define    DIMCHARSIZE 32                  // standard size for char services
#define    DIMMAXSIZE  1024                // max size for service names

uint32_t   no_link_32    = 0xdeadbeef;
uint64_t   no_link_64    = 0xdeadbeefce420651;
char       no_link_str[] = "NO_LINK";

char       disVersion[DIMCHARSIZE];
char       disState[DIMCHARSIZE];
char       disHostname[DIMCHARSIZE];
uint64_t   disStatus;
uint32_t   disNTransfer;
setval_t   dicSetval[B2B_NSID];
getval_t   dicGetval[B2B_NSID];
diagval_t  disDiagval[B2B_NSID];
diagstat_t disDiagstat[B2B_NSID];


uint32_t   disVersionId      = 0;
uint32_t   disStateId        = 0;
uint32_t   disHostnameId     = 0;
uint32_t   disStatusId       = 0;
uint32_t   disNTransferId    = 0;
uint32_t   dicSetvalId[B2B_NSID];
uint32_t   dicGetvalId[B2B_NSID];
uint32_t   disDiagvalId[B2B_NSID];
uint32_t   disDiagstatId[B2B_NSID];
uint32_t   disClearDiagId;

int        flagSetValid[B2B_NSID];
int        flagGetValid[B2B_NSID];

// extraction DDS match
uint32_t  ext_ddsOffN[B2B_NSID];
int32_t   ext_ddsOffMin[B2B_NSID];
int32_t   ext_ddsOffMax[B2B_NSID];
double    ext_ddsOffAveOld[B2B_NSID];
double    ext_ddsOffStreamOld[B2B_NSID];

// extraction rf phase match
uint32_t  ext_rfOffN[B2B_NSID];
int32_t   ext_rfOffMin[B2B_NSID];
int32_t   ext_rfOffMax[B2B_NSID];
double    ext_rfOffAveOld[B2B_NSID];
double    ext_rfOffStreamOld[B2B_NSID];

// extraction DDS match
uint32_t  inj_ddsOffN[B2B_NSID];
int32_t   inj_ddsOffMin[B2B_NSID];
int32_t   inj_ddsOffMax[B2B_NSID];
double    inj_ddsOffAveOld[B2B_NSID];
double    inj_ddsOffStreamOld[B2B_NSID];

// extraction rf phase match
uint32_t  inj_rfOffN[B2B_NSID];
int32_t   inj_rfOffMin[B2B_NSID];
int32_t   inj_rfOffMax[B2B_NSID];
double    inj_rfOffAveOld[B2B_NSID];
double    inj_rfOffStreamOld[B2B_NSID];

// b2b rf phase difference
uint32_t  phaseOffN[B2B_NSID];
int32_t   phaseOffMin[B2B_NSID];
int32_t   phaseOffMax[B2B_NSID];
double    phaseOffAveOld[B2B_NSID];
double    phaseOffStreamOld[B2B_NSID];

// extraction rf frequency
uint32_t ext_rfNueN[B2B_NSID];                             
double   ext_rfNueAveOld[B2B_NSID];
double   ext_rfNueStreamOld[B2B_NSID];

// injection, rf frequency
uint32_t inj_rfNueN[B2B_NSID];                             
double   inj_rfNueAveOld[B2B_NSID];
double   inj_rfNueStreamOld[B2B_NSID];

// offset from deadline EKS to time when we are done
uint32_t  eks_doneOffN[B2B_NSID];
int32_t   eks_doneOffMin[B2B_NSID];
int32_t   eks_doneOffMax[B2B_NSID];
double    eks_doneOffAveOld[B2B_NSID];
double    eks_doneOffStreamOld[B2B_NSID];

// offset from deadline EKS to measured extraction phase
uint32_t  eks_preOffN[B2B_NSID];
int32_t   eks_preOffMin[B2B_NSID];
int32_t   eks_preOffMax[B2B_NSID];
double    eks_preOffAveOld[B2B_NSID];
double    eks_preOffStreamOld[B2B_NSID];

// offset from deadline EKS to measured injection phase
uint32_t  eks_priOffN[B2B_NSID];
int32_t   eks_priOffMin[B2B_NSID];
int32_t   eks_priOffMax[B2B_NSID];
double    eks_priOffAveOld[B2B_NSID];
double    eks_priOffStreamOld[B2B_NSID];

// offset from deadline EKS to KTE
uint32_t  eks_kteOffN[B2B_NSID];
int32_t   eks_kteOffMin[B2B_NSID];
int32_t   eks_kteOffMax[B2B_NSID];
double    eks_kteOffAveOld[B2B_NSID];
double    eks_kteOffStreamOld[B2B_NSID];

// offset from deadline EKS to KTI
uint32_t  eks_ktiOffN[B2B_NSID];
int32_t   eks_ktiOffMin[B2B_NSID];
int32_t   eks_ktiOffMax[B2B_NSID];
double    eks_ktiOffAveOld[B2B_NSID];
double    eks_ktiOffStreamOld[B2B_NSID];

// offset electronics monitor to KTE
uint32_t  ext_monRemN[B2B_NSID];
int32_t   ext_monRemMin[B2B_NSID];
int32_t   ext_monRemMax[B2B_NSID];
double    ext_monRemAveOld[B2B_NSID];
double    ext_monRemStreamOld[B2B_NSID];

// offset electronics monitor to KTI
uint32_t  inj_monRemN[B2B_NSID];
int32_t   inj_monRemMin[B2B_NSID];
int32_t   inj_monRemMax[B2B_NSID];
double    inj_monRemAveOld[B2B_NSID];
double    inj_monRemStreamOld[B2B_NSID];


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <server name>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to analyze and display get values of the B2B system\n");
  fprintf(stderr, "Example1: '%s pro_sis18'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(B2B_ANALYZER_VERSION));
} //help


// find nearest rising edge of h=1 signal
int32_t fixTS(int32_t  ts,                                  // timestamp [ns]
              int32_t  corr,                                // (trigger)correction [ns]
              uint64_t TH1                                  // h=1 period [as]
                )
{
  int64_t ts0;                                              // timestamp with correction removed [ns]
  int32_t dtMatch;
  int64_t ts0as;                                            // t0 [as]
  int64_t remainder;                     
  int64_t half;
  int     flagNeg; 

  if (TH1 == 0) return ts;                                  // can't fix
  ts0       = ts - corr;
  if (ts0 < 0) {ts0 = -ts0; flagNeg = 1;}                   // make this work for negative numbers too
  else         flagNeg = 0;

  ts0as     = ts0 * (int64_t)1000000000;
  half      = TH1 >> 1;
  remainder = ts0as % TH1;                                 
  if (remainder > half) ts0as = remainder - TH1;
  else                  ts0as = remainder;
  dtMatch   = (int32_t)(ts0as / 1000000000);
  
  if (flagNeg) dtMatch = -dtMatch;

  return dtMatch + corr;                 // we have to add back the correction (!)
} //fixTS


// clears diag data
void clearStats(uint32_t sid)
{
  disNTransfer             = 0;
  
  ext_ddsOffN[sid]         = 0;
  ext_ddsOffMax[sid]       = 0x80000000;       
  ext_ddsOffMin[sid]       = 0x7fffffff;         
  ext_ddsOffAveOld[sid]    = 0;   
  ext_ddsOffStreamOld[sid] = 0;

  inj_ddsOffN[sid]         = 0;
  inj_ddsOffMax[sid]       = 0x80000000;       
  inj_ddsOffMin[sid]       = 0x7fffffff;         
  inj_ddsOffAveOld[sid]    = 0;   
  inj_ddsOffStreamOld[sid] = 0;

  ext_rfOffN[sid]          = 0;
  ext_rfOffMax[sid]        = 0x80000000;       
  ext_rfOffMin[sid]        = 0x7fffffff;         
  ext_rfOffAveOld[sid]     = 0;   
  ext_rfOffStreamOld[sid]  = 0;

  inj_rfOffN[sid]          = 0;
  inj_rfOffMax[sid]        = 0x80000000;       
  inj_rfOffMin[sid]        = 0x7fffffff;         
  inj_rfOffAveOld[sid]     = 0;   
  inj_rfOffStreamOld[sid]  = 0;

  phaseOffN[sid]           = 0;
  phaseOffMax[sid]         = 0x80000000;       
  phaseOffMin[sid]         = 0x7fffffff;         
  phaseOffAveOld[sid]      = 0;   
  phaseOffStreamOld[sid]   = 0;

  ext_rfNueN[sid]          = 0;                             
  ext_rfNueAveOld[sid]     = 0;
  ext_rfNueStreamOld[sid]  = 0;

  inj_rfNueN[sid]          = 0;                             
  inj_rfNueAveOld[sid]     = 0;
  inj_rfNueStreamOld[sid]  = 0;

  eks_doneOffN[sid]        = 0;
  eks_doneOffMax[sid]      = 0x80000000;       
  eks_doneOffMin[sid]      = 0x7fffffff;         
  eks_doneOffAveOld[sid]   = 0;   
  eks_doneOffStreamOld[sid]= 0;

  eks_preOffN[sid]         = 0;
  eks_preOffMax[sid]       = 0x80000000;       
  eks_preOffMin[sid]       = 0x7fffffff;         
  eks_preOffAveOld[sid]    = 0;   
  eks_preOffStreamOld[sid] = 0;

  eks_priOffN[sid]         = 0;
  eks_priOffMax[sid]       = 0x80000000;       
  eks_priOffMin[sid]       = 0x7fffffff;         
  eks_priOffAveOld[sid]    = 0;   
  eks_priOffStreamOld[sid] = 0;

  eks_kteOffN[sid]         = 0;
  eks_kteOffMax[sid]       = 0x80000000;       
  eks_kteOffMin[sid]       = 0x7fffffff;         
  eks_kteOffAveOld[sid]    = 0;   
  eks_kteOffStreamOld[sid] = 0;

  eks_ktiOffN[sid]         = 0;
  eks_ktiOffMax[sid]       = 0x80000000;       
  eks_ktiOffMin[sid]       = 0x7fffffff;         
  eks_ktiOffAveOld[sid]    = 0;   
  eks_ktiOffStreamOld[sid] = 0;

  ext_monRemN[sid]         = 0;
  ext_monRemMax[sid]       = 0x80000000;       
  ext_monRemMin[sid]       = 0x7fffffff;         
  ext_monRemAveOld[sid]    = 0;   
  ext_monRemStreamOld[sid] = 0;

  inj_monRemN[sid]         = 0;
  inj_monRemMax[sid]       = 0x80000000;       
  inj_monRemMin[sid]       = 0x7fffffff;         
  inj_monRemAveOld[sid]    = 0;   
  inj_monRemStreamOld[sid] = 0;
} // clearDiagData


// calc basic statistic properties
void calcStats(double *meanNew,         // new mean value, please remember for later
               double meanOld,          // old mean value (required for 'running stats')
               double *streamNew,       // new stream value, please remember for later
               double streamOld,        // old stream value (required for 'running stats')
               double val,              // the new value :-)
               uint32_t n,              // number of values (required for 'running stats')
               double *var,             // standard variance
               double *sdev             // standard deviation
               )
{
  // see  ”The Art of ComputerProgramming, Volume 2: Seminumerical Algorithms“, Donald Knuth, or
  // http://www.netzmafia.de/skripten/hardware/Control/auswertung.pdf
  if (n > 1) {
    *meanNew   = meanOld + (val - meanOld) / (double)n;
    *streamNew = streamOld + (val - meanOld)*(val - *meanNew);
    *var       = *streamNew / (double)(n - 1);
    *sdev      = sqrt(*var);
  }
  else {
    *meanNew = val;
    *var     = 0;
  }
} // calcStats


// calculate DDS frequency from observed phase offset
int calcNue(double *nue,                // frequency value [Hz]
            double obsOffset,           // observed mean value of deviation from 'soll value' [ns]
            uint64_t TObs,              // observation interval [ns]
            uint64_t TH1                // H=1 gDDS period [as]
            )
{
  int64_t  nPeriod;                     // # of rf periods within T
  uint64_t half;
  int64_t  offsetAs;                    // offset [as]
  int64_t  TAs;                         // TObs [as]
  int64_t  TH1ObsAs;                    // observed TH1 [as]
  double   TH1ObsNs;                    // observed TH1 [ns]

  if ((TH1 != 0) && (TObs != 0)) {
    TAs       = TObs * 1000000000;
    half      = TH1 >> 1;
    nPeriod   = TAs / TH1;
    if ((TAs % TH1) > half) nPeriod++;              
    offsetAs  = (int64_t)(obsOffset * 1000000000.0);
    TH1ObsAs  = TH1 + offsetAs / (double)nPeriod;
    TH1ObsNs  = (double)TH1ObsAs / 1000000000.0;
    *nue      = 1000000000.0 / TH1ObsNs;
    return 0;
  } // avoid division by zero
  else return 0;
} // calcNue


// estimate 'real' DDS frequency from given frequency
double calcDdsNue(double nue)
{
  double nue1, nue2;
  double diff1, diff2; 

  nue1 =  b2b_flsa2fdds(nue);
  nue2 =  b2b_flsa2fdds(nue + DDSSTEP);

  diff1 = nue - nue1;
  diff2 = nue - nue2;

  if (fabs(diff1) < fabs(diff2)) return nue1;
  else                           return nue2;
} // calcDdsNue


// clear diagnostic information
void cmdClearDiag(long *tag, uint32_t *address, int *size)
{
  int32_t sid;
  
  if (*size != sizeof(uint32_t)) return;
  sid = (uint32_t)(*address);

  clearStats(sid);
} // cmdClearDiag


// receive set values
void recGetvalue(long *tag, diagval_t *address, int *size)
{
  uint32_t  sid;
  uint32_t  mode;
  int32_t   cor;
  int32_t   act;
  double    actD;
  uint32_t  n;
  double    sdev = 0;
  double    aveNew;
  double    streamNew = 0;
  double    dummy;
  double    tmp;
  uint64_t  tmp64;

  sid = *tag;
  if ((sid < 0) || (sid >= B2B_NSID)) return;
  if (!flagSetValid[sid])             return;
  flagGetValid[sid] = (*size != sizeof(uint32_t));

  mode = dicSetval[sid].mode;
  if (mode <  1) return;                                    // no further analysis
  if (mode >= 1) {

    disNTransfer++;
      
    // offset from deadline EKS to time when we are done
    act = dicGetval[sid].doneOff;
    n   = ++(eks_doneOffN[sid]);

    // statistics
    calcStats(&aveNew, eks_doneOffAveOld[sid], &streamNew, eks_doneOffStreamOld[sid], act, n , &dummy, &sdev);
    eks_doneOffAveOld[sid]          = aveNew;
    eks_doneOffStreamOld[sid]       = streamNew;
    if (act < eks_doneOffMin[sid]) eks_doneOffMin[sid] = act;
    if (act > eks_doneOffMax[sid]) eks_doneOffMax[sid] = act;

    // copy
    disDiagstat[sid].eks_doneOffAct  = act;
    disDiagstat[sid].eks_doneOffN    = n;
    disDiagstat[sid].eks_doneOffAve  = aveNew;
    disDiagstat[sid].eks_doneOffSdev = sdev;
    disDiagstat[sid].eks_doneOffMin  = eks_doneOffMin[sid];
    disDiagstat[sid].eks_doneOffMax  = eks_doneOffMax[sid];    

    // offset from deadline EKS to KTE
    act = dicGetval[sid].kteOff;
    n   = ++(eks_kteOffN[sid]);

    // statistics
    calcStats(&aveNew, eks_kteOffAveOld[sid], &streamNew, eks_kteOffStreamOld[sid], act, n , &dummy, &sdev);
    eks_kteOffAveOld[sid]          = aveNew;
    eks_kteOffStreamOld[sid]       = streamNew;
    if (act < eks_kteOffMin[sid]) eks_kteOffMin[sid] = act;
    if (act > eks_kteOffMax[sid]) eks_kteOffMax[sid] = act;

    // copy
    disDiagstat[sid].eks_kteOffAct  = act;
    disDiagstat[sid].eks_kteOffN    = n;
    disDiagstat[sid].eks_kteOffAve  = aveNew;
    disDiagstat[sid].eks_kteOffSdev = sdev;
    disDiagstat[sid].eks_kteOffMin  = eks_kteOffMin[sid];
    disDiagstat[sid].eks_kteOffMax  = eks_kteOffMax[sid];    

    // remainder of h=1 phase at electronics monitor
    if ((!((dicGetval[sid].flag_nok >> 1) & 0x1)) && (dicSetval[sid].ext_T != 0)) {
      tmp64 = dicGetval[sid].tCBS + dicGetval[sid].kteOff + dicGetval[sid].ext_dKickMon;   // TAI of dKickMon [ns]
      tmp64 = (tmp64 - dicGetval[sid].ext_phase) * 1000000000;                             // difference to measured phase [as]
      act   = (int32_t)((tmp64 % (dicSetval[sid].ext_T) / 1000000000));                    // remainder [ns]
      n   = ++(ext_monRemN[sid]);
      
      // statistics
      calcStats(&aveNew, ext_monRemAveOld[sid], &streamNew, ext_monRemStreamOld[sid], act, n , &dummy, &sdev);
      ext_monRemAveOld[sid]          = aveNew;
      ext_monRemStreamOld[sid]       = streamNew;
      if (act < ext_monRemMin[sid]) ext_monRemMin[sid] = act;
      if (act > ext_monRemMax[sid]) ext_monRemMax[sid] = act;
      
      // copy
      disDiagstat[sid].ext_monRemAct  = act;
      disDiagstat[sid].ext_monRemN    = n;
      disDiagstat[sid].ext_monRemAve  = aveNew;
      disDiagstat[sid].ext_monRemSdev = sdev;
      disDiagstat[sid].ext_monRemMin  = ext_monRemMin[sid];
      disDiagstat[sid].ext_monRemMax  = ext_monRemMax[sid];
    } // if dicGetval
  } // if mode >= 1
  
  if (mode >= 2) {                                          // analysis for extraction trigger and rf
    // match diagnostics; theoretical value is '0'
    cor = dicSetval[sid].ext_cTrig;
    act = fixTS(dicGetval[sid].ext_diagMatch, cor, dicSetval[sid].ext_T) - cor;
    n   = ++(ext_ddsOffN[sid]);

    // statistics
    calcStats(&aveNew, ext_ddsOffAveOld[sid], &streamNew, ext_ddsOffStreamOld[sid], act, n , &dummy, &sdev);
    //printf("ave %7.3f, sdev %7.3f\n", aveNew, sdev);
    ext_ddsOffAveOld[sid]          = aveNew;
    ext_ddsOffStreamOld[sid]       = streamNew;
    if (act < ext_ddsOffMin[sid]) ext_ddsOffMin[sid] = act;
    if (act > ext_ddsOffMax[sid]) ext_ddsOffMax[sid] = act;

    // copy
    disDiagval[sid].ext_ddsOffAct  = act;
    disDiagval[sid].ext_ddsOffN    = n;
    disDiagval[sid].ext_ddsOffAve  = aveNew;
    disDiagval[sid].ext_ddsOffSdev = sdev;
    disDiagval[sid].ext_ddsOffMin  = ext_ddsOffMin[sid];
    disDiagval[sid].ext_ddsOffMax  = ext_ddsOffMax[sid];

    // rf phase diagnostics; theoretical value is '0'
    cor = 0;
    act = fixTS(dicGetval[sid].ext_diagPhase, cor, dicSetval[sid].ext_T) - cor;
    n   = ++(ext_rfOffN[sid]);

    // statistics
    calcStats(&aveNew, ext_rfOffAveOld[sid], &streamNew, ext_rfOffStreamOld[sid], act, n , &dummy, &sdev);
    ext_rfOffAveOld[sid]           = aveNew;
    ext_rfOffStreamOld[sid]        = streamNew;
    if (act < ext_rfOffMin[sid]) ext_rfOffMin[sid] = act;
    if (act > ext_rfOffMax[sid]) ext_rfOffMax[sid] = act;

    // copy
    disDiagval[sid].ext_rfOffAct   = act;
    disDiagval[sid].ext_rfOffN     = n;
    disDiagval[sid].ext_rfOffAve   = aveNew;
    disDiagval[sid].ext_rfOffSdev  = sdev;
    disDiagval[sid].ext_rfOffMin   = ext_rfOffMin[sid];
    disDiagval[sid].ext_rfOffMax   = ext_rfOffMax[sid];

    // rf frequency diagnostics; theoretical value is '0'
    calcNue(&actD, disDiagval[sid].ext_rfOffAct, (double)B2B_TDIAGOBS, dicSetval[sid].ext_T);
    if (dicSetval[sid].ext_T != 0) tmp = 1000000000000000000.0 /  (double)(dicSetval[sid].ext_T);
    else                           tmp = 0.0;
    n   = ++(ext_rfNueN[sid]);

    // statistics
    calcStats(&aveNew, ext_rfNueAveOld[sid], &streamNew, ext_rfNueStreamOld[sid], actD, n ,&dummy , &sdev);
    ext_rfNueAveOld[sid]           = aveNew;
    ext_rfNueStreamOld[sid]        = streamNew;

    // copy
    disDiagval[sid].ext_rfNueN     = n;
    disDiagval[sid].ext_rfNueAve   = aveNew;
    disDiagval[sid].ext_rfNueSdev  = sdev;
    disDiagval[sid].ext_rfNueDiff  = aveNew - tmp ;
    disDiagval[sid].ext_rfNueEst   = calcDdsNue(aveNew);

    // offset from deadline EKS to measured extraction phase
    act = dicGetval[sid].preOff;
    n   = ++(eks_preOffN[sid]);

    // statistics
    calcStats(&aveNew, eks_preOffAveOld[sid], &streamNew, eks_preOffStreamOld[sid], act, n , &dummy, &sdev);
    //printf("ave %7.3f, sdev %7.3f\n", aveNew, sdev);
    eks_preOffAveOld[sid]          = aveNew;
    eks_preOffStreamOld[sid]       = streamNew;
    if (act < eks_preOffMin[sid]) eks_preOffMin[sid] = act;
    if (act > eks_preOffMax[sid]) eks_preOffMax[sid] = act;

    // copy
    disDiagstat[sid].eks_preOffAct  = act;
    disDiagstat[sid].eks_preOffN    = n;
    disDiagstat[sid].eks_preOffAve  = aveNew;
    disDiagstat[sid].eks_preOffSdev = sdev;
    disDiagstat[sid].eks_preOffMin  = eks_preOffMin[sid];
    disDiagstat[sid].eks_preOffMax  = eks_preOffMax[sid];    
  } // if mode >=2

  if (mode >= 3) {
    // offset from deadline EKS to KTI
    act = dicGetval[sid].ktiOff;
    n   = ++(eks_ktiOffN[sid]);

    // statistics
    calcStats(&aveNew, eks_ktiOffAveOld[sid], &streamNew, eks_ktiOffStreamOld[sid], act, n , &dummy, &sdev);
    eks_ktiOffAveOld[sid]          = aveNew;
    eks_ktiOffStreamOld[sid]       = streamNew;
    if (act < eks_ktiOffMin[sid]) eks_ktiOffMin[sid] = act;
    if (act > eks_ktiOffMax[sid]) eks_ktiOffMax[sid] = act;

    // copy
    disDiagstat[sid].eks_ktiOffAct  = act;
    disDiagstat[sid].eks_ktiOffN    = n;
    disDiagstat[sid].eks_ktiOffAve  = aveNew;
    disDiagstat[sid].eks_ktiOffSdev = sdev;
    disDiagstat[sid].eks_ktiOffMin  = eks_ktiOffMin[sid];
    disDiagstat[sid].eks_ktiOffMax  = eks_ktiOffMax[sid];

    // remainder phase to electronics monitor
    if ((!((dicGetval[sid].flag_nok >> 6) & 0x1)) && (dicSetval[sid].ext_T != 0)) {
      tmp64 = dicGetval[sid].tCBS + dicGetval[sid].ktiOff + dicGetval[sid].inj_dKickMon;   // TAI of dKickMon [ns]
      tmp64 = (tmp64 - dicGetval[sid].ext_phase) * 1000000000;                             // difference to measured phase [as]; NB: everyting relative to extraction phase
      act   = (int32_t)((tmp64 % (dicSetval[sid].ext_T) / 1000000000));                    // remainder [ns]
      n   = ++(inj_monRemN[sid]);
      
      // statistics
      calcStats(&aveNew, inj_monRemAveOld[sid], &streamNew, inj_monRemStreamOld[sid], act, n , &dummy, &sdev);
      inj_monRemAveOld[sid]          = aveNew;
      inj_monRemStreamOld[sid]       = streamNew;
      if (act < inj_monRemMin[sid]) inj_monRemMin[sid] = act;
      if (act > inj_monRemMax[sid]) inj_monRemMax[sid] = act;
      
      // copy
      disDiagstat[sid].inj_monRemAct  = act;
      disDiagstat[sid].inj_monRemN    = n;
      disDiagstat[sid].inj_monRemAve  = aveNew;
      disDiagstat[sid].inj_monRemSdev = sdev;
      disDiagstat[sid].inj_monRemMin  = inj_monRemMin[sid];
      disDiagstat[sid].inj_monRemMax  = inj_monRemMax[sid];
    } // if dicGetval
  } // if mode >= 3

  if (mode == 4) {
    // match diagnostics; theoretical value is '0'
    cor = dicSetval[sid].inj_cTrig - dicSetval[sid].cPhase;
    act = fixTS(dicGetval[sid].inj_diagMatch, cor, dicSetval[sid].inj_T) - cor;
    n   = ++(inj_ddsOffN[sid]);

    // statistics
    calcStats(&aveNew, inj_ddsOffAveOld[sid], &streamNew, inj_ddsOffStreamOld[sid], act, n , &dummy, &sdev);
    inj_ddsOffAveOld[sid]          = aveNew;
    inj_ddsOffStreamOld[sid]       = streamNew;
    if (act < inj_ddsOffMin[sid]) inj_ddsOffMin[sid] = act;
    if (act > inj_ddsOffMax[sid]) inj_ddsOffMax[sid] = act;

    // copy
    disDiagval[sid].inj_ddsOffAct  = act;
    disDiagval[sid].inj_ddsOffN    = n;
    disDiagval[sid].inj_ddsOffAve  = aveNew;
    disDiagval[sid].inj_ddsOffSdev = sdev;
    disDiagval[sid].inj_ddsOffMin  = inj_ddsOffMin[sid];
    disDiagval[sid].inj_ddsOffMax  = inj_ddsOffMax[sid];

    // rf phase diagnostics raw values; theoretical value is '0'
    cor = 0;
    act = fixTS(dicGetval[sid].inj_diagPhase, cor, dicSetval[sid].inj_T);
    n   = ++(inj_rfOffN[sid]);

    // statistics
    calcStats(&aveNew, inj_rfOffAveOld[sid], &streamNew, inj_rfOffStreamOld[sid], act, n , &dummy, &sdev);
    inj_rfOffAveOld[sid]           = aveNew;
    inj_rfOffStreamOld[sid]        = streamNew;
    if (act < inj_rfOffMin[sid]) inj_rfOffMin[sid] = act;
    if (act > inj_rfOffMax[sid]) inj_rfOffMax[sid] = act;

    // copy
    disDiagval[sid].inj_rfOffAct   = act;
    disDiagval[sid].inj_rfOffN     = n;
    disDiagval[sid].inj_rfOffAve   = aveNew;
    disDiagval[sid].inj_rfOffSdev  = sdev;
    disDiagval[sid].inj_rfOffMin   = inj_rfOffMin[sid];
    disDiagval[sid].inj_rfOffMax   = inj_rfOffMax[sid];

    // b2bphase diagnostics; theoretical value is 'phase correction'
    act = disDiagval[sid].inj_ddsOffAct - disDiagval[sid].ext_ddsOffAct + dicSetval[sid].cPhase;
    n   = ++(phaseOffN[sid]);

    // statistics
    calcStats(&aveNew, phaseOffAveOld[sid], &streamNew, phaseOffStreamOld[sid], act, n , &dummy, &sdev);
    phaseOffAveOld[sid]          = aveNew;
    phaseOffStreamOld[sid]       = streamNew;
    if (act < phaseOffMin[sid]) phaseOffMin[sid] = act;
    if (act > phaseOffMax[sid]) phaseOffMax[sid] = act;

    // copy
    disDiagval[sid].phaseOffAct  = act;
    disDiagval[sid].phaseOffN    = n;
    disDiagval[sid].phaseOffAve  = aveNew;
    disDiagval[sid].phaseOffSdev = sdev;
    disDiagval[sid].phaseOffMin  = phaseOffMin[sid];
    disDiagval[sid].phaseOffMax  = phaseOffMax[sid];

    // rf frequency diagnostics; theoretical value is '0'
    calcNue(&actD, disDiagval[sid].inj_rfOffAct, (double)B2B_TDIAGOBS, dicSetval[sid].inj_T);
    if (dicSetval[sid].inj_T != 0) tmp = 1000000000000000000.0 /  (double)(dicSetval[sid].inj_T);
    else                           tmp = 0.0;
    n   = ++(inj_rfNueN[sid]);

    // statistics
    calcStats(&aveNew, inj_rfNueAveOld[sid], &streamNew, inj_rfNueStreamOld[sid], actD, n ,&dummy , &sdev);
    inj_rfNueAveOld[sid]           = aveNew;
    inj_rfNueStreamOld[sid]        = streamNew;

    // copy
    disDiagval[sid].inj_rfNueN     = n;
    disDiagval[sid].inj_rfNueAve   = aveNew;
    disDiagval[sid].inj_rfNueSdev  = sdev;
    disDiagval[sid].inj_rfNueDiff  = aveNew - tmp ;
    disDiagval[sid].inj_rfNueEst   = calcDdsNue(aveNew);

    // offset from deadline EKS to measured injection phase
    act = dicGetval[sid].priOff;
    n   = ++(eks_priOffN[sid]);

    // statistics
    calcStats(&aveNew, eks_priOffAveOld[sid], &streamNew, eks_priOffStreamOld[sid], act, n , &dummy, &sdev);
    eks_priOffAveOld[sid]          = aveNew;
    eks_priOffStreamOld[sid]       = streamNew;
    if (act < eks_priOffMin[sid]) eks_priOffMin[sid] = act;
    if (act > eks_priOffMax[sid]) eks_priOffMax[sid] = act;

    // copy
    disDiagstat[sid].eks_priOffAct  = act;
    disDiagstat[sid].eks_priOffN    = n;
    disDiagstat[sid].eks_priOffAve  = aveNew;
    disDiagstat[sid].eks_priOffSdev = sdev;
    disDiagstat[sid].eks_priOffMin  = eks_priOffMin[sid];
    disDiagstat[sid].eks_priOffMax  = eks_priOffMax[sid];    
  } // mode == 4
  
  dis_update_service(disDiagvalId[sid]);
  dis_update_service(disDiagstatId[sid]);
  dis_update_service(disNTransferId);
} // recGetvalue
  
// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  uint32_t sid;

  sid = *tag;
  if ((sid < 0) || (sid >= B2B_NSID)) return;

  flagSetValid[sid] = (*size != sizeof(uint32_t));
} // recSetValue
  

// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_setval", prefix, i);
    //printf("name %s\n", name);
    dicSetvalId[i] = dic_info_service_stamped(name, MONITORED, 0, &(dicSetval[i]), sizeof(setval_t), recSetvalue, i, &no_link_32, sizeof(uint32_t));

    sprintf(name, "%s-raw_sid%02d_getval", prefix, i);
    //printf("name %s\n", name);
    dicGetvalId[i] = dic_info_service_stamped(name, MONITORED, 0, &(dicGetval[i]), sizeof(getval_t), recGetvalue, i, &no_link_32, sizeof(uint32_t));
  } // for i
} // dicSubscribeServices


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

    // 'generic' services
  sprintf(name, "%s-cal_version_fw", prefix);
  sprintf(disVersion, "%s",  b2b_version_text(B2B_ANALYZER_VERSION));
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s-cal_state", prefix);
  sprintf(disState, "%s", b2b_state_text(COMMON_STATE_OPREADY));
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s-cal_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", &disHostname, 32, 0 , 0);

  sprintf(name, "%s-cal_status", prefix);
  disStatus       = 0x1;   
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  sprintf(name, "%s-cal_ntransfer", prefix);
  disNTransferId  = dis_add_service(name, "I", &disNTransfer, sizeof(disNTransfer), 0 , 0);
  
  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s-cal_diag_sid%02d", prefix, i);
    disDiagvalId[i]  = dis_add_service(name, "I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:1;D:4;I:1;D:4", &(disDiagval[i]), sizeof(diagval_t), 0 , 0);

    sprintf(name, "%s-cal_stat_sid%02d", prefix, i);
    disDiagstatId[i] = dis_add_service(name, "I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2", &(disDiagstat[i]), sizeof(diagstat_t), 0 , 0);

    sprintf(name, "%s-cal_cmd_cleardiag", prefix);
    disClearDiagId   = dis_add_cmnd(name, "I:1", cmdClearDiag, 0);
  } // for i
} // disAddServices


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;

  int      getVersion;
  int      subscribe;


  char     prefix[DIMMAXSIZE];
  char     disName[DIMMAXSIZE];
  int      i;


  program       = argv[0];
  getVersion    = 0;
  subscribe     = 1;

  while ((opt = getopt(argc, argv, "s:eh")) != -1) {
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

  gethostname(disHostname, 32);
  if (optind< argc) sprintf(prefix, "b2b_%s", argv[optind]);
  else              sprintf(prefix, "b2b_%s", disHostname);
   sprintf(disName, "%s-cal", prefix);

  if (getVersion) printf("%s: version %s\n", program, b2b_version_text(B2B_ANALYZER_VERSION));

  if (subscribe) {
    printf("%s: starting analyzer using prefix %s\n", program, prefix);
    for (i=0; i<B2B_NSID; i++) clearStats(i);
    disAddServices(prefix);
    dis_start_serving(disName);
    dicSubscribeServices(prefix);
    
    while (1) sleep(1);
  } // if subscribe
  
  return exitCode;
}

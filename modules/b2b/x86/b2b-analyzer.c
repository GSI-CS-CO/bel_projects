/*******************************************************************************************
 *  b2b-analyzer.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 03-Jan-2025
 *
 * analyzes and publishes get values
 * 
 * units for time are always [ns]; if not, variable names have a suffix such as 'As' [as]
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
#define B2B_ANALYZER_VERSION 0x000803

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
#define FLTMAX      1.0E38;                
#define FLTMIN      -1.0E38;

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
double    ext_ddsOffMin[B2B_NSID];
double    ext_ddsOffMax[B2B_NSID];
double    ext_ddsOffAveOld[B2B_NSID];
double    ext_ddsOffStreamOld[B2B_NSID];

// extraction rf phase match
uint32_t  ext_rfOffN[B2B_NSID];
double    ext_rfOffMin[B2B_NSID];
double    ext_rfOffMax[B2B_NSID];
double    ext_rfOffAveOld[B2B_NSID];
double    ext_rfOffStreamOld[B2B_NSID];

// extraction DDS match
uint32_t  inj_ddsOffN[B2B_NSID];
double    inj_ddsOffMin[B2B_NSID];
double    inj_ddsOffMax[B2B_NSID];
double    inj_ddsOffAveOld[B2B_NSID];
double    inj_ddsOffStreamOld[B2B_NSID];

// extraction rf phase match
uint32_t  inj_rfOffN[B2B_NSID];
double    inj_rfOffMin[B2B_NSID];
double    inj_rfOffMax[B2B_NSID];
double    inj_rfOffAveOld[B2B_NSID];
double    inj_rfOffStreamOld[B2B_NSID];

// b2b rf phase difference
uint32_t  phaseOffN[B2B_NSID];
double    phaseOffMin[B2B_NSID];
double    phaseOffMax[B2B_NSID];
double    phaseOffAveOld[B2B_NSID];
double    phaseOffStreamOld[B2B_NSID];

// extraction rf frequency
uint32_t  ext_rfNueN[B2B_NSID];                             
double    ext_rfNueAveOld[B2B_NSID];
double    ext_rfNueStreamOld[B2B_NSID];

// injection, rf frequency
uint32_t  inj_rfNueN[B2B_NSID];                             
double    inj_rfNueAveOld[B2B_NSID];
double    inj_rfNueStreamOld[B2B_NSID];

// offset from deadline CBS to time when we are done
uint32_t  cbs_finOffN[B2B_NSID];
double    cbs_finOffMin[B2B_NSID];
double    cbs_finOffMax[B2B_NSID];
double    cbs_finOffAveOld[B2B_NSID];
double    cbs_finOffStreamOld[B2B_NSID];

// offset from deadline CBS to time when we received the PRE message
uint32_t  cbs_prrOffN[B2B_NSID];
double    cbs_prrOffMin[B2B_NSID];
double    cbs_prrOffMax[B2B_NSID];
double    cbs_prrOffAveOld[B2B_NSID];
double    cbs_prrOffStreamOld[B2B_NSID];

// offset from deadline CBS to measured extraction phase
uint32_t  cbs_preOffN[B2B_NSID];
double    cbs_preOffMin[B2B_NSID];
double    cbs_preOffMax[B2B_NSID];
double    cbs_preOffAveOld[B2B_NSID];
double    cbs_preOffStreamOld[B2B_NSID];

// offset from deadline CBS to measured injection phase
uint32_t  cbs_priOffN[B2B_NSID];
double    cbs_priOffMin[B2B_NSID];
double    cbs_priOffMax[B2B_NSID];
double    cbs_priOffAveOld[B2B_NSID];
double    cbs_priOffStreamOld[B2B_NSID];

// offset from deadline CBS to KTE
uint32_t  cbs_kteOffN[B2B_NSID];
double    cbs_kteOffMin[B2B_NSID];
double    cbs_kteOffMax[B2B_NSID];
double    cbs_kteOffAveOld[B2B_NSID];
double    cbs_kteOffStreamOld[B2B_NSID];

// offset from deadline CBS to KTI
uint32_t  cbs_ktiOffN[B2B_NSID];
double    cbs_ktiOffMin[B2B_NSID];
double    cbs_ktiOffMax[B2B_NSID];
double    cbs_ktiOffAveOld[B2B_NSID];
double    cbs_ktiOffStreamOld[B2B_NSID];

// offset electronics monitor to KTE
uint32_t  ext_monRemN[B2B_NSID];
double    ext_monRemMin[B2B_NSID];
double    ext_monRemMax[B2B_NSID];
double    ext_monRemAveOld[B2B_NSID];
double    ext_monRemStreamOld[B2B_NSID];

// offset electronics monitor to KTI
uint32_t  inj_monRemN[B2B_NSID];
double    inj_monRemMin[B2B_NSID];
double    inj_monRemMax[B2B_NSID];
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


// clears diag data
void clearStats(uint32_t sid)
{
  disNTransfer             = 0;
  
  ext_ddsOffN[sid]         = 0;
  ext_ddsOffMax[sid]       = FLTMIN;
  ext_ddsOffMin[sid]       = FLTMAX;
  ext_ddsOffAveOld[sid]    = 0;   
  ext_ddsOffStreamOld[sid] = 0;

  inj_ddsOffN[sid]         = 0;
  inj_ddsOffMax[sid]       = FLTMIN;
  inj_ddsOffMin[sid]       = FLTMAX;
  inj_ddsOffAveOld[sid]    = 0;   
  inj_ddsOffStreamOld[sid] = 0;

  ext_rfOffN[sid]          = 0;
  ext_rfOffMax[sid]        = FLTMIN;
  ext_rfOffMin[sid]        = FLTMAX;
  ext_rfOffAveOld[sid]     = 0;   
  ext_rfOffStreamOld[sid]  = 0;

  inj_rfOffN[sid]          = 0;
  inj_rfOffMax[sid]        = FLTMIN;
  inj_rfOffMin[sid]        = FLTMAX;
  inj_rfOffAveOld[sid]     = 0;   
  inj_rfOffStreamOld[sid]  = 0;

  phaseOffN[sid]           = 0;
  phaseOffMax[sid]         = FLTMIN;
  phaseOffMin[sid]         = FLTMAX;
  phaseOffAveOld[sid]      = 0;   
  phaseOffStreamOld[sid]   = 0;

  ext_rfNueN[sid]          = 0;                             
  ext_rfNueAveOld[sid]     = 0;
  ext_rfNueStreamOld[sid]  = 0;

  inj_rfNueN[sid]          = 0;                             
  inj_rfNueAveOld[sid]     = 0;
  inj_rfNueStreamOld[sid]  = 0;

  cbs_finOffN[sid]         = 0;
  cbs_finOffMax[sid]       = FLTMIN;
  cbs_finOffMin[sid]       = FLTMAX;
  cbs_finOffAveOld[sid]    = 0;   
  cbs_finOffStreamOld[sid] = 0;

  cbs_prrOffN[sid]         = 0;
  cbs_prrOffMax[sid]       = FLTMIN;
  cbs_prrOffMin[sid]       = FLTMAX;
  cbs_prrOffAveOld[sid]    = 0;   
  cbs_prrOffStreamOld[sid] = 0;

  cbs_preOffN[sid]         = 0;
  cbs_preOffMax[sid]       = FLTMIN;
  cbs_preOffMin[sid]       = FLTMAX;
  cbs_preOffAveOld[sid]    = 0;   
  cbs_preOffStreamOld[sid] = 0;

  cbs_priOffN[sid]         = 0;
  cbs_priOffMax[sid]       = FLTMIN;
  cbs_priOffMin[sid]       = FLTMAX;
  cbs_priOffAveOld[sid]    = 0;   
  cbs_priOffStreamOld[sid] = 0;

  cbs_kteOffN[sid]         = 0;
  cbs_kteOffMax[sid]       = FLTMIN;
  cbs_kteOffMin[sid]       = FLTMAX;
  cbs_kteOffAveOld[sid]    = 0;   
  cbs_kteOffStreamOld[sid] = 0;

  cbs_ktiOffN[sid]         = 0;
  cbs_ktiOffMax[sid]       = FLTMIN;
  cbs_ktiOffMin[sid]       = FLTMAX;
  cbs_ktiOffAveOld[sid]    = 0;   
  cbs_ktiOffStreamOld[sid] = 0;

  ext_monRemN[sid]         = 0;
  ext_monRemMax[sid]       = FLTMIN;
  ext_monRemMin[sid]       = FLTMAX;
  ext_monRemAveOld[sid]    = 0;   
  ext_monRemStreamOld[sid] = 0;

  inj_monRemN[sid]         = 0;
  inj_monRemMax[sid]       = FLTMIN;
  inj_monRemMin[sid]       = FLTMAX;
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


// calculate DDS frequency from observed phase offset and uncertainty for a single measurement
int calcNue(double  *nue,               // frequency value [Hz]
            double  *nueErr,            // estimated uncertainty [Hz]
            double   obsOffset,         // observed mean value of deviation from 'soll value'
            uint64_t TObs,              // observation interval
            uint64_t TH1As,             // H=1 gDDS period [as],
            double   sysmax             // max systematic error of phase measurement [ns]
            )
{
  int64_t  nPeriod;                     // # of rf periods within T
  uint64_t half;
  int64_t  offsetAs;                    // offset [as]
  int64_t  TAs;                         // TObs [as]
  int64_t  TH1ObsAs;                    // observed TH1 [as]
  double   TH1ObsNs;                    // observed TH1 [ns]
  double   diffStat;                    // statistical uncertainty of a phase measurement [ns]
  double   diffSys;                     // systematic uncertainty of a phase measuremetn [ns]
  double   diffErr;                     // total estimated uncertainty of a phase measurement [ns]
  double   relErr;                      // relative total uncertainty

  if ((TH1As != 0) && (TObs != 0)) {
    // frequency
    TAs       = TObs * 1000000000;
    half      = TH1As >> 1;
    nPeriod   = TAs / TH1As;
    if ((TAs % TH1As) > half) nPeriod++;              
    offsetAs  = (int64_t)(obsOffset * 1000000000.0);
    TH1ObsAs  = TH1As + offsetAs / (double)nPeriod;
    TH1ObsNs  = (double)TH1ObsAs / 1000000000.0;
    *nue      = 1000000000.0 / TH1ObsNs;

    // uncertainty for a single (!!!) meausurement
    // here it is assumed, that for a single (!) measurement
    // - systematic uncertainty equals 1/3 of 'sysmax'
    // - statistical uncertainty equals the assumed jitter
    // - statistical and systematic uncertainty can be added quadratically [1]
    // -  as a frequency measurement involves two phase measurements, the uncertainties
    //    resulting from two phase measurements are added quadratically, sqrt(2)
    // - finally the uncertainty needs to be scaled down with the number of rf-periods
    // [1] systematic error may not cancel out over many measurements; thus, when the
    // results of many measurements are analyzed, it should not be included into the
    // uncertainties the individual data but only added to the final result
    diffStat = (double)B2B_WR_JITTER / 1000000.0;
    diffSys  = sysmax / 3.0;
    diffErr  = sqrt(diffStat*diffStat + diffSys*diffSys);
    diffErr  = sqrt(2) * diffErr;
    diffErr  = diffErr / (double)nPeriod;

    relErr   = diffErr / ((double)TH1As / 1000000000.0);
    *nueErr  = fabs(*nue * relErr);

    // printf("stat %f, sys %f, tot %f, rel %f, nue %9.3f, nue_err %0.3f\n", diffStat, diffSys, diffErr, relErr, *nue, *nueErr);

    return 0;
  } // avoid division by zero
  else return 1;
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
  double    cor;
  double    act;
  double    actErr;
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

  disDiagstat[sid].cbs_preOffAct  = NAN;
  disDiagstat[sid].cbs_finOffAct  = NAN;
  disDiagstat[sid].cbs_prrOffAct  = NAN;
  disDiagstat[sid].cbs_kteOffAct  = NAN;
  disDiagstat[sid].cbs_ktiOffAct  = NAN;
  disDiagstat[sid].cbs_priOffAct  = NAN;
  disDiagstat[sid].inj_monRemAct  = NAN;
  disDiagstat[sid].ext_monRemAct  = NAN;
  disDiagval[sid].ext_ddsOffAct   = NAN;
  disDiagval[sid].ext_rfOffAct    = NAN;
  disDiagval[sid].ext_rfNueActErr = NAN;
  disDiagval[sid].ext_rfNueDiff   = NAN;
  disDiagval[sid].ext_rfNueAct    = NAN;
  disDiagval[sid].inj_ddsOffAct   = NAN;
  disDiagval[sid].inj_rfOffAct    = NAN;
  disDiagval[sid].inj_rfNueAct    = NAN;
  disDiagval[sid].inj_rfNueActErr = NAN;
  disDiagval[sid].inj_rfNueDiff   = NAN;
  disDiagval[sid].phaseOffAct     = NAN;
 
  if (mode >=  B2B_MODE_OFF) {

    disNTransfer++;

    // offset from deadline CBS to measured extraction phase
    if (!isnan(dicGetval[sid].preOff)) {
      act = dicGetval[sid].preOff;
      n   = ++(cbs_preOffN[sid]);

      // statistics
      calcStats(&aveNew, cbs_preOffAveOld[sid], &streamNew, cbs_preOffStreamOld[sid], act, n , &dummy, &sdev);
      //printf("ave %7.3f, sdev %7.3f\n", aveNew, sdev);
      cbs_preOffAveOld[sid]          = aveNew;
      cbs_preOffStreamOld[sid]       = streamNew;
      if (act < cbs_preOffMin[sid]) cbs_preOffMin[sid] = act;
      if (act > cbs_preOffMax[sid]) cbs_preOffMax[sid] = act;

      // copy
      disDiagstat[sid].cbs_preOffAct  = act;
      disDiagstat[sid].cbs_preOffN    = n;
      disDiagstat[sid].cbs_preOffAve  = aveNew;
      disDiagstat[sid].cbs_preOffSdev = sdev;
      disDiagstat[sid].cbs_preOffMin  = cbs_preOffMin[sid];
      disDiagstat[sid].cbs_preOffMax  = cbs_preOffMax[sid];
    } // if isnan

    // rf phase diagnostics; theoretical value is '0'
    if (!isnan(dicGetval[sid].ext_diagPhase)) {
      cor = 0.0;
      act = b2b_fixTS(dicGetval[sid].ext_diagPhase, cor, dicSetval[sid].ext_T) - cor;
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
    } // if isnan

    // rf frequency diagnostics; theoretical value is set value
    if (!isnan(disDiagval[sid].ext_rfOffAct)) {
      if (isnan(dicGetval[sid].ext_phaseShift)) tmp = disDiagval[sid].ext_rfOffAct;
      else                                      tmp = disDiagval[sid].ext_rfOffAct - dicGetval[sid].ext_phaseShift;
      calcNue(&act, &actErr, tmp, (double)B2B_TDIAGOBS, dicSetval[sid].ext_T, dicGetval[sid].ext_phaseSysmaxErr);
      if (dicSetval[sid].ext_T != 0) tmp = 1000000000000000000.0 /  (double)(dicSetval[sid].ext_T);
      else                           tmp = 0.0;
      n   = ++(ext_rfNueN[sid]);
      
      // statistics
      calcStats(&aveNew, ext_rfNueAveOld[sid], &streamNew, ext_rfNueStreamOld[sid], act, n ,&dummy , &sdev);
      ext_rfNueAveOld[sid]           = aveNew;
      ext_rfNueStreamOld[sid]        = streamNew;
      
      // copy
      disDiagval[sid].ext_rfNueAct   = act;
      disDiagval[sid].ext_rfNueActErr= actErr;    
      disDiagval[sid].ext_rfNueN     = n;
      disDiagval[sid].ext_rfNueAve   = aveNew;
      disDiagval[sid].ext_rfNueSdev  = sdev;
      disDiagval[sid].ext_rfNueDiff  = aveNew - tmp ;
      disDiagval[sid].ext_rfNueEst   = calcDdsNue(aveNew);
    } // if isnan

  } // if mode B2B_MODE_OFF
  
  if (mode >= B2B_MODE_BSE) {
  
    // offset from deadline CBS to time when CBU is done
    if (!isnan(dicGetval[sid].finOff)) {
      act = dicGetval[sid].finOff;
      n   = ++(cbs_finOffN[sid]);
      
      // statistics
      calcStats(&aveNew, cbs_finOffAveOld[sid], &streamNew, cbs_finOffStreamOld[sid], act, n , &dummy, &sdev);
      cbs_finOffAveOld[sid]          = aveNew;
      cbs_finOffStreamOld[sid]       = streamNew;
      if (act < cbs_finOffMin[sid]) cbs_finOffMin[sid] = act;
      if (act > cbs_finOffMax[sid]) cbs_finOffMax[sid] = act;
      
      // copy
      disDiagstat[sid].cbs_finOffAct  = act;
      disDiagstat[sid].cbs_finOffN    = n;
      disDiagstat[sid].cbs_finOffAve  = aveNew;
      disDiagstat[sid].cbs_finOffSdev = sdev;
      disDiagstat[sid].cbs_finOffMin  = cbs_finOffMin[sid];
      disDiagstat[sid].cbs_finOffMax  = cbs_finOffMax[sid];
    } // if isnan


    // offset from deadline CBS to time when the PRE messages is received by CBU
    if (!isnan(dicGetval[sid].prrOff)) {
      act = (double)dicGetval[sid].prrOff;
      n   = ++(cbs_prrOffN[sid]);

      // statistics
      calcStats(&aveNew, cbs_prrOffAveOld[sid], &streamNew, cbs_prrOffStreamOld[sid], act, n , &dummy, &sdev);
      cbs_prrOffAveOld[sid]          = aveNew;
      cbs_prrOffStreamOld[sid]       = streamNew;
      if (act < cbs_prrOffMin[sid]) cbs_prrOffMin[sid] = act;
      if (act > cbs_prrOffMax[sid]) cbs_prrOffMax[sid] = act;

      // copy
      disDiagstat[sid].cbs_prrOffAct  = act;
      disDiagstat[sid].cbs_prrOffN    = n;
      disDiagstat[sid].cbs_prrOffAve  = aveNew;
      disDiagstat[sid].cbs_prrOffSdev = sdev;
      disDiagstat[sid].cbs_prrOffMin  = cbs_prrOffMin[sid];
      disDiagstat[sid].cbs_prrOffMax  = cbs_prrOffMax[sid];
    } // if isnan

    // offset from deadline CBS to KTE
    if (!isnan(dicGetval[sid].kteOff)) {
      act = (double)dicGetval[sid].kteOff;
      n   = ++(cbs_kteOffN[sid]);
      
      // statistics
      calcStats(&aveNew, cbs_kteOffAveOld[sid], &streamNew, cbs_kteOffStreamOld[sid], act, n , &dummy, &sdev);
      cbs_kteOffAveOld[sid]          = aveNew;
      cbs_kteOffStreamOld[sid]       = streamNew;
      if (act < cbs_kteOffMin[sid]) cbs_kteOffMin[sid] = act;
      if (act > cbs_kteOffMax[sid]) cbs_kteOffMax[sid] = act;
      
      // copy
      disDiagstat[sid].cbs_kteOffAct  = act;
      disDiagstat[sid].cbs_kteOffN    = n;
      disDiagstat[sid].cbs_kteOffAve  = aveNew;
      disDiagstat[sid].cbs_kteOffSdev = sdev;
      disDiagstat[sid].cbs_kteOffMin  = cbs_kteOffMin[sid];
      disDiagstat[sid].cbs_kteOffMax  = cbs_kteOffMax[sid];
    } // if isnan

    // remainder of h=1 phase at electronics monitor
    if ((dicSetval[sid].ext_T != -1) && !isnan(dicGetval[sid].kteOff) && !isnan(dicGetval[sid].ext_dKickMon) && (dicGetval[sid].ext_phase != -1)) {
      tmp64 = dicGetval[sid].tCBS + dicGetval[sid].kteOff + dicGetval[sid].ext_dKickMon;   // TAI of dKickMon [ns]
      tmp64 = (tmp64 - dicGetval[sid].ext_phase) * 1000000000;                             // difference to measured phase [as]
      act   = (double)((tmp64 % (dicSetval[sid].ext_T) / 1000000000));                     // remainder [ns]
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
    
  } // if mode B2B_MODE_BSE
  
  if (mode >=  B2B_MODE_B2E) {                                          // analysis for extraction trigger and rf
    // match diagnostics; theoretical value is '0'
    if (!isnan(dicGetval[sid].ext_diagMatch)) {
      cor = (double)dicSetval[sid].ext_cTrig;
      act = b2b_fixTS(dicGetval[sid].ext_diagMatch, cor, dicSetval[sid].ext_T) - cor;
      act = -act; // diagMatch is offset of trigger event to phase; but we want offset of phase to trigger
      // printf("EXT match %8.3f, cor %8.3f, act %8.3f\n", dicGetval[sid].ext_diagMatch, cor, act);
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
    } // if isnan

  } // if mode B2B_MODE_B2E

  if ((mode >= B2B_MODE_B2C) && (mode != B2B_MODE_B2EPSHIFT)){
    // offset from deadline CBS to KTI
    if (!isnan(dicGetval[sid].ktiOff)) {
      act = dicGetval[sid].ktiOff;
      n   = ++(cbs_ktiOffN[sid]);
      
      // statistics
      calcStats(&aveNew, cbs_ktiOffAveOld[sid], &streamNew, cbs_ktiOffStreamOld[sid], act, n , &dummy, &sdev);
      cbs_ktiOffAveOld[sid]          = aveNew;
      cbs_ktiOffStreamOld[sid]       = streamNew;
      if (act < cbs_ktiOffMin[sid]) cbs_ktiOffMin[sid] = act;
      if (act > cbs_ktiOffMax[sid]) cbs_ktiOffMax[sid] = act;
      
      // copy
      disDiagstat[sid].cbs_ktiOffAct  = act;
      disDiagstat[sid].cbs_ktiOffN    = n;
      disDiagstat[sid].cbs_ktiOffAve  = aveNew;
      disDiagstat[sid].cbs_ktiOffSdev = sdev;
      disDiagstat[sid].cbs_ktiOffMin  = cbs_ktiOffMin[sid];
      disDiagstat[sid].cbs_ktiOffMax  = cbs_ktiOffMax[sid];
    } // if isnan

    // remainder phase to electronics monitor
    if ((dicSetval[sid].ext_T != -1) && !isnan(dicGetval[sid].ktiOff) && !isnan(dicGetval[sid].inj_dKickMon) && (dicGetval[sid].ext_phase != -1)) {
      tmp64 = dicGetval[sid].tCBS + dicGetval[sid].ktiOff + dicGetval[sid].inj_dKickMon;   // TAI of dKickMon [ns]
      tmp64 = (tmp64 - dicGetval[sid].ext_phase) % 1000000000;                             // difference to measured phase [as]; NB: everyting relative to extraction phase
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

    // rf phase diagnostics raw values; theoretical value is '0'
    if (!isnan(dicGetval[sid].inj_diagPhase)) {
      cor = 0.0;
      act = b2b_fixTS(dicGetval[sid].inj_diagPhase, cor, dicSetval[sid].inj_T);
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
    } // if isnan

    // rf frequency diagnostics; theoretical value is '0'
    if (!isnan(disDiagval[sid].inj_rfOffAct) && (dicSetval[sid].inj_T != -1)) {
      if (isnan(dicGetval[sid].inj_phaseShift)) tmp = disDiagval[sid].inj_rfOffAct;
      else                                      tmp = disDiagval[sid].inj_rfOffAct - dicGetval[sid].inj_phaseShift;
      calcNue(&act, &actErr, tmp, (double)B2B_TDIAGOBS, dicSetval[sid].inj_T, dicGetval[sid].inj_phaseSysmaxErr);
      tmp = 1000000000000000000.0 /  (double)(dicSetval[sid].inj_T);
      n   = ++(inj_rfNueN[sid]);
      
      // statistics
      calcStats(&aveNew, inj_rfNueAveOld[sid], &streamNew, inj_rfNueStreamOld[sid], act, n ,&dummy , &sdev);
      inj_rfNueAveOld[sid]           = aveNew;
      inj_rfNueStreamOld[sid]        = streamNew;
      
      // copy
      disDiagval[sid].inj_rfNueAct   = act;
      disDiagval[sid].inj_rfNueActErr= actErr;
      disDiagval[sid].inj_rfNueN     = n;
      disDiagval[sid].inj_rfNueAve   = aveNew;
      disDiagval[sid].inj_rfNueSdev  = sdev;
      disDiagval[sid].inj_rfNueDiff  = aveNew - tmp ;
      disDiagval[sid].inj_rfNueEst   = calcDdsNue(aveNew);
    } // if isnan

    // offset from deadline CBS to measured injection phase
    if (!isnan(dicGetval[sid].priOff)) {
      act = dicGetval[sid].priOff;
      n   = ++(cbs_priOffN[sid]);

      // statistics
      calcStats(&aveNew, cbs_priOffAveOld[sid], &streamNew, cbs_priOffStreamOld[sid], act, n , &dummy, &sdev);
      cbs_priOffAveOld[sid]          = aveNew;
      cbs_priOffStreamOld[sid]       = streamNew;
      if (act < cbs_priOffMin[sid]) cbs_priOffMin[sid] = act;
      if (act > cbs_priOffMax[sid]) cbs_priOffMax[sid] = act;
      
      // copy
      disDiagstat[sid].cbs_priOffAct  = act;
      disDiagstat[sid].cbs_priOffN    = n;
      disDiagstat[sid].cbs_priOffAve  = aveNew;
      disDiagstat[sid].cbs_priOffSdev = sdev;
      disDiagstat[sid].cbs_priOffMin  = cbs_priOffMin[sid];
      disDiagstat[sid].cbs_priOffMax  = cbs_priOffMax[sid];
    } // if isnan
    
  } // if mode B2B_MODE_B2C

  if ((mode == B2B_MODE_B2BFBEAT) || (mode == B2B_MODE_B2BPSHIFTE)) {

    // match diagnostics; theoretical value is '0'
    if (!isnan(dicGetval[sid].inj_diagMatch) && !isnan(dicSetval[sid].cPhase) && !isnan(dicSetval[sid].inj_cTrig) && (dicSetval[sid].inj_T != -1)) { 
      cor = dicSetval[sid].inj_cTrig - dicSetval[sid].cPhase;
      act = b2b_fixTS(dicGetval[sid].inj_diagMatch, cor, dicSetval[sid].inj_T) - cor;
      act = -act; // diagMatch is offset of trigger event to phase; but we want offset of phase to trigger
      // printf("INJ match %8.3f, cor %8.3f, act %8.3f\n", dicGetval[sid].inj_diagMatch, cor, act);
      
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
    } // if isnan

    // b2bphase diagnostics; theoretical value is 'phase correction'
    if (!isnan(disDiagval[sid].inj_ddsOffAct) && !isnan(disDiagval[sid].ext_ddsOffAct) && !isnan(dicSetval[sid].cPhase)) {
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
    } // if isnan

  } // if mode B2B_MODE_B2B
  
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
    disDiagvalId[i]  = dis_add_service(name, "D:1;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:6;I:1;D:6;I:1;D:4", &(disDiagval[i]), sizeof(diagval_t), 0 , 0);

    sprintf(name, "%s-cal_stat_sid%02d", prefix, i);
    disDiagstatId[i] = dis_add_service(name, "D:1;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:5;I:1;D:4", &(disDiagstat[i]), sizeof(diagstat_t), 0 , 0);

    sprintf(name, "%s-cal_cmd_cleardiag", prefix);
    disClearDiagId   = dis_add_cmnd(name, "I:1", cmdClearDiag, 0);
  } // for i
} // disAddServices


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;

  int      getVersion;
  int      subscribe;


  char     prefix[132];
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

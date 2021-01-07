// @file saft-b2b-mon.cpp
// @brief Command-line interface for saftlib. This is a simple monitoring tool for the bunch to bucket system.
// @author Dietrich Beck  <d.beck@gsi.de>
//
// Copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
// Have a chat with saftlib and B2B
//
//*****************************************************************************
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//*****************************************************************************
// version: 2021-Jan-06

#define __STDC_FORMAT_MACROS
#define __STDC_CONSTANT_MACROS

#include <iostream>
#include <iomanip>

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>

#include <b2blib.h>

#include "SAFTd.h"
#include "TimingReceiver.h"
#include "SoftwareActionSink.h"
#include "SoftwareCondition.h"
#include "iDevice.h"
#include "iOwned.h"
#include "CommonFunctions.h"

using namespace std;

static const char* program;
static uint32_t pmode = PMODE_NONE;     // how data are printed (hex, dec, verbosity)
bool UTC            = false;            // show UTC instead of TAI
bool UTCleap        = false;
uint32_t reqSid;


// GID
#define GGSI        0x3a                // B2B prefix existing facility
#define SIS18       0x12c               // SIS18
#define ESR         0x154               // ESR

// EVTNO
#define KICKSTART1  0x031 
#define PMEXT 	    0x800
#define PMINJ 	    0x801
#define PREXT 	    0x802
#define PRINJ 	    0x803
#define TRIGGEREXT  0x804
#define TRIGGERINJ  0x805
#define DIAGKICKEXT 0x806
#define DIAGKICKINJ 0x807
#define DIAGEXT     0x808
#define DIAGINJ     0x809
#define DIAGMATCH   0x80c

#define FID         0x1

#define TUPDATE     50000000            // delay for updating screen after EVT_KICK_START [ns]
#define TDIAGOBS    20000000            // observation time for diagnostic [ns]
#define DDSSTEP     0.046566129         // min frequency step of gDDS

#define TXTERROR    "ERROR"
#define TXTUNKWN    "UNKWN"
#define TXTNA       "  N/A"
#define TXTOK       "   OK"

saftlib::Time  nextUpdate;              // time for next update [ns]

uint32_t gid;                           // GID used for transfer
uint32_t sid;                           // SID user for transfer
uint32_t mode;                          // mode for transfer
uint64_t TH1Ext;                        // h=1 period of extraction machine
uint32_t nHExt;                         // harmonic number of extraction machine 0..15
uint64_t TH1Inj;                        // h=1 period of injection machine
uint32_t nHInj;                         // harmonic number of injection machine 0..15
uint64_t TBeat;                         // beating frquency
int32_t  cPhase;                        // correction for phase matching [ns]
int32_t  cTrigExt;                      // correction for extraction trigger
int32_t  cTrigInj;                      // correction for injection trigger
uint64_t tH1Ext;                        // h=1 phase  [ns] of extraction machine
uint64_t tH1Inj;                        // h=1 phase  [ns] of injection machine
double   fH1Ext;                        // DDS frequency extraction
double   fH1Inj;                        // DDS frequency injection
uint64_t tKickStart;                    // time of EVT_KICK_START
uint64_t tTrigExt;                      // time of kicker trigger extraction
uint64_t tTrigInj;                      // time of kicker trigger injection
int32_t  kickCorrExt;                   // kicker correction offset extraction
int32_t  kickCorrInj;                   // kicker correction offset injection
int32_t  kickElecDelExt;                // delay kicker electronics extraction
int32_t  kickElecDelInj;                // delay kicker electronics injection
int32_t  kickProbDelExt;                // delay kicker magnet probe extraction
int32_t  kickProbDelInj;                // delay kicker magnet probe injection
int32_t  diagPhaseExt;                  // phase diagnostics extraction
int32_t  diagPhaseInj;                  // phase diagnostics injection
int32_t  diagMatchExt;                  // match diagnostics extraction
int32_t  diagMatchInj;                  // match diagnostics injection
int      flagTransStart;                // flag transfer started
int      flagErrPmExt;                  // error flag phase measurement extraction
int      flagErrPmInj;                  // error flag phase measurement injection
int      flagErrCbu;                    // error flag CBU
int      flagErrKickExt;                // error flag kicker extraction
int      flagErrKickInj;                // error flag kicker injection


// this will be called, in case we are snooping for events - dummy routine
static void on_action(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags)
{
  std::cout << "tDeadline: " << tr_formatDate(deadline, pmode);
  std::cout << tr_formatActionEvent(id, pmode);
  std::cout << tr_formatActionParam(param, 0xFFFFFFFF, pmode);
  std::cout << tr_formatActionFlags(flags, executed - deadline, pmode);
  std::cout << std::endl;
} // on_action


void calcStats(double *meanNew, double meanOld, double *streamNew, double streamOld, double val, uint32_t n, double *var, double *sdev)
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


void calcNue(double *nue, double obsOffset, uint64_t TObs, uint64_t TH1)
{
  // nue frequency [Hz]
  // obsOffset mean value of deviation [ns]
  // TObs observation time [ns]
  // H=1 gDDS period [as]
  int64_t  nPeriod;      // # of rf periods within T
  uint64_t half;
  int64_t  offsetAs;     // offset [as]
  int64_t  TAs;          // TObs [as]
  int64_t  TH1ObsAs;     // observed TH1 [as]
  double   TH1ObsNs;     // observed TH1 [ns]

  if ((TH1 != 0) && (TObs != 0)) {
    TAs       = TObs * 1000000000;
    half      = TH1 >> 1;
    nPeriod   = TAs / TH1;
    if ((TAs % TH1) > half) nPeriod++;              
    offsetAs  = (int64_t)(obsOffset * 1000000000.0);
    TH1ObsAs  = TH1 + offsetAs / (double)nPeriod;
    TH1ObsNs  = (double)TH1ObsAs / 1000000000.0;
    *nue      = 1000000000.0 / TH1ObsNs;
  } // avoid division by zero
  else *nue  = 0.0;
} // calcNue


double ns2Degree(double phase, uint64_t T)
{
  // phase [ns]
  // T     [as]
  // returns degree

  double period;    // [ns]
  double degree;

  period = (double)T / 1000000000.0;

  degree = phase / period * 360.0;

  return degree;
}


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


int32_t getAlignedTS(int32_t ts,     // timestamp [ns]
                    int32_t corr,   // (trigger)correction [ns]
                    uint64_t TH1    // H=1 period [as]
                    )
{
  int32_t TH1Ns;                    // H=1 period [ns]
  int32_t half;                       
  int32_t ts0;                      // timestamp with correction removed
  int32_t min;
  int32_t dtTmp;
  int32_t dtMatch;
    
  ts0   = ts - corr;
  TH1Ns = TH1 / 1000000000;
  min   = 0x7fffffff;

  for (dtTmp = ts0 - 5 * TH1Ns; dtTmp < ts0 + 5 * TH1Ns; dtTmp += TH1Ns) {
    if (fabs(dtTmp) < min) {
      min     = fabs(dtTmp);
      dtMatch = dtTmp;
    } // if fabs
  } // for dtTmp

  return dtMatch + corr;                 // we have to add back the correction (!)
} //getAlignedTS


// clear all data
void clearAllData()
{
  gid            = 0x0;    
  sid            = 0x0;    
  mode           = 0x0;   
  TH1Ext         = 0x0;
  nHExt          = 0x0;  
  TH1Inj         = 0x0; 
  nHInj          = 0x0;  
  TBeat          = 0x0;  
  cPhase         = 0x7fffffff;
  cTrigExt       = 0x7fffffff;
  cTrigInj       = 0x7fffffff;
  tH1Ext         = 0x0; 
  tH1Inj         = 0x0;
  kickElecDelExt = 0x7fffffff;
  kickProbDelExt = 0x7fffffff;
  kickElecDelInj = 0x7fffffff;
  kickProbDelInj = 0x7fffffff;
  diagPhaseExt   = 0x7fffffff;
  diagPhaseInj   = 0x7fffffff;
  diagMatchExt   = 0x7fffffff;
  diagMatchInj   = 0x7fffffff;
  flagErrPmExt   = 0x1;
  flagErrPmInj   = 0x1;
  flagErrCbu     = 0x1;
  flagErrKickExt = 0x1;
  flagErrKickInj = 0x1;
} // clear all date

// print status
void printStatus()
{
  static uint32_t iter = 0;

  printf("--- Status ----------------------------------------------- #transfer %5u ---\n", iter);
  iter++;
  printf("CBU     ");
  if (mode == 0) printf(TXTNA);
  else {
    if (flagErrCbu) printf(TXTERROR);
    else            printf(TXTOK);
  }
  printf(", SID   %2d", reqSid); 
  printf(", mode ");
  switch (mode) {
    case 0 :
      printf("'off'");
      break;
    case 1 :
      printf("'EVT_KICK_START'");
      break;
    case 2 :
      printf("'bunch 2 fast extraction'");
      break;
    case 3 :
      printf("'bunch 2 coasting beam'");
      break;
    case 4 :
      printf("'bunch 2 bucket'");
      break;
    default :
      printf("'unknonwn'");
  } // switch mode
  printf("\n");

  printf("ext: PM ");
  if (mode < 2) printf(TXTNA);
  else {
    if (flagErrPmExt) printf(TXTERROR);
    else printf(TXTOK);
  }
  printf(", KD ");
  if (mode == 0) printf(TXTNA);
  else {
    if (flagErrKickExt) printf(TXTERROR);
    else printf(TXTOK);
  }
  printf("\n");

  printf("inj: PM ");
  if (mode < 4) printf(TXTNA);
  else {
    if (flagErrPmInj) printf(TXTERROR);
    else printf(TXTOK);
  }
  printf(", KD ");
  if (mode < 3) printf(TXTNA);
  else {
    if (flagErrKickInj) printf(TXTERROR);
    else printf(TXTOK);
  }
  printf("\n");
} // print status

void printSetValues()
{
  double   lambdaExt;
  double   lambdaInj;
  uint64_t Tdiff;
  uint64_t TBeat;
  double   fBeat;
  uint64_t THighFast;
  uint64_t THighSlow;

  printf("--- Set Values ---------------------------------------------------------------\n");
  printf("ext: kick  corr");
  if (mode < 1) printf("   %s", TXTNA);
  else          printf(" %4d ns", cTrigExt);
  printf("; gDDS  ");
  if (mode < 2) printf(TXTNA);
  else {
    lambdaExt = (double)TH1Ext / 1000000000.0;
    fH1Ext    = 1000000000.0 / lambdaExt;
    printf(" %15.6f Hz, %15.6f ns, H =%2d", fH1Ext, lambdaExt, nHExt);
  }
  printf("\n");

  printf("inj: kick  corr");
  if (mode < 3) printf("   %s", TXTNA);
  else          printf(" %4d ns", cTrigInj);
  printf("; gDDS  ");
  if (mode < 4) printf(TXTNA);
  else {
    lambdaInj = (double)TH1Inj / 1000000000.0;
    fH1Inj    = 1000000000.0 / lambdaInj;
    printf(" %15.6f Hz, %15.6f ns, H =%2d", fH1Inj, lambdaInj, nHInj);
  }
  printf("\n");

  printf("B2B: ");
  if (mode < 4) printf("%s\n", TXTNA);
  else {
    printf("phase corr %4d ns, %8.3f°\n", cPhase, ns2Degree(cPhase, TH1Ext)); printf("     ");
    if (nHExt * fH1Ext < nHInj * fH1Inj) {
      THighFast = TH1Inj * nHExt;
      THighSlow = TH1Ext * nHInj;
    } // extracion is slower
    else {
      THighFast = TH1Ext * nHInj;
      THighSlow = TH1Inj * nHExt;
    } // extraction is faster

    Tdiff = THighSlow - THighFast;
    if (Tdiff > 0) TBeat = THighSlow / Tdiff;
    else           TBeat = 0;
    if ((TBeat % Tdiff) > (Tdiff >> 1)) TBeat++;
    TBeat = (TBeat * THighSlow);
    fBeat = 1000000000000000000.0 / double(TBeat);
    printf("beat [step / RF period]      %13.6f Hz, %12.3f ns [%5.3f ns]", fBeat, (double)TBeat/1000000000.0, (double)Tdiff/1000000000.0);
  }
  printf("\n");

  printf("ext: next gDDS step @ LSA value");
  if (mode < 2) printf(TXTNA);
  else printf(" %15.6f Hz", fH1Ext + DDSSTEP);
  printf("\n");

  printf("inj: next gDDS step @ LSA value");
  if (mode < 4) printf(TXTNA);
  else printf(" %15.6f Hz", fH1Inj + DDSSTEP);
  printf("\n");

} // printSetValues

void printGetValues()
{
  static int32_t diagMatchExtMax     = 0x80000000;
  static int32_t diagMatchExtMin     = 0x7fffffff;
  static double  diagMatchExtAveNew  = 0;
  double         diagMatchExtAveOld  = 0;
  static double  diagMatchExtStrNew  = 0;
  double         diagMatchExtStrOld  = 0;
  double         diagMatchExtSdev    = 0;
  static int32_t diagExtN            = 0;
  static int32_t diagMatchInjMax     = 0x80000000;
  static int32_t diagMatchInjMin     = 0x7fffffff;
  static double  diagMatchInjAveNew  = 0;
  double         diagMatchInjAveOld  = 0;
  static double  diagMatchInjStrNew  = 0;
  double         diagMatchInjStrOld  = 0;
  double         diagMatchInjSdev    = 0;
  static int32_t diagInjN            = 0;

  double         dummy;
  
  int32_t        diagMatchInjCorr;       // diagMatchInj corrected for cPhase (to match trigger)
  int64_t DKick;                         // time difference between EVT_KICK_START and kicker trigger (without corrections)

  if (mode == 1) DKick = 0;              // trigger upon EVT_KICK_START
  else           DKick = 1000000;        // trigger at least 1ms after EVT_KICK_START
  
  printf("--- Get Values ---------------------------------------------------------------\n");
  printf("ext: 'kick delay'  ");
  if (mode < 1) printf("    %s", TXTNA);
  else {
    printf(" electronics");
    if (kickElecDelExt != (int)0x7fffffff) printf(" %5d ns", kickElecDelExt);
    else                                   printf("    %s", TXTUNKWN);
    printf(", magnet");
    if (kickProbDelExt != (int)0x7fffffff) printf(" %5d ns", kickProbDelExt);
    else                                   printf("    %s", TXTUNKWN);
    printf(", RF wait %8ld ns", tTrigExt - tKickStart - DKick);
  }
  printf("\n");

  printf("inj: 'kick delay'  ");
  if (mode < 3) printf("    %s", TXTNA);
  else {
    printf(" electronics");
    if (kickElecDelInj != (int)0x7fffffff) printf(" %5d ns", kickElecDelInj);
    else                                   printf("    %s", TXTUNKWN);
    printf(", magnet");
    if (kickProbDelInj != (int)0x7fffffff) printf(" %5d ns", kickProbDelInj);
    else                                   printf("    %s", TXTUNKWN);
    printf(", RF wait %8ld ns", tTrigInj - tKickStart - DKick);
  }
  printf("\n");

  printf("ext:");
  if (mode < 2) printf("%s", TXTNA);
  else {
    if (diagMatchExt  != (int)0x7fffffff) {
      diagExtN++;
      if (diagMatchExt > diagMatchExtMax) diagMatchExtMax = diagMatchExt;
      if (diagMatchExt < diagMatchExtMin) diagMatchExtMin = diagMatchExt;
      diagMatchExtAveOld  = diagMatchExtAveNew;
      diagMatchExtStrOld  = diagMatchExtStrNew;
      calcStats(&diagMatchExtAveNew, diagMatchExtAveOld, &diagMatchExtStrNew, diagMatchExtStrOld, (double)diagMatchExt, diagExtN, &dummy, &diagMatchExtSdev );
      printf(" 'kick-gDDS [ns]'  act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagMatchExt, diagMatchExtAveNew, diagMatchExtSdev, diagMatchExtMin, diagMatchExtMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("inj:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if (diagMatchInj  != (int)0x7fffffff) {
      diagInjN++; diagMatchInjCorr    = diagMatchInj + cPhase;
      if (diagMatchInjCorr > diagMatchInjMax) diagMatchInjMax = diagMatchInjCorr;
      if (diagMatchInjCorr < diagMatchInjMin) diagMatchInjMin = diagMatchInjCorr;
      diagMatchInjAveOld  = diagMatchInjAveNew;
      diagMatchInjStrOld  = diagMatchInjStrNew;
      calcStats(&diagMatchInjAveNew, diagMatchInjAveOld, &diagMatchInjStrNew, diagMatchInjStrOld, (double)diagMatchInjCorr, diagInjN, &dummy, &diagMatchInjSdev);
      printf(" 'kick-gDDS [ns]'  act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagMatchInjCorr, diagMatchInjAveNew, diagMatchInjSdev, diagMatchInjMin, diagMatchInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

} // printGetValues


void printRFDiagnostics()
{
  static int32_t diagPhaseExtMax     = 0x80000000;
  static int32_t diagPhaseExtMin     = 0x7fffffff;
  static double  diagPhaseExtAveNew  = 0;
  double         diagPhaseExtAveOld  = 0;
  static double  diagPhaseExtStrNew  = 0;
  double         diagPhaseExtStrOld  = 0;
  double         diagPhaseExtSdev    = 0;
  static int32_t diagExtN            = 0;
  static int32_t diagPhaseInjMax     = 0x80000000;
  static int32_t diagPhaseInjMin     = 0x7fffffff;
  static double  diagPhaseInjAveNew  = 0;
  double         diagPhaseInjAveOld  = 0;
  static double  diagPhaseInjStrNew  = 0;
  double         diagPhaseInjStrOld  = 0;
  double         diagPhaseInjSdev    = 0;
  static int32_t diagInjN            = 0;

  static double  diagNueExtAveNew    = 0;
  double         diagNueExtAveOld    = 0;
  static double  diagNueExtStrNew    = 0;
  double         diagNueExtStrOld    = 0;
  double         diagNueExtSdev      = 0;
  static double  diagNueInjAveNew    = 0;
  double         diagNueInjAveOld    = 0;
  static double  diagNueInjStrNew    = 0;
  double         diagNueInjStrOld    = 0;
  double         diagNueInjSdev      = 0;

  double         dummy;

  double         nue;                    // observed frequency
    
  printf("--- RF Diagnostics @ %4.1f ms ---------------------- #ext %5u, #inj %5u ---\n", (double)TDIAGOBS/1000000.0, diagExtN, diagInjN);
  printf("ext:");
  if (mode < 2) printf("%s", TXTNA);
  else {
    if (diagPhaseExt  != (int)0x7fffffff) {
      diagExtN++;
      if (diagPhaseExt > diagPhaseExtMax) diagPhaseExtMax = diagPhaseExt;
      if (diagPhaseExt < diagPhaseExtMin) diagPhaseExtMin = diagPhaseExt;
      diagPhaseExtAveOld  = diagPhaseExtAveNew;
      diagPhaseExtStrOld  = diagPhaseExtStrNew;
      calcStats(&diagPhaseExtAveNew, diagPhaseExtAveOld, &diagPhaseExtStrNew, diagPhaseExtStrOld, (double)diagPhaseExt, diagExtN, &dummy, &diagPhaseExtSdev);
      printf(" 'gDDS raw   [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagPhaseExt, diagPhaseExtAveNew, diagPhaseExtSdev, diagPhaseExtMin, diagPhaseExtMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
  
  printf("inj:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if (diagPhaseInj  != (int)0x7fffffff) {
      diagInjN++;
      if (diagPhaseInj > diagPhaseInjMax) diagPhaseInjMax = diagPhaseInj;
      if (diagPhaseInj < diagPhaseInjMin) diagPhaseInjMin = diagPhaseInj;
      diagPhaseInjAveOld  = diagPhaseInjAveNew;
      diagPhaseInjStrOld  = diagPhaseInjStrNew;
      calcStats(&diagPhaseInjAveNew, diagPhaseInjAveOld, &diagPhaseInjStrNew, diagPhaseInjStrOld, (double)diagPhaseInj, diagInjN, &dummy, &diagPhaseInjSdev);
      printf(" 'gDDS raw   [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagPhaseInj, diagPhaseInjAveNew, diagPhaseInjSdev, diagPhaseInjMin, diagPhaseInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("ext:");
  if (mode < 2) printf("%s\n\n", TXTNA);
  else {
    if (diagPhaseExt  != (int)0x7fffffff) {
      calcNue(&nue, diagPhaseExtAveNew, (double)TDIAGOBS, TH1Ext);
      diagNueExtAveOld  = diagNueExtAveNew;
      diagNueExtStrOld  = diagNueExtStrNew;
      calcStats(&diagNueExtAveNew, diagNueExtAveOld, &diagNueExtStrNew, diagNueExtStrOld, nue, diagExtN, &dummy, &diagNueExtSdev);
      printf(" 'gDDS       [Hz]' ave(sdev) %12.6f(%8.6f), diff %9.6f\n", nue, diagNueExtSdev, nue - fH1Ext); printf("    ");
      printf(" 'gDDS       [Hz]' calc      %12.6f - best guess\n", calcDdsNue(nue)); printf("    ");
      printf(" 'LSA        [Hz]' calc      %12.6f - proposed safe value", calcDdsNue(nue) + DDSSTEP/2);      
    }
    else printf("    %s\n\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
  
  printf("inj:");
  if (mode < 4) printf("%s\n\n", TXTNA);
  else {
    if (diagPhaseInj  != (int)0x7fffffff) {
      calcNue(&nue, diagPhaseInjAveNew, (double)TDIAGOBS, TH1Inj);
      diagNueInjAveOld  = diagNueInjAveNew;
      diagNueInjStrOld  = diagNueInjStrNew;
      calcStats(&diagNueInjAveNew, diagNueInjAveOld, &diagNueInjStrNew, diagNueInjStrOld, nue, diagInjN, &dummy, &diagNueInjSdev);
      printf(" 'gDDS       [Hz]' ave(sdev) %12.6f(%8.6f), diff %9.6f\n", nue, diagNueInjSdev, nue - fH1Inj); printf("    ");
      printf(" 'gDDS       [Hz]' calc      %12.6f - best guess\n", calcDdsNue(nue)); printf("    ");
      printf(" 'LSA        [Hz]' calc      %12.6f - proposed safe value", calcDdsNue(nue) + DDSSTEP/2);            
    }
    else printf("    %s\n\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

} // printRFDiagnostics


void printB2BDiagnostics()
{
  static int32_t diagMatchExtMax     = 0x80000000;
  static int32_t diagMatchExtMin     = 0x7fffffff;
  static double  diagMatchExtAveNew  = 0;
  double         diagMatchExtAveOld  = 0;
  static double  diagMatchExtStrNew  = 0;
  double         diagMatchExtStrOld  = 0;
  double         diagMatchExtSdev    = 0;
  static int32_t diagExtN            = 0;
  static int32_t diagMatchInjMax     = 0x80000000;
  static int32_t diagMatchInjMin     = 0x7fffffff;
  static double  diagMatchInjAveNew  = 0;
  double         diagMatchInjAveOld  = 0;
  static double  diagMatchInjStrNew  = 0;
  double         diagMatchInjStrOld  = 0;
  double         diagMatchInjSdev    = 0;
  static int32_t diagInjN            = 0;
  static int32_t diagMatchH1Max      = 0x80000000;
  static int32_t diagMatchH1Min      = 0x7fffffff;
  static double  diagMatchH1AveNew   = 0;
  double         diagMatchH1AveOld   = 0;
  static double  diagMatchH1StrNew   = 0;
  double         diagMatchH1StrOld   = 0;
  double         diagMatchH1Sdev     = 0;
  static int32_t diagH1N             = 0;

  double         dummy;

  int            flagExtOk           = 0;
  int            flagInjOk           = 0;
  int32_t        diagMatchExtCorr;       // diagMatchExt corrected for cTrigExt (to match phase)
  int32_t        diagMatchInjCorr;       // diagMatchInj corrected for cTrigInj and cPhase (to match phase)
  int32_t        diagMatchH1Corr;        // diagMatchExt/Inj corrected for cTrigExt, cTrigInj and cPhase (to match phase)
  double         cPhaseD;

    
  printf("--- B2B Diagnostics ------------------- #ext %5u, #inj %5u, #B2B %5u ---\n", diagExtN, diagInjN, diagH1N);
  printf("ext:");
  if (mode < 2) printf("%s", TXTNA);
  else {
    if (diagMatchExt  != (int)0x7fffffff) {
      flagExtOk = 1;
      diagExtN++;
      diagMatchExtCorr    = diagMatchExt - cTrigExt;
      if (diagMatchExtCorr > diagMatchExtMax) diagMatchExtMax = diagMatchExtCorr;
      if (diagMatchExtCorr < diagMatchExtMin) diagMatchExtMin = diagMatchExtCorr;
      diagMatchExtAveOld  = diagMatchExtAveNew;
      diagMatchExtStrOld  = diagMatchExtStrNew;
      calcStats(&diagMatchExtAveNew, diagMatchExtAveOld, &diagMatchExtStrNew, diagMatchExtStrOld, (double)diagMatchExtCorr, diagExtN, &dummy, &diagMatchExtSdev);
      printf(" 'gDDS       [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagMatchExtCorr, diagMatchExtAveNew, diagMatchExtSdev, diagMatchExtMin, diagMatchExtMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("inj:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if (diagMatchInj  != (int)0x7fffffff) {
      flagInjOk = 1;
      diagInjN++;
      diagMatchInjCorr    = diagMatchInj - cTrigInj + cPhase;
      if (diagMatchInjCorr > diagMatchInjMax) diagMatchInjMax = diagMatchInjCorr;
      if (diagMatchInjCorr < diagMatchInjMin) diagMatchInjMin = diagMatchInjCorr;
      diagMatchInjAveOld  = diagMatchInjAveNew;
      diagMatchInjStrOld  = diagMatchInjStrNew;
      calcStats(&diagMatchInjAveNew, diagMatchInjAveOld, &diagMatchInjStrNew, diagMatchInjStrOld, (double)diagMatchInjCorr, diagInjN, &dummy, &diagMatchInjSdev);
      printf(" 'gDDS       [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagMatchInjCorr, diagMatchInjAveNew, diagMatchInjSdev, diagMatchInjMin, diagMatchInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("B2B:");
  if (mode < 4) printf("%s\n", TXTNA);
  else {
    if (flagExtOk and flagInjOk) {
      diagH1N++;
      diagMatchH1Corr    = +(diagMatchExt -cTrigExt) - (diagMatchInj - cTrigInj);
      if (diagMatchH1Corr > diagMatchH1Max) diagMatchH1Max = diagMatchH1Corr;
      if (diagMatchH1Corr < diagMatchH1Min) diagMatchH1Min = diagMatchH1Corr;
      diagMatchH1AveOld  = diagMatchH1AveNew;
      diagMatchH1StrOld  = diagMatchH1StrNew;
      calcStats(&diagMatchH1AveNew, diagMatchH1AveOld, &diagMatchH1StrNew, diagMatchH1StrOld, (double)diagMatchH1Corr, diagH1N, &dummy, &diagMatchH1Sdev);
      printf(" 'phase      [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d\n", diagMatchH1Corr, diagMatchH1AveNew, diagMatchH1Sdev, diagMatchH1Min, diagMatchH1Max); printf("    ");
      printf(" 'phase-corr [ns]' act %4d, ave(sdev) %8.3f(%5.3f), minmax %4d, %4d", diagMatchH1Corr - cPhase, diagMatchH1AveNew - cPhase, diagMatchH1Sdev,
             diagMatchH1Min - cPhase, diagMatchH1Max - cPhase);
    }
    else printf("    %s\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("ext:");
  if (mode < 2) printf("%s", TXTNA);
  else {
    if (flagExtOk) {
      printf(" 'gDDS        [°]' ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f", ns2Degree(diagMatchExtAveNew, TH1Ext), ns2Degree(diagMatchExtSdev, TH1Ext),
             ns2Degree(diagMatchExtMin, TH1Ext), ns2Degree(diagMatchExtMax, TH1Ext));
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("inj:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if (flagExtOk) {
      printf(" 'gDDS        [°]' ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f", ns2Degree(diagMatchInjAveNew, TH1Ext), ns2Degree(diagMatchInjSdev, TH1Ext),
             ns2Degree(diagMatchInjMin, TH1Ext), ns2Degree(diagMatchInjMax, TH1Ext));
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("B2B:");
  if (mode < 4) printf("%s\n", TXTNA);
  else {
    if (flagExtOk and flagInjOk) {
      cPhaseD = ns2Degree(cPhase, TH1Ext);
      printf(" 'phase       [°]' ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n", ns2Degree(diagMatchH1AveNew, TH1Ext), ns2Degree(diagMatchH1Sdev, TH1Ext),
             ns2Degree(diagMatchH1Min, TH1Ext), ns2Degree(diagMatchH1Max, TH1Ext));  printf("    ");
      printf(" 'phase-corr  [°]' ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f", ns2Degree(diagMatchH1AveNew, TH1Ext) - cPhaseD, ns2Degree(diagMatchH1Sdev, TH1Ext),
             ns2Degree(diagMatchH1Min, TH1Ext) - cPhaseD, ns2Degree(diagMatchH1Max, TH1Ext) - cPhaseD);
    }
    else printf("    %s\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
  
} // printB2BDiagnostics

// print stuff to screen
void updateScreen()
{
  int      i;

  //clear screen
  for (i=0;i<60;i++) printf("\n");

  printStatus();
  printf("\n");
  printSetValues();
  printf("\n");
  printGetValues();
  printf("\n");
  printB2BDiagnostics();
  printf("\n");
  printRFDiagnostics();
  
} // updateScreen


// this will be called, in case we are snooping B2B
static void on_action_sequence(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags)
{
  uint32_t recGid;
  uint32_t recEvtNo;
  uint32_t recSid;

  recGid      = ((id    & 0x0fff000000000000) >> 48);
  recEvtNo    = ((id    & 0x0000fff000000000) >> 36);
  recSid      = ((id    & 0x00000000fff00000) >> 20);

  if (deadline > nextUpdate) {
    updateScreen();
    nextUpdate = saftlib::makeTimeTAI(0xffffffffffffffff);      // only one update per transfer
    flagTransStart = 0;
  }
  if (recSid != reqSid) return;

  switch (recEvtNo) {
    case KICKSTART1 :
      if (!flagTransStart){
        clearAllData();
        flagTransStart=1;
      }
      tKickStart = deadline.getTAI();
      nextUpdate = saftlib::makeTimeTAI(tKickStart + (uint64_t)TUPDATE);
      break;
    case PMEXT :
      nHExt          = ((param & 0xff00000000000000) >> 56);
      TH1Ext         = ((param & 0x00ffffffffffffff));
      mode           = 2;                                       // mode B2E
      break;
    case PMINJ :
      nHInj          = ((param & 0xff00000000000000) >> 56);
      TH1Inj         = ((param & 0x00ffffffffffffff));
      mode           = 4;                                       // mode B2B
      break;
    case PREXT :
      flagErrPmExt   = ((id    & 0x0000000000000001));
      break;
    case PRINJ :
      flagErrPmInj   = ((id    & 0x0000000000000004) >> 2);
      break;
    case TRIGGEREXT :
      if (!flagTransStart){                                     // argh: TRIGGEREXT might happen prio to EVT_KICK_START in case mode EKS AND negative cTrigExt
        clearAllData();
        flagTransStart=1;
      }
      if (mode == 0) mode = 1;                                  // mode EKS (at least)
      flagErrCbu     = ((id    & 0x0000000000000010) >> 4);
      cTrigExt       = ((param & 0x00000000ffffffff));
      tTrigExt       = deadline.getTAI();
      break;
    case TRIGGERINJ :
      cTrigInj       = ((param & 0x00000000ffffffff));
      cPhase         = ((param & 0xffffffff00000000) >> 32);
      tTrigInj       = deadline.getTAI();
      if (mode != 4) mode = 3;                                  // mode B2C
      break;
    case DIAGKICKEXT :
      flagErrKickExt = ((id    & 0x0000000000000002) >> 1);
      kickElecDelExt = ((param & 0xffffffff00000000) >> 32);
      kickProbDelExt = ((param & 0x00000000ffffffff));
      break;
    case DIAGKICKINJ :
      flagErrKickInj = ((id    & 0x0000000000000002) >> 1);
      kickElecDelInj = ((param & 0xffffffff00000000) >> 32);
      kickProbDelInj = ((param & 0x00000000ffffffff));
      break;
    case DIAGEXT :
      diagPhaseExt   = ((param & 0xffffffff00000000) >> 32);
      diagMatchExt   = ((param & 0x00000000ffffffff));
      diagMatchExt   = getAlignedTS(diagMatchExt, cTrigExt, TH1Ext);
      break;
    case DIAGINJ :
      diagPhaseInj   = ((param & 0xffffffff00000000) >> 32);
      diagMatchInj   = ((param & 0x00000000ffffffff));
      diagMatchInj   = getAlignedTS(diagMatchInj, cTrigInj - cPhase, TH1Inj);
      break;
    default :
      ;
  } // switch recEvtNo
} // on_action_sequence

using namespace saftlib;
using namespace std;

// display help
static void help(void) {
  std::cout << std::endl << "Usage: " << program << " <device name> [OPTIONS] [command]" << std::endl;
  std::cout << std::endl;
  std::cout << "  -h                   display this help and exit" << std::endl;
  std::cout << "  -f                   use the first attached device (and ignore <device name>)" << std::endl;
  std::cout << "  -d                   display values in dec format" << std::endl;
  std::cout << "  -x                   display values in hex format" << std::endl;
  std::cout << "  -v                   more verbosity, usefull with command 'snoop'" << std::endl;
  std::cout << "  -U                   display/inject absolute time in UTC instead of TAI" << std::endl;
  std::cout << "  -L                   used with command 'inject' and -U: if injected UTC second is ambiguous choose the later one" << std::endl;
  std::cout << std::endl;
  std::cout << "  snoop  <SID>         snoop events of the B2B system. Select SID of transfer" << std::endl;
  std::cout << std::endl;
  std::cout << "This tool snoops and diplays B2B specific info." <<std::endl;
  std::cout << std::endl;
  std::cout << "Report bugs to <d.beck@gsi.de> !!!" << std::endl;
  std::cout << "Licensed under the GPL v3." << std::endl;
  std::cout << std::endl;
} // help


int main(int argc, char** argv)
{
  // variables and flags for command line parsing
  int  opt;
  bool b2bSnoop       = false;
  bool useFirstDev    = false;
  int  nCondition;
  char *value_end;

  // variables snoop event
  uint64_t snoopID     = 0x0;

  char    *deviceName = NULL;

  const char *command;

  pmode       = PMODE_NONE;
  nextUpdate  = saftlib::makeTimeTAI(0xffffffffffffffff);

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "hdfULxv")) != -1) {
    switch (opt) {
      case 'f' :
        useFirstDev = true;
        break;
      case 'U':
        UTC = true;
        pmode = pmode + PMODE_UTC;
        break;
      case 'L':
        if (UTC) {
          UTCleap = true;
        } else {
          std::cerr << "-L only works with -U" << std::endl;
          return -1;
        }
        break;
      case 'd':
        pmode = pmode + PMODE_DEC;
        break;
      case 'x':
        pmode = pmode + PMODE_HEX;
        break;
      case 'v':
        pmode = pmode + PMODE_VERBOSE;
        break;
      case 'h':
        help();
        return 0;
      default:
        std::cerr << program << ": bad getopt result" << std::endl;
        return 1;
    } // switch opt
  }   // while opt
  
  if (optind >= argc) {
    std::cerr << program << " expecting one non-optional argument: <device name>" << std::endl;
    help();
    return 1;
  }

  deviceName = argv[optind];

  // parse for commands
  if (optind + 1< argc) {
    command = argv[optind+1];
    
    if (strcasecmp(command, "snoop") == 0) {
      if (optind+3  != argc) {
        std::cerr << program << ": expecting exactly one argument: snoop <SID>>" << std::endl;
        return 1;
      }
      reqSid   = strtoull(argv[optind+2], &value_end, 0);
      b2bSnoop = true;
    } // "snoop"

    else std::cerr << program << ": unknown command: " << command << std::endl;
  } // commands

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  }

  try {
    // initialize required stuff
    std::shared_ptr<SAFTd_Proxy> saftd = SAFTd_Proxy::create();

    // get a specific device
    map<std::string, std::string> devices = SAFTd_Proxy::create()->getDevices();
    std::shared_ptr<TimingReceiver_Proxy> receiver;
    if (useFirstDev) {
      receiver = TimingReceiver_Proxy::create(devices.begin()->second);
    } else {
      if (devices.find(deviceName) == devices.end()) {
        std::cerr << "Device '" << deviceName << "' does not exist" << std::endl;
        return -1;
      } // find device
      receiver = TimingReceiver_Proxy::create(devices[deviceName]);
    } //if(useFirstDevice);

    std::shared_ptr<SoftwareActionSink_Proxy> sink = SoftwareActionSink_Proxy::create(receiver->NewSoftwareActionSink(""));

    // snoop for B2B
    if (b2bSnoop) {

      nCondition = 3;
      
      std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

      snoopID = ((uint64_t)FID << 60) | ((uint64_t)GGSI << 52);
      condition[0] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfff0000000000000, 0));
     
      snoopID = ((uint64_t)FID << 60) | ((uint64_t)SIS18 << 48);
      condition[1] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, 0));

      snoopID = ((uint64_t)FID << 60) | ((uint64_t)ESR << 48);
      condition[2] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, 0));

      for (int i=0; i<nCondition; i++) {
        condition[i]->setAcceptLate(true);
        condition[i]->setAcceptEarly(true);
        condition[i]->setAcceptConflict(true);
        condition[i]->setAcceptDelayed(true);
        condition[i]->SigAction.connect(sigc::ptr_fun(&on_action_sequence));
        condition[i]->setActive(true);    
      } // for i

      while(true) {
        saftlib::wait_for_signal();
      }
    } // eventSnoop

  } catch (const saftbus::Error& error) {
    std::cerr << "Failed to invoke method: " << error.what() << std::endl;
  }

  return 0;
}


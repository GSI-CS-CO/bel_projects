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
// version: 2021-Jan-05

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

  iter++;
  printf("--- Status --- %9u -----------------------------------------------------\n", iter);
  printf("CBU     ");
  if (mode == 0) printf(TXTNA);
  else {
    if (flagErrCbu) printf(TXTERROR);
    else            printf(TXTOK);
  }
  printf(", ");
  switch (mode) {
    case 0 :
      printf("off");
      break;
    case 1 :
      printf("EVT_KICK_START");
      break;
    case 2 :
      printf("bunch 2 fast extraction");
      break;
    case 3 :
      printf("bunch 2 coasting beam");
      break;
    case 4 :
      printf("bunch 2 bucket");
      break;
    default :
      printf("unknonwn");
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
  printf("ext: kick corr");
  if (mode < 1) printf("    %s", TXTNA);
  else          printf("  %4d ns", cTrigExt);
  printf("; gDDS ");
  if (mode < 2) printf(TXTNA);
  else {
    lambdaExt = (double)TH1Ext / 1000000000.0;
    fH1Ext    = 1000000000.0 / lambdaExt;
    printf(" %15.6f Hz (%15.6f ns), H =%2d", fH1Ext, lambdaExt, nHExt);
  }
  printf("\n");

  printf("inj: kick corr");
  if (mode < 3) printf("    %s", TXTNA);
  else          printf("  %4d ns", cTrigInj);
  printf("; gDDS ");
  if (mode < 4) printf(TXTNA);
  else {
    lambdaInj = (double)TH1Inj / 1000000000.0;
    fH1Inj    = 1000000000.0 / lambdaInj;
    printf(" %15.6f Hz (%15.6f ns), H =%2d", fH1Inj, lambdaInj, nHInj);
  }
  printf("\n");

  printf("B2B: ");
  if (mode < 4) printf(TXTNA);
  else {
    printf("phase corr %4d ns", cPhase);
    printf("; beat ");
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
    printf(" %15.6f Hz (%15.6f ns)", fBeat, (double)TBeat/1000000000.0);
  }
  printf("\n");
} // printSetValues

void printGetValues()
{
  int64_t DKick;                       // time difference between EVT_KICK_START and kicker trigger (without corrections)

  if (mode == 1) DKick = 0;            // trigger upon EVT_KICK_START
  else           DKick = 1000000;      // trigger at least 1ms after EVT_KICK_START
  
  printf("--- Get Values ---------------------------------------------------------------\n");
  printf("ext: 'kicker delay'");
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

  printf("inj: 'kicker delay'");
  if (mode < 4) printf("    %s", TXTNA);
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
} // printGetValues


void calcMean(double *meanNew, double meanOld, double *sdevNew, double sdevOld, double val, uint32_t n)
{
  // see  ”The Art of ComputerProgramming, Volume 2: Seminumerical Algorithms“, Donald Knuth
  if (n > 1) {
    *meanNew = meanOld + (val - meanOld) / (double)n;
    *sdevNew = sqrt((sdevOld + (val - meanOld)*(val - *meanNew))/((double)(n-1)));
  }
  else {
    *meanNew = val;
    *sdevNew = 0;
  }
} // calcMean


void calcNue(double *nue, double obsOffset, double *dNue, double dObsOffset, uint64_t TObs, uint64_t TH1)
{
  // nue frequency [Hz]
  // obsOffset mean value of deviation [ns]
  // dNue uncertainty [Hz]
  // dObsOffset uncertainty of deviation [ns]
  // TObs observation time [ns]
  // H=1 gDDS period [as]
  int64_t nPeriod;      // # of rf periods within T
  int64_t offsetAs;     // offset [as]
  int64_t TAs;          // TObs [as]
  int64_t TH1ObsAs;     // observed TH1 [as]
  double  TH1ObsNs;     // observed TH1 [ns]

  int64_t dOffsetAs;    // delta offset [as]
  int64_t dTH1ObsAs;    // delta observed TH1 [as]
  double  dTH1ObsNs;    // delta observed TH1 [ns]
  double  nue2;
  
  TAs       = TObs * 1000000000;
  nPeriod   = TAs / TH1;
  offsetAs  = (int64_t)(obsOffset * 1000000000.0);
  TH1ObsAs  = TH1 + offsetAs / nPeriod;
  TH1ObsNs  = (double)TH1ObsAs / 1000000000.0;
  *nue      = 1000000000.0 / TH1ObsNs;

  // poor man's error propagation to avoid rounding errors
  dOffsetAs = (int64_t)(dObsOffset * 1000000000.0);
  dTH1ObsAs = TH1ObsAs + dOffsetAs / nPeriod;
  dTH1ObsNs = (double)dTH1ObsAs / 1000000000.0;
  nue2      = 1000000000.0 / dTH1ObsNs;
  *dNue     = fabs(nue2 - *nue);
} // calcNue


void printRFDiagnostics()
{
  static int32_t diagPhaseExtMax     = 0x80000000;
  static int32_t diagPhaseExtMin     = 0x7FFFFFFF;
  static double  diagPhaseExtAveNew  = 0;
  double         diagPhaseExtAveOld  = 0;
  static double  diagPhaseExtSDevNew = 0;
  double         diagPhaseExtSDevOld = 0;
  static int32_t diagExtN            = 0;
  static int32_t diagPhaseInjMax     = 0x80000000;
  static int32_t diagPhaseInjMin     = 0x7FFFFFFF;
  static double  diagPhaseInjAveNew  = 0;
  double         diagPhaseInjAveOld  = 0;
  static double  diagPhaseInjSDevNew = 0;
  double         diagPhaseInjSDevOld = 0;
  static int32_t diagInjN            = 0;

  double         nue;                    // observed frequency
  double         dNue;                   // uncertainty of observed frequency

    
  printf("--- RF Diagnostics -----------------------------------------------------------\n");
  printf("ext:");
  if (mode < 2) printf("%s\n", TXTNA);
  else {
    if (diagPhaseExt  != (int)0x7fffffff) {
      diagExtN++;
      if (diagPhaseExt > diagPhaseExtMax) diagPhaseExtMax = diagPhaseExt;
      if (diagPhaseExt < diagPhaseExtMin) diagPhaseExtMin = diagPhaseExt;
      diagPhaseExtAveOld  = diagPhaseExtAveNew;
      diagPhaseExtSDevOld = diagPhaseExtSDevNew;
      calcMean(&diagPhaseExtAveNew, diagPhaseExtAveOld, &diagPhaseExtSDevNew, diagPhaseExtSDevOld, (double)diagPhaseExt, diagExtN);
      printf(" 'gDDS diff  [ns]' act %4d, ave(sdev) %6.3f(%5.3f), bounds %4d, %4d\n", diagPhaseExt, diagPhaseExtAveNew, diagPhaseExtSDevNew, diagPhaseExtMin, diagPhaseExtMax);
      calcNue(&nue, diagPhaseExtAveNew, &dNue, diagPhaseExtSDevNew, (double)TDIAGOBS, TH1Ext);
      printf("     'gDDS nue   [Hz]' ave(sdev) %12.6f(%8.6f), diff %8.6f", nue, dNue, nue - fH1Ext);
    }
    else printf("    %s\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
  
  printf("inj:");
  if (mode < 4) printf("%s\n", TXTNA);
  else {
    if (diagPhaseInj  != (int)0x7fffffff) {
      diagInjN++;
      if (diagPhaseInj > diagPhaseInjMax) diagPhaseInjMax = diagPhaseInj;
      if (diagPhaseInj < diagPhaseInjMin) diagPhaseInjMin = diagPhaseInj;
      diagPhaseInjAveOld  = diagPhaseInjAveNew;
      diagPhaseInjSDevOld = diagPhaseInjSDevNew;
      calcMean(&diagPhaseInjAveNew, diagPhaseInjAveOld, &diagPhaseInjSDevNew, diagPhaseInjSDevOld, (double)diagPhaseInj, diagInjN);
      printf(" 'gDDS diff  [ns]' act %4d, ave(sdev) %6.3f(%5.3f), bounds %4d, %4d\n", diagPhaseInj, diagPhaseInjAveNew, diagPhaseInjSDevNew, diagPhaseInjMin, diagPhaseInjMax);
      calcNue(&nue, diagPhaseInjAveNew, &dNue, diagPhaseInjSDevNew, (double)TDIAGOBS, TH1Inj);
      printf("     'gDDS nue   [Hz]' ave(sdev) %12.6f(%8.6f), diff %8.6f", nue, dNue, nue - fH1Inj);
    }
    else printf("    %s\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
} // printRFDiagnostics


void printB2BDiagnostics()
{
  static int32_t diagMatchExtMax     = 0x80000000;
  static int32_t diagMatchExtMin     = 0x7FFFFFFF;
  static double  diagMatchExtAveNew  = 0;
  double         diagMatchExtAveOld  = 0;
  static double  diagMatchExtSDevNew = 0;
  double         diagMatchExtSDevOld = 0;
  static int32_t diagExtN            = 0;
  static int32_t diagMatchInjMax     = 0x80000000;
  static int32_t diagMatchInjMin     = 0x7FFFFFFF;
  static double  diagMatchInjAveNew  = 0;
  double         diagMatchInjAveOld  = 0;
  static double  diagMatchInjSDevNew = 0;
  double         diagMatchInjSDevOld = 0;
  static int32_t diagInjN            = 0;
  static int32_t diagMatchH1Max      = 0x80000000;
  static int32_t diagMatchH1Min      = 0x7FFFFFFF;
  static double  diagMatchH1AveNew   = 0;
  double         diagMatchH1AveOld   = 0;
  static double  diagMatchH1SDevNew  = 0;
  double         diagMatchH1SDevOld  = 0;
  static int32_t diagH1N             = 0;

  int32_t        diagMatchExtCorr;       // diagMatchExt corrected for cTrigExt (to match phase)
  int32_t        diagMatchInjCorr;       // diagMatchInj corrected for cTrigInj and cPhase (to match phase)
  int32_t        diagMatchH1Corr;        // diagMatchExt/Inj corrected for cTrigExt, cTrigInj and cPhase (to match phase)

    
  printf("--- B2B Diagnostics ----------------------------------------------------------\n");
  printf("ext:");
  if (mode < 2) printf("%s", TXTNA);
  else {
    if (diagPhaseExt  != (int)0x7fffffff) {
      diagExtN++;
      diagMatchExtCorr    = diagMatchExt - cTrigExt;
      if (diagMatchExtCorr > diagMatchExtMax) diagMatchExtMax = diagMatchExtCorr;
      if (diagMatchExtCorr < diagMatchExtMin) diagMatchExtMin = diagMatchExtCorr;
      diagMatchExtAveOld  = diagMatchExtAveNew;
      diagMatchExtSDevOld = diagMatchExtSDevNew;
      calcMean(&diagMatchExtAveNew, diagMatchExtAveOld, &diagMatchExtSDevNew, diagMatchExtSDevOld, (double)diagMatchExtCorr, diagExtN);
      printf(" 'trig match [ns]' act %4d, ave(sdev) %6.3f(%5.3f), bounds %4d, %4d", diagMatchExtCorr, diagMatchExtAveNew, diagMatchExtSDevNew, diagMatchExtMin, diagMatchExtMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("inj:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if (diagPhaseInj  != (int)0x7fffffff) {
      diagInjN++;
      diagMatchInjCorr    = diagMatchInj - cTrigInj -cPhase;
      if (diagMatchInjCorr > diagMatchInjMax) diagMatchInjMax = diagMatchInjCorr;
      if (diagMatchInjCorr < diagMatchInjMin) diagMatchInjMin = diagMatchInjCorr;
      diagMatchInjAveOld  = diagMatchInjAveNew;
      diagMatchInjSDevOld = diagMatchInjSDevNew;
      calcMean(&diagMatchInjAveNew, diagMatchInjAveOld, &diagMatchInjSDevNew, diagMatchInjSDevOld, (double)diagMatchInjCorr, diagInjN);
      printf(" 'trig match [ns]' act %4d, ave(sdev) %6.3f(%5.3f), bounds %4d, %4d", diagMatchInjCorr, diagMatchInjAveNew, diagMatchInjSDevNew, diagMatchInjMin, diagMatchInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("B2B:");
  if (mode < 4) printf("%s", TXTNA);
  else {
    if ((diagPhaseExt  != (int)0x7fffffff) && (diagPhaseInj != (int)0x7fffffff)) {
      diagH1N++;
      diagMatchH1Corr    = tTrigExt + diagMatchExtCorr - (tTrigInj + diagMatchInjCorr); /* chk signs */
      if (diagMatchH1Corr > diagMatchH1Max) diagMatchH1Max = diagMatchH1Corr;
      if (diagMatchH1Corr < diagMatchH1Min) diagMatchH1Min = diagMatchH1Corr;
      diagMatchH1AveOld  = diagMatchH1AveNew;
      diagMatchH1SDevOld = diagMatchH1SDevNew;
      calcMean(&diagMatchH1AveNew, diagMatchH1AveOld, &diagMatchH1SDevNew, diagMatchH1SDevOld, (double)diagMatchH1Corr, diagH1N);
      printf(" 'H=1 match  [ns]' act %4d, ave(sdev) %6.3f(%5.3f), bounds %4d, %4d", diagMatchH1Corr, diagMatchH1AveNew, diagMatchH1SDevNew, diagMatchH1Min, diagMatchInjMax);
    }
    else printf("    %s", TXTUNKWN);
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
  printRFDiagnostics();
  printf("\n");
  printB2BDiagnostics();
  
} // updateScreen


// this will be called, in case we are snooping B2B
static void on_action_sequence(uint64_t id, uint64_t param, saftlib::Time deadline, saftlib::Time executed, uint16_t flags)
{
  uint32_t recGid;
  uint32_t recEvtNo;
  uint32_t recSid;

  static uint32_t nCycle = 0x0;
  
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
      break;
    case DIAGINJ :
      diagPhaseInj   = ((param & 0xffffffff00000000) >> 32);
      diagMatchInj   = ((param & 0x00000000ffffffff));
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
  uint64_t snoopMask   = 0x0;

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


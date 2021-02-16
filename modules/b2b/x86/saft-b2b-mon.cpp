// @file saft-b2b-mon.cpp
// @brief Command-line interface for saftlib. This is a simple monitoring tool for the bunch to bucket system.
// @author Dietrich Beck  <d.beck@gsi.de>
//
// Copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH 
//
// Have a chat with saftlib and B2B. This is tool is 'quick and dirty'
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
// version: 2021-Jan-14

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

uint32_t reqSid;                        // requested sequence ID
uint32_t recGid;                        // received group ID
uint32_t reqExtRing;                    // requested extraction ring

// GID
#define GGSI        0x3a                // B2B prefix existing facility
#define SIS18       0x12c               // SIS18
#define ESR         0x154               // ESR
#define CRYRING     0x0d2               // CRYRING

// EVTNO
#define KICKSTART1  0x031               // event numbers used by B2B...
#define KICKSTART2  0x045
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

#define FID         0x1                 // format ID of timing messages

#define TUPDATE     100000000           // delay for updating screen after EVT_KICK_START [ns]
#define TDIAGOBS    20000000            // observation time for diagnostic [ns]
#define DDSSTEP     0.046566129         // min frequency step of gDDS
#define MSKRECMODE0 0x0                 // mask defining events that should be received for the different modes, mode off
#define MSKRECMODE1 0x050               // ... mode EKS
#define MSKRECMODE2 0x155               // ... mode B2E
#define MSKRECMODE3 0x1f5               // ... mode B2C
#define MSKRECMODE4 0x3ff               // ... mode B2B

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
int      flagEvtErr;                    // error flag;               0: PMEXT, 1: PMINJ, 2: PREXT, 3: PRINJ, 4: TRIGGEREXT, 5: TRIGGERNJ, 6: DIAGKICKEXT, 7: DIAGKICKINJ, 8: DIAGEXT, 9: DIAGINJ 
int      flagEvtRec;                    // flag for events received; 0: PMEXT, 1: PMINJ, 2: PREXT, 3: PRINJ, 4: TRIGGEREXT, 5: TRIGGERNJ, 6: DIAGKICKEXT, 7: DIAGKICKINJ, 8: DIAGEXT, 9: DIAGINJ
int      flagEvtLate;                   // flag for events late;     0: PMEXT, 1: PMINJ, 2: PREXT, 3: PRINJ, 4: TRIGGEREXT, 5: TRIGGERNJ, 6: DIAGKICKEXT, 7: DIAGKICKINJ, 8: DIAGEXT, 9: DIAGINJ


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


// calculate frequency from observed phase offset
int calcNue(double *nue,                // frequency value [Hz]
            double obsOffset,           // observed mean value of deviation from 'soll value' [ns]
            uint64_t TObs,              // observation interval [as]
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
} 


// calculate 'real' DDS frequency from given frequency
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


// basic fixing of timestamps
int32_t fixAlignedTS(int32_t ts,         // timestamp [ns]
                    int32_t corr,        // (trigger)correction [ns]
                    uint64_t TH1         // H=1 period [as]
                    )
{
  int32_t TH1Ns;                         // H=1 period [ns]
  int32_t ts0;                           // timestamp with correction removed
  int32_t min;
  int32_t dtTmp;
  int32_t dtMatch;
    
  ts0       = ts - corr;
  TH1Ns     = TH1 / 1000000000;
  min       = 0x7fffffff;
  dtMatch = 0;

  if (fabs(corr) > 1000000) return 0;    // corr > 1ms 
  if (TH1Ns      <     100) return corr; // nue > 10 MHz
  if (TH1Ns      >  100000) return corr; // nue < 10 kHz
  if (fabs(ts0) >   100000) return corr; // max period (10 kHz)    

  for (dtTmp = ts0 - 30 * TH1Ns; dtTmp < ts0 + 30 * TH1Ns; dtTmp += TH1Ns) {
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
  gid             = 0x0;    
  sid             = 0x0;    
  mode            = 0x0;   
  TH1Ext          = 0x0;
  nHExt           = 0x0;  
  TH1Inj          = 0x0; 
  nHInj           = 0x0;  
  TBeat           = 0x0;  
  cPhase          = 0x7fffffff;
  cTrigExt        = 0x7fffffff;
  cTrigInj        = 0x7fffffff;
  tTrigExt        = 0x0;
  tTrigInj        = 0x0;
  tH1Ext          = 0x0; 
  tH1Inj          = 0x0;
  kickElecDelExt  = 0x7fffffff;
  kickProbDelExt  = 0x7fffffff;
  kickElecDelInj  = 0x7fffffff;
  kickProbDelInj  = 0x7fffffff;
  diagPhaseExt    = 0x7fffffff;
  diagPhaseInj    = 0x7fffffff;
  diagMatchExt    = 0x7fffffff;
  diagMatchInj    = 0x7fffffff;
  flagEvtErr      = 0x0;
  flagEvtRec      = 0x0;
  flagEvtLate     = 0x0;
} // clear all date

// print heaader
void printHeader()
{
  switch (recGid) {
    case SIS18 :
      printf("        ___ ___ ___   ___ ___ ___ _ ___   _                           \n");
      printf("       | _ )_  ) _ ) / __|_ _/ __/ ( _ ) | |_ ___                     \n");
      printf("       | _ \\/ /| _ \\ \\__ \\| |\\__ \\ / _ \\ |  _/ _ \\  _   _   _ \n");
      printf("       |___/___|___/ |___/___|___/_\\___/  \\__\\___/ (_) (_) (_)     \n");
    break;
    case ESR :
      printf("        ___ ___ ___   ___ ___ ___   _                            \n");
      printf("       | _ )_  ) _ ) | __/ __| _ \\ | |_ ___                     \n");
      printf("       | _ \\/ /| _ \\ | _|\\__ \\   / |  _/ _ \\  _   _   _     \n");
      printf("       |___/___|___/ |___|___/_|_\\  \\__\\___/ (_) (_) (_)      \n");
      break;
    default :
      ;    // ASCII art inspired by http://patorjk.com/software
  } // switch gid
} // printHeader


// print status
void printStatus()
{
  static uint32_t iter = 0;
  int i;
  char modeStr[64];
  int  modeMask;
  int  flagEvtMiss;

  switch (mode) {
    case 0 :
      sprintf(modeStr, "'off'");
      modeMask = 0;
      break;
    case 1 :
      sprintf(modeStr, "'EVT_KICK_START'");
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
      modeMask = MSKRECMODE0;
  } // switch mode

  // a missing event is an error
  flagEvtMiss = modeMask   & ~flagEvtRec;
  flagEvtErr  = flagEvtErr | flagEvtMiss;
  
  printf("--- Status ----------- SID %2d, %25s, #transfer %5u ---\n", reqSid, modeStr, iter);
  iter++;
  printf("events  :   PME  PMI  PRE  PRI  KTE  KTI  KDE  KDI  PDE  PDI @ %5.1f ms\n", (double)TUPDATE/1000000.0);

  printf("required:");
  for (i=0; i<10; i++) if ((modeMask    >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");
  
  printf("received:");
  for (i=0; i<10; i++) if ((flagEvtRec  >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");

  printf("late    :");
  for (i=0; i<10; i++) if ((flagEvtLate >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");
  
  printf("error   :");
  for (i=0; i<10; i++) if ((flagEvtErr  >> i) & 0x1) printf("    X"); else printf("     ");
  printf("\n");  
} // print status


// print set values
void printSetValues()
{
  double   TH1ExtNs = 0.0;     // H=1 period [ns]
  double   TH1InjNs = 0.0;     // H=1 period [ns]
  uint64_t TRfExt   = 0;       // true RF period
  uint64_t TRfInj   = 0;       // true RF period
  uint64_t TRfFast;            // RF period of faster frequency
  uint64_t TRfSlow;            // RF period of slower frequency
  uint64_t Tdiff;              // difference between true RF periods
  uint64_t nPeriods;           // helper variable
  uint64_t TBeat;              // beating period
  double   fBeat;              // beating frequency
  double   nBeatExt;           // number of RF cycles within TBeat extraction
  double   nBeatInj;           // number of RF cycles within TBeat injection
  double   TBeatDelta;         // maximum deviation after integer RF cycles within TBeat

  printf("--- Set Values ---------------------------------------------------------------\n");
  printf("ext: kick  corr");
  if (mode < 1) printf("   %s", TXTNA);
  else          printf(" %4d ns", cTrigExt);
  printf("; gDDS  ");
  if (mode < 2) printf(TXTNA);
  else {
    TH1ExtNs  = (double)TH1Ext / 1000000000.0;
    fH1Ext    = 1000000000.0 / TH1ExtNs;
    printf(" %15.6f Hz, %15.6f ns, H =%2d", fH1Ext, TH1ExtNs, nHExt);
  }
  printf("\n");

  printf("     next gDDS step @ LSA value");
  if (mode < 2) printf(TXTNA);
  else printf(" %15.6f Hz", fH1Ext + DDSSTEP);
  printf("\n");

  printf("inj: kick  corr");
  if (mode < 3) printf("   %s", TXTNA);
  else          printf(" %4d ns", cTrigInj);
  printf("; gDDS  ");
  if (mode < 4) printf(TXTNA);
  else {
    TH1InjNs  = (double)TH1Inj / 1000000000.0;
    fH1Inj    = 1000000000.0 / TH1InjNs;
    printf(" %15.6f Hz, %15.6f ns, H =%2d", fH1Inj, TH1InjNs, nHInj);
  }
  printf("\n");

  printf("     next gDDS step @ LSA value");
  if (mode < 4) printf(TXTNA);
  else printf(" %15.6f Hz", fH1Inj + DDSSTEP);
  printf("\n");

  printf("B2B: ");
  if (mode < 4) printf("%s\n\n\n\n\n\n\n", TXTNA);
  else {
    printf("phase corr %4d ns,            %8.3f°\n", cPhase, ns2Degree(cPhase, TH1Ext));
    printf("     Beating -----------------------------------------------------------------\n");
    TRfExt = TH1Ext / nHExt;
    TRfInj = TH1Inj / nHInj;
    if (TRfExt != TRfInj) {
      if (TRfExt > TRfInj) {
        TRfFast = TRfInj;
        TRfSlow = TRfExt;;
      } // extraction has slower frequency
      else {
        TRfFast = TRfExt;
        TRfSlow = TRfInj;
      } // injection has slower frequency
      
      Tdiff = TRfSlow - TRfFast;
      if (Tdiff > 0) nPeriods = TRfSlow / Tdiff;
      else           nPeriods = 0;
      /* chk if ((TRfSlow % Tdiff) > (Tdiff >> 1)) nPeriods++; */
      TBeat = (nPeriods * TRfSlow);

      fBeat      = 1000000000000000000.0 / double(TBeat);
      nBeatExt   = (double)TBeat / (double)TRfExt;
      nBeatInj   = (double)TBeat / (double)TRfInj;
      TBeatDelta = (round(nBeatExt) * TRfExt - round(nBeatInj)*TRfInj) / 1000000000.0;
      printf("     ext                        %15.6f Hz,  %15.6f ns\n", fH1Ext * (double)nHExt, TH1ExtNs / (double)nHExt);
      printf("     inj                        %15.6f Hz,  %15.6f ns\n", fH1Inj * (double)nHInj, TH1InjNs / (double)nHInj);
      printf("     diff                           %8.3f°               %8.6f ns\n", ns2Degree((double)Tdiff/1000000000.0, TRfExt),(double)Tdiff/1000000000.0);
      printf("     beating                      %13.6f Hz,  %15.6f ns\n", fBeat, (double)TBeat/1000000000.0); 
      printf("     ext                                             %15.6f periods\n", nBeatExt);
      printf("     inj                                             %15.6f periods\n", nBeatInj);
      printf("     calc delta [@1ns]             %9.3f° [%6.3f°]    %8.6f ns", ns2Degree(TBeatDelta, TRfExt), ns2Degree(1.0, TRfExt), TBeatDelta);
    }
    else {
      printf("     no beating: identical frequencies for injection and extraction\n\n\n\n\n\n");
    } // else
  }
  printf("\n");
  
} // printSetValues


// print get values
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
    printf(", RF bonus %6ld ns", (long int)(tTrigExt - tKickStart - DKick -cTrigExt));
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
    printf(", RF bonus %6ld ns", (long int)(tTrigInj - tKickStart - DKick - cTrigInj));
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
      printf(" 'kick-gDDS [ns]'  act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagMatchExt, diagMatchExtAveNew, diagMatchExtSdev, diagMatchExtMin, diagMatchExtMax);
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
      printf(" 'kick-gDDS [ns]'  act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagMatchInjCorr, diagMatchInjAveNew, diagMatchInjSdev, diagMatchInjMin, diagMatchInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

} // printGetValues


// print rf diagnostics
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
  int            error;
  
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
      printf(" 'gDDS raw   [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagPhaseExt, diagPhaseExtAveNew, diagPhaseExtSdev, diagPhaseExtMin, diagPhaseExtMax);
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
      printf(" 'gDDS raw   [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagPhaseInj, diagPhaseInjAveNew, diagPhaseInjSdev, diagPhaseInjMin, diagPhaseInjMax);
    }
    else printf("    %s", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

  printf("ext:");
  if (mode < 2) printf("%s\n\n", TXTNA);
  else {
    if (diagPhaseExt  != (int)0x7fffffff) {
      error = calcNue(&nue, diagPhaseExtAveNew, (double)TDIAGOBS, TH1Ext);
      diagNueExtAveOld  = diagNueExtAveNew;
      diagNueExtStrOld  = diagNueExtStrNew;
      if (!error) calcStats(&diagNueExtAveNew, diagNueExtAveOld, &diagNueExtStrNew, diagNueExtStrOld, nue, diagExtN, &dummy, &diagNueExtSdev);
      printf(" 'gDDS       [Hz]' ave(sdev)%13.6f(%8.6f), diff %9.6f\n", nue, diagNueExtSdev, nue - fH1Ext); printf("    ");
      printf(" 'gDDS       [Hz]' calc     %13.6f - best guess\n", calcDdsNue(nue)); printf("    ");
      printf(" 'LSA        [Hz]' calc     %13.6f - proposed safe value", calcDdsNue(nue) + DDSSTEP/2);      
    }
    else printf("    %s\n\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");
  
  printf("inj:");
  if (mode < 4) printf("%s\n\n", TXTNA);
  else {
    if (diagPhaseInj  != (int)0x7fffffff) {
      error = calcNue(&nue, diagPhaseInjAveNew, (double)TDIAGOBS, TH1Inj);
      diagNueInjAveOld  = diagNueInjAveNew;
      diagNueInjStrOld  = diagNueInjStrNew;
      if (!error) calcStats(&diagNueInjAveNew, diagNueInjAveOld, &diagNueInjStrNew, diagNueInjStrOld, nue, diagInjN, &dummy, &diagNueInjSdev);
      printf(" 'gDDS       [Hz]' ave(sdev)%13.6f(%8.6f), diff %9.6f\n", nue, diagNueInjSdev, nue - fH1Inj); printf("    ");
      printf(" 'gDDS       [Hz]' calc     %13.6f - best guess\n", calcDdsNue(nue)); printf("    ");
      printf(" 'LSA        [Hz]' calc     %13.6f - proposed safe value", calcDdsNue(nue) + DDSSTEP/2);            
    }
    else printf("    %s\n\n", TXTUNKWN);
  } // else: mode >= 2
  printf("\n");

} // printRFDiagnostics


// print diagnostic info on B2B
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
      printf(" 'gDDS       [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagMatchExtCorr, diagMatchExtAveNew, diagMatchExtSdev, diagMatchExtMin, diagMatchExtMax);
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
      printf(" 'gDDS       [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagMatchInjCorr, diagMatchInjAveNew, diagMatchInjSdev, diagMatchInjMin, diagMatchInjMax);
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
      printf(" 'phase      [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d\n", diagMatchH1Corr, diagMatchH1AveNew, diagMatchH1Sdev, diagMatchH1Min, diagMatchH1Max); printf("    ");
      printf(" 'phase-corr [ns]' act %4d, ave(sdev) %8.3f(%6.3f), minmax %4d, %4d", diagMatchH1Corr - cPhase, diagMatchH1AveNew - cPhase, diagMatchH1Sdev,
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

  printHeader();
  printf("\n");
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
  uint32_t recEvtNo;
  uint32_t recSid;
  int      flagLate;
  int      flagErr;

  recEvtNo    = ((id    & 0x0000fff000000000) >> 36);
  recSid      = ((id    & 0x00000000fff00000) >> 20);
  flagLate    = flags & 0x1;

  if (deadline > nextUpdate) {
    updateScreen();
    nextUpdate = saftlib::makeTimeTAI(0xffffffffffffffff);      // only one update per transfer
    flagTransStart = 0;
  }
  if (recSid != reqSid) return;

  switch (recEvtNo) {
    case KICKSTART1 :                                           // this is an OR, no 'break' on purpose
    case KICKSTART2 : 
      if (!flagTransStart){
        recGid       = ((id    & 0x0fff000000000000) >> 48);
        if (recGid != reqExtRing) return;
        clearAllData();
        flagTransStart=1;
      }
      tKickStart = deadline.getTAI();
      nextUpdate = saftlib::makeTimeTAI(tKickStart + (uint64_t)TUPDATE);
      break;
    case PMEXT :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      nHExt          = ((param & 0xff00000000000000) >> 56);
      TH1Ext         = ((param & 0x00ffffffffffffff));
      mode           = 2;                                       // mode B2E
      break;
    case PMINJ :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      nHInj          = ((param & 0xff00000000000000) >> 56);
      TH1Inj         = ((param & 0x00ffffffffffffff));
      mode           = 4;                                       // mode B2B
      break;
    case PREXT :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000001));
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      break;
    case PRINJ :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000004) >> 2);
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      break;
    case TRIGGEREXT :
      if (!flagTransStart){                                     // argh: TRIGGEREXT might happen prior to EVT_KICK_START in case mode EKS AND negative cTrigExt
        recGid       = ((id    & 0x0fff000000000000) >> 48);
        if (recGid != reqExtRing) return;
        clearAllData();
        flagTransStart=1;
      }
      if (mode == 0) mode = 1;                                  // mode EKS (at least)
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000010) >> 4);
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      cTrigExt       = ((param & 0x00000000ffffffff));
      tTrigExt       = deadline.getTAI();
      break;
    case TRIGGERINJ :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000010) >> 4);
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      cTrigInj       = ((param & 0x00000000ffffffff));
      cPhase         = ((param & 0xffffffff00000000) >> 32);
      tTrigInj       = deadline.getTAI();
      if (mode != 4) mode = 3;                                  // mode B2C
      break;
    case DIAGKICKEXT :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000002) >> 1);
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      kickElecDelExt = ((param & 0xffffffff00000000) >> 32);
      kickProbDelExt = ((param & 0x00000000ffffffff));
      break;
    case DIAGKICKINJ :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      flagErr        = ((id    & 0x0000000000000008) >> 3);
      flagEvtErr    |= flagErr << (recEvtNo - PMEXT);
      kickElecDelInj = ((param & 0xffffffff00000000) >> 32);
      kickProbDelInj = ((param & 0x00000000ffffffff));
      break;
    case DIAGEXT :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      diagPhaseExt   = ((param & 0xffffffff00000000) >> 32);
      diagMatchExt   = ((param & 0x00000000ffffffff));
      diagMatchExt   = fixAlignedTS(diagMatchExt, cTrigExt, TH1Ext);
      break;
    case DIAGINJ :
      flagEvtRec    |= 1 << (recEvtNo - PMEXT);
      flagEvtLate   |= flagLate << (recEvtNo - PMEXT);
      diagPhaseInj   = ((param & 0xffffffff00000000) >> 32);
      diagMatchInj   = ((param & 0x00000000ffffffff));
      diagMatchInj   = fixAlignedTS(diagMatchInj, cTrigInj - cPhase, TH1Inj);
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
  std::cout << "  -e<index>            species extraction ring (0:SIS18[default], 1: ESR)" << std::endl;
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

  char       *deviceName = NULL;
  const char *command;
  char       *tail;

  nextUpdate  = saftlib::makeTimeTAI(0xffffffffffffffff);
  reqExtRing  = SIS18;

  // parse for options
  program = argv[0];
  while ((opt = getopt(argc, argv, "e:hf")) != -1) {
    switch (opt) {
      case 'f' :
        useFirstDev = true;
        break;
      case 'e' :
        switch (strtol(optarg, &tail, 0)) {
          case 0 : reqExtRing = SIS18; break;
          case 1 : reqExtRing = ESR;   break;
          default: ;
        } // switch optarg
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        break;
      case 'h' :
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

      nCondition = 4;
      
      std::shared_ptr<SoftwareCondition_Proxy> condition[nCondition];

      snoopID = ((uint64_t)FID << 60) | ((uint64_t)GGSI << 52);
      condition[0] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xfff0000000000000, 0));
     
      snoopID = ((uint64_t)FID << 60) | ((uint64_t)SIS18 << 48);
      condition[1] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, 0));

      snoopID = ((uint64_t)FID << 60) | ((uint64_t)ESR << 48);
      condition[2] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, 0));

      snoopID = ((uint64_t)FID << 60) | ((uint64_t)CRYRING << 48);
      condition[3] = SoftwareCondition_Proxy::create(sink->NewCondition(false, snoopID, 0xffff000000000000, 0));

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


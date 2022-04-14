/*******************************************************************************************
 *  b2b-viewer.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 14-Apr-2022
 *
 * subscribes to and displays status of a b2b transfer
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
#define B2B_VIEWER_VERSION 0x000400

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

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

setval_t   dicSetval;
getval_t   dicGetval;
diagval_t  dicDiagval;
diagstat_t dicDiagstat;

uint32_t  dicSetvalId;
uint32_t  dicGetvalId;
uint32_t  dicDiagvalId;
uint32_t  dicDiagstatId;

enum {SETVAL, GETVAL} what;
#define  TXTNA       "  N/A"
#define  TXTUNKWN    "UNKWN"
#define  TXTERROR    "ERROR"

// set values
uint32_t flagSetValid;                                      // flag set data are valid 
uint32_t set_mode;                                          // b2b mode
double   set_extT;                                          // extraction, h=1 period [as]
double   set_extNue;                                        // extraction, h=1 frequency [Hz]
uint32_t set_extH;                                          // extraction, harmonic number
int32_t  set_extCTrig;                                      // extraction, kick trigger correction
double   set_injT;                                          // injection ...
double   set_injNue;
uint32_t set_injH;
int32_t  set_injCTrig;
int32_t  set_cPhase;                                        // b2b: phase correction [ns]
double   set_cPhaseD;                                       // b2b: phase correction [degree]
uint32_t set_msecs;                                         // CBS deadline, fraction [ms]
time_t   set_secs;                                          // CBS deadline, time [s]

// b2b values
double   flagB2bValid;                                      // flag b2b data are valid
double   b2b_extNue;                                        // extraction, rf frequency [Hz]
double   b2b_extT;                                          // extraction, rf period [ns]
double   b2b_extN;                                          // extraction, number of rf periods within beat period
double   b2b_injNue;                                        // injection ...
double   b2b_injT;
double   b2b_injN;
double   b2b_diff;                                          // difference of rf periods [ns]
double   b2b_diffD;                                         // difference of rf periods [degree]
double   b2b_beatNue;                                       // beat frequency
double   b2b_beatT;                                         // beat period

#define MSKRECMODE0 0x0                 // mask defining events that should be received for the different modes, mode off
#define MSKRECMODE1 0x050               // ... mode CBS
#define MSKRECMODE2 0x155               // ... mode B2E
#define MSKRECMODE3 0x1f5               // ... mode B2C
#define MSKRECMODE4 0x3ff               // ... mode B2B

// other
int      flagPrintSet;                                      // flag: print set values
int      flagPrintBeat;                                     // flag: print b2b info
int      flagPrintDiag;                                     // flag: print diagnostic info
int      flagPrintRf;                                       // flag: print rf info
int      flagPrintKick;                                     // flag: print kick info
int      flagPrintStat;                                     // flag: print status info
int      modeMask;                                          // mask: marks events used in actual mode

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <name>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -s <SID>            display b2b info for Sequence ID\n");
  fprintf(stderr, "  -o <what>           print info only once and exit (useful with '-s')\n");
  fprintf(stderr, "                      'what' 0: set val; 1: get val; .... \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display information on the B2B system\n");
  fprintf(stderr, "Example1: '%s pro_sis18 -s7'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(B2B_VIEWER_VERSION));
} //help


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


// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  setval_t *tmp;
  uint32_t secs;
  uint32_t nok;

  flagSetValid = (*size != sizeof(uint32_t));

  if (flagSetValid) {
    tmp = address;

    nok           = (*tmp).flag_nok;
    set_mode      = (*tmp).mode;
    if ((nok >> 1) & 0x1) {
      set_extT    = 0.0;
      set_extNue  = 0.0;
    } // if not valid
    else {
      set_extT    = (double)((*tmp).ext_T)/1000000000.0;
      set_extNue  = 1000000000.0 / set_extT;
      set_cPhaseD = (double)((*tmp).cPhase) / (double)set_extT * 360.0; 
    } // valid
    set_extH      = (*tmp).ext_h;
    set_extCTrig  = (*tmp).ext_cTrig;
    if ((nok >> 4) & 0x1) {
      set_injT    = 0.0;
      set_injNue  = 0.0;
    } // if not valid
    else {
      set_injT    = (double)((*tmp).inj_T)/1000000000.0;
      set_injNue  = 1000000000.0 / set_injT;
    } // valid
    set_injH      = (*tmp).inj_h;
    set_injCTrig  = (*tmp).inj_cTrig;
    set_cPhase    = (*tmp).cPhase;

    dic_get_timestamp(0, &secs, &set_msecs);
    set_secs      = (time_t)(secs);

  } // if flagSetValid
  else set_mode = 0;
} // recSetValue


// add all dim services
void dicSubscribeServices(char *prefix, uint32_t sid)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s-raw_sid%02d_setval", prefix, sid);
  printf("name %s\n", name);
  dicSetvalId = dic_info_service_stamped(name, MONITORED, 0, &dicSetval, sizeof(setval_t), recSetvalue, 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s-raw_sid%02d_getval", prefix, sid);
  printf("name %s\n", name);
  dicGetvalId = dic_info_service_stamped(name, MONITORED, 0, &dicGetval, sizeof(getval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s-cal_diag_sid%02d", prefix, sid);
  printf("name %s\n", name);
  dicDiagvalId = dic_info_service_stamped(name, MONITORED, 0, &dicDiagval, sizeof(diagval_t), 0 , 0, &no_link_32, sizeof(uint32_t));

  sprintf(name, "%s-cal_stat_sid%02d", prefix, sid);
  printf("name %s\n", name);
  dicDiagstatId = dic_info_service_stamped(name, MONITORED, 0, &dicDiagstat, sizeof(diagstat_t), 0 , 0, &no_link_32, sizeof(uint32_t));
} // dicSubscribeServices


// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t sid)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s-cal_cmd_cleardiag", prefix);
  dic_cmnd_service(name, &sid, sizeof(sid));
} // dicCmdClearDiag


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


// print set values
int printSet(uint32_t sid)
{
  char   modeStr[50];
  char   tCBS[100];
  
  switch (set_mode) {
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

  strftime(tCBS, 19, "%H:%M:%S", gmtime(&set_secs));
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


// print diagnostic values
int printDiag(uint32_t sid)
{
  printf("--- diag diff DDS ---                        #b2b %5u, #ext %5u, #inj %5u\n", dicDiagval.phaseOffN, dicDiagval.ext_ddsOffN, dicDiagval.inj_ddsOffN);
  switch(set_mode) {
    case 0 ... 1 :
      printf("ext: %s\n", TXTNA);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 2 ... 3 :
      if (dicDiagval.ext_ddsOffN == 0) printf("ext: %s\n", TXTNA);
      else  printf("ext [ns]: act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                   dicDiagval.ext_ddsOffAct, dicDiagval.ext_ddsOffAve, dicDiagval.ext_ddsOffSdev, dicDiagval.ext_ddsOffMin, dicDiagval.ext_ddsOffMax);
      printf("inj: %s\n", TXTNA);
      printf("b2b: %s\n", TXTNA);
      break;
    case 4      :
      if (dicDiagval.ext_ddsOffN == 0) printf("ext: %s\n", TXTNA);
      else  printf("ext [ns]: act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                   dicDiagval.ext_ddsOffAct, dicDiagval.ext_ddsOffAve, dicDiagval.ext_ddsOffSdev, dicDiagval.ext_ddsOffMin, dicDiagval.ext_ddsOffMax);
      if (dicDiagval.inj_ddsOffN == 0) printf("inj: %s\n", TXTNA);
      else  printf("inj [ns]': act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                   dicDiagval.inj_ddsOffAct, dicDiagval.inj_ddsOffAve, dicDiagval.inj_ddsOffSdev, dicDiagval.inj_ddsOffMin, dicDiagval.inj_ddsOffMax);
      if (dicDiagval.phaseOffN == 0) printf("inj: %s\n", TXTNA);
      else  printf("b2b [ns]: act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                   dicDiagval.phaseOffAct, dicDiagval.phaseOffAve, dicDiagval.phaseOffSdev, dicDiagval.phaseOffMin, dicDiagval.phaseOffMax);
      break;
    default :
      ;
  } // switch set mode
  return 4;                                                 // 4 lines
} // printDiag


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
      printf("     mon h=1 ph [ns] act %4f, ave(sdev) %8.3f(%6.3f), minmax %4f, %4f\n", dicDiagstat.ext_monRemAct, dicDiagstat.ext_monRemAve, dicDiagstat.ext_monRemSdev,
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
      printf("     mon h=1 ph [ns] act %4f, ave(sdev) %8.3f(%6.3f), minmax %4f, %4f\n", dicDiagstat.inj_monRemAct, dicDiagstat.inj_monRemAve, dicDiagstat.inj_monRemSdev,
             dicDiagstat.inj_monRemMin, dicDiagstat.inj_monRemMax);
    } // else flag_nok
  } // else mode < 3

  return 5;                                                 // 5 lines
} // printKick

// print status info
int printStatus(uint32_t sid)
{
  int i;
  uint32_t flagEvtErr;
  double   sdevKteFin;

  flagEvtErr  = dicGetval.flagEvtErr | (modeMask   & ~(dicGetval.flagEvtRec));

  printf("--- status (expert) ---                      #b2b %5u, #ext %5u, #inj %5u\n", dicDiagstat.cbs_priOffN, dicDiagstat.cbs_kteOffN, dicDiagstat.cbs_ktiOffN);

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
    sdevKteFin = sqrt(pow(dicDiagstat.cbs_doneOffSdev, 2)+pow(dicDiagstat.cbs_kteOffSdev, 2));
    printf("fin-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.cbs_doneOffAct/1000.0, dicDiagstat.cbs_doneOffAve/1000.0, dicDiagstat.cbs_doneOffSdev/1000.0,
           (double)dicDiagstat.cbs_doneOffMin/1000.0, (double)dicDiagstat.cbs_doneOffMax/1000.0);
    printf("KTE-fin [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)(dicDiagstat.cbs_kteOffAct-dicDiagstat.cbs_doneOffAct)/1000.0, (dicDiagstat.cbs_kteOffAve-dicDiagstat.cbs_doneOffAve)/1000.0, sdevKteFin/1000.0,
           (double)(dicDiagstat.cbs_kteOffMin-dicDiagstat.cbs_doneOffMax)/1000.0, (double)(dicDiagstat.cbs_kteOffMax-dicDiagstat.cbs_doneOffMin)/1000.0);
    printf("KTE-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.cbs_kteOffAct/1000.0, dicDiagstat.cbs_kteOffAve/1000.0, dicDiagstat.cbs_kteOffSdev/1000.0,
           (double)dicDiagstat.cbs_kteOffMin/1000.0, (double)dicDiagstat.cbs_kteOffMax/1000.0);
  }
  if (set_mode < 3)
    printf("KTI-CBS [us]: %s\n", TXTNA);
  else
    printf("KTI-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.cbs_ktiOffAct/1000.0, dicDiagstat.cbs_ktiOffAve/1000.0, dicDiagstat.cbs_ktiOffSdev/1000.0,
           (double)dicDiagstat.cbs_ktiOffMin/1000.0, (double)dicDiagstat.cbs_ktiOffMax/1000.0);
  if (set_mode <  2)
    printf("t0E-CBS [us]: %s\n", TXTNA);
  else
    printf("t0E-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.cbs_preOffAct/1000.0, dicDiagstat.cbs_preOffAve/1000.0, dicDiagstat.cbs_preOffSdev/1000.0,
           (double)dicDiagstat.cbs_preOffMin/1000.0, (double)dicDiagstat.cbs_preOffMax/1000.0);
  if (set_mode < 4)
    printf("t0I-CBS [us]: %s\n", TXTNA);
  else
    printf("t0I-CBS [us]: act %8.2f ave(sdev) %7.2f(%8.2f) minmax %7.2f, %8.2f\n",
           (double)dicDiagstat.cbs_priOffAct/1000.0, dicDiagstat.cbs_priOffAve/1000.0, dicDiagstat.cbs_priOffSdev/1000.0,
           (double)dicDiagstat.cbs_priOffMin/1000.0, (double)dicDiagstat.cbs_priOffMax/1000.0);
  return 12;                                                // 12 lines
} // printStatus


// print rf values
int printRf(uint32_t sid)
{
  printf("--- rf DDS ---                                           #ext %5u, #inj %5u\n", dicDiagval.ext_rfOffN, dicDiagval.inj_rfOffN);
  switch(set_mode) {
    case 0 ... 1 :
      printf("ext: %s\n", TXTNA);
      printf("inj: %s\n", TXTNA);
      break;
    case 2 ... 3 :
      if (dicDiagval.ext_rfOffN == 0) printf("ext: %s\n", TXTNA);
      else printf("ext: [ns] act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                  dicDiagval.ext_rfOffAct, dicDiagval.ext_rfOffAve, dicDiagval.ext_rfOffSdev, dicDiagval.ext_rfOffMin, dicDiagval.ext_rfOffMax);
      printf("inj: %s\n", TXTNA);
      if (dicDiagval.ext_rfNueN == 0) printf("ext: %s\n\n", TXTNA);
      else {
           printf("ext: calc [Hz] ave(sdev) %14.6f(%8.6f), diff %9.6f\n", dicDiagval.ext_rfNueAve, dicDiagval.ext_rfNueSdev, dicDiagval.ext_rfNueDiff);
           printf("     calc [Hz] estimate  %14.6f,        stepsize 0.046566\n", dicDiagval.ext_rfNueEst);
      } // else
      printf("inj: %s\n\n", TXTNA);
      break;
    case 4      :
      if (dicDiagval.ext_rfOffN == 0) printf("ext: %s\n", TXTNA);
      else printf("ext: [ns] act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                  dicDiagval.ext_rfOffAct, dicDiagval.ext_rfOffAve, dicDiagval.ext_rfOffSdev, dicDiagval.ext_rfOffMin, dicDiagval.ext_rfOffMax);
      if (dicDiagval.inj_rfOffN == 0) printf("inj: %s\n", TXTNA);
      else printf("inj: [ns] act %8.3f, ave(sdev) %8.3f(%6.3f), minmax %8.3f, %8.3f\n",
                  dicDiagval.inj_rfOffAct, dicDiagval.inj_rfOffAve, dicDiagval.inj_rfOffSdev, dicDiagval.inj_rfOffMin, dicDiagval.inj_rfOffMax);
      if (dicDiagval.ext_rfNueN == 0) printf("ext: %s\n\n", TXTNA);
      else {
           printf("ext: calc [Hz] ave(sdev) %14.6f(%8.6f), diff %9.6f\n", dicDiagval.ext_rfNueAve, dicDiagval.ext_rfNueSdev, dicDiagval.ext_rfNueDiff);
           printf("     calc [Hz] estimate  %14.6f,        stepsize 0.046566\n", dicDiagval.ext_rfNueEst);
      } // else
      if (dicDiagval.inj_rfNueN == 0) printf("inj: %s\n\n", TXTNA);
      else {
           printf("inj: calc [Hz] ave(sdev) %14.6f(%8.6f), diff %9.6f\n", dicDiagval.inj_rfNueAve, dicDiagval.inj_rfNueSdev, dicDiagval.inj_rfNueDiff);
           printf("     calc [Hz] estimate  %14.6f,        stepsize 0.046566\n", dicDiagval.inj_rfNueEst);
      } // else
      break;
    default :
      ;
  } // switch set mode
  return 7;                                                 // 7 lines
} // printRf


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


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  int      getVersion;
  int      subscribe;
  int      once;

  char     userInput;
  int      quit;

  char     prefix[DIMMAXSIZE];
  char     name[DIMMAXSIZE];
  uint32_t sid;                             // sequence ID


  program       = argv[0];
  getVersion    = 0;
  subscribe     = 0;
  once          = 0;
  quit          = 0;
  what          = SETVAL;
  flagPrintSet  = 1;
  flagPrintBeat = 0;
  flagPrintDiag = 0;
  flagPrintRf   = 0;
  flagPrintKick = 0;
  flagPrintStat = 0;

  while ((opt = getopt(argc, argv, "s:o:eh")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 's':
        sid = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        subscribe = 1;
        break;
      case 'o':
        once = 1;
        what = strtol(optarg, &tail, 0);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if *tail
        if (what > GETVAL) {
          printf("option '-o': parameter out of range\n");
          exit(1);
        } // if what
        if (what == SETVAL) flagPrintSet = 1;
        // more stuff
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

  if (getVersion) printf("%s: version %s\n", program, b2b_version_text(B2B_VIEWER_VERSION));

  if (subscribe) {
    printf("%s: starting client using prefix %s\n", program, prefix);

    dicSubscribeServices(prefix, sid);

    while (!quit) {
      /*if (once) {sleep(1); quit=1;}                 // wait a bit to get the values */
      printData(once, sid, name);
      if (!quit) {
        userInput = comlib_term_getChar();
        switch (userInput) {
          case 'c' :
            dicCmdClearDiag(prefix, sid);
            break;
          case 'b' :
            flagPrintBeat = !flagPrintBeat;
            break;
          case 'd' :
            flagPrintDiag = !flagPrintDiag;
            break;
          case 'k' :
            flagPrintKick = !flagPrintKick;
            break;
          case 'r' :
            flagPrintRf = !flagPrintRf;
            break;
          case 's' :
            flagPrintStat = !flagPrintStat;
            break;
          case 'q'         :
            quit = 1;
            break;
          default          :
            usleep(1000000);
        } // switch
      } // if !once
    } // while
  } // if subscribe

  return exitCode;
}

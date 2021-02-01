/*******************************************************************************************
 *  b2b-analyzer.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 1-February-2021
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
#define B2B_ANALYZER_VERSION 0x000227

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
#include <common-lib.h>                  // COMMON
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW

const char* program;

// dim stuff
#define   DIMCHARSIZE 32                   // standard size for char services
#define   DIMMAXSIZE  1024                 // max size for service names

uint32_t  no_link_32    = 0xdeadbeef;
uint64_t  no_link_64    = 0xdeadbeefce420651;
char      no_link_str[] = "NO_LINK";

setval_t  dicSetval[B2B_NSID];
getval_t  dicGetval[B2B_NSID];
diagval_t disDiagval[B2B_NSID]; 

uint32_t  dicSetvalId[B2B_NSID];
uint32_t  dicGetvalId[B2B_NSID];
uint32_t  disDiagvalId[B2B_NSID];

int       flagSetValid[B2B_NSID];
int       flagGetValid[B2B_NSID];
int       flagDiagValid[B2B_NSID];

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

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] [PREFIX]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to analyze and display get values of the B2B system\n");
  fprintf(stderr, "Example1: '%s sis18\n", program);
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
  int32_t ts0;                                              // timestamp with correction removed [ns]
  int32_t dtMatch;
  int64_t ts0as;                                            // t0 [as]
  int64_t remainder;                     
  int64_t half;
  int     flagNeg; 

  if (TH1 == 0) return ts;                                  // can't fix
  ts0       = ts - corr;
  if (ts0 < 0) {ts0 = -ts0; flagNeg = 1;}                   // make this work for negative numbers too
  else         flagNeg = 0;
    
  ts0as     = ts0 * 1000000000;
  half      = TH1 >> 1;
  remainder = ts0as % TH1;                                 
  if (remainder > half) ts0as = remainder - TH1;
  dtMatch   = (int32_t)(ts0as / 1000000000);

  if (flagNeg) dtMatch = -dtMatch;

  return dtMatch + corr;                 // we have to add back the correction (!)
} //fixTS


// clears diag data
void clearStats(uint32_t sid)
{
  ext_ddsOffN[sid]         = 0;
  ext_ddsOffMax[sid]       = 0x80000000;       
  ext_ddsOffMin[sid]       = 0x7fffffff;         
  ext_ddsOffAveOld[sid]    = 0;   
  ext_ddsOffStreamOld[sid] = 0;

  ext_rfOffN[sid]          = 0;
  ext_rfOffMax[sid]        = 0x80000000;       
  ext_rfOffMin[sid]        = 0x7fffffff;         
  ext_rfOffAveOld[sid]     = 0;   
  ext_rfOffStreamOld[sid]  = 0;

  
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


// receive set values
void recGetvalue(long *tag, diagval_t *address, int *size)
{
  uint32_t  sid;
  uint32_t  mode;
  int32_t   act;
  uint32_t  n;
  double    sdev = 0;
  double    aveNew;
  double    streamNew = 0;
  double    dummy;

  sid = *tag;
  if ((sid < 0) || (sid >= B2B_NSID)) return;
  if (!flagSetValid[sid]) {flagDiagValid[sid] = 0; return;}
  flagGetValid[sid] = (*size != sizeof(uint32_t));

  mode = dicSetval[sid].mode;
  if (mode <  2) return;                                    // no further analysis
  if (mode >= 2) {                                          // analysis for extraction trigger and rf
    // match diagnostics; theoretical value is '0'
    act = fixTS(dicGetval[sid].ext_diagMatch - dicSetval[sid].ext_cTrig, 0, dicSetval[sid].ext_T);
    n   = ++(ext_ddsOffN[sid]);

    // statistics
    calcStats(&aveNew, ext_ddsOffAveOld[sid], &streamNew, ext_ddsOffStreamOld[sid], act, n , &dummy, &sdev);
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

    // match diagnostics; theoretical value is '0'
    act = fixTS(dicGetval[sid].ext_diagPhase, 0, dicSetval[sid].ext_T);
    n   = ++(ext_rfOffN[sid]);
    printf("act %d, n %d\n", act, n);

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
  } // if mode >=2
  

  dis_update_service(disDiagvalId[sid]);
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

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s-diag_sid%02d", prefix, i);
    disDiagvalId[i] = dis_add_service(name, "I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2;I:2;D:2;I:2", &(disDiagval[i]), sizeof(diagval_t), 0 , 0);
  } // for i
} // disAddServices

// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t indexServer)
{
  char name[DIMMAXSIZE];

  //sprintf(name, "%s_%s_cmd_cleardiag", prefix, sysShortNames[indexServer]);
  //dic_cmnd_service(name, 0, 0);
} // dicCmdClearDiag


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
  char     disName[DIMMAXSIZE];
  uint32_t sid;                             // sequence ID
  int      i;


  program       = argv[0];
  getVersion    = 0;
  subscribe     = 1;
  once          = 0;
  quit          = 0;

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

  if (optind< argc) sprintf(prefix, "b2b_%s", argv[optind]);
  else              sprintf(prefix, "b2b");
   sprintf(disName, "%s-diag", prefix);

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

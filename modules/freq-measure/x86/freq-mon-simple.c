/*******************************************************************************************
 *  freq-mon-simple.c
 *
 *  created : 2022
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 15-Mar-2022
 *
 *  a simple viewer and archiving tool for measured DDS frequencies
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
#define FREQ_MON_SIMPLE_VERSION "00.03.19"

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
/* #include <dis.h> */

// b2b
/*#include <common-lib.h>                    // COMMON
#include <b2blib.h>                        // API
#include <b2b.h>                           // FW*/

const char* program;

// data type for measured frequency values; this is a copy;
// consider including .../modules/b2b/include/b2blib.h instead 
typedef struct {
  double   nueSet;                                   // DDS set value; just a crosscheck [Hz]
  double   nueGet;                                   // DDS measured value [Hz]
  double   nueDiff;                                  // difference nue - nueSet [Hz]
  double   nueErr;                                   // uncertainty of measured nue [Hz]
  double   nuerChi2;                                 // reduced chi square
  double   nueSlope;                                 // slope of measuared values [kHz/s], should be 0
  double   nueSlopeErr;                              // uncertainty of measured slope
  int32_t  nSeries;                                  // # of data series, a series contains multiple timestamps
  int32_t  nTS;                                      // # total number of time stamps used for calculus
  int32_t  nBadTS;                                   // # total number of bad (= dropped) time stamps
} nueMeas_t;
#define B2B_NSID                               16    // max number of SID settings

/*#define TDIAGOBS    20000000               // observation time for diagnostic [ns]
  #define DDSSTEP     0.046566129            // min frequency step of gDDS*/

// dim stuff
#define    DIMCHARSIZE 32                  // standard size for char services
#define    DIMMAXSIZE  1024                // max size for service names

uint32_t   no_link_32    = 0xdeadbeef;
uint64_t   no_link_64    = 0xdeadbeefce420651;
char       no_link_str[] = "NO_LINK";

/*setval_t   dicSetval[B2B_NSID];
  getval_t   dicGetval[B2B_NSID];*/
nueMeas_t  dicNueMeasExt[B2B_NSID];
char       dicPName[B2B_NSID][DIMMAXSIZE];

/*uint32_t   dicSetvalId[B2B_NSID];
  uint32_t   dicGetvalId[B2B_NSID];*/
uint32_t   dicNueMeasExtId[B2B_NSID];
uint32_t   dicPNameId[B2B_NSID];

// global variables
int        flagNMExtValid[B2B_NSID];        // flag: received measured frequency values are valid
/* int        flagGetValid[B2B_NSID];          // flag: received get value */
int        flagPrintData;                   // flag: print data to screen
int        flagWriteData;                   // flag: write data to file

time_t     utc_secs[B2B_NSID];              // time of measurement in UTC
uint32_t   utc_msecs[B2B_NSID];             // time of measurement in UTC

char       filename[B2B_NSID][DIMMAXSIZE];  // file names


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <PREFIX> \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -f<fprefix>         sets a prefix for the file names; if set: write data to file\n");
  fprintf(stderr, "  -n                  create new files, erases existing files\n");
  fprintf(stderr, "  -p                  print data to screen\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool for display or archive measured frequency data\n");
  fprintf(stderr, "Example1: '%s pro_sis18 -ftest\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", FREQ_MON_SIMPLE_VERSION);
} //help


/*
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
*/

// header String for file
char * headerString()
{
  return "                             patternName;                 time_UTC;    sid;     ext_nueSet;    ext_nueMeas; ext_nueMeasErr;    ext_nueDiff;   ext_nuerChi2;   ext_nueSlope; ext_nueSlopeEr;nSeries;    nTS; nBadTS";
  //     "                                 NO_LINK; 16-Mar-2022_14:47:57.637; SID  0;       0.000000;       0.000000;       0.000000;       0.000000;       0.000000;       0.000000;       0.000000;      0;      0;      0"

} // headerString

// receive frequency values for extraction
void recNuevalueExt(long *tag, nueMeas_t *address, int *size)
{
#define STRMAXLEN 2048
  uint32_t  sid;
  char      date[256];

  char strNuevalExt[STRMAXLEN];
  char *new;

  FILE     *dataFile;                       // file for data

  sid = *tag;

  if ((sid < 0) || (sid >= B2B_NSID)) return;
  flagNMExtValid[sid] = (*size != sizeof(uint32_t));
  if (!flagNMExtValid[sid])           return;

  dic_get_timestamp (dicNueMeasExtId[sid], &(utc_secs[sid]), &(utc_msecs[sid]));
  strftime(date, 52, "%d-%b-%Y_%H:%M:%S", gmtime(&(utc_secs[sid])));

  // set values
  new  = strNuevalExt;
  new += sprintf(new, "%s.%03d; SID %2d", date, utc_msecs[sid], sid);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueSet);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueGet);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueErr);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueDiff);  
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nuerChi2);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueSlope);
  new += sprintf(new, "; %14.6f"   , dicNueMeasExt[sid].nueSlopeErr);
  new += sprintf(new, "; %6d"      , dicNueMeasExt[sid].nSeries);  
  new += sprintf(new, "; %6d"      , dicNueMeasExt[sid].nTS);
  new += sprintf(new, "; %6d"      , dicNueMeasExt[sid].nBadTS);    

  if (flagWriteData) {
    if (!(dataFile = fopen(filename[sid], "a"))) return;
    fprintf(dataFile, "%40s; %s\n", dicPName[sid], strNuevalExt);
    fclose(dataFile);
  } // if flagPrintData

  if (flagPrintData) printf("%40s; %s\n", dicPName[sid], strNuevalExt);
} // recNuevalueExt
  

/*
// receive set values
void recSetvalue(long *tag, setval_t *address, int *size)
{
  uint32_t sid;
  uint32_t secs, msecs;

  sid = *tag;
  if ((sid < 0) || (sid >= B2B_NSID)) return;

  dic_get_timestamp(0, &secs, &msecs);
  utc_secs[sid]     = (time_t)(secs);
  utc_msecs[sid]    = msecs;
  flagSetValid[sid] = (*size != sizeof(uint32_t));
} // recSetValue
*/
  

// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name,"%s-pname_sid%02d", prefix, i);
    //printf("name %s\n", name);
    dicPNameId[i]      = dic_info_service_stamped(name, MONITORED, 0, &(dicPName[i]), DIMMAXSIZE, 0 , 0, &no_link_str, sizeof(no_link_str));
  } // for i
  
  sleep (2);  // data is taken upon callback of set-values; wait a bit until the other services have connected to their servers

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s-other-rf_sid%02d_ext", prefix, i);
    // printf("name %s\n", name);
    dicNueMeasExtId[i] = dic_info_service_stamped(name, MONITORED, 0, &(dicNueMeasExt[i]), sizeof(nueMeas_t), recNuevalueExt, i, &no_link_32, sizeof(uint32_t));
  } // for i    
} // dicSubscribeServices


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;
  char *tmp;

  int      getVersion;

  char     sprefix[DIMMAXSIZE];             // prefix for system like 'pro'
  char     fprefix[DIMMAXSIZE];             // prefix for file like 'test'
  int      i;
  FILE     *dataFile;                       // file for data
  char     fileMode[10];                    // mode for file


  program       = argv[0];
  getVersion    = 0;
  sprintf(fprefix, "%s", "");
  sprintf(fileMode, "a");
  flagWriteData = 0;
  flagPrintData = 0;

  while ((opt = getopt(argc, argv, "f:ehnp")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 'h':
        help();
        return 0;
        error = 1;
        break;
      case 'n' :
        sprintf(fileMode, "w");
        break;
      case 'f' :
        tmp = strtok(optarg, " ");
        if (strlen(tmp) == 0) {
          fprintf(stderr, "Specify a proper name, not '%s'!\n", optarg);
          exit(1);
        } // if strlen
        sprintf(fprefix, "%s", tmp);
        flagWriteData = 1;
        break;
      case 'p' :
        flagPrintData = 1;
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

  if (optind < argc) sprintf(sprefix, "b2b_%s", argv[optind]);
  else {
    fprintf(stderr, "%s: missing non optional argument <system prefix>\n", program);
    exit(1);
  } // else optind

  if (getVersion) printf("%s: version %s\n", program, FREQ_MON_SIMPLE_VERSION);

  if (flagWriteData) {
    for (i=0; i<B2B_NSID; i++) {
      sprintf(filename[i], "%s_%s_sid%02d.dat", fprefix, sprefix, i);
      printf("open data file %s\n", filename[i]);
      if (!(dataFile = fopen(filename[i], fileMode))) {
        fprintf(stderr, "%s: can't open file %s\n", program, filename[i]);
        exit (1);
      } // if !file
      fprintf(dataFile, "%s\n", headerString());
      fclose(dataFile);
    } // for i
  } // if flagPrintData;

  if (flagPrintData) printf("%s\n", headerString());

  dicSubscribeServices(sprefix);
    
  while (1) sleep(1);

  // hm... maybe we should close the files cleanly
  
  return exitCode;
}

/*******************************************************************************************
 *  b2b-archiver.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-dec-2024
 *
 * archives set and get values to data files
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
#define B2B_ARCHIVER_VERSION 0x000805

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

// dim stuff
#define    DIMCHARSIZE 32                  // standard size for char services
#define    DIMMAXSIZE  1024                // max size for service names

uint32_t   no_link_32    = 0xdeadbeef;
uint64_t   no_link_64    = 0xdeadbeefce420651;
double     no_link_dbl   = NAN;
char       no_link_str[] = "NO_LINK";
char       nan_str[]     = "nan";

setval_t   dicSetval[B2B_NSID]; 
getval_t   dicGetval[B2B_NSID];
diagval_t  dicDiagval[B2B_NSID];
char       dicPName[B2B_NSID][DIMMAXSIZE];

uint32_t   dicSetvalId[B2B_NSID];
uint32_t   dicGetvalId[B2B_NSID];
uint32_t   dicDiagvalId[B2B_NSID];
uint32_t   dicPNameId[B2B_NSID];

// global variables
int        flagSetValid[B2B_NSID];          // flag: received set value
int        flagGetValid[B2B_NSID];          // flag: received get value

time_t     utc_secs[B2B_NSID];              // time of CBS in UTC
uint32_t   utc_msecs[B2B_NSID];             // time of CBS in UTC

char       filename[B2B_NSID][DIMMAXSIZE];  // file names


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <PREFIX> \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -f<fprefix>         sets a prefix for the file names\n");
  fprintf(stderr, "  -n                  create new files, erases existing files\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to archive data of the B2B system\n");
  fprintf(stderr, "Example1: '%s pro_sis18 -ftest\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(B2B_ARCHIVER_VERSION));
} //help


// header String for file
char * headerString()
{
  return "patternName; time_CBS_UTC; ext_gid; ext_sid; mode; ext_T [as]; ext_h; ext_cTrig; inj_T; inj_h; inj_cTrig; cPhase; ext_phase; ext_phaseFract; ext_phaseErr; ext_maxsysErr; ext_dKickMon; ext_dkickProb; ext_kickLen; ext_kickLevel; ext_diagPhase; ext_diag_Match; inj_gid; inj_sid; inj_phase; inj_phaseFract; inj_phaseErr; inj_maxsysErr; inj_dKickMon; inj_dKickProb; inj_kickLen; inj_kickLevel; inj_diagPhase; inj_diagMatch; received PME; PMI; PRE; PRI; PSE; PSI; KTE; KTI; KDE; KDI; PDE; PDI; error PME; PMI; PRE; PRI; PSE; PSI; KTE; KTI; KDE; KDI; PDE; PDI; late PME; PMI; PRE; PRI; PSE; PSI; KTE; KTI; KDE; KDI; PDE; PDI; fin-CBS; prr-CBS; t0E-CBS; t0I-CBS; kte-CBS; kti-CBS; ext_nueGet; ext_dNueGet; inj_nueGet; inj_dNueGet";
} // headerString

// receive get values
void recGetvalue(long *tag, diagval_t *address, int *size)
{
#define STRMAXLEN 2048
  uint32_t  sid;
  double    cor;
  double    act;
  char      tCBS[256];;

  char      strSetval[STRMAXLEN];
  char      strGetval[STRMAXLEN];
  char      strNueval[STRMAXLEN];
  char      *new;

  int       i;

  FILE      *dataFile;                       // file for data

  // usleep: see comment in routine 'dicSubscribeServices'
  usleep(100000);  // 100 ms should be safe
  
  sid = *tag;
  if ((sid < 0) || (sid >= B2B_NSID)) return;
  if (!flagSetValid[sid])             return;
  flagGetValid[sid] = (*size != sizeof(uint32_t));
  
  strftime(tCBS, 52, "%d-%b-%Y_%H:%M:%S", gmtime(&(utc_secs[sid])));

  // set values
  new  = strSetval;
  new += sprintf(new, "%s.%03d; 0x%x; %d; %d", tCBS, utc_msecs[sid], dicSetval[sid].ext_gid, sid, dicSetval[sid].mode);
  if (dicSetval[sid].ext_T == -1) new += sprintf(new, "; %s"    , nan_str);
  else new += sprintf(new, "; %lu"   , dicSetval[sid].ext_T);
  if (dicSetval[sid].ext_h == -1) new += sprintf(new, "; %s"    , nan_str);
  else new += sprintf(new, "; %d"    , dicSetval[sid].ext_h);
  new += sprintf(new, "; %8.3f" , dicSetval[sid].ext_cTrig);
  if (dicSetval[sid].inj_T == -1) new += sprintf(new, "; %s"    , nan_str);
  else new += sprintf(new, "; %lu"   , dicSetval[sid].inj_T);
  if (dicSetval[sid].inj_h == -1) new += sprintf(new, "; %s"    , nan_str);
  else new += sprintf(new, "; %d"    , dicSetval[sid].inj_h);
  new += sprintf(new, "; %8.3f" , dicSetval[sid].inj_cTrig);
  new += sprintf(new, "; %8.3f" , dicSetval[sid].cPhase);

  // get values
  new  = strGetval;
  if (dicGetval[sid].ext_phase == -1) new += sprintf(new, "; %s"   , nan_str);
  else new += sprintf(new, "; %lu" , dicGetval[sid].ext_phase);
  new += sprintf(new, "; %7.3f"    , dicGetval[sid].ext_phaseFract);
  new += sprintf(new, "; %7.3f"    , dicGetval[sid].ext_phaseErr);
  new += sprintf(new, "; %5.3f"    , dicGetval[sid].ext_phaseSysmaxErr);
  new += sprintf(new, "; %7.1f"    , dicGetval[sid].ext_dKickMon);
  new += sprintf(new, "; %7.1f"    , dicGetval[sid].ext_dKickProb);
  new += sprintf(new, "; %7.1f"    , dicGetval[sid].ext_dKickProbLen);
  new += sprintf(new, "; %7.3f"    , dicGetval[sid].ext_dKickProbLevel);

  if (isnan(dicGetval[sid].ext_diagPhase) || (dicSetval[sid].ext_T == -1)) new += sprintf(new, "; %s"    , nan_str);
  else {
    cor  = 0;
    act  = b2b_fixTS(dicGetval[sid].ext_diagPhase, cor, dicSetval[sid].ext_T) - cor;
    new += sprintf(new, "; %8.3f",  act);
  } // is isnan

  if (isnan(dicGetval[sid].ext_diagMatch) || (dicSetval[sid].ext_T == -1)) new += sprintf(new, "; %s"    , nan_str);
  else {
    cor  = dicSetval[sid].ext_cTrig;
    act  = b2b_fixTS(dicGetval[sid].ext_diagMatch, cor, dicSetval[sid].ext_T) - cor;
    new += sprintf(new, "; %8.3f", act);
  } // else isnan

  new += sprintf(new, "; 0x%x; %d", dicSetval[sid].inj_gid, dicSetval[sid].inj_sid);
  if (dicGetval[sid].inj_phase == -1) new += sprintf(new, "; %s"   , nan_str);
  else new += sprintf(new, "; %lu", dicGetval[sid].inj_phase);
  new += sprintf(new, "; %7.3f"   , dicGetval[sid].inj_phaseFract);
  new += sprintf(new, "; %7.3f"   , dicGetval[sid].inj_phaseErr);
  new += sprintf(new, "; %5.3f"   , dicGetval[sid].inj_phaseSysmaxErr);
  new += sprintf(new, "; %7.1f"   , dicGetval[sid].inj_dKickMon);
  new += sprintf(new, "; %7.1f"   , dicGetval[sid].inj_dKickProb);
  new += sprintf(new, "; %7.1f"   , dicGetval[sid].inj_dKickProbLen);
  new += sprintf(new, "; %7.3f"   , dicGetval[sid].inj_dKickProbLevel);

  if (isnan(dicGetval[sid].inj_diagPhase) || (dicSetval[sid].inj_T == -1)) new += sprintf(new, "; %s"   , nan_str);
  else {
    cor  = 0;
    act  = b2b_fixTS(dicGetval[sid].inj_diagPhase, cor, dicSetval[sid].inj_T) - cor;
    new += sprintf(new, "; %8.3f",  act);
  } // else isnan

  if (isnan(dicSetval[sid].inj_cTrig) || isnan(dicGetval[sid].inj_diagMatch) || (dicSetval[sid].inj_T == -1)) new += sprintf(new, "; %s"   , nan_str);
  else {
    cor = dicSetval[sid].inj_cTrig - dicSetval[sid].cPhase;
    act = b2b_fixTS(dicGetval[sid].inj_diagMatch, cor, dicSetval[sid].inj_T) - cor;
    new += sprintf(new, "; %8.3f",  act);
  } // else isnan

  for (i=0; i<tagStart; i++) new += sprintf(new, "; %d", ((dicGetval[sid].flagEvtRec  >> i) & 0x1));
  for (i=0; i<tagStart; i++) new += sprintf(new, "; %d", ((dicGetval[sid].flagEvtErr  >> i) & 0x1));
  for (i=0; i<tagStart; i++) new += sprintf(new, "; %d", ((dicGetval[sid].flagEvtLate >> i) & 0x1));
  new += sprintf(new, "; %f; %f; %f; %f; %f; %f", dicGetval[sid].finOff, dicGetval[sid].prrOff, dicGetval[sid].preOff, dicGetval[sid].priOff, dicGetval[sid].kteOff, dicGetval[sid].ktiOff);

  // frequency values; chk: in principle we should check the timestammp of the service too?
  new = strNueval;
  if (*(uint32_t *)&(dicDiagval[sid]) == no_link_32)
    new += sprintf(new, "; NOLINK; NOLINK; NOLINK; NOLINK");
  else 
    new += sprintf(new, "; %13.6f; %13.6f; %13.6f; %13.6f", dicDiagval[sid].ext_rfNueAct, dicDiagval[sid].ext_rfNueActErr, dicDiagval[sid].inj_rfNueAct, dicDiagval[sid].inj_rfNueActErr);

  if (!(dataFile = fopen(filename[sid], "a"))) return;
  fprintf(dataFile, "%s; %s%s%s\n", dicPName[sid], strSetval, strGetval, strNueval);
  fclose(dataFile); 
} // recGetvalue
  

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


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<B2B_NSID; i++) {
    sprintf(name, "%s-raw_sid%02d_setval", prefix, i);
    //printf("name %s\n", name);
    dicSetvalId[i]     = dic_info_service_stamped(name, MONITORED, 0, &(dicSetval[i]), sizeof(setval_t), recSetvalue, i, &no_link_32, sizeof(uint32_t));

    sprintf(name,"%s-pname_sid%02d", prefix, i);
    //printf("name %s\n", name);
    dicPNameId[i]      = dic_info_service_stamped(name, MONITORED, 0, &(dicPName[i]), DIMMAXSIZE, 0 , 0, &no_link_str, sizeof(no_link_str));

    sprintf(name, "%s-cal_diag_sid%02d", prefix, i);
    dicDiagvalId[i]    = dic_info_service_stamped(name, MONITORED, 0, &(dicDiagval[i]), sizeof(diagval_t), 0, 0, &no_link_32, sizeof(uint32_t));

    sleep (2);  // data is taken upon callback of set-values; wait a bit until the other services have connected to their servers

    sprintf(name, "%s-raw_sid%02d_getval", prefix, i);
    //printf("name %s\n", name);
    dicGetvalId[i]     = dic_info_service_stamped(name, MONITORED, 0, &(dicGetval[i]), sizeof(getval_t), recGetvalue, i, &no_link_32, sizeof(uint32_t));

    // note: presently, the archiver is driven by get values. However, the 'analyzer' only calculates and publishes the the frequency values
    // _after_ the get values are available. Thus, the 'archiver' starts processing the data prior the 'analyzer' has finished its work.
    // Presently, the frequency values are an 'experimental feature' and a sleep is added to the 'recGetValue' routine to wait for the analyzer.
    // In pinciple, one could consider using a change of the analyzed values as a 'trigger' for the archiver; but then the archiver will not
    // work if the 'analyzer' is not available. Thus, we presently use the workaround of an additional sleep in routine 'recGetValue'. 
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

  while ((opt = getopt(argc, argv, "f:ehn")) != -1) {
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

  if (getVersion) printf("%s: version %s\n", program, b2b_version_text(B2B_ARCHIVER_VERSION));

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

  dicSubscribeServices(sprefix);
    
  while (1) sleep(1);

  // hm... maybe we should close the files cleanly
  
  return exitCode;
}

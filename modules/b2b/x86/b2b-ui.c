/********************************************************************************************
 *  b2b-ui.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 19-Aug-2024
 *
 *  user interface for B2B
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
 * Last update: 03-October-2020
 *********************************************************************************************/

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>

// b2b
#include <common-lib.h>                  // COMMON
#include <ivtpar.h>                      // IVTPAR
#include <b2blib.h>                      // B2B API
#include <b2b.h>                         // FW

#define  MENUTXT_MAIN       "b2bivt_main.txt"
#define  MENUPAR_MAIN       "b2bivt_main.par"
#define  MENUTXT_EXPERT     "b2bivt_expert.txt"
#define  MENUPAR_EXPERT     "b2bivt_expert.par"
#define  MENUTXT_SIS18      "b2bivt_sis18.txt"
#define  MENUPAR_SIS18      "b2bivt_sis18.par"
#define  MENUTXT_SIS18CONF  "b2bivt_sis18conf.txt"
#define  MENUPAR_SIS18CONF  "b2bivt_sis18conf.par"
#define  MENUTXT_SIS18SID   "b2bivt_sis18sid.txt"
#define  MENUPAR_SIS18SID   "b2bivt_sis18sid"      // name is completed at run-time
#define  MENUTXT_SIS18KNOB  "b2bivt_sis18knob.txt"
#define  MENUPAR_SIS18KNOB  "b2bivt_sis18knob.par"
#define  MENUTXT_ESR        "b2bivt_esr.txt"
#define  MENUPAR_ESR        "b2bivt_esr.par"
#define  MENUTXT_ESRCONF    "b2bivt_esrconf.txt"
#define  MENUPAR_ESRCONF    "b2bivt_esrconf.par"
#define  MENUTXT_ESRSID     "b2bivt_esrsid.txt"
#define  MENUPAR_ESRSID     "b2bivt_esrsid"        // name is completed at run-time
#define  MENUTXT_ESRKNOB    "b2bivt_esrknob.txt"
#define  MENUPAR_ESRKNOB    "b2bivt_esrknob.par"
#define  MENUTXT_YR         "b2bivt_yr.txt"
#define  MENUPAR_YR         "b2bivt_yr.par"
#define  MENUTXT_YRCONF     "b2bivt_yrconf.txt"
#define  MENUPAR_YRCONF     "b2bivt_yrconf.par"
#define  MENUTXT_YRSID      "b2bivt_yrsid.txt"
#define  MENUPAR_YRSID      "b2bivt_yrsid"         // name is completed at run-time
#define  MENUTXT_YRKNOB     "b2bivt_yrknob.txt"
#define  MENUPAR_YRKNOB     "b2bivt_yrknob.par"

#define  MAXLEN              256                   // max string length

const char*  program;
const char*  path;


static void die(const char* where, uint32_t  status) {
  fprintf(stderr, "%s: %s failed: %d\n",
          program, where, status);
  exit(1);
} //die


static void help(void)
{
  uint32_t version;

  fprintf(stderr, "Usage: %s [OPTION] <path prefix>  \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to work with B2B\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  b2b_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(version));
} //help


void getEbDevice(ring_t ring, char *ebDevice)
{
  FILE   *parfile;
  char   parname[MAXLEN];
  char   ebsis[MAXLEN];
  char   ebesr[MAXLEN];
  char   ebyr[MAXLEN];

  sprintf(parname, "%s/%s", path, MENUPAR_EXPERT);
  if ((parfile = fopen(parname,"r"))) {
    fscanf(parfile,"%s",ebsis);
    fscanf(parfile,"%s",ebesr);
    fscanf(parfile,"%s",ebyr);
    fclose(parfile);
    
    if (ring == SIS18)   sprintf(ebDevice, "%s", ebsis);
    if (ring == ESR)     sprintf(ebDevice, "%s", ebesr);
    if (ring == CRYRING) sprintf(ebDevice, "%s", ebyr);    
  } // if parfile
} // getEbDevice


void parfileReadSID(char *filename, char *comment, int *mode, int *ringInj, double *fH1Ext, int *nHExt, double *fH1Inj, int *nHInj, int *flagLsa ,double *cTrigExt, double *cTrigInj, double *cPhase)
{
  int   ok;
  FILE  *parfile;
  
  // read file with parameters
  if ((parfile = fopen(filename,"r"))) {
    ok         = fscanf(parfile,"%s" , comment);
    if (ok) ok = fscanf(parfile,"%d" , mode);
    if (ok) ok = fscanf(parfile,"%d" , ringInj);
    if (ok) ok = fscanf(parfile,"%lf", fH1Ext);
    if (ok) ok = fscanf(parfile,"%d" , nHExt);
    if (ok) ok = fscanf(parfile,"%lf", fH1Inj);
    if (ok) ok = fscanf(parfile,"%d" , nHInj);
    if (ok) ok = fscanf(parfile,"%d" , flagLsa);
    if (ok) ok = fscanf(parfile,"%lf", cTrigExt);
    if (ok) ok = fscanf(parfile,"%lf", cTrigInj);
    if (ok) ok = fscanf(parfile,"%lf", cPhase);
    fclose(parfile);
    if (!ok) die("read config for SID: ", 1);
  } // if parfile
  else printf("can't read parfile - %s!\n", filename);
} // parfileReadSID

void parfileWriteSID(char *filename, char *comment, int mode, int ringInj, double fH1Ext, int nHExt, double fH1Inj, int nHInj, int flagLsa, double cTrigExt, double cTrigInj, double cPhase)
{
  int   ok;
  FILE  *parfile;

  
  // write file with parameters
  if ((parfile = fopen(filename,"w"))) {
    ok         = fprintf(parfile,"%s\n"   , comment);
    if (ok) ok = fprintf(parfile,"%d\n"   , mode);
    if (ok) ok = fprintf(parfile,"%d\n"   , ringInj);
    if (ok) ok = fprintf(parfile,"%.9lf\n", fH1Ext);
    if (ok) ok = fprintf(parfile,"%d\n"   , nHExt);
    if (ok) ok = fprintf(parfile,"%.9lf\n", fH1Inj);
    if (ok) ok = fprintf(parfile,"%d\n"   , nHInj);
    if (ok) ok = fprintf(parfile,"%d\n"   , flagLsa);
    if (ok) ok = fprintf(parfile,"%.3lf\n", cTrigExt);
    if (ok) ok = fprintf(parfile,"%.3lf\n", cTrigInj);
    if (ok) ok = fprintf(parfile,"%.3lf\n", cPhase);
    fclose(parfile);
    
    if (!ok) die("write config for SID: ", 1);
  } // if parfile
} // parfileWriteSID


void parfileWriteDefaultSID(char *filename, uint32_t sid)
{
  char comment[MAXLEN];

  sprintf(comment, "SID-%d", sid);
  parfileWriteSID(filename, comment, 0, 0, 1000000, 1, 1000000, 1, 1, 0, 0, 0);
} // parfileWriteDefaultSID


void submitSid(uint64_t ebDevice, ring_t ring, uint32_t sid)
{
  char     parname[MAXLEN];
  char     comment[MAXLEN];
  int      mode;
  int      ringInj;
  double   fH1Ext;
  int      nHExt;
  double   fH1Inj;
  int      nHInj;
  int      flagLsa;
  double   cTrigExt;
  double   cTrigInj;
  double   cPhase;

  uint32_t gid;
  uint32_t status;
  int      errorFlag;
  

  // get name of par file and gid of extraction ring
  switch (ring) {
    case SIS18 :
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_SIS18SID, sid);
      gid = SIS18_RING;
      break;
    case ESR :
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_ESRSID, sid);
      gid = ESR_RING;
      break;
    case CRYRING :
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_YRSID, sid);
      gid = CRYRING_RING;
      break;
      
    default :
      gid = GID_INVALID;
  } // switch ring
  
  // read file with parameters
  parfileReadSID(parname, comment, &mode, &ringInj, &fH1Ext, &nHExt, &fH1Inj, &nHInj, &flagLsa, &cTrigExt, &cTrigInj, &cPhase);

  // submit parameters to FW
  errorFlag = 0;
  status    = COMMON_STATUS_OK;
  if (!errorFlag)   if ((status = b2b_context_ext_upload(ebDevice, sid, gid, mode, fH1Ext, 1, nHExt, cTrigExt, 1, cPhase, 1, 1)) != COMMON_STATUS_OK) errorFlag = 1;
  if (mode == B2B_MODE_B2C || mode == B2B_MODE_B2BFBEAT || mode == B2B_MODE_B2BPSHIFTE || mode == B2B_MODE_B2BPSHIFTI) 
    if (!errorFlag) if ((status = b2b_context_inj_upload(ebDevice, sid, ringInj, 3, 4, 0x42, fH1Inj, 1, nHInj, cTrigInj, 1)) != COMMON_STATUS_OK) errorFlag = 1;

  if (errorFlag) {
    printf("%s\n", b2b_status_text(status));
    getchar();
  } // if errorFlag
} // submitSid


void menuExpert()
{
  // ivtpar
  int    l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];

  sprintf(txtname, "%s/%s", path, MENUTXT_EXPERT);
  sprintf(parname, "%s/%s", path, MENUPAR_EXPERT);

  ivtpar (txtname, parname, &l0, lchange);
} // menuExpert


void menuIKnob(uint64_t ebDevice, ring_t ring, uint32_t sid, char *sidparname, knob_t knob)
{
  // ivtpar
  int      i, l0, lchange[IVTMAXPAR];
  char     parname[MAXLEN];
  char     txtname[MAXLEN];
  int      done      = 0;

  char     comment[MAXLEN];
  int      mode;
  int      ringInj;
  double   fH1Ext;
  int      nHExt;
  double   fH1Inj;
  int      nHInj;
  int      flagLsa;
  double   cTrigExt;
  double   cTrigInj;
  double   cPhase;

  int      increment = 10;
  double   *ipar     = 0x0;
  char     commentText[MAXLEN];
  char     cDummy[MAXLEN];

  FILE     *parfile;
  int      ok;


  // set point to parameter
  switch (knob) {
    case TRIGEXT :
      ipar = &cTrigExt;
      sprintf(commentText, "extr-kicker");
      break;
    case TRIGINJ :
      ipar = &cTrigInj;
      sprintf(commentText, "inj-kicker");
      break;
    case PHASE :
      ipar = &cPhase;
      sprintf(commentText, "RF-phase");
      break;
    default :
      die("illegal parameter", knob);
  } // switch inum
  
  parfileReadSID(sidparname, comment, &mode, &ringInj, &fH1Ext, &nHExt, &fH1Inj, &nHInj, &flagLsa, &cTrigExt, &cTrigInj, &cPhase);

  switch (ring) {
    case SIS18 :
      sprintf(txtname, "%s/%s", path, MENUTXT_SIS18KNOB);
      sprintf(parname, "%s/%s", path, MENUPAR_SIS18KNOB);
      break;
    case ESR :
      sprintf(txtname, "%s/%s", path, MENUTXT_ESRKNOB);
      sprintf(parname, "%s/%s", path, MENUPAR_ESRKNOB);
      break;      
    case CRYRING :
      sprintf(txtname, "%s/%s", path, MENUTXT_YRKNOB);
      sprintf(parname, "%s/%s", path, MENUPAR_YRKNOB);
      break;      
    default :
      ;
  } // switch ring

  while (!done) {
    // write file with parameters
    if ((parfile = fopen(parname,"w"))) {
      ok         = fprintf(parfile,"%s_%s\n" , comment, commentText);
      if (ok) ok = fprintf(parfile,"%d\n"    , increment);
      if (ok) ok = fprintf(parfile,"%.3f\n"  , *ipar);
      fclose (parfile);
      if (!ok) die("error writing parameter file for knob", ok);
    } // if parfile

    i = ivtpar(txtname, parname, &l0, lchange);

    // read file with parameters
    if ((parfile = fopen(parname,"r"))) {
      ok         = fscanf(parfile,"%s" , cDummy);
      if (ok) ok = fscanf(parfile,"%d" , &increment);
      if (ok) ok = fscanf(parfile,"%lf", ipar);
      fclose (parfile);
      if (!ok) die("error reading parameter file for knob", ok);
    } // if parfile

    
    switch (i) {
      case 1 :
        *ipar += increment;
        break;
      case 2 :
        *ipar -= increment;
        break;
      case 3 :
        help();
        getchar();
        break;
      case 4 :
        done = 1;
        break;
      default :
        ;
    } // switch i
    parfileWriteSID(sidparname, comment, mode, ringInj, fH1Ext, nHExt, fH1Inj, nHInj, flagLsa, cTrigExt, cTrigInj, cPhase);
    submitSid(ebDevice, ring, sid);
  } // while not done
}  // menuIKnob


void menuSID(uint64_t ebDevice, ring_t ring, uint32_t sid)
{
  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];
  int    done=0;

  switch (ring) {
    case SIS18 :
      sprintf(txtname, "%s/%s", path, MENUTXT_SIS18SID);
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_SIS18SID, sid);
      break;
    case ESR :
      sprintf(txtname, "%s/%s", path, MENUTXT_ESRSID);
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_ESRSID, sid);
      break;
    case CRYRING :
      sprintf(txtname, "%s/%s", path, MENUTXT_YRSID);
      sprintf(parname, "%s/%s%d.par", path, MENUPAR_YRSID, sid);
      break;
    default :
      ;
  } // switch ring

  while (!done) {
    i = ivtpar (txtname, parname, &l0, lchange);
    switch (i) {
      case 1 :
        menuIKnob(ebDevice, ring, sid, parname, TRIGEXT);
        break;
      case 2 :
        menuIKnob(ebDevice, ring, sid, parname, TRIGINJ);
        break;
      case 3 :
        menuIKnob(ebDevice, ring, sid, parname, PHASE);
        break;
      case 4 :
        submitSid(ebDevice, ring, sid);
        break;
      case 5 :
        parfileWriteDefaultSID(parname, sid);
        break;
      case 6 :
        help();
        getchar();
        break;
      case 7 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not done
} //menuSID


void menuConfig(uint64_t ebDevice, ring_t ring)
{
  // ivtpar
  int      i, l0, lchange[IVTMAXPAR];
  char     parname[MAXLEN];
  char     txtname[MAXLEN];
  char     parnameSidPrefix[MAXLEN];
  char     parnameSid[MAXLEN+6];  
  int      done=0;

  int      sid;
  
  switch (ring) {
    case SIS18 :
      sprintf(txtname,          "%s/%s", path, MENUTXT_SIS18CONF);
      sprintf(parname,          "%s/%s", path, MENUPAR_SIS18CONF);
      sprintf(parnameSidPrefix, "%s/%s", path, MENUPAR_SIS18SID);          
      break;
    case ESR :
      sprintf(txtname,          "%s/%s", path, MENUTXT_ESRCONF);
      sprintf(parname,          "%s/%s", path, MENUPAR_ESRCONF);
      sprintf(parnameSidPrefix, "%s/%s", path, MENUPAR_ESRSID);          
      break;
    case CRYRING :
      sprintf(txtname,          "%s/%s", path, MENUTXT_YRCONF);
      sprintf(parname,          "%s/%s", path, MENUPAR_YRCONF);
      sprintf(parnameSidPrefix, "%s/%s", path, MENUPAR_YRSID);          
      break;
    default :
      ;
  } // switch ring

  while (!done) {
    i = ivtpar(txtname, parname, &l0, lchange);
    switch (i) {
      case 1 ... 16 :
        menuSID(ebDevice, ring, i-1);
        break;
      case 17 :
        for (sid=0; sid <= 0xf; sid++) submitSid(ebDevice, ring, sid);
        break;
      case 18 :
        for (sid=0; sid <= 0xf; sid++) {
          sprintf(parnameSid, "%s%d.par", parnameSidPrefix, sid);
          parfileWriteDefaultSID(parnameSid, sid);
        } // for sid
        break;
      case 19 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not done
} //menuConfig


void menuMonitor(uint64_t ebDevice, ring_t ring)
{
  int      i;
  int      done=0;
  char     machine[MAXLEN];

  uint32_t sid, gid, mode, nHExt, nHInj;
  uint64_t TH1Ext, TH1Inj, TBeat;
  double   cPhase, cTrigExt, cTrigInj;
  int      j=0;

  static struct termios oldt, newt;
  char ch = 0;
  int  len;

  switch(ring) {
    case SIS18 :
      sprintf(machine, "SIS18");
      break;
    case ESR :
      sprintf(machine, "ESR");
      break;
    case CRYRING :
      sprintf(machine, "CRYRING");
      break;
    default :
      ;
  } // switch ring

  for (i=0; i<40; i++) printf("\n");
  printf("Poor wo*mans B2B monitor for %s. Press any key to return.\n", machine);
  sleep(1);
  
  while (!done) {
    // wait and clear screen
    sleep(1);

    for (i=0; i<40; i++) printf("\n");
    
    // read and display data
    j++;
    printf("%d ...\n\n", j);
    b2b_info_read(ebDevice, &sid, &gid, &mode, &TH1Ext, &nHExt, &TH1Inj, &nHInj, &TBeat, &cPhase, &cTrigExt, &cTrigInj, 1);

    // check for any character....
    // get current terminal settings
    tcgetattr(STDIN_FILENO, &oldt);

    // set non canonical mode
    newt = oldt;
    //newt.c_lflag &= ~(ICANON);
    newt.c_lflag &= ~(ICANON | ECHO); 

    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 0;
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);

    len = read(STDIN_FILENO, &ch, 1);
    if (len) done = 1;
    
    // reset to old terminal settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
  }
} //menuMonitor


void menuRing(ring_t ring)
{
  // ivtpar
  int      i, l0, lchange[IVTMAXPAR];
  char     parname[MAXLEN];
  char     txtname[MAXLEN];
  int      done = 0;

  char     ebDevName[MAXLEN];
  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  uint64_t dummy64a, dummy64b, dummy64c;
  uint32_t dummy32a, dummy32b, dummy32c, dummy32d, dummy32e;
  double   dummy32f, dummy32g, dummy32h;

  // open connection to firmware
  getEbDevice(ring, ebDevName);
  /*printf("ebdev %s\n", ebDevName);
    getchar();*/
  if ((status =  b2b_firmware_open(&ebDevice, ebDevName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  switch(ring) {
    case SIS18 : 
      sprintf(txtname, "%s/%s", path, MENUTXT_SIS18);
      sprintf(parname, "%s/%s", path, MENUPAR_SIS18);
      break;
    case ESR : 
      sprintf(txtname, "%s/%s", path, MENUTXT_ESR);
      sprintf(parname, "%s/%s", path, MENUPAR_ESR);
      break;
    case CRYRING : 
      sprintf(txtname, "%s/%s", path, MENUTXT_YR);
      sprintf(parname, "%s/%s", path, MENUPAR_YR);
      break;
    default :
      ;
  } // switch ring
  
  while (!done) {
    i = ivtpar (txtname, parname, &l0, lchange);
    switch (i) {
      case 1 :
        menuConfig(ebDevice, ring);
        break;
      case 2 :
        menuMonitor(ebDevice, ring);
        break;
      case 3 :
        b2b_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, &dummy32d, &dummy32e, 1);
        b2b_info_read(ebDevice, &dummy32a, &dummy32b, &dummy32c, &dummy64a, &dummy32d, &dummy64b, &dummy32e, &dummy64c, &dummy32f, &dummy32g, &dummy32h, 1);
        getchar();
        break;
      case 4 :
        b2b_cmd_cleardiag(ebDevice);
        break;
      case 5 :
        help();
        getchar();
        break;
      case 6 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not flagDone

  // close connection to firmware
  if ((status = b2b_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);
} // menuSIS18


int main(int argc, char** argv)
{
  // CLI
  int opt, error = 0;
  int exitCode   = 0;

  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];
  
  // local variables
  program = argv[0];    

  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
      case ':':
      case '?':
        error = 1;
      break;
    default:
      return 1;
    } // switch opt
  } // while opt

  if (error) {
    help();
    return 1;
  }

  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <path prefix>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  path = argv[optind];
  sprintf(txtname, "%s/%s", path, MENUTXT_MAIN);
  sprintf(parname, "%s/%s", path, MENUPAR_MAIN);  
  

  // main  menu
  while (1) {
    i = ivtpar(txtname, parname, &l0, lchange);
    
    switch (i) {
      case 1 :
        menuRing(SIS18);
        break;
      case 2 :
        menuRing(ESR);
        break;
      case 3 :
        menuRing(CRYRING);
        break;
      case 4 :
        menuExpert();
        break;
      case 5 :
        exit(0);
      default :
        ;
    } // switch i
  } // while

  return exitCode;
}

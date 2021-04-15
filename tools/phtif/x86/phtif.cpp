/********************************************************************************************
 *  phtif.cpp
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 15-Apr-2021
 *
 *  Poor Humans TIF; this is a quick and dirty hack for the 2021 beam time; configure pulses
 *  for certain Event IDs.
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
 * Last update: 15-April-2021
 *********************************************************************************************/
#define PHTIF_X86_VERSION 0x000001

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>

// common stuff
#include <ivtpar.h>                      // IVTPAR

#define  MENUTXT_MAIN       "phtif_main.txt"
#define  MENUPAR_MAIN       "phtif_main.par"
#define  MENUTXT_RULE       "phtif_rule.txt"
#define  MENUPAR_RULE       "phtif_rule"           // name is completed at run-time

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
  fprintf(stderr, "Use this tool as quick and dirty hack to configure IOs \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %x. Licensed under the LGPL v3.\n", PHTIF_X86_VERSION);
} //help


// apply all rules that are marked as 'active'
void applyActiveRules()
{
} // applyActiveRules


// clear all rules
void clearAllRules()
{
} // clearAllRules


// clear rule
void clearRule(int rule, char *parname)
{
} // clearRule


/*
void getEbDevice(ring_t ring, char *ebDevice)
{
  FILE   *parfile;
  char   parname[MAXLEN];
  char   ebsis[MAXLEN];
  char   ebesr[MAXLEN];

  sprintf(parname, "%s/%s", path, MENUPAR_EXPERT);
  if ((parfile = fopen(parname,"r"))) {
    fscanf(parfile,"%s",ebsis);
    fscanf(parfile,"%s",ebesr);
    fclose(parfile);
    
    if (ring == SIS18) sprintf(ebDevice, "%s", ebsis);
    if (ring == ESR)   sprintf(ebDevice, "%s", ebesr);
  } // if parfile
} // getEbDevice
*/

void parfileReadRule(char *filename, char *comment, char *device, int *io, int *active, int *matchTil, int *fid, int *gid, int *evtno, int *flags, int *sid, int *bpid, int *reserved, int *length, int *offset)
{
  int   ok;
  FILE  *parfile;
  
  // read file with parameters
  if ((parfile = fopen(filename,"r"))) {
    ok         = fscanf(parfile,"%s" , comment);
    if (ok) ok = fscanf(parfile,"%s" , device);
    if (ok) ok = fscanf(parfile,"%d" , io);
    if (ok) ok = fscanf(parfile,"%d" , active);
    if (ok) ok = fscanf(parfile,"%d" , matchTil);
    if (ok) ok = fscanf(parfile,"%d" , fid);
    if (ok) ok = fscanf(parfile,"%d" , gid);
    if (ok) ok = fscanf(parfile,"%d" , evtno);
    if (ok) ok = fscanf(parfile,"%d" , flags);
    if (ok) ok = fscanf(parfile,"%d" , sid);
    if (ok) ok = fscanf(parfile,"%d" , bpid);
    if (ok) ok = fscanf(parfile,"%d" , reserved);
    if (ok) ok = fscanf(parfile,"%d" , length);
    if (ok) ok = fscanf(parfile,"%d" , offset);
    fclose(parfile);
    
    if (!ok) die("read config for ECA rule: ", 1);
  } // if parfile
} // parfileReadRule

void parfileWriteRule(char *filename, char *comment, char *device, int io, int active, int matchTil, int fid, int gid, int evtno, int flags, int sid, int bpid, int reserved, int length, int offset)
{
  int   ok;
  FILE  *parfile;

  
  // write file with parameters
  if ((parfile = fopen(filename,"w"))) {
    ok         = fprintf(parfile,"%s\n" , comment);
    if (ok) ok = fprintf(parfile,"%s\n" , device);
    if (ok) ok = fprintf(parfile,"%d" , io);
    if (ok) ok = fprintf(parfile,"%d" , active);
    if (ok) ok = fprintf(parfile,"%d" , matchTil);
    if (ok) ok = fprintf(parfile,"%d" , fid);
    if (ok) ok = fprintf(parfile,"%d" , gid);
    if (ok) ok = fprintf(parfile,"%d" , evtno);
    if (ok) ok = fprintf(parfile,"%d" , flags);
    if (ok) ok = fprintf(parfile,"%d" , sid);
    if (ok) ok = fprintf(parfile,"%d" , bpid);
    if (ok) ok = fprintf(parfile,"%d" , reserved);
    if (ok) ok = fprintf(parfile,"%d" , length);
    if (ok) ok = fprintf(parfile,"%d" , offset);
    fclose(parfile);
    
    if (!ok) die("write config for ECA rule: ", 1);
  } // if parfile
} // parfileWriteRule


void parfileWriteDefaultRule(char *filename, int rule)
{
  char comment[MAXLEN];
  char device[MAXLEN];

  sprintf(comment, "rule-%d", rule);
  sprintf(device, "tr0");
  parfileWriteRule(filename, comment, device, 1, 0, 1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 1000, 0);
} // parfileWriteDefaultRule


void applyRule(char *parname)
{
  char     comment[MAXLEN];
  char     device[MAXLEN];
  int io;
  int active;
  int matchTil;
  int fid;
  int gid;
  int evtno;
  int flags;
  int sid;
  int bpid;
  int reserved;
  int length;
  int offset;
  
  // read file with parameters
  parfileReadRule(parname, comment, device, &io, &active, &matchTil, &fid, &gid, &evtno, &flags, &sid, &bpid, &reserved, &length, &offset);

  // write rule to ECA
  // bla bla
} // submitRule

/*
void menuExpert()
{
  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];

  sprintf(txtname, "%s/%s", path, MENUTXT_EXPERT);
  sprintf(parname, "%s/%s", path, MENUPAR_EXPERT);

  i = ivtpar (txtname, parname, &l0, lchange);
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
  int      cTrigExt;
  int      cTrigInj;
  int      cPhase;

  int      increment = 10;
  int      *ipar     = 0x0;
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
    default :
      ;
  } // switch ring

  while (!done) {
    // write file with parameters
    if ((parfile = fopen(parname,"w"))) {
      ok         = fprintf(parfile,"%s_%s\n" , comment, commentText);
      if (ok) ok = fprintf(parfile,"%d\n" , increment);
      if (ok) ok = fprintf(parfile,"%d\n" , *ipar);
      fclose (parfile);
      if (!ok) die("error writing parameter file for knob", ok);
    } // if parfile

    i = ivtpar(txtname, parname, &l0, lchange);

    // read file with parameters
    if ((parfile = fopen(parname,"r"))) {
      ok         = fscanf(parfile,"%s" , cDummy);
      if (ok) ok = fscanf(parfile,"%d" , &increment);
      if (ok) ok = fscanf(parfile,"%d" , ipar);
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
*/

void menuRule(int rule)
{
  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];
  int    done=0;

  sprintf(txtname, "%s/%s", path, MENUTXT_RULE);
  sprintf(parname, "%s/%s%d.par", path, MENUPAR_RULE, rule);
  
  while (!done) {
    i = ivtpar (txtname, parname, &l0, lchange);
    switch (i) {
      case 2 :
        applyRule(parname);
        break;
      case 3 :
        clearRule(rule, parname);
        break;
      case 4 :
        help();
        getchar();
        break;
      case 5 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not done
} //menuRule

/*
void menuConfig(uint64_t ebDevice, ring_t ring)
{
  // ivtpar
  int      i, l0, lchange[IVTMAXPAR];
  char     parname[MAXLEN];
  char     txtname[MAXLEN];
  char     parnameSidPrefix[MAXLEN];
  char     parnameSid[MAXLEN];
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
  int32_t  cPhase, cTrigExt, cTrigInj, comLatency;
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
    default :
      ;
  } // switch ring

  while (!done) {
    // wait and clear screen
    sleep(1);

    for (i=0; i<40; i++) printf("\n");
    
    // read and display data
    j++;
    printf("%d ...\n\n", j);
    // chk b2b_info_read(ebDevice, &sid, &gid, &mode, &TH1Ext, &nHExt, &TH1Inj, &nHInj, &TBeat, &cPhase, &cTrigExt, &cTrigInj, &comLatency, 1);

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
  int32_t  dummy32f, dummy32g, dummy32h, dummy32i;

  // open connection to firmware
  getEbDevice(ring, ebDevName);
  // chk printf("ebdev %s\n", ebDevName);
  //  getchar();
  // chk if ((status =  b2b_firmware_open(&ebDevice, ebDevName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);

  switch(ring) {
    case SIS18 : 
      sprintf(txtname, "%s/%s", path, MENUTXT_SIS18);
      sprintf(parname, "%s/%s", path, MENUPAR_SIS18);
      break;
    case ESR : 
      sprintf(txtname, "%s/%s", path, MENUTXT_ESR);
      sprintf(parname, "%s/%s", path, MENUPAR_ESR);
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
        // chk b2b_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, &dummy32d, &dummy32e, 1);
        //  b2b_info_read(ebDevice, &dummy32a, &dummy32b, &dummy32c, &dummy64a, &dummy32d, &dummy64b, &dummy32e, &dummy64c, &dummy32f, &dummy32g, &dummy32h, &dummy32i, 1);
        getchar();
        break;
      case 4 :
        // chk b2b_cmd_cleardiag(ebDevice);
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
  // chk if ((status = b2b_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);
} // menuSIS18
*/

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
      case 1 ... 26 :
        menuRule(i);
      case 27 :
        applyActiveRules();
        break;
      case 28 :
        clearAllRules();
        break;
      case 29 :
        exit(0);
      default :
        ;
    } // switch i
  } // while

  return exitCode;
}

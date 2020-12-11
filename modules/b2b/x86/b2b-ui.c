/********************************************************************************************
 *  b2b-ui.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 11-December-2020
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

#define  MAXLEN              256                   // max string length

const char*  program;

static void die(const char* where, uint32_t  status) {
  fprintf(stderr, "%s: %s failed: %d\n",
          program, where, status);
  exit(1);
} //die


static void help(void)
{
  uint32_t version;

  fprintf(stderr, "Usage: %s [OPTION] \n", program);
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
  char   ebsis[MAXLEN];
  char   ebesr[MAXLEN];

  if ((parfile = fopen(MENUPAR_EXPERT,"r"))) {
    fscanf(parfile,"%s",ebsis);
    fscanf(parfile,"%s",ebesr);
    fclose(parfile);
    
    if (ring == SIS18) sprintf(ebDevice, "%s", ebsis);
    if (ring == ESR)   sprintf(ebDevice, "%s", ebesr);
  } // if parfile
} // getEbDevice


void submitSid(uint64_t ebDevice, ring_t ring, uint32_t sid)
{
  FILE     *parfile;
  char     parname[MAXLEN];
  int      ok;

  char     comment[MAXLEN];
  int      mode;
  int      ringInj;
  double   fH1Ext;
  int      nHExt;
  double   fH1Inj;
  int      nHInj;
  int      cTrigExt;
  int      cTrigInj;
  int      cPhase;

  uint32_t gid;
  uint64_t TH1Ext;
  uint64_t TH1Inj;
  
  // get name of file and gid
  switch (ring) {
    case SIS18 :
      sprintf(parname, "%s%d.par", MENUPAR_SIS18SID, sid);
      gid = 0x3a0;
      break;
    default :
      ;
  } // switch ring

  // read file with parameters
  if ((parfile = fopen(parname,"r"))) {
    ok         = fscanf(parfile,"%s",comment);
    if (ok) ok = fscanf(parfile,"%d",&mode);
    if (ok) ok = fscanf(parfile,"%d",&ringInj);
    if (ok) ok = fscanf(parfile,"%lf",&fH1Ext);
    if (ok) ok = fscanf(parfile,"%d",&nHExt);
    if (ok) ok = fscanf(parfile,"%lf",&fH1Inj);
    if (ok) ok = fscanf(parfile,"%d",&nHInj);
    if (ok) ok = fscanf(parfile,"%d",&cTrigExt);
    if (ok) ok = fscanf(parfile,"%d",&cTrigInj);
    if (ok) ok = fscanf(parfile,"%d",&cPhase);
    fclose(parfile);

    if (!ok) die("read config for SID: ", sid);
  } // if parfile

  /* chk range checking ? */

  // some gymnastics
  if (mode >=  3) gid += ringInj;
  TH1Ext = (double)1000000000000000000.0 / b2b_flsa2fdds(fH1Ext);  // period in attoseconds
  TH1Inj = (double)1000000000000000000.0 / b2b_flsa2fdds(fH1Inj);  // period in attoseconds

  /*
  printf(" sid %d, gid %d\n", sid, gid);
  getchar();
  */
  
  // submit parameters to FW
  b2b_context_upload(ebDevice, sid, gid, mode, TH1Ext, nHExt, TH1Inj, nHInj, cPhase, cTrigExt, cTrigInj);
} // submitSid


void menuExpert()
{
  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];

  i = ivtpar (MENUTXT_EXPERT, MENUPAR_EXPERT, &l0, lchange);
} // menuExpert


void menuSID(uint64_t ebDevice, ring_t ring, uint32_t sid)
{
  // ivtpar
  int    i, l0, lchange[IVTMAXPAR];
  char   parname[MAXLEN];
  char   txtname[MAXLEN];
  int    done=0;

  switch (ring) {
    case SIS18 :
      sprintf(txtname, MENUTXT_SIS18SID);
      sprintf(parname, "%s%d.par", MENUPAR_SIS18SID, sid);
      break;
    default :
      ;
  } // switch ring

  while (!done) {
    i = ivtpar (txtname, parname, &l0, lchange);
    switch (i) {
      case 1 :
        // tune extraction trigger
        break;
      case 2 :
        // tune injection trigger
        break;
      case 3 :
        // tune phase
        break;
      case 4 :
        submitSid(ebDevice, ring, sid);
        break;
      case 5 :
        // help
        break;
      case 6 :
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
  int      done=0;
  
  switch (ring) {
    case SIS18 :
      sprintf(txtname, MENUTXT_SIS18CONF);
      sprintf(parname, MENUPAR_SIS18CONF);
      break;
    default :
      ;
  } // switch ring

  while (!done) {
    i = ivtpar (txtname, parname, &l0, lchange);
    switch (i) {
      case 1 ... 16 :
        menuSID(ebDevice, ring, i-1);
        break;
      case 17 :
        // submit all
        break;
      case 18 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not done
} //menuSID


void menuSIS18()
{
  // ivtpar
  int      i, l0, lchange[IVTMAXPAR];
  int      done = 0;

  char     ebDevName[MAXLEN];
  uint64_t ebDevice;
  uint32_t cpu;
  uint32_t status;

  uint64_t dummy64a, dummy64b, dummy64c;
  uint32_t dummy32a, dummy32b, dummy32c, dummy32d, dummy32e;
  int32_t  dummy32f, dummy32g, dummy32h, dummy32i;

  // open connection to firmware
  getEbDevice(SIS18, ebDevName);
  if ((status =  b2b_firmware_open(&ebDevice, ebDevName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);
  
  while (!done) {
    i = ivtpar (MENUTXT_SIS18, MENUPAR_SIS18, &l0, lchange);
    switch (i) {
      case 1 :
        menuConfig(ebDevice, SIS18);
        break;
      case 2 :
        //monitor
        break;
      case 3 :
        b2b_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, &dummy32d, &dummy32e, 1);
        b2b_info_read(ebDevice, &dummy32a, &dummy32b, &dummy32c, &dummy64a, &dummy32d, &dummy64b, &dummy32e, &dummy64c, &dummy32f, &dummy32g, &dummy32h, &dummy32i, 1);
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
  char   par0[32];
  double par1;
  FILE   *parfile;

  int  j;
  int  ok;
  
  // local variables
  uint32_t status;
  ring_t   ring;

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

  // main  menu
  while (1) {
    i = ivtpar (MENUTXT_MAIN, MENUPAR_MAIN, &l0, lchange);
    
    switch (i) {
      case 1 :
        menuSIS18();
        break;
      case 2 :
        break;
      case 3 :
        menuExpert();
        break;
      case 4 :
        exit(0);
      default :
        ;
    } // switch i
  } // while
  /*
      
    // demo: quick analysis
  printf("ivtpar returned:\n");
  printf("-- menu item selected   : %d\n", i);
  if (l0) {
    printf("-- changes of parameters:\n");
    for (j=0; j<100; j++) if (lchange[j]) printf("-- parameter %d changed...\n", j);
  } // if l0

  // demo: read parameters
  if ((parfile = fopen(parname,"r"))) {
    ok            = fscanf(parfile,"%s",par0);
    if (ok==1) ok = fscanf(parfile,"%le",&par1);
    // if (ok==1) .... other parameters
    fclose(parfile);
  } // if parfile
  else ok = 0;
  if (!ok) {printf("reading parameter file failed\n"); exit(1);}

  printf("parameter 0 has value '%s', parameter 1 has value '%le'\n", par0, par1);
  }
  */

  return exitCode;
}

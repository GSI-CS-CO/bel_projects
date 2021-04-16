/********************************************************************************************
 *  phtif.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 16-Apr-2021
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
#define  NRULES              26

const char*  program;
const char*  path;


static void die(const char* where, uint32_t  status) {
  fprintf(stderr, "%s: %s failed: %d\n",
          program, where, status);
  exit(1);
} //die


static void help(void)
{
  fprintf(stderr, "Usage: %s [OPTION] <path prefix>  \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool as quick and dirty hack to configure IOs \n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %6x. Licensed under the LGPL v3.\n", PHTIF_X86_VERSION);
} //help


// execute linux command
void executeCommand(char *cmd, char *output)
{
  FILE *pipe;
  char rbuff[1024];
  char wbuff[1024];
  char *tmp;

  // clear output
  output[0] = '\0';

  // catch stderr by redirection to stdin
  sprintf(wbuff, "%s 2>&1", cmd);
  
  pipe = popen(wbuff, "r");
  if (pipe == NULL) die(cmd, 1);

  tmp = output;
  while(fgets(rbuff, sizeof(rbuff), pipe) != NULL) {strcpy(tmp, rbuff); tmp += strlen(rbuff);}
} // executeCommand


void parfileReadRule(char *filename, char *comment, char *device, int *io, int *enabled, int *matchTil, int *fid, int *gid, int *evtno, int *flags, int *sid, int *bpid, int *reserved, int *length, int *offset)
{
  int   ok;
  FILE  *parfile;
  
  // read file with parameters
  if ((parfile = fopen(filename,"r"))) {
    ok         = fscanf(parfile,"%s" , comment);
    if (ok) ok = fscanf(parfile,"%s" , device);
    if (ok) ok = fscanf(parfile,"%i" , io);
    if (ok) ok = fscanf(parfile,"%i" , enabled);
    if (ok) ok = fscanf(parfile,"%i" , matchTil);
    if (ok) ok = fscanf(parfile,"%i" , fid);
    if (ok) ok = fscanf(parfile,"%i" , gid);
    if (ok) ok = fscanf(parfile,"%i" , evtno);
    if (ok) ok = fscanf(parfile,"%i" , flags);
    if (ok) ok = fscanf(parfile,"%i" , sid);
    if (ok) ok = fscanf(parfile,"%i" , bpid);
    if (ok) ok = fscanf(parfile,"%i" , reserved);
    if (ok) ok = fscanf(parfile,"%i" , length);
    if (ok) ok = fscanf(parfile,"%i" , offset);
    fclose(parfile);
    
    if (!ok) die("read config for ECA rule: ", 1);
  } // if parfile
} // parfileReadRule


void parfileWriteRule(char *filename, char *comment, char *device, int io, int enabled, int matchTil, int fid, int gid, int evtno, int flags, int sid, int bpid, int reserved, int length, int offset)
{
  int   ok;
  FILE  *parfile;

  
  // write file with parameters
  if ((parfile = fopen(filename,"w"))) {
    ok         = fprintf(parfile,"%s\n" , comment);
    if (ok) ok = fprintf(parfile,"%s\n" , device);
    if (ok) ok = fprintf(parfile,"%d\n" , io);
    if (ok) ok = fprintf(parfile,"%d\n" , enabled);
    if (ok) ok = fprintf(parfile,"%d\n" , matchTil);
    if (ok) ok = fprintf(parfile,"%d\n" , fid);
    if (ok) ok = fprintf(parfile,"%d\n" , gid);
    if (ok) ok = fprintf(parfile,"%d\n" , evtno);
    if (ok) ok = fprintf(parfile,"%d\n" , flags);
    if (ok) ok = fprintf(parfile,"%d\n" , sid);
    if (ok) ok = fprintf(parfile,"%d\n" , bpid);
    if (ok) ok = fprintf(parfile,"%d\n" , reserved);
    if (ok) ok = fprintf(parfile,"%d\n" , length);
    if (ok) ok = fprintf(parfile,"%d\n" , offset);
    fclose(parfile);
    
    if (!ok) die("write config for ECA rule: ", 1);
  } // if parfile
} // parfileWriteRule


void parfileWriteDefaultRule(char *parname, int rule)
{
  char comment[MAXLEN];
  char device[MAXLEN];

  sprintf(comment, "rule-%d", rule);
  sprintf(device, "tr0");
  parfileWriteRule(parname, comment, device, 1, 0, 0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 1000, 0);
} // parfileWriteDefaultRule


// creates conditions for a rule
void cmdCreateCondition(char *parname)
{
  char comment[MAXLEN];
  char device[MAXLEN];
  int  io;
  int  enabled;
  int  matchTil;
  int  fid;
  int  gid;
  int  evtno;
  int  flags;
  int  sid;
  int  bpid;
  int  reserved;
  int  length;
  int  offset;

  char cmd[1024];
  char out[1024];

  uint64_t evtId;
  uint64_t mask;
  char negFlag[32];
  
  // read file with parameters
  parfileReadRule(parname, comment, device, &io, &enabled, &matchTil, &fid, &gid, &evtno, &flags, &sid, &bpid, &reserved, &length, &offset);

  if (!enabled) return;

  printf("phtif: configure ECA of device %s, for IO%d\n", device, io);

  // configure output
  // build and execute command
  sprintf(cmd, "saft-io-ctl %s -n IO%d -o1 -t0 -a1", device, io);
  executeCommand(cmd, out);
  if (strlen(out) > 1) {
    printf("result:\n%s\npress RETURN to continue\n", out); 
    getchar();
  } // if strlen
  
  // evtID and mask
  switch (matchTil) {
    case 0 :
      mask   = 0xf000000000000000;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      break;
    case 1 :
      mask   = 0xffff000000000000;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      break;
    case 2 :
      mask   = 0xfffffff000000000;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
      break;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
    case 3 :
      mask   = 0xffffffff00000000;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
      evtId |= ((uint64_t)(flags & 0xf))    << 32;   
      break;
    case 4 :
      mask   = 0xfffffffffff00000;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
      evtId |= ((uint64_t)(flags & 0xf))    << 32;   
      evtId |= ((uint64_t)(sid   & 0xfff))  << 20;   
      break;
    case 5 :
      mask   = 0xffffffffffffffc0;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
      evtId |= ((uint64_t)(flags & 0xf))    << 32;   
      evtId |= ((uint64_t)(sid   & 0xfff))  << 20;   
      evtId |= ((uint64_t)(bpid  & 0x3fff)) <<  6;   
      break;
    case 6 :
      mask   = 0xffffffffffffffff;
      evtId  = ((uint64_t)(fid   & 0xf))    << 60;
      evtId |= ((uint64_t)(gid   & 0xfff))  << 48;
      evtId |= ((uint64_t)(evtno & 0xfff))  << 36;   
      evtId |= ((uint64_t)(flags & 0xf))    << 32;   
      evtId |= ((uint64_t)(sid   & 0xfff))  << 20;   
      evtId |= ((uint64_t)(bpid  & 0x3fff)) <<  6;   
      evtId |= ((uint64_t)(reserved & 0x3f));   
      break;

    default :
      die("illegal value; 'match til' must be within 0..6", 1);
      break;
  } // switch matchTil

  // configure ECA for rising edge
  // flag for negativ offset
  if (offset < 0) sprintf(negFlag, "-g");
  else            sprintf(negFlag, "");

  sprintf(cmd, "saft-io-ctl %s -n IO%d -c 0x%lx 0x%lx %d 0x0 1 -u %s", device, io, evtId, mask, abs(offset), negFlag);
  executeCommand(cmd, out);
  if (strlen(out) > 1) {
    printf("result:\n%s\npress RETURN to continue\n", out); 
    getchar();
  } // if strlen

  // configure ECA for falling edge
  // flag for negativ offset
  if ((offset + length) < 0) sprintf(negFlag, "-g");
  else                       sprintf(negFlag, "");

  sprintf(cmd, "saft-io-ctl %s -n IO%d -c 0x%lx 0x%lx %d 0x0 0 -u %s", device, io, evtId, mask, abs(offset+length), negFlag);
  executeCommand(cmd, out);
  if (strlen(out) > 1) {
    printf("result:\n%s\npress RETURN to continue\n", out); 
    getchar();
  } // if strlen
  
} // cmdCreateConditions


// destroys all ECA rules for the IO channel
void cmdDestroyAllConditions()
{
  char cmd[1024];
  char out[1024];

  // tr0
  printf("destroy all IO conditions for device tr0\n");
  sprintf(cmd, "saft-io-ctl tr0 -x");
  executeCommand(cmd, out);

  // tr1
  printf("destroy all IO conditions for device tr1\n");
  sprintf(cmd, "saft-io-ctl tr1 -x");
  executeCommand(cmd, out);
} // cmdDestroyAllConditions


// list all ECA rules
void cmdListAllConditions()
{
  char cmd[1024];
  char out[2048];

  // tr0
  sprintf(cmd, "saft-io-ctl tr0 -l");
  executeCommand(cmd, out);
  if (strlen(out) > 1) printf("%s\n", out); 

  // tr1
  sprintf(cmd, "saft-io-ctl tr1 -l");
  executeCommand(cmd, out);
  if (strstr(out, "tr1 does not exist") == NULL) {
    if (strlen(out) > 1) printf("%s\n", out);
  } // if strstr: avoid error message in case device 'tr1 does not exist'
} // cmdListAllConditions


// creates conditions for all enabled rules
void cmdCreateAllConditions()
{
  char parname[MAXLEN];
  int  i;

  for (i=1; i<=NRULES; i++) {
    sprintf(parname, "%s/%s%d.par", path, MENUPAR_RULE, i);
    cmdCreateCondition(parname);
  } // for i
} // cmdCreateAllConditions


// clear all rules
void clearAllRules()
{
  char parname[MAXLEN];
  int  i;

  for (i=1; i<=NRULES; i++) {
    sprintf(parname, "%s/%s%d.par", path, MENUPAR_RULE, i);
    parfileWriteDefaultRule(parname, i);
  } // for i
} // clearAllRules


// menu for rule configuration
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
      case 1 :
        parfileWriteDefaultRule(parname, rule);
        break;
      case 2 :
        done = 1;
        break;
      default :
        ;
    } // switch i
  } // while not done
} //menuRule


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
        break;
      case 27 :
        cmdDestroyAllConditions();
        cmdCreateAllConditions();
        break;
      case 28 :
        clearAllRules();
        break;
      case 29 :
        cmdListAllConditions();
        printf("press RETURN to continue\n");
        getchar();
        break;
      case 30 :
        exit(0);
        break;
      default :
        ;
    } // switch i
  } // while

  return exitCode;
}

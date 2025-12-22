/*******************************************************************************************
 *  wrmil-client-mon.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-Apr-2025
 *
 * subscribes to and displays status of a wr-mil gateway
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
#define WRMIL_CLIENT_MON_VERSION 0x000107

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

// wrmil
#include <common-lib.h>                  // COMMON
#include <wrmillib.h>                    // API
#include <wr-mil.h>                      // FW

const char* program;

#define WRMILNSYS   10                   // number of WRMIL systems

#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names
#define SCREENWIDTH 1024                 // width of screen

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

char     disWrmilPrefix[DIMMAXSIZE];

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     header[SCREENWIDTH+1];                             // header line to be printed
char     empty[SCREENWIDTH+1];                              // an empty line

const char * sysShortNames[] = {
  "pzu_qr",
  "pzu_ql",
  "pzu_qn",
  "pzu_un",
  "pzu_uh",
  "pzu_at",
  "pzu_tk",
  "pzu_f50",
  "sis18_ring",
  "esr_ring"
};

struct wrmilGw_t {
  char        version[DIMCHARSIZE];
  char        state[DIMCHARSIZE];
  char        hostname[DIMCHARSIZE];
  uint64_t    status;
  monval_t    monData;

  uint32_t    versionId;
  uint32_t    stateId;
  uint32_t    hostnameId;
  uint32_t    statusId;
  uint32_t    monDataId;
}; // struct wrmilGw_t

struct wrmilGw_t dicSystem[WRMILNSYS];


// help
static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] [ENVIRONMENT]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display system information on the WRMIL system\n");
  fprintf(stderr, "Example1: '%s -s pro'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", wrmil_version_text(WRMIL_CLIENT_MON_VERSION));
} //help


void buildHeader(char * environment)
{
  sprintf(title, "\033[7m WRMIL System Status %3s ------------------------------------------------------------------------------------------------ (units [us] unless explicitly given) -  v%8s\033[0m", environment, wrmil_version_text(WRMIL_CLIENT_MON_VERSION));
  sprintf(header, "  # MIL domain  version      state        status     #sent/fw    #missd/fw      #err/fw    #burst/fw   #match/x86  r[%%] mode     ave    sdev      min      max         node");    
  sprintf(empty , "                                                                                                                                                                            ");
  //       printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234\n");  
} // buildHeader


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<WRMILNSYS; i++) {
    sprintf(name, "%s_%s-mon_version_fw", prefix, sysShortNames[i]);
    dicSystem[i].versionId   = dic_info_service(name, MONITORED, 0, (dicSystem[i].version), DIMCHARSIZE, 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s-mon_state", prefix, sysShortNames[i]);
    dicSystem[i].stateId     = dic_info_service(name, MONITORED, 0, (dicSystem[i].state), DIMCHARSIZE, 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s-mon_hostname", prefix, sysShortNames[i]);
    dicSystem[i].hostnameId  = dic_info_service(name, MONITORED, 0, (dicSystem[i].hostname), DIMCHARSIZE, 0, 0, &no_link_32, sizeof(no_link_32));
    sprintf(name, "%s_%s-mon_status", prefix, sysShortNames[i]);
    dicSystem[i].statusId    = dic_info_service(name, MONITORED, 0, &(dicSystem[i].status), sizeof(uint64_t), 0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s-mon_data", prefix, sysShortNames[i]);
    dicSystem[i].monDataId    = dic_info_service(name, MONITORED, 0, &(dicSystem[i].monData), sizeof(dicSystem[i].monData), 0, 0, &no_link_32, sizeof(no_link_32));   
  } // for i
} // dicSubscribeServices


// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t indexServer)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s_%s-mon_cmd_cleardiag", prefix, sysShortNames[indexServer]);
  dic_cmnd_service(name, 0, 0);
} // dicCmdClearDiag


// print services to screen
void printServices()
{
  int i;

  char     cStatus[17];
  char     cVersion[9];
  char     cState[11];
  char     cHost[19];
  char     cData[512];
  char     cNFwSnd[24];
  char     cNFwMssd[24];
  char     cNFwErr[24];
  char     cNFwBrst[24];
  char     cNMatch[24];
  char     cRMatch[24];
  char     cMMatch[24];
  char     cTAve[24];
  char     cTSdev[24];
  char     cTMin[24];
  char     cTMax[24];
  char     buff[100];
  time_t   time_date;
  uint32_t *tmp;

  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");

  // footer with date and time
  time_date = time(0);
  strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
  sprintf(footer, "\033[7m exit <q> | clear status <digit> | print status <s> | help <h>                                                                                              %s\033[0m", buff);
  
  comlib_term_curpos(1,1);
  
  printf("%s\n", title);
  printf("%s\n", header);
  for (i=0; i<WRMILNSYS; i++) {
    if (dicSystem[i].status    == no_link_64)    sprintf(cStatus,  "%13s",         no_link_str);
    else                                         sprintf(cStatus,  "%13"PRIx64"",  dicSystem[i].status);
    tmp = (uint32_t *)(dicSystem[i].state);
    if (*tmp == no_link_32)                      sprintf(cState,   "%10s",         no_link_str);
    else                                         sprintf(cState,   "%10s",         dicSystem[i].state); 
    tmp = (uint32_t *)(dicSystem[i].version);
    if (*tmp == no_link_32)                      sprintf(cVersion, "%8s",          no_link_str);
    else                                         sprintf(cVersion, "%8s",          dicSystem[i].version); 
    tmp = (uint32_t *)(dicSystem[i].hostname);
    if (*tmp == no_link_32)                      sprintf(cHost,   "%12s",          no_link_str);
    else                                         sprintf(cHost,   "%12s",          dicSystem[i].hostname);
    tmp = (uint32_t *)(&(dicSystem[i].monData));
    if (*tmp == no_link_32)                      sprintf(cData,   "%96s",          no_link_str);
    else  {
      if (dicSystem[i].monData.nFwSnd  == -1)    sprintf(cNFwSnd, "%12s" , "nan");
      else                                       sprintf(cNFwSnd, "%12lu", dicSystem[i].monData.nFwSnd);
      if (dicSystem[i].monData.nFwRecT == -1)    sprintf(cNFwMssd,"%12s" , "nan");
      else                                       sprintf(cNFwMssd,"%12ld", (int64_t)(dicSystem[i].monData.nFwSnd - dicSystem[i].monData.nFwRecT));
      if (dicSystem[i].monData.nFwRecErr == -1)  sprintf(cNFwErr, "%12s" , "nan");
      else                                       sprintf(cNFwErr, "%12u" , dicSystem[i].monData.nFwRecErr);
      if (dicSystem[i].monData.nFwBurst  == -1)  sprintf(cNFwBrst,"%12s" , "nan");
      else                                       sprintf(cNFwBrst,"%12u" , dicSystem[i].monData.nFwBurst);
      if (dicSystem[i].monData.nMatch  == -1)    sprintf(cNMatch, "%12s" , "nan");        
      else                                       sprintf(cNMatch, "%12lu", dicSystem[i].monData.nMatch);
      sprintf(cRMatch, "%5.1f",  100.0 * dicSystem[i].monData.nMatch / dicSystem[i].monData.nStart);
      sprintf(cMMatch, "%4d"   ,  dicSystem[i].monData.cMode);
      if (isnan(dicSystem[i].monData.tAve))      sprintf(cTAve,   "%7s"  , "nan");        
      else                                       sprintf(cTAve,   "%7.3f", dicSystem[i].monData.tAve);
      if (isnan(dicSystem[i].monData.tSdev))     sprintf(cTSdev,  "%7s"  , "nan");        
      else                                       sprintf(cTSdev,  "%7.3f", dicSystem[i].monData.tSdev);
      if (isnan(dicSystem[i].monData.tMin))      sprintf(cTMin,   "%7s"  , "nan");        
      else                                       sprintf(cTMin,   "%8.3f", dicSystem[i].monData.tMin);
      if (isnan(dicSystem[i].monData.tMax))      sprintf(cTMax,   "%7s"  , "nan");        
      else                                       sprintf(cTMax,   "%8.3f", dicSystem[i].monData.tMax);
      sprintf(cData, "%12s %12s %12s %12s %12s %5s %4s %7s %7s %8s %8s",  cNFwSnd, cNFwMssd, cNFwErr, cNFwBrst, cNMatch, cRMatch, cMMatch, cTAve, cTSdev, cTMin, cTMax);
    } // else nolink
    printf(" %2x %10s %8s %10s %13s %109s %12s\n", i, sysShortNames[i], cVersion, cState, cStatus, cData, cHost);
  } // for i

  for (i=0; i<10; i++) printf("%s\n", empty);
  printf("%s\n", footer);
} // printServices


// print status text to screen
void printStatusText()
{
  int i,j;
  uint64_t status;

  for (i=0; i<WRMILNSYS; i++) {
    status = dicSystem[i].status;
    if ((status != 0x1) && (status != no_link_64)) {
      printf(" %12s:\n", sysShortNames[i]);
      for (j = COMMON_STATUS_OK + 1; j<(int)(sizeof(status)*8); j++) {
        if ((status >> j) & 0x1)  printf("  ---------- status bit is set : %s\n", wrmil_status_text(j));
      } // for j
    } // if status
  } // for i
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printStatusText


// print help text to screen
void printHelpText()
{
  int i;

  comlib_term_curpos(1,1);
  printf("%s\n", title);
  
  for (i=0; i<WRMILNSYS; i++) printf("%s\n", empty);
  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("please visit the following URL                                                  \n");
  printf("https://wiki.gsi.de/TOS/Timing/TimingSystemHowWrMil#Monitoring                  \n");
  printf("%s\n", empty);
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printHelpText


int main(int argc, char** argv) {
  int opt, error = 0;
  int exitCode   = 0;
  //  char *tail;

  int      getVersion;

  char     userInput;
  int      sysId;
  int      quit;

  char     prefix[DIMMAXSIZE];
  char     environment[132];

  program    = argv[0];
  getVersion = 0;
  quit       = 0;

  while ((opt = getopt(argc, argv, "eh")) != -1) {
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
    } /* switch opt */
  } /* while opt */

  if (error) {
    help();
    return 1;
  }

  // no parameters, no command: just display help and exit
  if ((optind == 1) && (argc == 1)) {
    help();
    return 0;
  } // if optind

  if (optind< argc) {
    sprintf(environment, "%s", argv[optind]);
    sprintf(prefix, "wrmil_%s", environment);
  } // if optind
  else {
    sprintf(environment, "%s", "n/a");
    sprintf(prefix, "wrmil");
  } // else optind

  comlib_term_clear();
  buildHeader(environment);
  if (getVersion) printf("%s: version %s\n", program, wrmil_version_text(WRMIL_CLIENT_MON_VERSION));

  printf("wrmil-client-mon: starting client using prefix %s\n", prefix);
  sleep(1);
  dicSubscribeServices(prefix);

  while (!quit) {
    printServices();
    if (!quit) {
      sysId = 0xffff;
      userInput = comlib_term_getChar();
      switch (userInput) {
        case 'a' ... 'f' :
          sysId = userInput - 87;                 // no break on purpose
        case '0' ... '9' :
          if (sysId == 0xffff) sysId = userInput - 48; // ugly
          dicCmdClearDiag(prefix, sysId);
          break;
        case 'h'         :
          printHelpText();
          break;
        case 'q'         :
          quit = 1;
          break;
        case 's'         :
          printStatusText();
          break;
        default          :
          usleep(1000000);
      } // switch
    } // if !quit
  } // while

  return exitCode;
}

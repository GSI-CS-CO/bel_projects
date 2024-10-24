/*******************************************************************************************
 *  unichop-client-mon.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 24-Oct-2024
 *
 * subscribes to and displays status of a uni-chop firmware
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
#define UNICHOP_CLIENT_MON_VERSION 0x000007

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

// unichop
#include <common-lib.h>                  // COMMON
#include <unichoplib.h>                  // API
#include <uni-chop.h>                    // FW

const char* program;

#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names
#define SCREENWIDTH 1024                 // width of screen

#define  TOLD       3600                 // [s]; if the previous data is more in the past than this value, the data is considered out of date

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

char     disUnichopPrefix[DIMMAXSIZE];

char     title[SCREENWIDTH+1];                              // title line to be printed
char     footer[SCREENWIDTH+1];                             // footer line to be printed
char     header[SCREENWIDTH+1];                             // header line to be printed
char     empty[SCREENWIDTH+1];                              // an empty line

const char * sysShortNames[] = {
  "hli",
  "hsi"
};

char        dicVersion[DIMCHARSIZE];
char        dicState[DIMCHARSIZE];
char        dicHostname[DIMCHARSIZE];
uint64_t    dicStatus;
//monval_t    dicMonDataHLI[UNICHOP_NSID];
//monval_t    dicMonDataHSI[UNICHOP_NSID];

uint32_t    dicVersionId;
uint32_t    dicStateId;
uint32_t    dicHostnameId;
uint32_t    dicStatusId;
uint32_t    dicMonDataHLIId[UNICHOP_NSID];
uint32_t    dicMonDataHSIId[UNICHOP_NSID];

monData_t   joinedMonData[UNICHOP_NSID];                    // a virtacc can be either HSI or HLI - thus, the data is joined into one set of virtaccs
time_t      monData_secs[UNICHOP_NSID];                     // UTC seconds
uint32_t    monData_msecs[UNICHOP_NSID];                    // UTC milliseconds
 

// help
static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] [ENVIRONMENT]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display UNILAC chopper status\n");
  fprintf(stderr, "Example1: '%s -s pro'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", unichop_version_text(UNICHOP_CLIENT_MON_VERSION));
} //help


void buildHeader(char * environment)
{
  sprintf(title, "\033[7m UNICHOP System Status %3s ----------------------------------------- (units [us] unless explicitly given) -  v%8s\033[0m", environment, unichop_version_text(UNICHOP_CLIENT_MON_VERSION));
  sprintf(header, " SID what      exe_time    #cycles         #ok #no_trigger   #no_pulse | lenTrig    tChop lenChop\n");    
  sprintf(empty , "                                                                                                                                                                            ");
  //       printf("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234\n");  
} // buildHeader


// receive monitoring data
void recMonData(long *tag, monData_t *address, int *size)
{
  uint32_t sid;
  uint32_t secs, msecs;
  

  sid = *tag;
  if ((sid < 0) || (sid >= UNICHOP_NSID)) return;

  if (*size == sizeof(monData_t)) {
    joinedMonData[sid] = *address;
    dic_get_timestamp(0, &secs, &msecs);
    monData_secs[sid]  = (time_t)secs;
    monData_msecs[sid] = msecs;
  } // if proper size
  else {
    monData_msecs[sid] = 0;
    monData_secs[sid]  = 0;
  } // else proper size 
} // recSetValue


// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];

  int i;

  sprintf(name, "%s_mon_version_fw", prefix);
  dicVersionId   = dic_info_service(name, MONITORED, 0, dicVersion,  DIMCHARSIZE,      0, 0, &no_link_32, sizeof(no_link_32));
  sprintf(name, "%s_mon_state", prefix);
  dicStateId     = dic_info_service(name, MONITORED, 0, dicState,    DIMCHARSIZE,      0, 0, &no_link_32, sizeof(no_link_32));
  sprintf(name, "%s_mon_hostname", prefix);
  dicHostnameId  = dic_info_service(name, MONITORED, 0, dicHostname, DIMCHARSIZE,      0, 0, &no_link_32, sizeof(no_link_32));
  sprintf(name, "%s_mon_status", prefix);
  dicStatusId    = dic_info_service(name, MONITORED, 0, &dicStatus,  sizeof(uint64_t), 0, 0, &no_link_64, sizeof(no_link_64));

  
  for (i=0; i<UNICHOP_NSID; i++) {
    // HLI
    sprintf(name, "%s_hli-data_sid%02d", prefix, i);
    dicMonDataHLIId[i] = dic_info_service(name, MONITORED, 0, 0, recMonData, i, &no_link_32, sizeof(no_link_32));

    // HSI
    sprintf(name, "%s_hsi-data_sid%02d", prefix, i);
    dicMonDataHSIId[i] = dic_info_service(name, MONITORED, 0, 0, recMonData, i, &no_link_32, sizeof(no_link_32));
  } // for i
          
} // dicSubscribeServices


// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t sid)
{
  char name[DIMMAXSIZE];
  static int data;

  data = sid;

  sprintf(name, "%s_mon_cmd_cleardiag", prefix);
  dic_cmnd_service(name, &data, sizeof(data));
} // dicCmdClearDiag


// print services to screen
void printServices()
{
  int i;

  time_t   time_date;
  uint64_t actNsecs;
  time_t   actT;


  char     cChopT[64];
  char     cWhat[32];
  char     cNCycles[24];
  char     cNOk[24];
  char     cNTriggerFails[24];
  char     cNChopperFails[24];
  char     cTriggerLen[24];
  char     cChopperT[24];
  char     cChopperLen[24];

  char     tmp1[32];
  char     buff[100];

  
  /*

  
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
  uint32_t *tmp;
  */

  actNsecs  = comlib_getSysTime();
  actT      = (time_t)(actNsecs / 1000000000);

  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");

  // footer with date and time
  time_date = time(0);
  strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
  sprintf(footer, "\033[7m exit <q> | clear status <digit> | help <h>                                                        %s\033[0m", buff);

  comlib_term_curpos(1,1);
  
  printf("%s\n", title);
  printf("%s\n", header);

  for (i=0; i<UNICHOP_NSID; i++) {
    if (monData_secs[i] == 0)                         printf(" %3d NOLINK                                                                          |                         \n", i);
    else if ((actT - monData_secs[i]) > (time_t)TOLD) printf(" %3d out of date                                                                     |                         \n", i);
    else {
      // what
      if (joinedMonData[i].machine == tagHLI) sprintf(cWhat, "HLI");
      else                                    sprintf(cWhat, "HSI");
      
      // execution time
      strftime(tmp1, 10, "%H:%M:%S", gmtime(&(monData_secs[i])));
      sprintf(cChopT, "%8s.%03d", tmp1, monData_msecs[i]);

      // # of cycles
      sprintf(cNCycles,        "%10d", joinedMonData[i].cyclesN);

      // # of succesful executions
      sprintf(cNOk,            "%10d", joinedMonData[i].pulseStopN);

      // # of trigger failures
      sprintf(cNTriggerFails,  "%10d", joinedMonData[i].triggerErrN);

      // # of choopper failures
      sprintf(cNChopperFails,  "%10d", joinedMonData[i].pulseStopErrN);
      
      // trigger length
      if (!joinedMonData[i].triggerFlag)    sprintf(cTriggerLen, "%7d", joinedMonData[i].triggerLen);
      else                                  sprintf(cTriggerLen, "    ---");

      // chopper time
      if (!joinedMonData[i].pulseStartFlag) sprintf(cChopperT  , "%7d", joinedMonData[i].pulseStartT);
      else                                  sprintf(cChopperT  , "    ---");

      // chopper length
      if (!joinedMonData[i].pulseStopFlag)  sprintf(cChopperLen, "%7d", joinedMonData[i].pulseLen);
      else                                  sprintf(cChopperLen, "    ---");

      printf(" %3x %5s %12s %10s %10s %10s | %7s %7s %7s\n", i, cWhat, cChopT, cNOk, cNTriggerFails, cNChopperFails, cTriggerLen, cChopperT, cChopperLen);

    } // else: data available
  } // for i

  for (i=0; i<10; i++) printf("%s\n", empty);
  printf("%s\n", footer);
} // printServices


/*
// print status text to screen
void printStatusText()
{
  int i,j;
  uint64_t status;

  status = dicSystem.status;
  if ((status != 0x1) && (status != no_link_64)) {
    printf(" %12s:\n", sysShortNames[i]);
    for (j = COMMON_STATUS_OK + 1; j<(int)(sizeof(status)*8); j++) {
      if ((status >> j) & 0x1)  printf("  ---------- status bit is set : %s\n", unichop_status_text(j));
    } // for j
  } // if status
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printStatusText
*/

// print help text to screen
void printHelpText()
{
  comlib_term_curpos(1,1);
  printf("%s\n", title);
  
  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  printf("please visit the following URL                                                  \n");
  printf("https://www-acc.gsi.de/wiki                                                     \n");
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
  int      sid;
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
    sprintf(prefix, "unichop_%s", environment);
  } // if optind
  else {
    sprintf(environment, "%s", "n/a");
    sprintf(prefix, "unichop");
  } // else optind

  comlib_term_clear();
  buildHeader(environment);
  if (getVersion) printf("%s: version %s\n", program, unichop_version_text(UNICHOP_CLIENT_MON_VERSION));

  printf("unichop-client-mon: starting client using prefix %s\n", prefix);
  sleep(1);
  dicSubscribeServices(prefix);

  while (!quit) {
    printServices();
    if (!quit) {
      sid = 0xffff;
      userInput = comlib_term_getChar();
      switch (userInput) {
        case 'a' ... 'f' :
          sid = userInput - 87;                 // no break on purpose
        case '0' ... '9' :
          if (sid == 0xffff) sid = userInput - 48; // ugly
          dicCmdClearDiag(prefix, (uint32_t)sid);
          break;
        case 'h'         :
          printHelpText();
          break;
        case 'q'         :
          quit = 1;
          break;
        case 's'         :
          //printStatusText();
          break;
        default          :
          usleep(1000000);
      } // switch
    } // if !quit
  } // while

  return exitCode;
}

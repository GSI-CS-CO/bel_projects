/*******************************************************************************************
 *  b2b-client-sys.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 26-January-2021
 *
 * subscribes to and displays status of a b2b system (CBU, PM, KD ...)
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
#define B2B_CLIENT_SYS_VERSION 0x000225

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <termios.h>

// dim
#include <dic.h>

// b2b
#include <common-lib.h>                  // COMMON
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW

const char* program;

#define B2BNSYS     6                    // number of B2B systems

#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names

uint32_t no_link_32    = 0xdeadbeef;
uint64_t no_link_64    = 0xdeadbeefce420651;
char     no_link_str[] = "NO_LINK";

char    disB2bPrefix[DIMMAXSIZE];

const char * sysShortNames[] = {
  "sis18-cbu",
  "sis18-pm",
  "sis18-kde",
  "esr-cbu",
  "esr-pm",
  "esr-kdx",
};

const char * ringNames[] = {
  " SIS18",
  " SIS18",
  " SIS18",
  "   ESR",
  "   ESR",
  "   ESR",
};

const char * typeNames[] = {
  "CBU",
  " PM",
  "KDE",
  "CBU",
  " PM",
  "KDX",
};

struct b2bSystem_t {
  char      version[DIMCHARSIZE];
  char      state[DIMCHARSIZE];
  char      hostname[DIMCHARSIZE];
  uint64_t  status;
  uint32_t  nTransfer;

  uint32_t  versionId;
  uint32_t  stateId;
  uint32_t  hostnameId;
  uint32_t  statusId;
  uint32_t  nTransferId;
}; // struct b2bSystem

struct b2bSystem_t dicSystem[B2BNSYS];


// exit with error message
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  uint32_t version;
  
  fprintf(stderr, "Usage: %s [OPTION] [PREFIX]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -s                  subscribe and display system info\n");
  fprintf(stderr, "  -o                  print info only once and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to display system information on the B2B system\n");
  fprintf(stderr, "Example1: '%s pro\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");

  b2b_version_library(&version);
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(version));
} //help

// add all dim services
void dicSubscribeServices(char *prefix)
{
  char name[DIMMAXSIZE];
  int  i;

  for (i=0; i<B2BNSYS; i++) {
    sprintf(name, "%s_%s_version_fw", prefix, sysShortNames[i]);
    dicSystem[i].versionId   = dic_info_service(name, MONITORED, 0, (dicSystem[i].version), 8, 0, 0, no_link_str, strlen(no_link_str));
    sprintf(name, "%s_%s_state", prefix, sysShortNames[i]);
    dicSystem[i].stateId     = dic_info_service(name, MONITORED, 0, (dicSystem[i].state), 10, 0, 0, no_link_str, strlen(no_link_str));
    sprintf(name, "%s_%s_hostname", prefix, sysShortNames[i]);
    dicSystem[i].hostnameId  = dic_info_service(name, MONITORED, 0, (dicSystem[i].hostname), 32, 0, 0, no_link_str, strlen(no_link_str));
    sprintf(name, "%s_%s_status", prefix, sysShortNames[i]);
    dicSystem[i].statusId    = dic_info_service(name, MONITORED, 0, &(dicSystem[i].status), sizeof(uint64_t), 0, 0, &no_link_64, sizeof(no_link_64));
    sprintf(name, "%s_%s_ntransfer", prefix, sysShortNames[i]);
    dicSystem[i].nTransferId = dic_info_service(name, MONITORED, 0, &(dicSystem[i].nTransfer), sizeof(dicSystem[i].nTransferId), 0, 0, &no_link_32, sizeof(no_link_32));
  } // for i
} // dicSubscribeServices


// send 'clear diag' command to server
void dicCmdClearDiag(char *prefix, uint32_t indexServer)
{
  char name[DIMMAXSIZE];

  sprintf(name, "%s_%s_cmd_cleardiag", prefix, sysShortNames[indexServer]);
  dic_cmnd_service(name, 0, 0);
} // dicCmdClearDiag

// get character from terminal, 0: no character
char getTermChar()
{
  static struct termios oldt, newt;
  char ch = 0;
  int  len;

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
    
    // reset to old terminal settings
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);

    if (len) return ch;
    else     return 0;
} // getTermChar


// printServices
void printServices(int flagOnce)
{
  int i;

  char   cTransfer[10];
  char   cStatus[17];
  char   buff[100];
  time_t time_date;              

  if (!flagOnce) {
    for (i=0;i<60;i++) printf("\n");
    time_date = time(0);
    strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
    printf("\033[7m B2B System Status ----------------------------------------------------- v%06x\033[0m\n", B2B_CLIENT_SYS_VERSION);
    //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");
  } // if not once
  
  printf("  #   ring sys   fw-ver     state  transfers           status               node\n");
  for (i=0; i<B2BNSYS; i++) {
    if (dicSystem[i].nTransfer == no_link_32) sprintf(cTransfer, "%9s", no_link_str);
    else                                      sprintf(cTransfer, "%9u", dicSystem[i].nTransfer);
    if (dicSystem[i].status    == no_link_64) sprintf(cStatus, "%16s", no_link_str);
    else                                      sprintf(cStatus, "%16"PRIx64"", dicSystem[i].status);
    
    printf(" %2d %6s %3s %8s %10s %9s %16s %18s\n", i, ringNames[i], typeNames[i], dicSystem[i].version, dicSystem[i].state, cTransfer, cStatus, dicSystem[i].hostname);
  } // for i

  if (!flagOnce) {
    printf("\n\n\n\n\n\n\n\n");
    printf("\033[7m exit with <q> | clear status with <digit>                       %s\033[0m\n", buff);
  } // if not once
} // printServices


int main(int argc, char** argv) {
  const char* devName;

  int opt, error = 0;
  int exitCode   = 0;
  char *tail;

  int      getVersion;
  int      subscribe;
  int      once;

  uint64_t statusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;
  uint32_t nTransfer;
  int32_t  getcomLatency;                      // message latency from ECA

  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t sleepTime;                          // time to sleep [us]
  uint32_t printFlag;                          // flag for printing
  uint32_t verLib;
  uint32_t verFw;

  int      i;
  char     userInput;
  int      quit;

  uint32_t cpu;
  uint32_t status;

  char     prefix[DIMMAXSIZE];
  char     disName[DIMMAXSIZE];
  //  char     hostname[DIMCHARSIZE];

  program    = argv[0];
  getVersion = 0;
  subscribe  = 0;
  once       = 0;
  quit       = 0;

  while ((opt = getopt(argc, argv, "seho")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 's':
        subscribe = 1;
        break;
      case 'o':
        once = 1;
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
  
  if (optind< argc) sprintf(prefix, "b2b_%s", argv[optind]);
  else              sprintf(prefix, "b2b");

  if (getVersion) {
    b2b_version_library(&verLib);
    printf("%s: version 0x%06x\n", program, B2B_CLIENT_SYS_VERSION);
  } // if getVersion

  if (subscribe) {
    printf("b2b-client-sys: starting client using prefix %s\n", prefix);

    dicSubscribeServices(prefix);

    while (!quit) {
      if (once) {sleep(1); quit=1;}                 // wait a bit to get the values
      printServices(once);
      if (!quit) {
        userInput = getTermChar();
        switch (userInput) {
          case 0x30 ... 0x39 :
            dicCmdClearDiag(prefix, (uint32_t)(userInput - 48));
            break;
          case 0x71        :
            quit = 1;
            break;
          default          :
            sleep(1);
        } // switch
      } // if !once
    } // while
  } // if subscribe

  return exitCode;
}

/*******************************************************************************************
 *  b2b-serv-sys.c
 *
 *  created : 2021
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 25-feb-2025
 *
 * publishes status of a b2b system (CBU, PM, KD ...)
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
#define B2B_SERVSYS_VERSION 0x000808

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// dim
#include <dis.h>

// b2b
#include <common-lib.h>                  // COMMON
#include <b2blib.h>                      // API
#include <b2b.h>                         // FW
#include <b2bcbu_shared_mmap.h>          // LM32

const char* program;
uint64_t   ebDevice;


#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names

char      disName[DIMMAXSIZE];

char      disVersion[DIMCHARSIZE];
char      disState[DIMCHARSIZE];
char      disHostname[DIMCHARSIZE];
uint64_t  disStatus;
uint32_t  disNTransfer;

uint32_t  disVersionId      = 0;
uint32_t  disStateId        = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disStatusId       = 0;
uint32_t  disNTransferId    = 0;
uint32_t  dicClearDiagId    = 0;

 
static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-device> <server name>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -s                  start server publishing system info\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to publish information on a B2B system (CBU, PM, KDE ...)\n");
  fprintf(stderr, "Example1: '%s dev/wbm0 pro_sis18-pm -s'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", b2b_version_text(B2B_SERVSYS_VERSION));
} //help


// clear diagnostic information
void cmdClearDiag(long *tag, char *cmnd_buffer, int *size)
{
  b2b_cmd_cleardiag(ebDevice);
} // cmdClearDiag


// add all dim services
void disAddServices(char *prefix)
{
  char name[DIMMAXSIZE];
  
  sprintf(name, "%s_version_fw", prefix);
  disVersionId   = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_state", prefix);
  disStateId      = dis_add_service(name, "C", disState, 10, 0 , 0);

  sprintf(name, "%s_hostname", prefix);
  disHostnameId   = dis_add_service(name, "C", disHostname, DIMCHARSIZE, 0 , 0);

  sprintf(name, "%s_status", prefix);
  disStatusId     = dis_add_service(name, "X", &disStatus, sizeof(disStatus), 0 , 0);

  sprintf(name, "%s_ntransfer", prefix);
  disNTransferId  = dis_add_service(name, "I", &disNTransfer, sizeof(disNTransfer), 0 , 0);

  sprintf(name, "%s_cmd_cleardiag", prefix);
  dicClearDiagId =  dis_add_cmnd(name, 0, cmdClearDiag, 0);
} // dimAddServices


int main(int argc, char** argv) {
  const char* devName;

  int opt, error = 0;
  int exitCode   = 0;

  int      getVersion;
  int      snoop;

  uint64_t statusArray;
  uint32_t state;
  uint32_t nBadStatus;
  uint32_t nBadState;

  uint32_t actState = COMMON_STATE_UNKNOWN;    // actual state of gateway
  uint32_t verLib;
  uint32_t verFw;
  uint32_t verFwOld;

  uint32_t cpu;
  uint32_t status;

  // most of this is just dummuy
  uint64_t mac;
  uint32_t ip;
  uint64_t tDiag;
  uint64_t tS0;
  uint32_t nTransfer;
  uint32_t nInjection;
  uint32_t statTrans;
  uint32_t nLate;
  uint32_t offsDone;
  uint32_t comLatency;
  uint32_t usedSize;

  // pointer that points to the relevant number
  uint32_t *nTransferPub;

  char     prefix[DIMMAXSIZE];
  char     disName[DIMMAXSIZE];

  program        = argv[0];
  getVersion     = 0;
  snoop          = 0;
  nTransferPub   = 0x0;
  

  while ((opt = getopt(argc, argv, "seh")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 's':
        snoop = 1;
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
  
  if (optind >= argc) {
    fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  devName = argv[optind];
  gethostname(disHostname, 32);

  if (optind+1 < argc) sprintf(prefix, "b2b_%s", argv[++optind]);
  else                 sprintf(prefix, "b2b_%s", disHostname);

  if ((status =  b2b_firmware_open(&ebDevice, devName, 0, &cpu)) != COMMON_STATUS_OK) die("firmware open", status);
  
  if (getVersion) {
    b2b_version_library(&verLib);
    printf("b2b: serv-sys / library / firmware /  version %s / %s",  b2b_version_text(verLib), b2b_version_text(B2B_SERVSYS_VERSION));     
    b2b_version_firmware(ebDevice, &verFw);
    printf(" / %s\n",  b2b_version_text(verFw));     
  } // if getVersion


  if (snoop) {
    printf("b2b-serv-sys: starting server using prefix %s\n", prefix);

    disAddServices(prefix);

    sprintf(disName, "%s", prefix);
    dis_start_serving(disName);

    

    b2b_common_read(ebDevice, &statusArray, &state, &nBadStatus, &nBadState, &verFw, &nTransfer, 0);
    sprintf(disVersion, "%s", b2b_version_text(verFw));
    dis_update_service(disVersionId);
    verFwOld = verFw;

    // standard systems (CBU, PM, ....)
    nTransferPub = &nTransfer;
    // system is PSM, misuse unused values from common
    if (strstr(prefix, "sis18-psm"))  nTransferPub = &nTransfer;
    if (strstr(prefix, "esr-psm"))    nTransferPub = &nInjection;
    if (strstr(prefix, "yr-psm"))     nTransferPub = &nInjection;
    if (strstr(prefix, "sis100-psm")) nTransferPub = &statTrans;;

    while (1) {
      // misuse nTransfer...statTrans for counting PSM activity
      comlib_readDiag(ebDevice, &statusArray, &state, &verFw, &mac, &ip, &nBadStatus, &nBadState, &tDiag, &tS0, &nTransfer, &nInjection, &statTrans, &nLate, &offsDone, &comLatency, &usedSize, 0);
      if (actState != state) {
        actState = state;
        sprintf(disState, "%s", b2b_state_text(state));
        dis_update_service(disStateId);
      } // if state has changed

      if (disStatus != statusArray) {
        disStatus = statusArray;
        dis_update_service(disStatusId);
      } // if disStatus

      if (disNTransfer != *nTransferPub) {
        disNTransfer = *nTransferPub;
        dis_update_service(disNTransferId);
      } // if disNTransfer  

      if (verFw != verFwOld) {
        sprintf(disVersion, "%s", b2b_version_text(verFw));
        dis_update_service(disVersionId);
        verFwOld = verFw;
      } // if verFw 
        
      sleep(1);
    } // while
  } // if snoop

  // close connection to firmware
  if ((status = b2b_firmware_close(ebDevice)) != COMMON_STATUS_OK) die("device close", status);

  return exitCode;
}

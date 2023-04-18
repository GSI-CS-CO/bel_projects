/*******************************************************************************************
 *  ftdimcp-ctl.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 18-Apr-2023
 *
 * command line program for MCP4725 connected via FT232H
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
// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// ftdi mcp
#include <ftdimcp-lib.h>

// DIM
#ifdef USEDIM
#include <dis.h>

#define DIMCHARSIZE 32                   // standard size for char services
#define DIMMAXSIZE  1024                 // max size for service names

char      disName[DIMMAXSIZE];           // name of DIM server

char      disVersion[DIMCHARSIZE];       // version 
uint32_t  disNTrigger;                   // approximate number of comparator 'triggers'

uint32_t  disVersionId      = 0;
uint32_t  disNTriggerId     = 0;
uint32_t  dicSetLevelId     = 0;

#endif //USEDIM

// public variables
const char* program;


static void die(const char* what) {
  fprintf(stderr, "%s: failed: %s\n", program, what);
  exit(1);
} //die

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <port number> [COMMAND]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -i                  display information on ftdi module connected via USB\n");
  fprintf(stderr, "  -l <level>          set level of DAC [%%]\n");
  fprintf(stderr, "  -o                  get actual state of comparator output\n");
  fprintf(stderr, "  -s                  get 'stretched' state of comparator output\n");
  fprintf(stderr, "  -d <name prefix>    daemonize as DIM server\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Tip: This tool requires libftd2xx.so and libmpsse.so to communicate with the device.\n");
  fprintf(stderr, "     The device must not use 'usbserial' and 'ftdi_sio' kernel drivers.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example1: '%s 0 -i -l10 -o\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %06x. Licensed under the LGPL v3.\n", FTDIMCP_LIB_VERSION);
} //help


// set comparator level
void cmdSetLevel(long *tag, char *cmnd_buffer, int *size)
{
  // bla
} // cmdSetLevel


// add all dim services
void disAddServices(char *prefix)
{
#ifdef USEDIM

  char name[DIMMAXSIZE];
  
  sprintf(name, "%s_version", prefix);
  disVersionId  = dis_add_service(name, "C", disVersion, 8, 0 , 0);

  sprintf(name, "%s_ntrigger", prefix);
  disNTriggerId = dis_add_service(name, "I", &disNTrigger, sizeof(disNTrigger), 0 , 0);

  sprintf(name, "%s_cmd_setlevel", prefix);
  dicSetLevelId =  dis_add_cmnd(name, 0, cmdSetLevel, 0);
  
#endif //USEDIM
} // dimAddServices


//main
int main(int argc, char** argv) {
  const char* command;
  int         opt, error = 0;
  int         exitCode   = 0;
  char        *tail;
  char        *p;

  int        getVersion = 0;
  int        getInfo    = 0;
  int        getOutput  = 0;
  int        setDac     = 0;
  int        daemon     = 0;
  double     dacLevel   = 0;              // level of DAC [%]

  FT_STATUS  ftStatus;                    // status returned by FTDI library calls
  FT_HANDLE  cHandle;                     // handle to channel
  int        cIdx;                        // index of channel; there can be more than one FTDI channel connected
  uint32_t   verLibFtdi;                  // version of ftdi library (required by mpsse library)
  uint32_t   verLibMpsse;                 // version of mpsse library
  uint32_t   verLibFtdiMcp;               // version of ftdimcp library
  uint32_t   outAct;                      // actual value of comparator output
  uint32_t   outOld;                      // previous value of comparator output

  char       prefix[1024];                // prefix for DIM services

  program = argv[0];

  while ((opt = getopt(argc, argv, "l:d:oeih")) != -1) {
    switch (opt) {
    case 'e':
      getVersion = 1;
      break;
    case 'i':
      getInfo = 1;
      break;
    case 'l':
      setDac = 1;
        dacLevel = strtod(optarg, &tail);
        if (*tail != 0) {
          fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
          exit(1);
        } // if tail 
        if ((dacLevel < 0.0) || (dacLevel > 100.0)) {
          fprintf(stderr, "value must be within 0..100, not %f!\n", dacLevel);
          exit(1);
        } // if dacLevel
      break;
    case 'd':
      daemon = 1;
      sprintf(prefix, "%s", optarg);
      break;
    case 'o':
      getOutput = 1;
      break;
    case 's':
      printf("option '-s' not yet implemented\n");
      break;
    case 'h':
      help();
      return 0;
    case ':':
    case '?':
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
    fprintf(stderr, "%s: expecting one non-optional argument: <port number>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  } // if optint

  cIdx = strtol(argv[optind], &p, 10);

  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;

  if (getVersion) {
    ftdimcp_getVersion(&verLibFtdiMcp, &verLibFtdi, &verLibMpsse);

    printf("ftdi-mcp library version 0x%x\n", verLibFtdiMcp); 
    printf("ftdi     library version 0x%x\n", verLibFtdi);
    printf("mpsse    library version 0x%x\n", verLibMpsse);
  } // if getVersion

  // open, init
  if ((ftStatus   = ftdimcp_open(cIdx, &cHandle, 1)) != FT_OK) die("can't open connection to FTDI channel");
  if ((ftStatus   = ftdimcp_init(cHandle)) != FT_OK) die("can't init channel");

  
  // info    
  if (getInfo) {
    if ((ftStatus = ftdimcp_info(cIdx))!= FT_OK) die("can't get info on FTDI channel");
  } // if getInfo

  if (setDac) {
    // activity LED -> ON
    ftdimpc_setLed(cHandle, 1);

    // write to DAC
    if ((ftStatus = ftdimcp_setLevel(cHandle, dacLevel, 0, 1)) != FT_OK) die ("can't write to DAC");

    // activity LED -> OFF
    sleep(1);
    ftdimpc_setLed(cHandle, 0);
  } // if setDac

  if (getOutput) {
    if ((ftStatus = ftdimpc_getCompOutAct(cHandle, &outAct)) != FT_OK) die("can't read GPIO");
    if (outAct) printf("output is HIGH\n");
    else        printf("output is LOW\n");
  } // if getOutput
    
  if (daemon) {
#ifdef USEDIM
    sprintf(disName, "");
    sprintf(disVersion, "");
    disNTrigger = 0;
    outAct      = 0;
    outOld      = 0;
    
    printf("%s: starting server using prefix %s\n", program, prefix);

    // add services, update 'constant' services
    disAddServices(prefix);

    sprintf(disName, "%s", prefix);
    dis_start_serving(disName);
    sprintf(disVersion, "%06x", FTDIMCP_LIB_VERSION);
    dis_update_service(disVersionId);
    printf("dis version id %d\n", disVersionId);
       
    while (1) {
      ftdimpc_getCompOutStretched(cHandle, &outAct);
      if (outAct > outOld) disNTrigger++;
      outOld = outAct;
      usleep(100000);
    } // while
#endif // USEDIM
  } // if daemon


  // dummy
  if (command) {
  } //if command
  
  // close connection ...
  ftdimcp_close(cHandle);

  return exitCode;
} // main

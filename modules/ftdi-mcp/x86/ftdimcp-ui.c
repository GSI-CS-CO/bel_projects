/*******************************************************************************************
 *  ftdimcp-ui.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 28-Mar-2024
 *
 * user interface that connects to a ftdimcp-ctl instance (started as daemon) via DIM
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
#define FTDIMCP_UI_VERSION 0x000008

// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

// ftdi mcp
#include <common-lib.h>
#include <ftdimcp-lib.h>

// DIM
#include <dic.h>

#define DIMCHARSIZE      32              // standard size for char services
#define DIMMAXSIZE       1024            // max size for service names
#define SCREENWIDTH      1024            // width of screen
#define FTDIMCP_UI_LINES 12              // number of lines at screen

char      disName[DIMMAXSIZE];           // name of DIM server

char      disVersion[DIMCHARSIZE];       // version
uint32_t  disActOutput;                  // actual (non-stretched) comparator output
uint32_t  disTriggered;                  // value of 'stretched' comparator output
uint32_t  disNTrigger;                   // approximate number of comparator 'triggers'
double    disSetLevel;                   // actual comparator level
double    dicSetLevel;                   // level to set from here
char      disHostname[DIMCHARSIZE];      // hostname of server
char      disStatus[DIMCHARSIZE];        // status of server
uint32_t  disDevice;                     // USB device ID
char      disSerial[DIMCHARSIZE];        // USB serial number

uint32_t  disVersionId      = 0;
uint32_t  disActOutputId    = 0;                  
uint32_t  disTriggeredId    = 0;
uint32_t  disNTriggerId     = 0;
uint32_t  disSetLevelId     = 0;
uint32_t  disHostnameId     = 0;
uint32_t  disStatusId       = 0;
uint32_t  disDeviceId       = 0;
uint32_t  disSerialId       = 0;

uint32_t  no_link_32        = 0xdeadbeef;
uint64_t  no_link_64        = 0xdeadbeefce420651;
double    no_link_dbl       = 0xdeadbeefce420651;
char      no_link_str[]     = "NO_LINK";


// public variables
const char* program;

char     empty[SCREENWIDTH+1];                              // an empty line
char     title[SCREENWIDTH+1];                              // title string
char     footer[SCREENWIDTH+1];                             // footer line to be printed

int      flagSchornstein;                                   // 1: use intermediate maximum level when settng a new value

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h                  display this help and exit\n");
  fprintf(stderr, "  -e                  display version\n");
  fprintf(stderr, "  -o                  print info only once and exit (useful with '-s')\n");
  fprintf(stderr, "  -y                  when setting a new value, avoid hysteris via an intermediate maximum level\n");
  fprintf(stderr, "  -s <name prefix>    connect to server with given prefix\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to disply information and change settings of a ftdi-mcp device.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example1: '%s -s b2b_int_sis18-kse'\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %06x. Licensed under the LGPL v3.\n", FTDIMCP_UI_VERSION);
} //help


void buildHeader()
{
  sprintf(title, "\033[7m FTDI-MCP UI ----------------------------------------------------------- v%06x\033[0m", FTDIMCP_UI_VERSION);
  sprintf(empty , "                                                                                ");
  //       printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");  
} // buildHeader


// add all dim services
void dicSubscribeServices(char *prefix)
{

  char name[DIMMAXSIZE];
  
  sprintf(name, "%s_version", prefix);
  disVersionId   = dic_info_service(name, MONITORED, 0, disVersion, 8, 0, 0, &no_link_32, sizeof(no_link_32));

  sprintf(name, "%s_actoutput", prefix);
  disActOutputId = dic_info_service(name, MONITORED, 0, &disActOutput, sizeof(disActOutput) , 0, 0, &no_link_32, sizeof(no_link_32));

  sprintf(name, "%s_triggered", prefix);
  disTriggeredId =  dic_info_service(name, MONITORED, 0, &disTriggered, sizeof(disTriggered), 0, 0, &no_link_32, sizeof(no_link_32));

  sprintf(name, "%s_ntrigger", prefix);
  disNTriggerId  = dic_info_service(name, MONITORED, 0, &disNTrigger, sizeof(disNTrigger), 0, 0, &no_link_32, sizeof(no_link_32));
  
  sprintf(name, "%s_setlevel", prefix);
  disSetLevelId = dic_info_service(name, MONITORED, 0, &disSetLevel, sizeof(disSetLevel), 0, 0, &no_link_dbl, sizeof(no_link_dbl));

  sprintf(name, "%s_hostname", prefix);
  disSetLevelId = dic_info_service(name, MONITORED, 0, disHostname, DIMCHARSIZE, 0, 0, &no_link_32,  sizeof(no_link_32));

  sprintf(name, "%s_status", prefix);
  disStatusId = dic_info_service(name, MONITORED, 0, disStatus, DIMCHARSIZE, 0, 0, &no_link_32, sizeof(no_link_dbl));

  sprintf(name, "%s_serial", prefix);
  disSerialId = dic_info_service(name, MONITORED, 0, disSerial, DIMCHARSIZE, 0, 0, &no_link_32,  sizeof(no_link_32));

  sprintf(name, "%s_deviceid", prefix);
  disDeviceId = dic_info_service(name, MONITORED, 0, &disDevice, sizeof(disDevice), 0, 0, &no_link_32, sizeof(no_link_32));
} // dimSubscribeServices


// set comparator level
void cmdSetLevel(char *prefix)
{
   char   name[DIMMAXSIZE];
   char   buff[64];
   double tmp;
   int    result;
   char   *endptr;

   comlib_term_clear();

   printf("Set level of comparator [%%]\n");
   printf("valid number are within 0.0..100.0\n");
   printf("\n");
   printf("enter new value: ");
   result = scanf("%s", buff);
   tmp = strtod(buff, &endptr);
   
   //printf("buff is %s, number is %lf, scanf result is %d\n", buff, tmp, result);

   if ((!result) || (endptr == buff) || (tmp > 100.0) || (tmp < 0.0)) {
     printf("ignoring invalid value ...\n");
     sleep(2);
   } // if result
   else {
     sprintf(name, "%s_cmd_setlevel", prefix);
     if (flagSchornstein) {  // set level to maximum, to avoid hysteresis
       dicSetLevel = 100.0;
       dic_cmnd_service(name, &dicSetLevel, sizeof(dicSetLevel));
       usleep(100000);
     } // if flagSchornstein
     dicSetLevel = tmp;
     //printf("name : %s, level %f\n", name, dicSetLevel);
     //sleep(2);
     
     dic_cmnd_service(name, &dicSetLevel, sizeof(dicSetLevel));
   } // else !result

   comlib_term_clear();
} // cmdSetLevel


// print services to screen
void printServices(char *prefix, int flagOnce)
{
  int      i;
  time_t   time_date;
  char     buff[100];
  uint32_t *tmp;

  //printf("12345678901234567890123456789012345678901234567890123456789012345678901234567890\n");

  // footer with date and time
  time_date = time(0);
  strftime(buff,50,"%d-%b-%y %H:%M",localtime(&time_date));
  sprintf(footer, "\033[7m exit <q> | set comp. level  <l> | help <h>                      %s\033[0m", buff);
  
  comlib_term_curpos(1,1);
  
  if (!flagOnce) printf("%s\n", title);

  if (disActOutput == no_link_32)  printf("output actual                 : %8s\n"    , no_link_str);
  else                             printf("output actual                 : % 8d\n"   , disActOutput);
  if (disTriggered == no_link_32)  printf("just triggered                : %8s\n"    , no_link_str);
  else                             printf("just triggered                : % 8d\n"   , disTriggered);
  if (disNTrigger  == no_link_32)  printf("# of detected triggers        : %8s\n"    , no_link_str);
  else                             printf("# of detected triggers        : % 8d\n"   , disNTrigger);
  if (disSetLevel  == no_link_dbl) printf("set value comparator level    : %8s\n"    , no_link_str);
  else                             printf("set value comparator level [%%]: %8.3f\n" , disSetLevel);
  printf(                                 "use 'Schornstein'             : % 8d\n"   , flagSchornstein);
  printf("\n");
  printf(                                 "server info\n");
  printf(                                 "name                          : %s\n"     , prefix);
  tmp = (uint32_t *)disHostname;
  if (*tmp == no_link_32)          printf("host                          : %-32s\n"  , no_link_str);
  else                             printf("host                          : %-32s\n"  , disHostname);
  tmp = (uint32_t *)disVersion;
  if (*tmp         == no_link_32)  printf("version                       : %-32s\n"  , no_link_str);
  else                             printf("version                       : %-32s\n"  , disVersion);
  tmp = (uint32_t *)disStatus;
  if (*tmp         == no_link_32)  printf("status                        : %-32s\n"  , no_link_str);
  else                             printf("status                        : %-32s\n"  , disStatus);

  if (disDevice    == no_link_32)  printf("USB device ID                 : %-32s\n"  , no_link_str);
  else                             printf("USB device ID                 : %08x\n"   , disDevice);
  tmp = (uint32_t *)disSerial;
  if (*tmp         == no_link_32)  printf("USB serial number             : %-32s\n"  , no_link_str);
  else                             printf("USB serial number             : %-32s\n"  , disSerial);
    
  for (i=0; i<FTDIMCP_UI_LINES - 4; i++) printf("%s\n", empty);
  if (!flagOnce) printf("%s\n", footer);
} // printServices


// print help text to screen
void printHelpText()
{
  int i;

  comlib_term_curpos(1,1);
  printf("%s\n", title);

  for (i=0; i<FTDIMCP_UI_LINES; i++) printf("%s\n", empty);
  printf("the help function is not yet implemented                                        \n");
  printf("%s\n", empty);
  printf("press any key to continue\n");
  while (!comlib_term_getChar()) {usleep(200000);}
} // printHelpText

//main
int main(int argc, char** argv) {
  const char* command;
  int         opt, error = 0;
  int         exitCode   = 0;
  //  char        *tail;

  int         getVersion;
  int         subscribe;
  int         once;

  char        userInput;
  int         quit;

  char       prefix[1024];                // prefix for DIM services

  program         = argv[0];
  getVersion      = 0;
  subscribe       = 0;
  once            = 0;
  quit            = 0;
  flagSchornstein = 0;
  
  while ((opt = getopt(argc, argv, "s:eoyh")) != -1) {
    switch (opt) {
      case 'e':
        getVersion = 1;
        break;
      case 's':
        subscribe = 1;
        sprintf(prefix, "%s", optarg);
        break;
      case 'o':
        once = 1;
        break;
      case 'y' :
        flagSchornstein = 1;
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
  
  if (optind+1 < argc)  command = argv[++optind];
  else command = NULL;

  if (getVersion) {
    printf("ftdi-mcp ui version     : %06x\n", FTDIMCP_UI_VERSION);
    printf("ftdi-mcp library version: %06x\n", FTDIMCP_LIB_VERSION);
  } // if getVersion

  if (subscribe) {
    comlib_term_clear();
    buildHeader();
    
    printf("%s: starting client using prefix %s\n", program, prefix);

    dicSubscribeServices(prefix);

    sleep(1);
    
    while (!quit) {
      if (once) {sleep(1); quit=1;}                 // wait a bit to get the values
      printServices(prefix, once);
      if (!quit) {
        userInput = comlib_term_getChar();
        switch (userInput) {
          case 'l' :
            cmdSetLevel(prefix);
            break;
          case 'h' :
            printHelpText();
            break;
          case 'q' :
            quit = 1;
            break;
          default :
            usleep(1000000);
        } // switch
      } // if !quit
    } // while
  } // if subscribe


  // dummy
  if (command) {
  } //if command
  

  return exitCode;
} // main

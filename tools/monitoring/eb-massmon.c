///////////////////////////////////////////////////////////////////////////////
//  eb-massmon.c
//
//  created : 2018
//  author  : Dietrich Beck, GSI-Darmstadt
//  version : 04-May-2018
//
// Command-line interface for WR monitoring of many nodes via Etherbone.
//
// -------------------------------------------------------------------------------------------
// License Agreement for this software:
//
// Copyright (C) 2018  Dietrich Beck
// GSI Helmholtzzentrum für Schwerionenforschung GmbH
// Planckstraße 1
// D-64291 Darmstadt
// Germany
//
// Contact: d.beck@gsi.de
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 3 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//  
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.
//
// For all questions and ideas contact: d.beck@gsi.de
// Last update: 27-April-2018
//////////////////////////////////////////////////////////////////////////////////////////////
#define EBMASSMON_VERSION "0.0.2"

// standard includes
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

// Etherbone
#include <etherbone.h>

// Wishbone api
#include <wb_api.h>
#include <wb_slaves.h>

const char* program;
static int  verbose=0;
eb_device_t device;           // needs to be global for 1-wire stuff

#define     MAXNODES  1024    // max number of nodes
#define     MAXLEN    1024    // max length of an array
#define     UTCOFFSET 37000   // UTC offset @ 2018; hm... configurable as option?

const char  *networkTypeNames[]  = {"all", "Production", "User", "Timing"};
#define     MAXNETWORKTYPES      4

const char  *nodeTypeNames[]     = {"all", "scuxl", "pexaria", "vmel", "expl"};
#define     MAXNODETYPES         5

#define     MAXFILEFORMATS       2 // only 0, and 1 

static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-protocol> <file with node names> <domain>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -a               include FPGA build version\n");
  fprintf(stderr, "  -c               check if IP and MAC are correct (requires options '-f1', '-i', 'm')\n");
  fprintf(stderr, "  -d               include WR time\n");
  fprintf(stderr, "  -e               display etherbone version\n");
  fprintf(stderr, "  -f<format>       file format\n");
  fprintf(stderr, "                   0: one node per line (default)\n");
  fprintf(stderr, "                   1: ATD format\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "  -i               include WR IP\n");
  fprintf(stderr, "  -j               display additional info (requires option '-f1'\n");
  fprintf(stderr, "  -l               include WR link status\n");
  fprintf(stderr, "  -m               include WR MAC\n");
  fprintf(stderr, "  -o               include offset between WR time and system time [ms]\n");
  fprintf(stderr, "  -q               justs parse file and list nodes. No hardware access. Useful for ATD file analysis\n");
  fprintf(stderr, "  -s               include WR sync status\n");
  fprintf(stderr, "  -t               display table with all nodes\n");
  fprintf(stderr, "  -u               display statistics for all nodes\n");
  fprintf(stderr, "  -w<timeout>      timeout [ms] for probing each device (default 1000); might be useful for WRS\n");
  fprintf(stderr, "  -x<network>      network type (requires option '-f1'\n");
  fprintf(stderr, "                   0: all\n");
  fprintf(stderr, "                   1: production (default)\n");
  fprintf(stderr, "                   2: user\n");
  fprintf(stderr, "                   3: timing (TTF)\n");
  fprintf(stderr, "  -y<nodes>        node type\n");
  fprintf(stderr, "                   0: all (default)\n");
  fprintf(stderr, "                   1: SCU\n");
  fprintf(stderr, "                   2: PEXARIA\n");
  fprintf(stderr, "                   3: VETAR\n");
  fprintf(stderr, "                   4: EXPLODER\n");
  fprintf(stderr, "  -z               include FPGA uptime [h]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Use this tool to get some WR related info of many timing receiver nodes.\n");
  fprintf(stderr, "Warning: This tool is causing some traffic on the network. Use with care.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example: '%s udp allTimingDevices.txt timing.acc.gsi.de -f1 -x0 -y0 -t -u -q'\n", program);
  fprintf(stderr, "          don't query (option '-q') but only display all nodes of all networks\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example: '%s udp allTimingDevices.txt timing.acc.gsi.de -f1 -x1 -y1 -t -u -d -o -l -s -a -z'\n", program);
  fprintf(stderr, "          display info on all SCUs of the production network queried via the White Rabbit network\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Example: '%s tcp allTimingDevices.txt acc.gsi.de -f1 -x1 -y1 -t -u -d -o -l -s -a -z'\n", program);
  fprintf(stderr, "          display info on all SCUs of the production network queried via the ACC network\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMASSMON_VERSION);
} //help


static eb_status_t checkDevGetDateOffset(eb_device_t device, int devIndex, uint64_t *nsecs, uint64_t *offset, uint32_t timeout)
{
  eb_status_t    status;
  struct timeval htm;
  uint64_t       hostmsecs64;
  uint64_t       msecs64;

  *offset = 0;
  *nsecs  = 0;

  // check device by querying WR time, inlcuding a 2nd chance
  if ((status = wb_wr_get_time(device, devIndex, nsecs)) != EB_OK) {
    usleep(timeout);
    if ((status = wb_wr_get_time(device, devIndex, nsecs)) != EB_OK) return status;
  } // operation failed

  gettimeofday(&htm, NULL);
  hostmsecs64  = htm.tv_sec*1000 + htm.tv_usec/1000;
  msecs64      = *nsecs / 1000000.0; 
  if (msecs64 > hostmsecs64) *offset = msecs64 - hostmsecs64;
  else                       *offset = hostmsecs64 - msecs64;
  
  return status;
} // checkDevGetDateOffset


static int deviceNameOk(const char* proto, char* name)
{
  int   type_ok  = 0;  // device type ok?
  int   proto_ok = 0;  // EB proto ok?

  // white list: only allow upd and tcp
  if (strstr(proto, "udp")    != NULL)  proto_ok = 1;
  if (strstr(proto, "tcp")    != NULL)  proto_ok = 1;
  
  // strip trailing "t" from name in case proto is "tcp"
  // check: this fails in case node future names may contain a non-trainling "t". But ok for now ...
  if (strstr(proto, "tcp") != NULL) {
    if (name[strlen(name) - 1] == 't') name[strlen(name) - 1] = '\0';
  } // if proto is tcp

  type_ok = 1;
  // exclude unreasonable length of name ("pexaria1234t" is 12 chars and longest name)
  if(strlen(name) > 12) type_ok =  0;    

  // exclude unreasonable length of name ("scuxl123" is 8 chars and shortest name)
  if(strlen(name) < 8)  type_ok =  0;

  // printf("name %s len %d\n", name, strlen(name));
     
  if (type_ok && proto_ok) return 1;
  else                     return 0;
} //deviceNameOk


static int nodeTypeOk(char *node, int nodeType)
{
  int type_ok = 0;
  int i;

  switch (nodeType) {
  case 0 :
    // all known node types
    for (i = 1; i < MAXNODETYPES; i++) if (strstr(node, nodeTypeNames[i]) != NULL) type_ok = 1;
    break;
  case 1 ... (MAXNODETYPES - 1) :
    // node is not of respective type
    type_ok = 1;
    if (strstr(node, nodeTypeNames[nodeType]) == NULL) type_ok = 0;
    break;
  default: ;
  } // switch network type

  return type_ok;
} // nodeTypeOk


static int protoOk(const char *proto, int nodeType)
{
  // exclude tcp and non-SCU 
  if ((strstr(proto, "tcp") != NULL) && (nodeType != 1)) return 0;
  else                                                   return 1;
} // protoOk


static void removeNonAlphaNumeric(char *text)                                                  
{
  int  i = 0;
  int  j = 0;
  char c;
  
  while ((c = text[i]) != '\0') {
    if (isalnum(c)) {text[j] = c; j++;}
    i++;
  } // while
  text[j] = '\0';
} // removeNonAlphaNumeric


static int parseSimpleLineOk(char* fileLine, char* node)
{
  removeNonAlphaNumeric(fileLine);
  sprintf(node, "%s", fileLine);
  return 1;
} // parseSimpleLineOk


static int parseATDLineOk(char* fileLine, char* network, char* node, char* info, uint64_t *mac, uint32_t *ip)
{
  // format '00:26:7b:00:03:ff,id:timing,192.168.161.1,scuxl0249t       #NW Production; SCU; ...
  char*         ptr1;
  char*         ptr2;
  char          tmp[MAXLEN+1];
  unsigned int  bytes[6];
  int           node_ok    = 0;
  int           network_ok = 0;
  int           i;

  if (fileLine         == NULL)    return 0;      // empty string: error
  if (fileLine[0]      == '#')     return 0;      // line is commented: return error
  if (strlen(fileLine) >=  MAXLEN) return 0;      // line too long: return error


  // parse first part including hostname
  sprintf(tmp, "%s", fileLine);   // make a copy
  ptr1 = NULL;

  // mac
  *mac = 0x0;
  ptr1 = strtok(tmp,  ",");

  if (ptr1 != NULL) {
    if (sscanf(ptr1, "%x:%x:%x:%x:%x:%x", &(bytes[5]), &(bytes[4]), &(bytes[3]), &(bytes[2]), &(bytes[1]), &(bytes[0])) == 6){
      for (i = 0; i < 6; i++) *mac = *mac + ((uint64_t)(bytes[i]) << (i*8));
    } // mac scan ok
  } // ptr != 0

  // dummy
  ptr1 = strtok(NULL, ",");

  // ip
  *ip = 0x0;
  ptr1 = strtok(NULL, ",");
  if (ptr1 != NULL) {
    if (sscanf(ptr1, "%u.%u.%u.%u", &(bytes[3]), &(bytes[2]), &(bytes[1]), &(bytes[0])) == 4){
      for (i = 0; i < 4; i++) *ip = *ip + (bytes[i] << (i*8));
    } // ip scan ok
  } // ptr != 0
 
  // hostname
  ptr1 = strtok(NULL, " ");
  if (ptr1 != NULL) {
    // valid hostname
    sprintf(node, "%s", ptr1);
    node_ok = 1;
  } // ptr != 0

  // parse 2nd part which is the comment after hostname

  // network name  
  sprintf(tmp, "%s", fileLine);               // make another copy
  if (tmp  != NULL) {
    sprintf(network, "N/A");
    ptr1 = NULL;
    ptr2 = NULL;
    
    ptr1 = strstr(tmp, "#NW");                // comment starts here
    if (ptr1 != NULL) {                 
      ptr1 = &(ptr1[3]);                      // first character after '#NW
      ptr2 = strtok(ptr1, ";");         
      if ((ptr2 != NULL) && (strlen(ptr2) < MAXLEN)) {
        sprintf(network, "%s", ptr2);
        network_ok = 1;
      } // ptr2 != 0
    } // ptr1 != 0
  } // tmp != 0
    
  
  // info
  sprintf(tmp, "%s", fileLine);               // make another copy
  if (tmp != NULL) {
    sprintf(info, "N/A");
    ptr1 = NULL;
    ptr2 = NULL;
    
    ptr1 = strstr(tmp, "#NW");                // comment starts here
    if (ptr1 != NULL) {
      ptr2 = strstr(ptr1, ";");               // info starts here
      if ((ptr2 != NULL) && (strlen(ptr2) < MAXLEN)) sprintf(info, "%s", &(ptr2[1]));
    } // ptr1 != 0
  } // tmp != 0

  removeNonAlphaNumeric(node);
  removeNonAlphaNumeric(network);

  if (node_ok && network_ok) return 1;
  else                       return 0;
} // parseATDLineOk


static int networkOk(char* network, int networkType)
{
  int network_ok = 1;

  switch (networkType) {
  case 0 :
    // all networks
    break;
  case 1 ... (MAXNETWORKTYPES - 1) :
    // node is not in respective network
    if (strcmp(network, networkTypeNames[networkType]) != 0) network_ok = 0;
    break;
  default: ;
  } // switch network type
  
  return network_ok;
} // networkOk


static void printDate(uint64_t nsecs)
{
  if (nsecs == ~0) fprintf(stdout, ", %10s", "---");
  else             fprintf(stdout, ", %10lu", (nsecs / 1000000000));
} // printDate


static void printOffset(uint64_t offset)
{
  if (offset == ~0) fprintf(stdout, ", %13s", "---");
  else              fprintf(stdout, ", %13lu", (offset));
} // printOffset


static void printSyncState(uint32_t syncState)
{
  if (syncState == ~0)                             fprintf(stdout, ", %8s", "---");
  else {
    if      (syncState == WR_PPS_GEN_ESCR_MASK)    fprintf(stdout, ", %8s", "TRACKING");
    else if (syncState == WR_PPS_GEN_ESCR_MASKTS)  fprintf(stdout, ", %8s", "TIME");
    else if (syncState == WR_PPS_GEN_ESCR_MASKPPS) fprintf(stdout, ", %8s", "PPS");
    else                                           fprintf(stdout, ", %8s", "NO SYNC");
  }
} // printSyncState


static void printLink(uint32_t link)
{
  if (link == ~0) fprintf(stdout, ", %4s", "---");
  else {
    if (link)     fprintf(stdout, ", %4s", "UP");
    else          fprintf(stdout, ", %4s", "DOWN");
  }
} // printLink


static void printUptime(uint32_t uptime)
{
  if (uptime == ~0) fprintf(stdout, ", %9s", "---");
  else              fprintf(stdout, ", %9.2f", ((double)uptime / 3600.0));
} // printOffset


static void printBuildVer(char *buildType)
{
  char *ptr;

  if (strcmp(buildType,"N/A") == 0) fprintf(stdout, ", %8s", "---");
  else {
    ptr = strstr(buildType, "-v");
    if (ptr != NULL) ptr = &(ptr[strlen("-v")]);
    if (ptr != NULL) fprintf(stdout, ", %8s", ptr);
    else             fprintf(stdout, ", %8s", "N/A");
  } // else 
} // printBuildVer


static void printMac(uint64_t mac)
{
  if (mac == ~0) fprintf(stdout, ", %12s", "---");
  else           fprintf(stdout, ", %012llx", (long long unsigned)mac);
} // printMac


static void printIp(uint32_t ip)
{
  if (ip == ~0) fprintf(stdout, ", %15s", "---");
  else          fprintf(stdout, ", %03d.%03d.%03d.%03d", (ip & 0xFF000000) >> 24, (ip & 0x00FF0000) >> 16, (ip & 0x0000FF00) >> 8, ip & 0x000000FF);
} // printIp


static void printUp(uint32_t exist)
{
  if (exist == ~0) fprintf(stdout, ", %2s", "--");
  else             fprintf(stdout, ", %2d", exist);
} // printUp


static void printInfo(char *info)
{
  int i;
  int max=128;

  if (info != NULL) {
    for (i = 0; i < strlen(info); i++) if iscntrl(info[i]) info[i] = info[i+1];
    if (strlen(info) > max) info[max] = '\0';

    if (strcmp(info,"N/A") == 0) fprintf(stdout, ", %s", "---");
    else                         fprintf(stdout, ", %-64s", info);
  } // if != NULL
} // printUp


static void printCheckIpMac(uint64_t nodeMac, uint64_t atdMac, uint32_t nodeIp, uint32_t atdIp)
{
  fprintf(stdout, ", ");
  if (nodeMac == ~0)     fprintf(stdout, "%4s", "---");
  else {
    if (nodeMac == atdMac) fprintf(stdout, "%4s", "ok");
    else                   fprintf(stdout, "%4s", "ERR");
  }

  if (nodeIp  == ~0)     fprintf(stdout, "%4s", "---");
  else {
    if (nodeIp  == atdIp)  fprintf(stdout, "%4s", "ok");
    else                   fprintf(stdout, "%4s", "ERR");
  }
} //printCheckIpMac


static void printEbStat(eb_status_t status)
{
  switch (status) {
  case EB_OK :
    fprintf(stdout, ", %8s", "ok");
    break;
  case EB_FAIL :
    fprintf(stdout, ", %8s", "FAIL");
    break;
  case EB_ADDRESS :
    fprintf(stdout, ", %8s", "BADADDR");
    break;
  case EB_BUSY :
    fprintf(stdout, ", %8s", "BUSY");
    break;
  case EB_TIMEOUT :
    fprintf(stdout, ", %8s", "TIMEOUT");
    break;
  case EB_ABI :
    fprintf(stdout, ", %8s", "EBLIBERR");
    break;
  case EB_SEGFAULT :
    fprintf(stdout, ", %8s", "SEGFAULT");
    break;
  default :
    fprintf(stdout, ", %8s", "EB ERROR");
  } // switch
} // print eb state


static void printHeader(int wrDate, int wrOffset, int wrSync, int wrMac, int wrLink, int wrIp, int wrUptime, int buildVer, int info, int checkIpMac)
{
  // always print header
  fprintf(stdout,   "%15s",   "node");
  fprintf(stdout,   ", %10s", "network");
  fprintf(stdout,   ", %8s" , "ebStatus");
  fprintf(stdout,   ", %2s" , "up");

  // depends on request
  if (wrDate)     fprintf(stdout, ", %10s", "time [s]");
  if (wrOffset)   fprintf(stdout, ", %13s", "offset[ms]");
  if (wrSync)     fprintf(stdout, ", %8s",  "sync");
  if (wrLink)     fprintf(stdout, ", %4s",  "link");
  if (wrUptime)   fprintf(stdout, ", %9s",  "uptime[h]");
  if (buildVer)   fprintf(stdout, ", %8s",  "gwBuild");
  if (wrMac)      fprintf(stdout, ", %12s", "MAC");
  if (wrIp)       fprintf(stdout, ", %15s", "IP");
  if (checkIpMac) fprintf(stdout, ", %8s",  "?MAC ?IP");
  if (info)       fprintf(stdout, ", %s",   "info");  // this is the last option


  // always linefeed  
  fprintf(stdout, "\n");
} // print Header

int main(int argc, char** argv) {
  eb_status_t  status;                 // EB status
  eb_socket_t  socket;                 // EB socket
  char         devName[MAXLEN+1];      // full EB device name
  char*        ebProto;                // EB protocol 'udp' ...
  int          devIndex=0;             // always grab 1st device on the WB bus

  char*        fileName;               // file name 
  char         node[MAXLEN+1];         // node name 'scuxl0815'
  char         network[MAXLEN+1];      // network name 'Producton', 'User' ...
  char*        domain;                 // domain '.gsi.de' ...

  FILE*        file;                   // file with nodes
  char         fileLine[MAXLEN];       // one line of the file
  int          nNodes;                 // number of nodes
  int          nodeOk;                 // flag (if it is ok: syntax, combination...)

  int          getEBVersion = 0;       // option '-e'
  int          getWRDate    = 0;       // option '-d'
  int          getWROffset  = 0;       // option '-o'
  int          getWRSync    = 0;       // option '-s'
  int          getWRMac     = 0;       // option '-m'
  int          getWRLink    = 0;       // option '-l'
  int          getWRIP      = 0;       // option '-i'
  int          getWRUptime  = 0;       // option '-z'
  int          getBuildVer  = 0;       // option '-a'
  int          timeout      = 1000000; // option '-w'; internal representation is [us]
  int          fileFormat   = 0;       // option '-f'
  int          printTable   = 0;       // option '-t'  
  int          printStats   = 0;       // option '-u'
  int          networkType  = 0;       // option '-x'
  int          nodeType     = 0;       // option '-y'
  int          quietMode    = 0;       // option '-q'
  int          dispInfo     = 0;       // option '-j'
  int          checkIpMac   = 0;       // option '-c'

  char         nodeName[MAXNODES][MAXLEN+1];      
  char         buildType[MAXNODES][MAXLEN+1];      
  uint64_t     nodeNsecs64[MAXNODES];
  uint64_t     nodeOffset[MAXNODES];
  uint64_t     nodeMac[MAXNODES];
  uint64_t     atdMac[MAXNODES];
  uint32_t     nodeLink[MAXNODES];
  uint32_t     nodeUptime[MAXNODES];
  uint32_t     nodeSyncState[MAXNODES];
  uint32_t     nodeIp[MAXNODES];
  uint32_t     atdIp[MAXNODES];
  uint32_t     nodeUp[MAXNODES];
  char         nodeNetwork[MAXNODES][MAXLEN+1];
  char         nodeInfo[MAXNODES][MAXLEN+1];
  eb_status_t  nodeEbStat[MAXNODES];

  int          i,j,k,l,m,nReachable;
  uint64_t     tmp64, temp64, atdTmp64=0;
  uint32_t     tmp32, atdTmp32=0;
  int          tmp;
  char         tmpStr[MAXLEN+1], atdTmpStr[MAXLEN+1];
  

  int          exitCode     = 0;
  int          opt, error=0;
  char         *tail;

  program = argv[0];

  while ((opt = getopt(argc, argv, "f:w:x:y:acdehijlmoqstuz")) != -1) {
    switch (opt) {
    case 'a':
      getBuildVer=1;
      break;
    case 'd':
      getWRDate=1;
      break;
    case 'o':
      getWROffset=1;
      break;
    case 'f':
      fileFormat = strtol(optarg, &tail, 0);
      if ((fileFormat < 0) || (fileFormat > MAXFILEFORMATS - 1)) {
        fprintf(stderr, "option file format: '%s' is out of range !\n", optarg);
        exit(1);
      }
      if (*tail != 0) {
        fprintf(stderr, "option file format: specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'j':
      dispInfo=1;
      break;
    case 'm':
      getWRMac=1;
      break;
    case 'l':
      getWRLink=1;
      break;
    case 'i':
      getWRIP=1;
      break;
    case 'c':
      if (fileFormat != 1) {
        fprintf(stderr, "option IP/MAC check type requires option '-f1'!\n");
        exit(1);
      } // option only valid when using ATD file
      if (!getWRIP) {
        fprintf(stderr, "option IP/MAC check type requires option '-i'!\n");
        exit(1);
      } 
      if (!getWRMac) {
        fprintf(stderr, "option IP/MAC check type requires option '-m'!\n");
        exit(1);
      } 
      checkIpMac=1;
      break;
    case 's':
      getWRSync=1;
      break;
    case 'z':
      getWRUptime=1;
      break;
    case 'e':
      getEBVersion=1;
      break;
    case 't':
      printTable=1;
      break;
    case 'q':
      quietMode=1;
      break;
    case 'u':
      printStats=1;
      break;
    case 'w':
      timeout = strtol(optarg, &tail, 0);
      timeout = timeout * 1000; // ms -> us
      if (*tail != 0) {
        fprintf(stderr, "option timeout: specify a proper timeout value, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'x' :
      if (fileFormat != 1) {
        fprintf(stderr, "option network type requires option '-f1'!\n");
        exit(1);
      } // option only valid when using ATD file
      networkType = strtol(optarg, &tail, 0);
      if ((networkType < 0) || (networkType > MAXNETWORKTYPES - 1)) {
        fprintf(stderr, "option network type: '%s' is out of range !\n", optarg);
        exit(1);
      }
      if (*tail != 0) {
        fprintf(stderr, "option network type: specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'y' :
      nodeType = strtol(optarg, &tail, 0);
      if ((nodeType < 0) || (nodeType > MAXNODETYPES - 1)) {
        fprintf(stderr, "option node type: '%s' is out of range !\n", optarg);
        exit(1);
      }
      if (*tail != 0) {
        fprintf(stderr, "option node type: specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
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
    } // switch opt
  } // while opt

  if (error) {
    help();
    return 1;
  }
  
  if (optind + 3 != argc) {
    fprintf(stderr, "%s: expecting two non-optional arguments: <etherbone-protocol> <file with node names> <domain>\n", program);
    fprintf(stderr, "\n");
    help();
    return 1;
  }

  ebProto  = argv[optind];
  if ((strstr(ebProto, "tcp") != NULL) && (nodeType != 1)){
    fprintf(stderr, "usage of protocol 'tcp' with non-SCU is not allowed; use option '-y1'\n");
    return 1;
  } // usage of tcp with non-SCU is not allowed
  if ((strstr(ebProto, "udp") != NULL) && (networkType == 0) && !quietMode){
    fprintf(stderr, "usage of protocol 'udp' for 'all' networks is not allowed; use option '-xn' with non-zero 'n'\n");
    return 1;
  } // usage of tcp with non-SCU is not allowed

  fileName = argv[optind+1];
  domain   = argv[optind+2];
  // finished with getopt stuff
  
  if (getEBVersion) {
    if (verbose) fprintf(stdout, "EB version / EB source: ");
    fprintf(stdout, "%s / %s\n", eb_source_version(), eb_build_info());
  }

  // init data
  nNodes = 0;
  for (i=0; i < MAXNODES; i++) {
    sprintf(nodeName[i]   , "N/A");
    sprintf(buildType[i]  , "N/A");
    sprintf(nodeNetwork[i], "N/A");
    sprintf(nodeInfo[i],    "N/A");
    nodeNsecs64[i]   = ~0;
    nodeOffset[i]    = ~0;
    nodeMac[i]       = ~0;
    atdMac[i]        = ~0;
    nodeLink[i]      = ~0;
    nodeUptime[i]    = ~0;
    nodeSyncState[i] = ~0;
    nodeIp[i]        = ~0;
    nodeUp[i]        = ~0;
    nodeEbStat[i]    = ~0;
  } // for i

  if ((file = fopen(fileName, "r")) == NULL) die("can't open file", EB_OOM);
  fprintf(stdout, "querying nodes...");
  
  while( (!feof(file)) && (nNodes < (MAXNODES - 1))) {
    nodeOk = 1;
    if (fgets(fileLine, MAXLEN, file) != NULL) {
      switch (fileFormat) {
      case 0 :
        if (!parseSimpleLineOk(fileLine, node))                                        nodeOk = 0;
        break;
      case 1 :
        if (!parseATDLineOk(fileLine, network, node, atdTmpStr, &atdTmp64, &atdTmp32)) nodeOk = 0;
        if (!networkOk(network, networkType))                                          nodeOk = 0;
        break;
      default:
        die("file format not supported", EB_OOM);
        return 1;
      } // switch fileformat     
        
      if (!deviceNameOk(ebProto, node))                                                nodeOk = 0;
      if (!nodeTypeOk(node, nodeType))                                                 nodeOk = 0;
      if (!protoOk(ebProto, nodeType))                                                 nodeOk = 0;
          
      if (nodeOk) {
        fprintf(stdout, "%s...", node); fflush(stdout);
        sprintf(devName, "%s/%s.%s", ebProto, node, domain);
        sprintf(nodeName[nNodes], "%s", node);
        sprintf(nodeNetwork[nNodes], "%s", network);
        if (dispInfo)   sprintf(nodeInfo[nNodes], "%s", atdTmpStr);
        if (checkIpMac) {atdMac[nNodes] = atdTmp64; atdIp[nNodes] = atdTmp32;}
        if (!quietMode) {
          // open EB device
          if ((nodeEbStat[nNodes] = wb_open(devName, &device, &socket)) == EB_OK) {
            // check if device is required due to possible unknown MAC addresses in WRS RTU
            if ((status = checkDevGetDateOffset(device, devIndex, &tmp64, &temp64, timeout)) == EB_OK) {
              nodeNsecs64[nNodes] = tmp64;
              nodeOffset[nNodes]  = temp64;
              nodeUp[nNodes]      = 1;
              if (getWRSync)   {if ((status = wb_wr_get_sync_state(device, devIndex, &tmp)) == EB_OK) nodeSyncState[nNodes] = tmp;}
              if (getWRMac)    {if ((status = wb_wr_get_mac(device, devIndex, &tmp64)) == EB_OK)      nodeMac[nNodes]       = tmp64;}
              if (getWRLink)   {if ((status = wb_wr_get_link(device, devIndex, &tmp)) == EB_OK)       nodeLink[nNodes]      = tmp;}
              if (getWRIP)     {if ((status = wb_wr_get_ip(device, devIndex, &tmp)) == EB_OK)         nodeIp[nNodes]        = tmp;}
              if (getWRUptime) {if ((status = wb_wr_get_uptime(device, devIndex, &tmp32)) == EB_OK)   nodeUptime[nNodes]    = tmp32;}
              if (getBuildVer) {if ((status = wb_get_build_type(device, MAXLEN, tmpStr)) == EB_OK)    sprintf(buildType[nNodes], "%s", tmpStr);}
            } // check device
            wb_close(device, socket);
          } // wb_open is ok
        } // not quiet mode
        nNodes++;
      } // nodeOk
    } // line is not empty
  } // while not EOF
  fclose(file);
  fprintf(stdout, "\n");
  
  if (printTable) {
    fprintf(stdout, "\nqueried data from %d nodes in network '%s'\n", nNodes, networkTypeNames[networkType]);
    
    printHeader(getWRDate, getWROffset, getWRSync, getWRMac, getWRLink, getWRIP, getWRUptime, getBuildVer, dispInfo, checkIpMac);
    for (i=0; i<nNodes; i++) {
      fprintf(stdout, "%15s", nodeName[i]);
      fprintf(stdout, ", %10s", nodeNetwork[i]);
      printEbStat(nodeEbStat[i]);
      printUp(nodeUp[i]);
      if (getWRDate)   printDate(nodeNsecs64[i]);
      if (getWROffset) printOffset(nodeOffset[i]);
      if (getWRSync)   printSyncState(nodeSyncState[i]);
      if (getWRLink)   printLink(nodeLink[i]);
      if (getWRUptime) printUptime(nodeUptime[i]);
      if (getBuildVer) printBuildVer(buildType[i]);
      if (getWRMac)    printMac(nodeMac[i]);
      if (getWRIP)     printIp(nodeIp[i]);
      if (checkIpMac)  printCheckIpMac(nodeMac[i], atdMac[i], nodeIp[i], atdIp[i]);
      if (dispInfo)    printInfo(nodeInfo[i]);
      printf("\n");
    } // for all nodes
  } // if print table
  
  if (printStats) {
    fprintf(stdout, "\nqueried data from %d nodes in network '%s'\n", nNodes, networkTypeNames[networkType]);    
    // eb status
    j = 0;
    k = 0;
    l = 0;
    for (i = 0; i < nNodes; i++) {
      if (nodeEbStat[i] == EB_OK)      j++;
      if (nodeEbStat[i] == EB_ADDRESS) k++;
      if (nodeEbStat[i] == EB_TIMEOUT) l++;
    } // for i
    fprintf(stdout, "\n");
    fprintf(stdout, "EB status of nodes\n");
    fprintf(stdout, "----------------------------\n");    
    fprintf(stdout, "ok                     : %d\n", j); 
    fprintf(stdout, "timed out              : %d\n", l);
    fprintf(stdout, "unknown address        : %d\n", k);
    fprintf(stdout, "other errors           : %d\n", (nNodes -j - k -l));
    fprintf(stdout, "\n");
    nReachable = j;

    // nodes
    j = 0;
    k = 0;
    l = 0;
    m = 0;
    for (i = 0; i < nNodes; i++) {
      if (strstr(nodeName[i], nodeTypeNames[1]) != NULL)  j++;
      if (strstr(nodeName[i], nodeTypeNames[2]) != NULL)  k++;
      if (strstr(nodeName[i], nodeTypeNames[3]) != NULL)  l++;
      if (strstr(nodeName[i], nodeTypeNames[4]) != NULL)  m++;
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "type of nodes\n");
    fprintf(stdout, "----------------------------\n");    
    fprintf(stdout, "%-10s             : %d\n", nodeTypeNames[1], j); 
    fprintf(stdout, "%-10s             : %d\n", nodeTypeNames[2], k); 
    fprintf(stdout, "%-10s             : %d\n", nodeTypeNames[3], l); 
    fprintf(stdout, "%-10s             : %d\n", nodeTypeNames[4], m); 
    fprintf(stdout, "\n");
    
    // link up
    if (getWRLink) {
      j = 0;
      for (i = 0; i < nNodes; i++) if (nodeLink[i] == 1) j++;
      fprintf(stdout, "\n");
      fprintf(stdout, "WR link status of nodes\n");
      fprintf(stdout, "----------------------------\n");    
      fprintf(stdout, "link up                : %d\n", j);
      fprintf(stdout, "link down              : %d\n", nReachable - j);
      fprintf(stdout, "\n");
    } // if getWRLink
 
    // WR state
    if (getWRSync) {
      j = 0;
      for (i = 0; i < nNodes; i++) if ((nodeUp[i] == 1) && ( nodeSyncState[i] == WR_PPS_GEN_ESCR_MASK)) j++;
      fprintf(stdout, "\n");
      fprintf(stdout, "WR state of nodes\n");
      fprintf(stdout, "----------------------------\n");    
      fprintf(stdout, "track phase            : %d\n", j);
      fprintf(stdout, "bad                    : %d\n", nReachable - j);
      fprintf(stdout, "\n");
    } // if getWRSync

    // WR offset
    if (getWROffset) {
      j = 0;
      for (i = 0; i < nNodes; i++) if (abs(nodeOffset [i] - UTCOFFSET) < 10) j++;
      fprintf(stdout, "\n");
      fprintf(stdout, "PTP time of nodes \n");
      fprintf(stdout, "----------------------------\n");    
      fprintf(stdout, "looks reasonable       : %d\n", j);
      fprintf(stdout, "bad                    : %d\n", nReachable - j);
      fprintf(stdout, "\n");
    } // if getWROffset

    // IP and MAC correct
    if (checkIpMac) {
      j = 0;
      k = 0;
      for (i = 0; i < nNodes; i++) 
        if (nodeUp[i] == 1) {
          if (nodeMac[i] == atdMac[i]) j++;
          if (nodeIp[i]  == atdIp[i])  k++;
        } // if nodeup
      fprintf(stdout, "\n");
      fprintf(stdout, "MAC of nodes \n");
      fprintf(stdout, "----------------------------\n");    
      fprintf(stdout, "consistent with ATD    : %d\n", j);
      fprintf(stdout, "bad                    : %d\n", nReachable - j);
      fprintf(stdout, "\n");
      fprintf(stdout, "\n");
      fprintf(stdout, "IP of nodes \n");
      fprintf(stdout, "----------------------------\n");    
      fprintf(stdout, "consistent with ATD    : %d\n", k);
      fprintf(stdout, "bad                    : %d\n", nReachable - k);
      fprintf(stdout, "\n");
    } // if checkIpMac


  } // print stats 
    
  return exitCode;
}

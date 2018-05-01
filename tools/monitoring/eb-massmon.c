///////////////////////////////////////////////////////////////////////////////
//  eb-massmon.c
//
//  created : 2018
//  author  : Dietrich Beck, GSI-Darmstadt
//  version : 27-Apr-2018
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
#define EBMASSMON_VERSION "0.0.0"

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
static int verbose=0;
eb_device_t device;        // needs to be global for 1-wire stuff

#define    MAXNODES 1024   // max number of nodes
#define    MAXLEN   1024   // max length of an array



static void die(const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die


static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <etherbone-protocol> <file with node names> <domain>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -a               include FPGA build version\n");
  fprintf(stderr, "  -d               include WR time\n");
  fprintf(stderr, "  -e               display etherbone version\n");
  fprintf(stderr, "  -f<format>       file format\n");
  fprintf(stderr, "                   0: one node per line (default)\n");
  fprintf(stderr, "                   1: ATD format\n");
  fprintf(stderr, "  -h               display this help and exit\n");
  fprintf(stderr, "  -i               include WR IP\n");
  fprintf(stderr, "  -l               include WR link status\n");
  fprintf(stderr, "  -m               include WR MAC\n");
  fprintf(stderr, "  -o               include offset between WR time and system time [ms]\n");
  fprintf(stderr, "  -q               justs parse file and list defices. No hardwar access. Useful for ATD file analysis\n");
  fprintf(stderr, "  -s               include WR sync status\n");
  fprintf(stderr, "  -t               display table with all nodes\n");
  fprintf(stderr, "  -u               display statistics for all nodes\n");
  fprintf(stderr, "  -w<timeout>      timeout [ms] for probing each device (default 1000); might be useful for WRS\n");
  fprintf(stderr, "  -x<network>      network type, only relevant with option '-f1'\n");
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
  fprintf(stderr, "Example: '%s udp allTimingDevices.txt timing.acc.gsi.de -f1 -x1 -y1 -o -l -s -z -t -q'\n", program);
  fprintf(stderr, "         query all SCUs of production network; here: silent mode (option '-q')\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <d.beck@gsi.de>\n");
  fprintf(stderr, "Version %s. Licensed under the LGPL v3.\n", EBMASSMON_VERSION);
} //help


eb_status_t checkDevGetDateOffset(eb_device_t device, int devIndex, uint64_t *nsecs, uint64_t *offset, uint32_t timeout)
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

int deviceNameOk(const char* proto, char* name)
{
  int   type_ok  = 0;  // device type ok?
  int   proto_ok = 0;  // EB proto ok?
  char  *ptr;

  // white list: check for known node types
  if (strstr(name, "scuxl")   != NULL)  type_ok = 1;
  if (strstr(name, "pexaria") != NULL)  type_ok = 1;
  if (strstr(name, "vmel")    != NULL)  type_ok = 1;
  if (strstr(name, "expl")    != NULL)  type_ok = 1;

  // white list: only allow upd and tcp
  if (strstr(proto, "udp")    != NULL)  proto_ok = 1;
  if (strstr(proto, "tcp")    != NULL)  proto_ok = 1;
  
  // strip trailing "t" from name in case proto is "tcp"
  // check: this fails in case node future names may contain a non-trainling "t". But ok for now ...
  if (strstr(proto, "tcp") != NULL) {
    ptr = strtok(name, "t");
    sprintf(name, "%s", ptr);
  }

  // exclude unreasonable length of name ("pexaria1234t" is 12 chars and longest name)
  if(strlen(name) > 12) type_ok =  0;    

  // exclude unreasonable length of name ("scuxl123" is 8 chars and shortest name)
  if(strlen(name) < 8)  type_ok =  0;

  // printf("name %s len %d\n", name, strlen(name));
     
  if (type_ok && proto_ok) return 1;
  else                     return 0;
} //deviceNameOk

int nodeTypeOk(char *node, int nodeType)
{
  int type_ok = 1;

  // exclude node in case nodeType != 0 (0: all node types)
  if (nodeType != 0) {
    if ((strstr(node, "scuxl")   != NULL)  && (nodeType != 1)) type_ok = 0;
    if ((strstr(node, "pexaria") != NULL)  && (nodeType != 2)) type_ok = 0;
    if ((strstr(node, "vmel")    != NULL)  && (nodeType != 3)) type_ok = 0;
    if ((strstr(node, "expl")    != NULL)  && (nodeType != 4)) type_ok = 0;
  } // if exclude nodes

  return type_ok;
} // nodeTypeOk

int protoOk(const char *proto, int nodeType)
{
  // exclude tcp and non-SCU 
  if ((strstr(proto, "tcp")    != NULL) && (nodeType != 1)) return 0;
  else                                                      return 1;
} // protoOk

void removeNonAlphaNumeric(char *text)                                                  
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

int parseSimpleLineOk(char* fileLine, char* node)
{
  removeNonAlphaNumeric(fileLine);
  sprintf(node, "%s", fileLine);
  return 1;
} // parseSimpleLine

int parseATDLineOk(char* fileLine, char* network, char* node)
{
  // format '00:26:7b:00:03:ff,id:timing,192.168.161.1,scuxl0249t       #NW Production; SCU; ...
  char* ptr1;
  char* ptr2;
  int   node_ok    = 0;
  int   network_ok = 0;

  // parse until node name
  ptr1 = strtok(fileLine, ",");
  ptr1 = strtok(NULL,     ",");
  ptr1 = strtok(NULL,     ",");
  ptr1 = strtok(NULL,     " ");
  if (ptr1 != NULL) {
    // valid hostname
    sprintf(node, "%s", ptr1);
    node_ok = 1;

    // parse until network
    ptr1 = strtok(NULL,     "#NW ");
    if (ptr1 != NULL) {
      ptr2 = strtok(ptr1, ";");
      if (ptr2 != NULL) {
        // valid network
        sprintf(network, "%s", ptr2);
        network_ok = 1;
      } // valid network
    } // valid format
  } // valid hostname

  removeNonAlphaNumeric(node);
  
  if (node_ok && network_ok) return 1;
  else                       return 0;
} // parseATDLine

int networkOk(char* network, int networkType)
{
  int allow = 1;

  switch (networkType) {
  case 0 :
    // all all nodes
    break;
  case 1 :
    if (strcmp(network, "Production") != 0) allow = 0;
    break;
  case 2 :
    if (strcmp(network, "User")       != 0) allow = 0;
    break;
  case 3 :
    if (strcmp(network, "Timing")     != 0) allow = 0;
    break;
  default: ;
  } // switch network type
  
  return allow;
} // network ok

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
    fprintf(stdout, ", %8s", "---");
    break;
  case EB_OOM :
    fprintf(stdout, ", %8s", "NO RAM");
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

static void printHeader(int wrDate, int wrOffset, int wrSync, int wrMac, int wrLink, int wrIp, int wrUptime, int buildVer)
{
  // always print header
  fprintf(stdout,   "%15s",  "node");
  fprintf(stdout,   ", %8s", "ebStatus");
  fprintf(stdout,   ", %2s", "up");

  // depends on request
  if (wrDate)   fprintf(stdout, ", %10s", "time [s]");
  if (wrOffset) fprintf(stdout, ", %13s", "offset[ms]");
  if (wrSync)   fprintf(stdout, ", %8s",  "sync");
  if (wrLink)   fprintf(stdout, ", %4s",  "link");
  if (wrUptime) fprintf(stdout, ", %9s",  "uptime[h]");
  if (buildVer) fprintf(stdout, ", %8s",  "gwBuild");
  if (wrMac)    fprintf(stdout, ", %12s", "MAC");
  if (wrIp)     fprintf(stdout, ", %15s", "IP");

  // always linefeed  
  fprintf(stdout, "\n");
} // print Header

int main(int argc, char** argv) {
  eb_status_t       status;
  eb_socket_t       socket;
  int               devIndex=0;       // grab 1st device on the WB bus
  char              devName[MAXLEN];
  char              network[MAXLEN];
  char              node[MAXLEN];

  const char* ebProto;                // EB protocol
  const char* fileName;
  const char* domain;                 // domain '.gsi.de'
  FILE*       file;                   // file with node names
  char        fileLine[MAXLEN];       // line in file
  int         nNodes;                 // number of nodes
  int         nodeOk;                 // flag a node (if it is ok: syntax, combination...

  int         getEBVersion = 0;
  int         getWRDate    = 0;
  int         getWROffset  = 0;
  int         getWRSync    = 0;
  int         getWRMac     = 0;
  int         getWRLink    = 0;
  int         getWRIP      = 0;
  int         getWRUptime  = 0;
  int         getBuildVer  = 0;
  int         exitCode     = 0;
  int         timeout      = 1000000; // [us]

  int         fileFormat   = 0;         // 0: text file, one hostname per line
  int         printTable   = 0;         
  int         printStats   = 0;
  int         networkType  = 0;
  int         nodeType     = 0;
  int         quietMode    = 0;

  uint64_t    tmp64;
  uint32_t    tmp32;
  int         tmp;
  char        tmpStr[MAXLEN];
  char        nodeName[MAXNODES][MAXLEN];      
  char        buildType[MAXNODES][MAXLEN];      
  uint64_t    nodeNsecs64[MAXNODES];
  uint64_t    nodeOffset[MAXNODES];
  uint64_t    nodeMac[MAXNODES];
  uint32_t    nodeLink[MAXNODES];
  uint32_t    nodeUptime[MAXNODES];
  uint32_t    nodeSyncState[MAXNODES];
  uint32_t    nodeIp[MAXNODES];
  uint32_t    nodeUp[MAXNODES];
  eb_status_t nodeEbStat[MAXNODES];

  uint64_t    nsecs;
  uint64_t    offset;

  int i;

  int opt, error=0;
  char *tail;

  program = argv[0];

  while ((opt = getopt(argc, argv, "f:w:x:y:adehilmoqstuz")) != -1) {
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
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
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
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'x' :
      networkType = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
        exit(1);
      } // if *tail
      break;
    case 'y' :
      nodeType = strtol(optarg, &tail, 0);
      if (*tail != 0) {
        fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
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
    sprintf(nodeName[i] , "N/A");
    sprintf(buildType[i], "N/A");
    nodeNsecs64[i]   = ~0;
    nodeOffset[i]    = ~0;
    nodeMac[i]       = ~0;
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
        if (!parseSimpleLineOk(fileLine, node))       nodeOk = 0;
        break;
      case 1 :
        if (!parseATDLineOk(fileLine, network, node)) nodeOk = 0;
        if (!networkOk(network, networkType))         nodeOk = 0;
        break;
      default:
        die("file format not supported", EB_OOM);
        return 1;
      } // switch fileformat     
        
      if (!deviceNameOk(ebProto, node))               nodeOk = 0;
      if (!nodeTypeOk(node, nodeType))                nodeOk = 0;
      if (!protoOk(ebProto, nodeType))                nodeOk = 0;
          
      if (nodeOk) {
        fprintf(stdout, "%s...", node); fflush(stdout);
        sprintf(devName, "%s/%s.%s", ebProto, node, domain);
        sprintf(nodeName[nNodes], "%s", node);
        if (!quietMode) {
          // open EB device
          if ((nodeEbStat[nNodes] = wb_open(devName, &device, &socket)) == EB_OK) {
            // check if device is required due to possible unknown MAC addresses in WRS RTU
            if ((status = checkDevGetDateOffset(device, devIndex, &nsecs, &offset, timeout)) == EB_OK) {
              nodeNsecs64[nNodes] = nsecs;
              nodeOffset[nNodes]  = offset;
              nodeUp[nNodes]   = 1;
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
  fprintf(stdout, "querried data from %d nodes\n", nNodes);

  if (printTable) {
    printHeader(getWRDate, getWROffset, getWRSync, getWRMac, getWRLink, getWRIP, getWRUptime, getBuildVer);
    for (i=0; i<nNodes; i++) {
      fprintf(stdout, "%15s", nodeName[i]);
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
      fprintf(stdout,"\n");
      
    } // for all nodes
      
  } // if printStats
    
    
  return exitCode;
}

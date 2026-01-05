/********************************************************************************************
 *  common-lib.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-Jan-2025
 *
 *  common x86 routines useful for CLIs handling firmware
 * 
 *  see common-lib.h for version, license and documentation 
 *
 ********************************************************************************************/
// standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h>
#include <math.h>

// etherbone
#include <etherbone.h>

// common stuff
#include <common-defs.h>
#include <common-lib.h>

// common core code
#include <common-core.c>

// eca queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h"   // register layout ECA queue
#include "../../../ip_cores/saftlib/src/eca_flags.h"                    // definitions for ECA queue

// public variables
eb_address_t common_statusLo;       // common status, read (low word)
eb_address_t common_statusHi;       // common status, read (high word)
eb_address_t common_state;          // state, read
eb_address_t common_transfers;      // # of transfers from UNILAC to SIS, read
eb_address_t common_injections;     // # of injections in ongoing transfer
eb_address_t common_statTrans;      // status of ongoing or last transfer, read
eb_address_t common_nLate;          // number of ECA 'late' incidents                                             
eb_address_t common_nEarly;         // number of ECA 'early' incidents                                           
eb_address_t common_nConflict;      // number of ECA 'conflict' incidents                                        
eb_address_t common_nDelayed;       // number of ECA 'delayed' incidents                                         
eb_address_t common_nSlow;          // number of incidents, when 'wait4eca' was called after the deadline        
eb_address_t common_offsSlow;       // if 'slow': offset deadline to start wait4eca; else '0' [ns]
eb_address_t common_offsSlowMax;    // if 'slow': offset deadline to start wait4eca; else '0' [ns]; max
eb_address_t common_offsSlowMin;    // if 'slow': offset deadline to start wait4eca; else '0' [ns]; min
eb_address_t common_comLatency;     // if 'slow': offset start to stop wait4eca; else deadline to stop wait4eca [ns]
eb_address_t common_comLatencyMax;  // if 'slow': offset start to stop wait4eca; else deadline to stop wait4eca [ns]; max
eb_address_t common_comLatencyMin;  // if 'slow': offset start to stop wait4eca; else deadline to stop wait4eca [ns]; min
eb_address_t common_offsDone;       // offset event deadline to time when we are done [ns]
eb_address_t common_offsDoneMax;    // offset event deadline to time when we are done [ns]; max
eb_address_t common_offsDoneMin;    // offset event deadline to time when we are done [ns]; min
eb_address_t common_cmd;            // command, write
eb_address_t common_version;        // version, read
eb_address_t common_nBadStatus;     // # of bad status ("ERROR") incidents
eb_address_t common_nBadState;      // # of bad state ("not in operation") incidents
eb_address_t common_macHi;          // MAC address, high word
eb_address_t common_macLo;          // MAC address, low word
eb_address_t common_Ip;             // IP
eb_address_t common_tDiagHi;        // time when diagnostics was cleared, high bits
eb_address_t common_tDiagLo;        // time when diagnostics was cleared, low bits
eb_address_t common_tS0Hi;          // time when FW was in S0 state (start of FW), high bits
eb_address_t common_tS0Lo;          // time when FW was in S0 state (start of FW), low bits
eb_address_t common_usedSize;       // used size of DP RAM

// public variables
eb_socket_t  common_socket;       // EB socket

/*
static void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s failed: %s\n", where, eb_status(status));
  exit(1);
} //die
*/

// get host system time
uint64_t comlib_getSysTime()
{
  uint64_t t;
  
  struct timeval tv;
  gettimeofday(&tv,NULL);
  t = tv.tv_sec*(uint64_t)1000000000+tv.tv_usec*1000;

  // argh: timespec not supported with old gcc on sl7
  //struct timespec ts;

  //timespec_get(&ts, TIME_UTC);

  //t = 1000000000 * (uint64_t)(ts.tv_sec) + (uint64_t)(ts.tv_nsec);

  return t;
} // comlib_getSysTime()


void comlib_nsleep(uint64_t t)
{
  struct timespec time;       // time to sleep
  struct timespec remaining;
  uint64_t        secs;
  uint64_t        nsecs;

  secs  = floor(t / 1000000000);
  nsecs = t - secs * 1000000000;

  time.tv_sec  = secs;
  time.tv_nsec = nsecs;

  nanosleep(&time, &remaining);
} // comlib_nsleep


// get character from stdin, 0: no character
char comlib_term_getChar()
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
} // comLib_term_getChar


// clear teminal windows and jump to 1,1
void comlib_term_clear()
{
  printf("\033[2J\033[1;1H");
} // comlib_term_clear


// move cursor position in terminal
void comlib_term_curpos(int column, int line)
{
  printf("\033[%d;%dH", line, column);
} // comlib_term_curpos


// returns state text
const char* comlib_stateText(uint32_t code)
{
  switch (code) {
  case COMMON_STATE_UNKNOWN      : return "UNKNOWN   ";
  case COMMON_STATE_S0           : return "S0        ";
  case COMMON_STATE_IDLE         : return "IDLE      ";                                       
  case COMMON_STATE_CONFIGURED   : return "CONFIGURED";
  case COMMON_STATE_OPREADY      : return "OpReady   ";
  case COMMON_STATE_STOPPING     : return "STOPPING  ";
  case COMMON_STATE_ERROR        : return "ERROR     ";
  case COMMON_STATE_FATAL        : return "FATAL(RIP)";
  default                        : return "undefined ";
  }
} // comlib_stateText


const char* comlib_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // comlib_version_text


// returns status text
const char* comlib_statusText(uint32_t bit)
{  
  static char message[256];

  switch (bit) {
    case COMMON_STATUS_OK               : sprintf(message, "OK"); break;
    case COMMON_STATUS_ERROR            : sprintf(message, "error %d, %s",    bit, "an error occured"); break;
    case COMMON_STATUS_TIMEDOUT         : sprintf(message, "error %d, %s",    bit, "a timeout occured"); break;
    case COMMON_STATUS_OUTOFRANGE       : sprintf(message, "error %d, %s",    bit, "some value is out of range"); break;
    case COMMON_STATUS_EB               : sprintf(message, "error %d, %s",    bit, "an Etherbone error occured"); break;
    case COMMON_STATUS_NOIP             : sprintf(message, "error %d, %s",    bit, "DHCP request via WR network failed"); break;
    case COMMON_STATUS_WRONGIP          : sprintf(message, "error %d, %s",    bit, "IP received via DHCP does not match local config"); break;
    case COMMON_STATUS_EBREADTIMEDOUT   : sprintf(message, "error %d, %s",    bit, "EB read via WR network timed out"); break;
    case COMMON_STATUS_WRBADSYNC        : sprintf(message, "error %d, %s",    bit, "White Rabbit: not in 'TRACK_PHASE'"); break;
    case COMMON_STATUS_AUTORECOVERY     : sprintf(message, "errorFix %d, %s", bit, "attempting auto-recovery from state ERROR"); break;
    case COMMON_STATUS_LATEMESSAGE      : sprintf(message, "error %d, %s",    bit, "late timing message received"); break;
    case COMMON_STATUS_BADSETTING       : sprintf(message, "error %d, %s",    bit, "bad setting data"); break;
    default                             : sprintf(message, "error %d, %s",    bit, "undefined error code"); break;
  }

  return message;
} // comlib_statusStext


// init for communicaiton with shared mem
void comlib_initShared(eb_address_t lm32_base, eb_address_t sharedOffset)
{
  common_statusLo      = lm32_base + sharedOffset + COMMON_SHARED_STATUSLO;
  common_statusHi      = lm32_base + sharedOffset + COMMON_SHARED_STATUSHI;
  common_cmd           = lm32_base + sharedOffset + COMMON_SHARED_CMD;
  common_state         = lm32_base + sharedOffset + COMMON_SHARED_STATE;
  common_version       = lm32_base + sharedOffset + COMMON_SHARED_VERSION;
  common_macHi         = lm32_base + sharedOffset + COMMON_SHARED_MACHI;
  common_macLo         = lm32_base + sharedOffset + COMMON_SHARED_MACLO;
  common_Ip            = lm32_base + sharedOffset + COMMON_SHARED_IP;
  common_nBadStatus    = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATUS;
  common_nBadState     = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATE;
  common_tDiagHi       = lm32_base + sharedOffset + COMMON_SHARED_TDIAGHI;
  common_tDiagLo       = lm32_base + sharedOffset + COMMON_SHARED_TDIAGLO;
  common_tS0Hi         = lm32_base + sharedOffset + COMMON_SHARED_TS0HI;
  common_tS0Lo         = lm32_base + sharedOffset + COMMON_SHARED_TS0LO;
  common_transfers     = lm32_base + sharedOffset + COMMON_SHARED_NTRANSFER;
  common_injections    = lm32_base + sharedOffset + COMMON_SHARED_NINJECT;
  common_statTrans     = lm32_base + sharedOffset + COMMON_SHARED_TRANSSTAT;
  common_nLate         = lm32_base + sharedOffset + COMMON_SHARED_NLATE;
  common_nEarly        = lm32_base + sharedOffset + COMMON_SHARED_NEARLY;
  common_nConflict     = lm32_base + sharedOffset + COMMON_SHARED_NCONFLICT;
  common_nDelayed      = lm32_base + sharedOffset + COMMON_SHARED_NDELAYED;
  common_nSlow         = lm32_base + sharedOffset + COMMON_SHARED_NSLOW;
  common_offsSlow      = lm32_base + sharedOffset + COMMON_SHARED_OFFSSLOW;
  common_offsSlowMax   = lm32_base + sharedOffset + COMMON_SHARED_OFFSSLOWMAX;
  common_offsSlowMin   = lm32_base + sharedOffset + COMMON_SHARED_OFFSSLOWMIN;
  common_comLatency    = lm32_base + sharedOffset + COMMON_SHARED_COMLATENCY;
  common_comLatencyMax = lm32_base + sharedOffset + COMMON_SHARED_COMLATENCYMAX;
  common_comLatencyMin = lm32_base + sharedOffset + COMMON_SHARED_COMLATENCYMIN;
  common_offsDone      = lm32_base + sharedOffset + COMMON_SHARED_OFFSDONE;
  common_offsDoneMax   = lm32_base + sharedOffset + COMMON_SHARED_OFFSDONEMAX;
  common_offsDoneMin   = lm32_base + sharedOffset + COMMON_SHARED_OFFSDONEMIN;
  common_usedSize      = lm32_base + sharedOffset + COMMON_SHARED_USEDSIZE;
} // comlib_initShared


void comlib_printDiag(uint64_t statusArray, uint32_t state, uint32_t version, uint64_t mac, uint32_t ip, uint32_t nBadStatus, uint32_t nBadState, uint64_t tDiag, uint64_t tS0,
                      uint32_t nTransfer, uint32_t nInjection, uint32_t statTrans, uint32_t nLate, uint32_t offsDone, uint32_t comLatency, uint32_t usedSize)
{
  comlib_printDiag2(statusArray, state, version, mac, ip, nBadStatus, nBadState, tDiag, tS0, nTransfer, nInjection, statTrans, nLate, 0, 0, 0, 0, 0, 0, 0, comLatency, 0, 0, offsDone, 0, 0, usedSize);
} // comlib_printDiag


void comlib_printDiag2(uint64_t statusArray, uint32_t state, uint32_t version, uint64_t mac, uint32_t ip, uint32_t nBadStatus, uint32_t nBadState, uint64_t tDiag, uint64_t tS0,
                       uint32_t nTransfer, uint32_t nInjection, uint32_t statTrans, uint32_t nLate, uint32_t nEarly, uint32_t nConflict, uint32_t nDelayed, uint32_t nSlow,
                       uint32_t offsSlow, uint32_t offsSlowMax, uint32_t offsSlowMin, uint32_t comLatency, uint32_t comLatencyMax, uint32_t comLatencyMin,
                       uint32_t offsDone, uint32_t offsDoneMax, uint32_t offsDoneMin, uint32_t usedSize)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;
  int              i;

  // display min offset values as '0' not as '4e9'
  if (offsSlowMin   == 0xffffffff) offsSlowMin   = 0;
  if (comLatencyMin == 0xffffffff) comLatencyMin = 0;
  if (offsDoneMin   == 0xffffffff) offsDoneMin   = 0;

  printf("common: diags ...\n");

  secs     = (unsigned long)((double)tS0 / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("firmware boot at                    : %s\n", timestr);

  secs     = (unsigned long)((double)tDiag / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("diagnostics reset at                : %s\n"       , timestr);
  printf("version                             : %06x\n"     , version);
  printf("mac                                 : 0x%012" PRIx64 "\n", mac);
  printf("ip                                  : %03d.%03d.%03d.%03d\n", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  printf("used shared mem [byte]              : %u\n"       , usedSize);
  printf("state (# of changes)                : %s (%u)\n"  , comlib_stateText(state), nBadState);
  printf("# of transfers                      : %012u\n"    , nTransfer);
  printf("# of injections                     : %012u\n"    , nInjection);  
  printf("status of act transfer              : 0x%x\n"     , statTrans);
  printf("# late events                       : %012u\n"    , nLate);
  printf("# early events                      : %012u\n"    , nEarly);
  printf("# conflict events                   : %012u\n"    , nConflict);
  printf("# delayed events                    : %012u\n"    , nDelayed);
  printf("# slow events (wait too late)       : %012u\n"    , nSlow);
  printf("offset slow     [us]                : %12.3f\n"   , (double)offsSlow/1000.0);
  printf("offset slow max [us]                : %12.3f\n"   , (double)offsSlowMax/1000.0);
  printf("offset slow min [us]                : %12.3f\n"   , (double)offsSlowMin/1000.0);
  printf("communication latency     [us]      : %12.3f\n"   , (double)comLatency/1000.0);
  printf("communication latency max [us]      : %12.3f\n"   , (double)comLatencyMax/1000.0);
  printf("communication latency min [us]      : %12.3f\n"   , (double)comLatencyMin/1000.0);
  printf("processing time     [us]            : %12.3f\n"   , (double)offsDone/1000.0);
  printf("processing time max [us]            : %12.3f\n"   , (double)offsDoneMax/1000.0);
  printf("processing time min [us]            : %12.3f\n"   , (double)offsDoneMin/1000.0);
  printf("sum status (# changes)              : 0x%" PRIx64 " (%u)\n"     , statusArray, nBadStatus);
  if ((statusArray >> COMMON_STATUS_OK) & 0x1)
    printf("overall status                      : OK\n");
  else
    printf("overall status                      : NOT OK\n");  
  for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
    if ((statusArray >> i) & 0x1)
      printf("    status bit is set               : %s\n", comlib_statusText(i));
  } // for i
} // comlib_printDiag2;


int comlib_readDiag(eb_device_t device, uint64_t  *statusArray, uint32_t  *state, uint32_t  *version, uint64_t  *mac, uint32_t  *ip, uint32_t  *nBadStatus,
                    uint32_t *nBadState, uint64_t  *tDiag, uint64_t  *tS0, uint32_t  *nTransfer, uint32_t  *nInjection, uint32_t  *statTrans,
                    uint32_t *nLate, uint32_t *offsDone, uint32_t *comLatency, uint32_t *usedSize, int  printFlag)
{
  uint32_t nEarly;
  uint32_t nConflict;
  uint32_t nDelayed;
  uint32_t nSlow;
  uint32_t offsSlow;
  uint32_t offsSlowMax;
  uint32_t offsSlowMin;
  uint32_t comLatencyMax;
  uint32_t comLatencyMin;
  uint32_t offsDoneMax;
  uint32_t offsDoneMin;
  
  return(comlib_readDiag2(device, statusArray, state, version, mac, ip, nBadStatus,  nBadState, tDiag, tS0, nTransfer, nInjection, statTrans,
                          nLate, &nEarly, &nConflict, &nDelayed, &nSlow, &offsSlow, &offsSlowMax, &offsSlowMin, comLatency, &comLatencyMax, &comLatencyMin,
                          offsDone, &offsDoneMax, &offsDoneMin, usedSize, printFlag));
} // comlib_readDiag


int comlib_readDiag2(eb_device_t device, uint64_t  *statusArray, uint32_t  *state, uint32_t  *version, uint64_t  *mac, uint32_t  *ip, uint32_t  *nBadStatus,
                     uint32_t *nBadState, uint64_t  *tDiag, uint64_t  *tS0, uint32_t  *nTransfer, uint32_t  *nInjection, uint32_t  *statTrans,
                     uint32_t *nLate, uint32_t *nEarly, uint32_t *nConflict, uint32_t *nDelayed, uint32_t *nSlow, uint32_t *offsSlow, uint32_t *offsSlowMax,
                     uint32_t *offsSlowMin, uint32_t *comLatency, uint32_t *comLatencyMax, uint32_t *comLatencyMin, uint32_t *offsDone, uint32_t *offsDoneMax,
                     uint32_t *offsDoneMin, uint32_t *usedSize, int  printFlag)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[31];
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return eb_status;
  eb_cycle_read(cycle, common_statusHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, common_statusLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, common_state,          EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, common_version,        EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, common_macHi,          EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, common_macLo,          EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, common_Ip,             EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, common_nBadStatus,     EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, common_nBadState,      EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, common_tDiagHi,        EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, common_tDiagLo,        EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, common_tS0Hi,          EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, common_tS0Lo,          EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, common_transfers,      EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, common_injections,     EB_BIG_ENDIAN|EB_DATA32, &(data[14]));  
  eb_cycle_read(cycle, common_statTrans,      EB_BIG_ENDIAN|EB_DATA32, &(data[15]));
  eb_cycle_read(cycle, common_nLate,          EB_BIG_ENDIAN|EB_DATA32, &(data[16]));
  eb_cycle_read(cycle, common_nEarly,         EB_BIG_ENDIAN|EB_DATA32, &(data[17]));
  eb_cycle_read(cycle, common_nConflict,      EB_BIG_ENDIAN|EB_DATA32, &(data[18]));
  eb_cycle_read(cycle, common_nDelayed,       EB_BIG_ENDIAN|EB_DATA32, &(data[19]));
  eb_cycle_read(cycle, common_nSlow,          EB_BIG_ENDIAN|EB_DATA32, &(data[20]));
  eb_cycle_read(cycle, common_offsSlow,       EB_BIG_ENDIAN|EB_DATA32, &(data[21]));
  eb_cycle_read(cycle, common_offsSlowMax,    EB_BIG_ENDIAN|EB_DATA32, &(data[22]));
  eb_cycle_read(cycle, common_offsSlowMin,    EB_BIG_ENDIAN|EB_DATA32, &(data[23]));
  eb_cycle_read(cycle, common_comLatency,     EB_BIG_ENDIAN|EB_DATA32, &(data[24]));
  eb_cycle_read(cycle, common_comLatencyMax,  EB_BIG_ENDIAN|EB_DATA32, &(data[25]));
  eb_cycle_read(cycle, common_comLatencyMin,  EB_BIG_ENDIAN|EB_DATA32, &(data[26]));
  eb_cycle_read(cycle, common_offsDone,       EB_BIG_ENDIAN|EB_DATA32, &(data[27]));
  eb_cycle_read(cycle, common_offsDoneMax,    EB_BIG_ENDIAN|EB_DATA32, &(data[28]));
  eb_cycle_read(cycle, common_offsDoneMin,    EB_BIG_ENDIAN|EB_DATA32, &(data[29]));
  eb_cycle_read(cycle, common_usedSize,       EB_BIG_ENDIAN|EB_DATA32, &(data[30]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) return eb_status;

  *statusArray      = ((uint64_t)(data[0]) << 32) | (uint64_t)(data[1]);
  *state            = data[2];
  *version          = data[3];
  *mac              = ((uint64_t)(data[4]) << 32) | (uint64_t)(data[5]);
  *ip               = data[6];
  *nBadStatus       = data[7];
  *nBadState        = data[8];
  *tDiag            = ((uint64_t)(data[9])  << 32) | (uint64_t)(data[10]);
  *tS0              = ((uint64_t)(data[11]) << 32) | (uint64_t)(data[12]);
  *nTransfer        = data[13];
  *nInjection       = data[14];
  *statTrans        = data[15];
  *nLate            = data[16];
  *nEarly           = data[17];
  *nConflict        = data[18];
  *nDelayed         = data[19];
  *nSlow            = data[20];
  *offsSlow         = data[21];
  *offsSlowMax      = data[22];
  *offsSlowMin      = data[23];
  *comLatency       = data[24];
  *comLatencyMax    = data[25];
  *comLatencyMin    = data[26];
  *offsDone         = data[27];
  *offsDoneMax      = data[28];
  *offsDoneMin      = data[29];
  *usedSize         = data[30];

  if (printFlag) comlib_printDiag2(*statusArray, *state, *version, *mac, *ip, *nBadStatus, *nBadState, *tDiag, *tS0, *nTransfer, *nInjection, *statTrans,
                                   *nLate, *nEarly, *nConflict, *nDelayed, *nSlow, *offsSlow, *offsSlowMax, *offsSlowMin,
                                   *comLatency, *comLatencyMax, *comLatencyMin, *offsDone, *offsDoneMax, *offsDoneMin, *usedSize);

  return eb_status;
} // comlib_readDiag2


uint32_t comlib_ecaq_open(const char* devName, uint32_t qIdx, eb_device_t *device, eb_address_t *ecaq_base)
{
  eb_status_t         status;
  int                 nDevices;                            // number of instantiated queues
  int                 maxDev = 3;                          // three ECA queues exist on a standard TR
  struct sdb_device   sdbDevice[maxDev];                   // instantiated ECA queues

  *device     = 0x0;
  *ecaq_base  = 0x0;
  nDevices    = maxDev;

  // open Etherbone device and socket
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDRX|EB_DATAX, &common_socket)) != EB_OK)    return COMMON_STATUS_EB;
  if ((status = eb_device_open(common_socket, devName, EB_ADDRX|EB_DATAX, 3, device)) != EB_OK) return COMMON_STATUS_EB;

  //  get Wishbone address of ecaq
  if ((status = eb_sdb_find_by_identity(*device, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID, sdbDevice, &nDevices)) != EB_OK) return COMMON_STATUS_EB;
  if (nDevices == 0)       return COMMON_STATUS_EB;
  //if (nDevices > maxDev)   return COMMON_STATUS_EB;
  if ((uint32_t)nDevices < qIdx + 1) return COMMON_STATUS_EB;
  *ecaq_base  = sdbDevice[qIdx].sdb_component.addr_first;

  //printf("open eca q, nDevices %d, idx %d, ecaq_base %lx\n", nDevices, qIdx, (uint32_t)(*ecaq_base));

  return COMMON_STATUS_OK;
} // comlib_ecaq_open


uint32_t comlib_ecaq_close(eb_device_t device)
{
  eb_status_t status;

  if (!device) return COMMON_STATUS_EB;

  // close Etherbone device and socket
  if ((status = eb_device_close(device))        != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(common_socket)) != EB_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // comlib_ecaq_close


uint32_t comlib_wait4ECAEvent(uint32_t timeout_ms,  eb_device_t device, eb_address_t ecaq_base, uint32_t *tag, uint64_t *deadline, uint64_t *evtId, uint64_t *param, uint32_t *tef, uint32_t *isLate, uint32_t *isEarly, uint32_t *isConflict, uint32_t *isDelayed)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];
  uint32_t    ecaFlag;             // ECA flag
  uint32_t    evtIdHigh;           // high 32bit of eventID   
  uint32_t    evtIdLow;            // low 32bit of eventID    
  uint32_t    evtDeadlHigh;        // high 32bit of deadline  
  uint32_t    evtDeadlLow;         // low 32bit of deadline   
  uint32_t    evtParamHigh;        // high 32 bit of parameter field
  uint32_t    evtParamLow ;        // low 32 bit of parameter field
  uint64_t    timeoutT;            // when to time out
  uint64_t    timeout;             // timeout
  //int32_t     t1, t2;

  timeout  = ((uint64_t)timeout_ms + 1) * 1000000;
  timeoutT = comlib_getSysTime() + timeout;

  while (comlib_getSysTime() < timeoutT) {
    // read flag from ECA queue
    if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return COMMON_STATUS_EB;
    eb_cycle_read(cycle, ecaq_base + ECA_QUEUE_FLAGS_GET, EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
    if ((eb_status = eb_cycle_close(cycle)) != EB_OK) return COMMON_STATUS_EB;
    ecaFlag = data[0];
   
    if (ecaFlag & (0x0001 << ECA_VALID)) {                          // if ECA data is valid

      // read data
      if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return COMMON_STATUS_EB;
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_EVENT_ID_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_EVENT_ID_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_DEADLINE_HI_GET, EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_DEADLINE_LO_GET, EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_TAG_GET        , EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_PARAM_HI_GET   , EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_PARAM_LO_GET   , EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
      eb_cycle_read(cycle,  ecaq_base + ECA_QUEUE_TEF_GET        , EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
      if ((eb_status = eb_cycle_close(cycle)) != EB_OK) return COMMON_STATUS_EB;

      // pop element from q
      if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return COMMON_STATUS_EB;
      eb_cycle_write(cycle, ecaq_base + ECA_QUEUE_POP_OWR        , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)0x1);
      if ((eb_status = eb_cycle_close(cycle)) != EB_OK) return COMMON_STATUS_EB;

      // copy data
      evtIdHigh    = data[0];
      evtIdLow     = data[1];
      evtDeadlHigh = data[2];
      evtDeadlLow  = data[3];
      *tag         = data[4];
      evtParamHigh = data[5];
      evtParamLow  = data[6];
      *tef         = data[7];
      
      *isLate      = ecaFlag & (0x0001 << ECA_LATE);
      *isEarly     = ecaFlag & (0x0001 << ECA_EARLY);
      *isConflict  = ecaFlag & (0x0001 << ECA_CONFLICT);
      *isDelayed   = ecaFlag & (0x0001 << ECA_DELAYED);
      *deadline    = ((uint64_t)evtDeadlHigh << 32) + (uint64_t)evtDeadlLow;
      *evtId       = ((uint64_t)evtIdHigh    << 32) + (uint64_t)evtIdLow;
      *param       = ((uint64_t)evtParamHigh << 32) + (uint64_t)evtParamLow;
      
      return COMMON_STATUS_OK;
    } // if data is valid
    
    comlib_nsleep(100*1000);  // sleep 100 us
  } // while not timed out

  *tag        = 0x0;
  *deadline   = 0x0;
  *evtId      = 0x0;
  *param      = 0x0;
  *tef        = 0x0;
  *isLate     = 0x0;
  *isEarly    = 0x0;
  *isConflict = 0x0;
  *isDelayed  = 0x0;

  //t2 = comlib_getSysTime(); printf("eca wait, timeout [ns] %ld\n", (int32_t)(t2 - timeoutT_ns));
  
  return COMMON_STATUS_TIMEDOUT;
} // comlib_wait4ECAEvent


uint16_t comlib_float2half(float f)
{
  return comcore_float2half(f);
} //comlib_float2half


float comlib_half2float(uint16_t h){
  return comcore_half2float(h);
} // comlib_half2float

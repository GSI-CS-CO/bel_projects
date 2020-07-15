/********************************************************************************************
 *  common-lib.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-Apr-2020
 *
 *  common x86 routines for firmware
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

// etherbone
#include <etherbone.h>

// common stuff
//#include <b2b-test.h>
#include <common-defs.h>
#include <common-lib.h>

// public variables
eb_address_t common_statusLo;     // common status, read (low word)
eb_address_t common_statusHi;     // common status, read (high word)
eb_address_t common_state;        // state, read
eb_address_t common_transfers;    // # of transfers from UNILAC to SIS, read
eb_address_t common_injections;   // # of injections in ongoing transfer
eb_address_t common_statTrans;    // status of ongoing or last transfer, read
eb_address_t common_cmd;          // command, write
eb_address_t common_version;      // version, read
eb_address_t common_nBadStatus;   // # of bad status ("ERROR") incidents
eb_address_t common_nBadState;    // # of bad state ("not in operation") incidents
eb_address_t common_macHi;        // MAC address, high word
eb_address_t common_macLo;        // MAC address, low word
eb_address_t common_Ip;           // IP
eb_address_t common_tDiagHi;      // time when diagnostics was cleared, high bits
eb_address_t common_tDiagLo;      // time when diagnostics was cleared, low bits
eb_address_t common_tS0Hi;        // time when FW was in S0 state (start of FW), high bits
eb_address_t common_tS0Lo;        // time when FW was in S0 state (start of FW), low bits
eb_address_t common_usedSize;     // used size of DP RAM

/*
static void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s failed: %s\n", where, eb_status(status));
  exit(1);
} //die
*/

uint64_t comlib_getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // small helper function


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
    default                             : sprintf(message, "error %d, %s",    bit, "undefined error code"); break;
  }

  return message;
} // comlib_statusStext


// init for communicaiton with shared mem
void comlib_initShared(eb_address_t lm32_base, eb_address_t sharedOffset)
{
  common_statusLo     = lm32_base + sharedOffset + COMMON_SHARED_STATUSLO;
  common_statusHi     = lm32_base + sharedOffset + COMMON_SHARED_STATUSHI;
  common_cmd          = lm32_base + sharedOffset + COMMON_SHARED_CMD;
  common_state        = lm32_base + sharedOffset + COMMON_SHARED_STATE;
  common_version      = lm32_base + sharedOffset + COMMON_SHARED_VERSION;
  common_macHi        = lm32_base + sharedOffset + COMMON_SHARED_MACHI;
  common_macLo        = lm32_base + sharedOffset + COMMON_SHARED_MACLO;
  common_Ip           = lm32_base + sharedOffset + COMMON_SHARED_IP;
  common_nBadStatus   = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATUS;
  common_nBadState    = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATE;
  common_tDiagHi      = lm32_base + sharedOffset + COMMON_SHARED_TDIAGHI;
  common_tDiagLo      = lm32_base + sharedOffset + COMMON_SHARED_TDIAGLO;
  common_tS0Hi        = lm32_base + sharedOffset + COMMON_SHARED_TS0HI;
  common_tS0Lo        = lm32_base + sharedOffset + COMMON_SHARED_TS0LO;
  common_transfers    = lm32_base + sharedOffset + COMMON_SHARED_NTRANSFER;
  common_injections   = lm32_base + sharedOffset + COMMON_SHARED_NINJECT;
  common_statTrans    = lm32_base + sharedOffset + COMMON_SHARED_TRANSSTAT;
  common_usedSize     = lm32_base + sharedOffset + COMMON_SHARED_USEDSIZE;
} // comlib_initShared


void comlib_printDiag(uint64_t statusArray, uint32_t state, uint32_t version, uint64_t mac, uint32_t ip, uint32_t nBadStatus, uint32_t nBadState, uint64_t tDiag, uint64_t tS0, uint32_t nTransfer, uint32_t nInjection, uint32_t statTrans, uint32_t usedSize)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;
  int              i;

  printf("common: diags ...\n");

  secs     = (unsigned long)((double)tS0 / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("firmware boot at      : %s\n", timestr);

  secs     = (unsigned long)((double)tDiag / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("diagnostics reset at  : %s\n", timestr);
  printf("version               : %06x\n", version);
  printf("mac                   : 0x%012"PRIx64"\n", mac);
  printf("ip                    : %03d.%03d.%03d.%03d\n", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  printf("used shared mem [byte]: %u\n", usedSize);
  printf("state (# of changes)  : %s (%u)\n", comlib_stateText(state), nBadState);
  printf("# of transfers        : %012u\n", nTransfer);
  printf("# of injections       : %012u\n", nInjection);  
  printf("status of act transfer: 0x%x\n", statTrans);
  printf("sum status (# changes): 0x%"PRIx64" (%u)\n", statusArray, nBadStatus);
  if ((statusArray >> COMMON_STATUS_OK) & 0x1)
    printf("overall status        : OK\n");
  else
    printf("overall status        : NOT OK\n");  
  for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
    if ((statusArray >> i) & 0x1)
      printf("    status bit is set : %s\n", comlib_statusText(i));
  } // for i
} // comlib_printDiag;


int comlib_readDiag(eb_device_t device, uint64_t  *statusArray, uint32_t  *state, uint32_t  *version, uint64_t  *mac, uint32_t  *ip, uint32_t  *nBadStatus, uint32_t  *nBadState, uint64_t  *tDiag, uint64_t  *tS0, uint32_t  *nTransfer, uint32_t  *nInjection, uint32_t  *statTrans, uint32_t *usedSize, uint32_t  printFlag)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return eb_status;
  eb_cycle_read(cycle, common_statusHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, common_statusLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, common_state,       EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, common_version,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, common_macHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, common_macLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, common_Ip,          EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, common_nBadStatus,  EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, common_nBadState,   EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, common_tDiagHi,     EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, common_tDiagLo,     EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, common_tS0Hi,       EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, common_tS0Lo,       EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, common_transfers,   EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, common_injections,  EB_BIG_ENDIAN|EB_DATA32, &(data[14]));  
  eb_cycle_read(cycle, common_statTrans,   EB_BIG_ENDIAN|EB_DATA32, &(data[15]));
  eb_cycle_read(cycle, common_usedSize,    EB_BIG_ENDIAN|EB_DATA32, &(data[16]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) return eb_status;

  *statusArray   = ((uint64_t)(data[0]) << 32) | (uint64_t)(data[1]);
  *state         = data[2];
  *version       = data[3];
  *mac           = ((uint64_t)(data[4]) << 32) | (uint64_t)(data[5]);
  *ip            = data[6];
  *nBadStatus    = data[7];
  *nBadState     = data[8];
  *tDiag         = ((uint64_t)(data[9])  << 32) | (uint64_t)(data[10]);
  *tS0           = ((uint64_t)(data[11]) << 32) | (uint64_t)(data[12]);
  *nTransfer     = data[13];
  *nInjection    = data[14];
  *statTrans     = data[15];
  *usedSize      = data[16];

  if (printFlag) comlib_printDiag(*statusArray, *state, *version, *mac, *ip, *nBadStatus, *nBadState, *tDiag, *tS0, *nTransfer, *nInjection, *statTrans, *usedSize);

  return eb_status;
} // 

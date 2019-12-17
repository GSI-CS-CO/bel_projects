/********************************************************************************************
 *  b2btest-api.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 05-November-2019
 *
 *  implementation for b2btest
 * 
 *  see b2btest-api.h for version, license and documentation 
 *
 ********************************************************************************************/
// standard includes
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

// etherbone
#include <etherbone.h>

// API (x86) b2b-test
#include <b2b-test.h>
#include <b2b-common.h>
#include <b2btest-api.h>

// public variables
eb_address_t b2btest_statusLo;     // status of b2btest, read (low word)
eb_address_t b2btest_statusHi;     // status of b2btest, read (high word)
eb_address_t b2btest_state;        // state, read
eb_address_t b2btest_transfers;    // # of transfers from UNILAC to SIS, read
eb_address_t b2btest_injections;   // # of injections in ongoing transfer
eb_address_t b2btest_statTrans;    // status of ongoing or last transfer, read
eb_address_t b2btest_cmd;          // command, write
eb_address_t b2btest_version;      // version, read
eb_address_t b2btest_nBadStatus;   // # of bad status ("ERROR") incidents
eb_address_t b2btest_nBadState;    // # of bad state ("not in operation") incidents
eb_address_t b2btest_macHi;        // MAC address, high word
eb_address_t b2btest_macLo;        // MAC address, low word
eb_address_t b2btest_Ip;           // IP
eb_address_t b2btest_tDiagHi;      // time when diagnostics was cleared, high bits
eb_address_t b2btest_tDiagLo;      // time when diagnostics was cleared, low bits
eb_address_t b2btest_tS0Hi;        // time when FW was in S0 state (start of FW), high bits
eb_address_t b2btest_tS0Lo;        // time when FW was in S0 state (start of FW), low bits


static void die(const char* where, eb_status_t status)
{
  fprintf(stderr, "%s failed: %s\n", where, eb_status(status));
  exit(1);
} //die


uint64_t getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // small helper function


const char* api_stateText(uint32_t code)
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
} // api_stateText


const char* api_statusText(uint32_t bit)
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
} // api_statusStext


// init for communicaiton with shared mem
void api_initShared(eb_address_t lm32_base, eb_address_t sharedOffset)
{
  b2btest_statusLo     = lm32_base + sharedOffset + COMMON_SHARED_STATUSLO;
  b2btest_statusHi     = lm32_base + sharedOffset + COMMON_SHARED_STATUSHI;
  b2btest_cmd          = lm32_base + sharedOffset + COMMON_SHARED_CMD;
  b2btest_state        = lm32_base + sharedOffset + COMMON_SHARED_STATE;
  b2btest_version      = lm32_base + sharedOffset + COMMON_SHARED_VERSION;
  b2btest_macHi        = lm32_base + sharedOffset + COMMON_SHARED_MACHI;
  b2btest_macLo        = lm32_base + sharedOffset + COMMON_SHARED_MACLO;
  b2btest_Ip           = lm32_base + sharedOffset + COMMON_SHARED_IP;
  b2btest_nBadStatus   = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATUS;
  b2btest_nBadState    = lm32_base + sharedOffset + COMMON_SHARED_NBADSTATE;
  b2btest_tDiagHi      = lm32_base + sharedOffset + COMMON_SHARED_TDIAGHI;
  b2btest_tDiagLo      = lm32_base + sharedOffset + COMMON_SHARED_TDIAGLO;
  b2btest_tS0Hi        = lm32_base + sharedOffset + COMMON_SHARED_TS0HI;
  b2btest_tS0Lo        = lm32_base + sharedOffset + COMMON_SHARED_TS0LO;
  b2btest_transfers    = lm32_base + sharedOffset + COMMON_SHARED_NTRANSFER;
  b2btest_injections   = lm32_base + sharedOffset + COMMON_SHARED_NINJECT;
  b2btest_statTrans    = lm32_base + sharedOffset + COMMON_SHARED_TRANSSTAT;
} // api_initShared


double api_flsa2fdds(double flsa)
{
  double twoep32;
  double twoem32;
  double fclk;
  double fdds;

  twoep32 = pow(2,  32);
  twoem32 = pow(2, -32);
  fclk    = (double)B2BTEST_F_CLK;

  fdds   = twoem32 * floor(twoep32 * flsa / fclk) * fclk;

  return fdds;
} // api_flsa2fdds


void api_printDiag(uint64_t  statusArray, uint32_t  state, uint32_t  version, uint64_t  mac, uint32_t  ip, uint32_t  nBadStatus, uint32_t  nBadState, uint64_t  tDiag, uint64_t  tS0, uint32_t  nTransfer, uint32_t  nInjection, uint32_t  statTrans)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;
  int              i;

  printf("b2b-api: diags ...\n\n");

  secs     = (unsigned long)((double)tS0 / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("firmware boot at      : %s\n", timestr);

  secs     = (unsigned long)((double)tDiag / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  printf("diagnostics reset at  : %s\n", timestr);
  
  printf("state (# of changes)  : %s (%u)\n", api_stateText(state), nBadState);
  printf("# of transfers        : %012u\n", nTransfer);
  printf("# of injections       : %012u\n", nInjection);  
  printf("status of act transfer: 0x%x\n", statTrans);
  printf("version               : %x\n", version);
  printf("mac                   : 0x%012"PRIx64"\n", mac);
  printf("ip                    : %03d.%03d.%03d.%03d\n", (ip & 0xff000000) >> 24, (ip & 0x00ff0000) >> 16, (ip & 0x0000ff00) >> 8, (ip & 0x000000ff));
  printf("sum status (# changes): 0x%"PRIx64" (%u)\n", statusArray, nBadStatus);
  if ((statusArray >> COMMON_STATUS_OK) & 0x1)
    printf("overall status        : OK\n");
  else
    printf("overall status        : NOT OK\n");  
  for (i= COMMON_STATUS_OK + 1; i<(int)(sizeof(statusArray)*8); i++) {
    if ((statusArray >> i) & 0x1)
      printf("    status bit is set : %s\n", api_statusText(i));
  } // for i
} // api_printDiag;


int api_readDiag(eb_device_t device, uint64_t  *statusArray, uint32_t  *state, uint32_t  *version, uint64_t  *mac, uint32_t  *ip, uint32_t  *nBadStatus, uint32_t  *nBadState, uint64_t  *tDiag, uint64_t  *tS0, uint32_t  *nTransfer, uint32_t  *nInjection, uint32_t  *statTrans, uint32_t  printFlag)
{
  eb_cycle_t  cycle;
  eb_status_t eb_status;
  eb_data_t   data[30];
  
  if ((eb_status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die("b2btest-api: eb_cycle_open", eb_status);
  eb_cycle_read(cycle, b2btest_statusHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(cycle, b2btest_statusLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(cycle, b2btest_state,       EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(cycle, b2btest_version,     EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(cycle, b2btest_macHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(cycle, b2btest_macLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(cycle, b2btest_Ip,          EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(cycle, b2btest_nBadStatus,  EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(cycle, b2btest_nBadState,   EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(cycle, b2btest_tDiagHi,     EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(cycle, b2btest_tDiagLo,     EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(cycle, b2btest_tS0Hi,       EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(cycle, b2btest_tS0Lo,       EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(cycle, b2btest_transfers,   EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(cycle, b2btest_injections,  EB_BIG_ENDIAN|EB_DATA32, &(data[14]));  
  eb_cycle_read(cycle, b2btest_statTrans,   EB_BIG_ENDIAN|EB_DATA32, &(data[15]));
  if ((eb_status = eb_cycle_close(cycle)) != EB_OK) die("dm-unipz: eb_cycle_close", eb_status);

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

  if (printFlag) api_printDiag(*statusArray, *state, *version, *mac, *ip, *nBadStatus, *nBadState, *tDiag, *tS0, *nTransfer, *nInjection, *statTrans);

  return eb_status;
} // 

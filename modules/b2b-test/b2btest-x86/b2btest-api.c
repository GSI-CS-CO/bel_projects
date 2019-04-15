/********************************************************************************************
 *  b2btest-api.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
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
#include <sys/time.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <b2b-test.h>
#include <b2btest-api.h>

uint64_t getSysTime() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // small helper function


const char* b2btest_state_text(uint32_t code) {
  switch (code) {
  case B2BTEST_STATE_UNKNOWN      : return "UNKNOWN   ";
  case B2BTEST_STATE_S0           : return "S0        ";
  case B2BTEST_STATE_IDLE         : return "IDLE      ";                                       
  case B2BTEST_STATE_CONFIGURED   : return "CONFIGURED";
  case B2BTEST_STATE_OPREADY      : return "OpReady   ";
  case B2BTEST_STATE_STOPPING     : return "STOPPING  ";
  case B2BTEST_STATE_ERROR        : return "ERROR     ";
  case B2BTEST_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined ";
  }
} // b2btest_state_text

const char* b2btest_status_text(uint32_t bit) {  
  static char message[256];

  switch (bit) {
  case B2BTEST_STATUS_OK               : sprintf(message, "OK"); break;
  case B2BTEST_STATUS_ERROR            : sprintf(message, "error %d, %s",    bit, "an error occured"); break;
  case B2BTEST_STATUS_TIMEDOUT         : sprintf(message, "error %d, %s",    bit, "a timeout occured"); break;
  case B2BTEST_STATUS_OUTOFRANGE       : sprintf(message, "error %d, %s",    bit, "some value is out of range"); break;
  case B2BTEST_STATUS_LATE             : sprintf(message, "error %d, %s",    bit, "a timing messages is not dispatched in time"); break;
  case B2BTEST_STATUS_EARLY            : sprintf(message, "error %d, %s",    bit, "a timing messages is dispatched unreasonably early (dt > UNILAC period)"); break;
  case B2BTEST_STATUS_TRANSACTION      : sprintf(message, "error %d, %s",    bit, "transaction failed"); break;
  case B2BTEST_STATUS_EB               : sprintf(message, "error %d, %s",    bit, "an Etherbone error occured"); break;
  case B2BTEST_STATUS_NOIP             : sprintf(message, "error %d, %s",    bit, "DHCP request via WR network failed"); break;
  case B2BTEST_STATUS_EBREADTIMEDOUT   : sprintf(message, "error %d, %s",    bit, "EB read via WR network timed out"); break;
  case B2BTEST_STATUS_WRONGVIRTACC     : sprintf(message, "error %d, %s",    bit, "mismatching virtual accelerator for EVT_READY_TO_SIS from UNIPZ"); break;
  case B2BTEST_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
  case B2BTEST_STATUS_NOTIMESTAMP      : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS in MIL FIFO but no TS via TLU -> ECA"); break;
  case B2BTEST_STATUS_BADTIMESTAMP     : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break;
  case B2BTEST_STATUS_WAIT4UNIEVENT    : sprintf(message, "error %d, %s",    bit, "timeout while waiting for EVT_READY_TO_SIS"); break;
  case B2BTEST_STATUS_WRBADSYNC        : sprintf(message, "error %d, %s",    bit, "White Rabbit: not in 'TRACK_PHASE'"); break;
  case B2BTEST_STATUS_AUTORECOVERY     : sprintf(message, "errorFix %d, %s", bit, "attempting auto-recovery from state ERROR"); break;
  default                              : sprintf(message, "error %d, %s",    bit, "wr-unipz: undefined error code"); break;
  }

  return message;
} // b2btest_status_text

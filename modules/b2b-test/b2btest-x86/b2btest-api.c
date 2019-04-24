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
#include <b2b-common.h>
#include <b2btest-api.h>

uint64_t getSysTime() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // small helper function


const char* common_state_text(uint32_t code) {
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
} // common_state_text


const char* common_status_text(uint32_t bit) {  
  static char message[256];

  switch (bit) {
  case COMMON_STATUS_OK               : sprintf(message, "OK"); break;
  case COMMON_STATUS_ERROR            : sprintf(message, "error %d, %s",    bit, "an error occured"); break;
  case COMMON_STATUS_TIMEDOUT         : sprintf(message, "error %d, %s",    bit, "a timeout occured"); break;
  case COMMON_STATUS_OUTOFRANGE       : sprintf(message, "error %d, %s",    bit, "some value is out of range"); break;
  case COMMON_STATUS_EB               : sprintf(message, "error %d, %s",    bit, "an Etherbone error occured"); break;
  case COMMON_STATUS_NOIP             : sprintf(message, "error %d, %s",    bit, "DHCP request via WR network failed"); break;
  case COMMON_STATUS_EBREADTIMEDOUT   : sprintf(message, "error %d, %s",    bit, "EB read via WR network timed out"); break;
  case COMMON_STATUS_WRBADSYNC        : sprintf(message, "error %d, %s",    bit, "White Rabbit: not in 'TRACK_PHASE'"); break;
  case COMMON_STATUS_AUTORECOVERY     : sprintf(message, "errorFix %d, %s", bit, "attempting auto-recovery from state ERROR"); break;
  default                             : sprintf(message, "error %d, %s",    bit, "undefined error code"); break;
  }

  return message;
} // common_status_text


const char* b2btest_status_text(uint32_t bit) {  
  static char message[256];

  switch (bit) {
  case B2BTEST_STATUS_PHASEFAILED      : sprintf(message, "error %d, %s",    bit, "phase measurement failed"); break;
  case B2BTEST_STATUS_TRANSACTION      : sprintf(message, "error %d, %s",    bit, "transaction failed"); break;
  case B2BTEST_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
  default                              : common_status_text(bit) ; break;
  }

  return message;
} // b2btest_status_text

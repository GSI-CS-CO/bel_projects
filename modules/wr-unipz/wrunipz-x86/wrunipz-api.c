/********************************************************************************************
 *  wrunipz-api.c
 *
 *  created : 2018
 *  author  : Dietrich Beck, GSI-Darmstadt
 *
 *  implementation for wrunipz
 * 
 *  see wrunipz-api.h for version, license and documentation 
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
#include <wr-unipz.h>
#include <wrunipz-api.h>

uint64_t getSysTime() {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // small helper function


const char* wrunipz_state_text(uint32_t code) {
  switch (code) {
  case WRUNIPZ_STATE_UNKNOWN      : return "UNKNOWN   ";
  case WRUNIPZ_STATE_S0           : return "S0        ";
  case WRUNIPZ_STATE_IDLE         : return "IDLE      ";                                       
  case WRUNIPZ_STATE_CONFIGURED   : return "CONFIGURED";
  case WRUNIPZ_STATE_OPREADY      : return "OpReady   ";
  case WRUNIPZ_STATE_STOPPING     : return "STOPPING  ";
  case WRUNIPZ_STATE_ERROR        : return "ERROR     ";
  case WRUNIPZ_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined ";
  }
} // wrunipz_state_text

const char* wrunipz_status_text(uint32_t bit) {  
  static char message[256];

  switch (bit) {
  case WRUNIPZ_STATUS_OK               : sprintf(message, "OK"); break;
  case WRUNIPZ_STATUS_ERROR            : sprintf(message, "error %d, %s",    bit, "an error occured"); break;
  case WRUNIPZ_STATUS_TIMEDOUT         : sprintf(message, "error %d, %s",    bit, "a timeout occured"); break;
  case WRUNIPZ_STATUS_OUTOFRANGE       : sprintf(message, "error %d, %s",    bit, "some value is out of range"); break;
  case WRUNIPZ_STATUS_LATE             : sprintf(message, "error %d, %s",    bit, "a timing messages is not dispatched in time"); break;
  case WRUNIPZ_STATUS_EARLY            : sprintf(message, "error %d, %s",    bit, "a timing messages is dispatched unreasonably early (dt > UNILAC period)"); break;
  case WRUNIPZ_STATUS_TRANSACTION      : sprintf(message, "error %d, %s",    bit, "transaction failed"); break;
  case WRUNIPZ_STATUS_EB               : sprintf(message, "error %d, %s",    bit, "an Etherbone error occured"); break;
  case WRUNIPZ_STATUS_NOIP             : sprintf(message, "error %d, %s",    bit, "DHCP request via WR network failed"); break;
  case WRUNIPZ_STATUS_EBREADTIMEDOUT   : sprintf(message, "error %d, %s",    bit, "EB read via WR network timed out"); break;
  case WRUNIPZ_STATUS_WRONGVIRTACC     : sprintf(message, "error %d, %s",    bit, "mismatching virtual accelerator for EVT_READY_TO_SIS from UNIPZ"); break;
  case WRUNIPZ_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
  case WRUNIPZ_STATUS_NOTIMESTAMP      : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS in MIL FIFO but no TS via TLU -> ECA"); break;
  case WRUNIPZ_STATUS_BADTIMESTAMP     : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break;
  case WRUNIPZ_STATUS_WAIT4UNIEVENT    : sprintf(message, "error %d, %s",    bit, "timeout while waiting for EVT_READY_TO_SIS"); break;
  case WRUNIPZ_STATUS_WRBADSYNC        : sprintf(message, "error %d, %s",    bit, "White Rabbit: not in 'TRACK_PHASE'"); break;
  case WRUNIPZ_STATUS_AUTORECOVERY     : sprintf(message, "errorFix %d, %s", bit, "attempting auto-recovery from state ERROR"); break;
  default                              : sprintf(message, "error %d, %s",    bit, "wr-unipz: undefined error code"); break;
  }

  return message;
} // wrunipz_status_text


uint32_t wrunipz_transaction_kill(eb_device_t device, eb_address_t DPcmd, eb_address_t DPstat) {

  eb_data_t data;
  int i;

  // clear any pending or ongoing transaction
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFKILL, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
  
  // wait until FW confirms idle mode
  /* chk: do this with proper timeout handling */
  i = 0;
  while (i < 100) {   
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_IDLE) continue;
    i++;
  } // while
  
  // FW is not in init mode: give up
  if (data != WRUNIPZ_CONFSTAT_IDLE) return WRUNIPZ_STATUS_TRANSACTION;
  
  return WRUNIPZ_STATUS_OK;
} // wrunipz_transaction_init


uint32_t wrunipz_transaction_init(eb_device_t device, eb_address_t DPcmd, eb_address_t DPvacc, eb_address_t DPstat, uint32_t vAcc) {

  eb_data_t     data;
  uint64_t      tTimeout;

  // check if _no_ transaction in progress
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
  if (data != WRUNIPZ_CONFSTAT_IDLE) return WRUNIPZ_STATUS_TRANSACTION;

  // init transaction
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFINIT, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;

  // wait until FW confirms init mode
  tTimeout = getSysTime() + (uint64_t)1000000000;
  while (getSysTime() < tTimeout) {   
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_INIT) break;
  } // while getSysTime

  // FW is not in init mode: give up
  if (data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;
  
  // set virt acc
  if (eb_device_write(device, DPvacc, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)vAcc, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;

  return WRUNIPZ_STATUS_OK;
} // wrunipz_transaction_init


uint32_t wrunipz_transaction_submit(eb_device_t device, eb_address_t DPcmd, eb_address_t DPstat) {

  eb_data_t     data;
  uint64_t      tTimeout;

  // check if transaction has been initialized
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
  if (data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // submit data already uploaded to DP RAM
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFSUBMIT, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;

  // wait until transaction has been completed
  tTimeout = getSysTime() + (uint64_t)1000000000;
  while(getSysTime() < tTimeout) {
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_IDLE) return WRUNIPZ_STATUS_OK;
  } // while getSysTime
  
  return WRUNIPZ_STATUS_TIMEDOUT;
} // wrunipz_transaction_submit


uint32_t wrunipz_transaction_upload(eb_device_t device, eb_address_t DPstat, eb_address_t DPpz, eb_address_t DPdata, eb_address_t DPflag, uint32_t pz, uint32_t *dataChn0, uint32_t nDataChn0, uint32_t *dataChn1, uint32_t nDataChn1)
{
  int          i;
  uint32_t     pzFlag;         // flag: PZ has new data
  uint32_t     validFlag;      // flag: data[n] is valid
  uint32_t     prepFlag;       // flag: data[n] is prep datum
  uint32_t     evtFlag;        // flag: data[n] is evt
  uint32_t     offset;         // helper variable
  eb_data_t    data;
  eb_cycle_t   cycle;

  /* check: its a mess to have practically the same code for Kanal0 and Kanal1 */
  
  // check if transaction has been initialized
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
  if (data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // pz flag 
  if (eb_device_read(device, DPpz, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return WRUNIPZ_STATUS_EB;
  pzFlag = (uint32_t)data | (1 << pz);

  // EB cycle
  if (eb_cycle_open(device, 0, eb_block, &cycle) != EB_OK) return WRUNIPZ_STATUS_EB;

  eb_cycle_write(cycle, DPpz, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)pzFlag);

  // UNILAC Kanal0
  validFlag = 0;
  prepFlag  = 0;
  evtFlag   = 0;

  // write data
  for (i=0; i < nDataChn0; i++) {
    offset    = (uint32_t)(dataChn0[i] >> 16); // get offset of event within UNILAC cycle
    
    validFlag = validFlag | (1 << i);
    evtFlag   = evtFlag   | (1 << i); /* chk, for now assume, that all data are 'event data'  */
    if (offset < WRUNIPZ_MAXPREPOFFSET)  prepFlag  = prepFlag | (1 << i);

    eb_cycle_write(cycle, DPdata + (eb_address_t)((pz * WRUNIPZ_NEVT * WRUNIPZ_NCHN + i) << 2), EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(dataChn0[i]));
  } // for i

  /* printf("pz %d, vF %x, eF %x, nData %d dN[10] %x \n", pz, validFlag, evtFlag, nDataChn0, dataChn0[10]); */
  // write flags
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + 0) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)validFlag);
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + 1) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)prepFlag);
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + 2) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)evtFlag);

  /* printf("valid addr %x flag %x\n", (uint32_t)DPflag + ((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + 0) << 2), validFlag); */

  // UNILAC Kanal 1
  validFlag = 0;
  prepFlag  = 0;
  evtFlag   = 0;

  // write data
  for (i=0; i < nDataChn1; i++) {
    validFlag = validFlag | (1 << i);
    evtFlag   = evtFlag   | (1 << i); /* chk, for now assume, that all data are 'event data'  */
    prepFlag  = prepFlag;             /* chk, treatment of prep events is still on open issue */
    
    eb_cycle_write(cycle, DPdata + (eb_address_t)((pz * WRUNIPZ_NEVT * WRUNIPZ_NCHN + WRUNIPZ_NEVT + i) << 2), EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(dataChn1[i]));
  } // for i
  
  // write flags
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG + 0) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)validFlag);
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG + 1) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)prepFlag);
  eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG + 2) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)evtFlag);

  if (eb_cycle_close(cycle) != EB_OK) return WRUNIPZ_STATUS_EB;

  return WRUNIPZ_STATUS_OK;
} // wrunipz_transaction_upload


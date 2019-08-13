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
#include <unistd.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <b2b-common.h>
#include <b2btest-api.h>
#include <wr-unipz.h>
#include <wrunipz-api.h>

const char* wrunipz_status_text(uint32_t bit) {  
  static char message[256];
  
  switch (bit) {
    case WRUNIPZ_STATUS_LATE             : sprintf(message, "error %d, %s",    bit, "a timing messages is not dispatched in time"); break;                            
    case WRUNIPZ_STATUS_EARLY            : sprintf(message, "error %d, %s",    bit, "a timing messages is dispatched unreasonably early (dt > UNILACPERIOD)"); break;
    case WRUNIPZ_STATUS_TRANSACTION      : sprintf(message, "error %d, %s",    bit, "transaction failed"); break;
    case WRUNIPZ_STATUS_MIL              : sprintf(message, "error %d, %s",    bit, "an error on MIL hardware occured (MIL piggy etc...)"); break;
    case WRUNIPZ_STATUS_NOMILEVENTS      : sprintf(message, "error %d, %s",    bit, "no MIL events from UNIPZ"); break;          
    case WRUNIPZ_STATUS_WRONGVIRTACC     : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS with wrong virt acc number"); break;
    case WRUNIPZ_STATUS_SAFETYMARGIN     : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;         
    case WRUNIPZ_STATUS_NOTIMESTAMP      : sprintf(message, "error %d, %s",    bit, "received EVT_READY_TO_SIS in MIL FIFO but not via TLU -> ECA"); break;
    case WRUNIPZ_STATUS_BADTIMESTAMP     : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA does not coincide with MIL Event from FIFO"); break; 
    case WRUNIPZ_STATUS_ORDERTIMESTAMP   : sprintf(message, "error %d, %s",    bit, "TS from TLU->ECA and MIL Events are out of order"); break;
    default                              : sprintf(message, "%s", api_statusText(bit)); break;
  } // switch bit
  
  return message;
} // wrunipz_status_text


uint32_t wrunipz_transaction_kill(eb_device_t device, eb_address_t DPcmd, eb_address_t DPstat) {

  eb_data_t data;
  int i;

  // clear any pending or ongoing transaction
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFKILL, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  
  // wait until FW confirms idle mode
  /* chk: do this with proper timeout handling */
  i = 0;
  while (i < 100) {   
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_IDLE) continue;
    i++;
  } // while
  
  // FW is not in init mode: give up
  if (data != WRUNIPZ_CONFSTAT_IDLE) return WRUNIPZ_STATUS_TRANSACTION;
  
  return COMMON_STATUS_OK;
} // wrunipz_transaction_init


uint32_t wrunipz_transaction_init(eb_device_t device, eb_address_t DPcmd, eb_address_t DPvacc, eb_address_t DPstat, uint32_t vAcc) {

  eb_data_t     data;
  uint64_t      tTimeout;

  // check if _no_ transaction in progress
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  if (data != WRUNIPZ_CONFSTAT_IDLE) return WRUNIPZ_STATUS_TRANSACTION;
  
  // init transaction
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFINIT, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;

  // wait until FW confirms init mode
  tTimeout = getSysTime() + (uint64_t)1000000;
  while (getSysTime() < tTimeout) {
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_INIT) break;
  } // while getSysTime

  // FW is not in init mode: give up
  if (data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // set virt acc
  if (eb_device_write(device, DPvacc, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)vAcc, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  
  return COMMON_STATUS_OK;
} // wrunipz_transaction_init


uint32_t wrunipz_transaction_submit(eb_device_t device, eb_address_t DPcmd, eb_address_t DPstat) {

  eb_data_t     data;
  uint64_t      tTimeout;

  // check if transaction has been initialized
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  if (data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // submit data already uploaded to DP RAM
  if (eb_device_write(device, DPcmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFSUBMIT, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;

  // wait until transaction has been completed
  tTimeout = getSysTime() + (uint64_t)1000000;
  while(getSysTime() < tTimeout) {
    if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
    if (data == WRUNIPZ_CONFSTAT_IDLE) return COMMON_STATUS_OK;
  } // while getSysTime
  
  return COMMON_STATUS_TIMEDOUT;
} // wrunipz_transaction_submit


uint32_t wrunipz_transaction_upload(eb_device_t device, eb_address_t DPstat, eb_address_t DPpz, eb_address_t DPdata, eb_address_t DPflag, uint32_t pz, uint32_t *dataChn0, uint32_t nDataChn0, uint32_t *dataChn1, uint32_t nDataChn1)
{
#define      NKANAL 2          // number of 'Kanal' in this routine
  int          i,k;
  uint32_t     pzFlag;         // flag: PZ has new data
  uint32_t     validFlag;      // flag: data[n] is valid
  uint32_t     prepFlag;       // flag: data[n] is prep datum
  uint32_t     evtFlag;        // flag: data[n] is evt
  uint32_t     *data[NKANAL];  // helper variable: pointer to arrays, each array represents one channel
  uint32_t     nData[NKANAL];  // helper variable: number of data for each channel
  uint32_t     offset;         // helper variable: offset of duetime of an event within an UNILAC cycle
  eb_data_t    eb_data;
  eb_cycle_t   cycle;

  // required for looping over the two channels
  data[0]  = dataChn0;
  data[1]  = dataChn1;
  nData[0] = nDataChn0;
  nData[1] = nDataChn1;

  // check if transaction has been initialized
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &eb_data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  if (eb_data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // pz flag 
  if (eb_device_read(device, DPpz, EB_BIG_ENDIAN|EB_DATA32, &eb_data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  pzFlag = (uint32_t)eb_data | (1 << pz);

  // EB cycle
  if (eb_cycle_open(device, 0, eb_block, &cycle) != EB_OK) return COMMON_STATUS_EB;

  eb_cycle_write(cycle, DPpz, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)pzFlag);

  for (k=0; k<NKANAL; k++) {
    validFlag = 0;
    prepFlag  = 0;
    evtFlag   = 0;
    
    // write data
    for (i=0; i < nData[k]; i++) {
      offset    = (uint32_t)((data[k])[i] >> 16); // get offset of event within UNILAC cycle

      validFlag = validFlag | (1 << i);
      evtFlag   = evtFlag   | (1 << i); /* chk: for now assume, that all data are 'event data'  */
      if (offset < WRUNIPZ_MAXPREPOFFSET)  prepFlag  = prepFlag | (1 << i);
      
      eb_cycle_write(cycle, DPdata + (eb_address_t)((pz * WRUNIPZ_NEVT * WRUNIPZ_NCHN + WRUNIPZ_NEVT*k + i) << 2), EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((data[k])[i]));
    } // for i
    
    // write flags
    eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG*k + 0) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)validFlag);
    eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG*k + 1) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)prepFlag);
    eb_cycle_write(cycle, DPflag + (eb_address_t)((pz * WRUNIPZ_NPZ * WRUNIPZ_NCHN + WRUNIPZ_NFLAG*k + 2) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)evtFlag);
  } // for k

  if (eb_cycle_close(cycle) != EB_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // wrunipz_transaction_upload


void wrunipz_fill_channel_file(char *filename, uint32_t pz, uint32_t vAcc, uint32_t *dataChn0, uint32_t *nDataChn0, uint32_t *dataChn1, uint32_t *nDataChn1)
{
#define  MAXLEN 4096

  int     i,j,k;
  char    charChn0[MAXLEN];
  char    charChn1[MAXLEN];
  FILE    *fp;
  char    *line = NULL;
  size_t  len = 0;
  ssize_t read;

  char     *data;       
  int      dataOffset;
  uint32_t evt, offset, dummy;
  
  // init
  for (i=0; i<WRUNIPZ_NEVT; i++) {
    dataChn0[i] = 0x0;
    dataChn1[i] = 0x0;
  } // for i

  for (i=0; i<MAXLEN; i++) {
    charChn0[i] = '\0';
    charChn1[i] = '\0';
  } // for i
  *nDataChn0 = 0;
  *nDataChn1 = 0;

 
  
  // read data for the two relevant channels from file
  fp = fopen(filename, "r"); 
  if (fp == NULL) {
    printf("wr-unipz: can't open file with event table\n");
    exit(1);
  } // if fp

  for (i=0; i<WRUNIPZ_NPZ; i++) {
    for (j=0; j<WRUNIPZ_NVACC; j++) {
      for (k=0; k<WRUNIPZ_NCHN; k++) {
        if((read = getline(&line, &len, fp)) != -1) {
          // printf("pz %d, vacc %d, ch %d, line %s\n", i, j, k, line);
          if ((i==pz) && (j==vAcc) && (k == 0)) strcpy(charChn0, (line+1)); // ommit leading '['
          if ((i==pz) && (j==vAcc) && (k == 1)) strcpy(charChn1, (line+1)); // ommit leading '['
        } // while
      } // for k
    } // for j
  } // for i
        
  fclose(fp);
  if (line) free (line);

  // printf("c0 %s\n", charChn0);
  // printf("c1 %s\n", charChn1);
  
  
  // extract data for channel0
  data       = charChn0;
  *nDataChn0 = 0;
  while (sscanf(data, "%u%n", &evt, &dataOffset) == 1) {
    data += dataOffset;
    sscanf(data, ", %uL%n", &offset, &dataOffset);
    data += dataOffset;
    sscanf(data, ", %uL, %n", &dummy, &dataOffset);
    data += dataOffset;

    dataChn0[*nDataChn0] = (offset << 16) | evt;
    // printf("c0: offset %d, data 0d%d 0x%x\n", offset, evt, evt);
    (*nDataChn0)++;
  } // while
  // extract data for channel1
  data       = charChn1;
  *nDataChn1 = 0;
  while (sscanf(data, "%u%n", &evt, &dataOffset) == 1) {
    data += dataOffset;
    sscanf(data, ", %uL%n", &offset, &dataOffset);
    data += dataOffset;
    sscanf(data, ", %uL, %n", &dummy, &dataOffset);
    data += dataOffset;

    dataChn1[*nDataChn1] = (offset << 16) | evt;
    // printf("c1: offset %d, data 0d%d 0x%x\n", offset, evt, evt);
    (*nDataChn1)++;
  } // while
  
} // wrunipz_fill_channel_file


void wrunipz_fill_channel_dummy(uint32_t offset, uint32_t pz, uint32_t vAcc, uint32_t *dataChn0, uint32_t *nDataChn0, uint32_t *dataChn1, uint32_t *nDataChn1)
{
  int i;
  
  *nDataChn0 = WRUNIPZ_NEVT;
  *nDataChn1 = WRUNIPZ_NEVT;

  for (i=0; i < (*nDataChn0 -1); i++) dataChn0[i] = ((uint16_t)(i + 100 * pz + offset) << 16) + i;  // time and evtno from formula
  dataChn0[*nDataChn0 -1] = ((uint16_t)offset << 16) + 64;                                          // time is offset, evtno 64 (diagnosis)

  for (i=0; i < (*nDataChn1 -1); i++) dataChn1[i] = ((uint16_t)(i + 100 * pz + offset) << 16) + i;
  dataChn1[*nDataChn1 -1] = ((uint16_t)offset << 16) + 64;
} // wrunipz_fill_channel_dummy


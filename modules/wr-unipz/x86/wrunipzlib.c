/******************************************************************************
 *  wrunipzlib.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version :02-April-2020
 *
 * library for wrunipz
 *
 * -------------------------------------------------------------------------------------------
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
 * Last update: 17-May-2017
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
#include <common-defs.h>                 // common definitions
#include <common-lib.h>                  // common routines
#include <wrunipz_shared_mmap.h>         // FW shared def
#include <wr-unipz.h>                    // FW defs
#include <wrunipzlib.h>                  // x86 library

// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// public variables
eb_socket_t  eb_socket;                 // EB socket

eb_address_t lm32_base;                 // lm32
eb_address_t wrunipz_cmd;               // command, write
eb_address_t wrunipz_state;             // state of state machine
eb_address_t wrunipz_cycles;            // # of UNILAC cycles
eb_address_t wrunipz_tCycleAvg;         // average cycle time
eb_address_t wrunipz_msgFreqAvg;        // average message rate
eb_address_t wrunipz_nLate;             // # of late messages
eb_address_t wrunipz_vaccAvg;           // virt acc played
eb_address_t wrunipz_pzAvg;             // PZs played
eb_address_t wrunipz_nMessageLo;        // number of messages, read
eb_address_t wrunipz_nMessageHi;        // number of messages, read
eb_address_t wrunipz_dtMax;             // delta T (max) between message time of dispatching and deadline, read
eb_address_t wrunipz_dtMin;             // delta T (min) between message time of dispatching and deadline, read
eb_address_t wrunipz_cycJmpMax;         // delta T (max) between expected and actual start of UNILAC cycle, read
eb_address_t wrunipz_cycJmpMin;         // delta T (min) between expected and actual start of UNILAC cycle, read
eb_address_t wrunipz_nLate;             // # of late messages, read
eb_address_t wrunipz_vaccAvg;           // virtual accelerators played over the past second, read
eb_address_t wrunipz_pzAvg;             // PZs used over the past second, read
eb_address_t wrunipz_evtData;           // event data
eb_address_t wrunipz_evtFlags;          // event flags


const char* wrunipz_status_text(uint32_t bit)
{  
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
    default                              : sprintf(message, "%s", comlib_statusText(bit)); break;
  } // switch bit
  
  return message;
} // wrunipz_status_text


const char* wrunipz_state_text(uint32_t code)
{
  return comlib_stateText(code);
} // wrunipz_state_text


const char* wrunipz_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // wrunipz_version_text


uint32_t wrunipz_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
{
  eb_status_t         status;
  eb_device_t         eb_device;  
  struct sdb_device   sdbDevice;                           // instantiated lm32 core
  int                 nDevices;                            // number of instantiated cores

  *ebDevice = 0x0;
  if (cpu != 0) return COMMON_STATUS_OUTOFRANGE;           // chk, only support 1st core (this is a quick hack)
  nDevices = 1;

  // open Etherbone device and socket 
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &eb_socket)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_device_open(eb_socket, devName, EB_ADDR32|EB_DATA32, 3, &eb_device)) != EB_OK) return COMMON_STATUS_EB;

  //  get Wishbone address of lm32 
  if ((status = eb_sdb_find_by_identity(eb_device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) return COMMON_STATUS_EB;

  lm32_base            = sdbDevice.sdb_component.addr_first;

  comlib_initShared(lm32_base, SHARED_OFFS);
  wrunipz_cmd          = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  wrunipz_tCycleAvg    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_TCYCLEAVG;
  wrunipz_cycles       = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NCYCLE;
  wrunipz_nMessageHi   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGEHI;
  wrunipz_nMessageLo   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NMESSAGELO;
  wrunipz_msgFreqAvg   = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_MSGFREQAVG;
  wrunipz_dtMax        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMAX;
  wrunipz_dtMin        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_DTMIN;
  wrunipz_cycJmpMax    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CYCJMPMAX;
  wrunipz_cycJmpMin    = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_CYCJMPMIN;
  wrunipz_nLate        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_NLATE;
  wrunipz_vaccAvg      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_VACCAVG;
  wrunipz_pzAvg        = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_PZAVG;
  wrunipz_evtData      = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_EVT_DATA;
  wrunipz_evtFlags     = lm32_base + SHARED_OFFS + WRUNIPZ_SHARED_EVT_FLAGS;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;
  
  return COMMON_STATUS_OK;
} // wrunipz_firmware_open


uint32_t wrunipz_firmware_close(uint64_t ebDevice)
{
  eb_status_t status;
  eb_device_t eb_device;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;
  
  // close Etherbone device and socket
  if ((status = eb_device_close(eb_device)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(eb_socket)) != EB_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // wrunipz_firmware_close


uint32_t wrunipz_version_firmware(uint64_t ebDevice, uint32_t *version)
{
  uint64_t       dummy64a;
  uint32_t       dummy32a, dummy32b, dummy32c;
  uint32_t       errorCode;

  if (!ebDevice) return COMMON_STATUS_EB;
  *version = 0xffffffff;
  
  errorCode = wrunipz_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, version, 0);

  return errorCode;
} // wrunipz_version_firmware


uint32_t wrunipz_version_library(uint32_t *version)
{
  *version = (uint32_t)WRUNIPZLIB_VERSION;

  return COMMON_STATUS_OK;
} // wrunipz_version_library


uint32_t wrunipz_info_read(uint64_t ebDevice, uint32_t *ncycles, uint32_t *tCycleAvg, uint32_t *msgFreqAvg, uint32_t *nLate, uint32_t *vaccAvg,
                           uint32_t *pzAvg, uint64_t *nMessages, int32_t  *dtMax, int32_t  *dtMin, int32_t  *cycJmpMax, int32_t  *cycJmpMin)
{
  eb_cycle_t  eb_cycle;
  eb_status_t eb_status;
  eb_device_t eb_device;
  eb_data_t   data[30];

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, wrunipz_cycles,        EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(eb_cycle, wrunipz_tCycleAvg,     EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(eb_cycle, wrunipz_msgFreqAvg,    EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(eb_cycle, wrunipz_nLate,         EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(eb_cycle, wrunipz_vaccAvg,       EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(eb_cycle, wrunipz_pzAvg,         EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(eb_cycle, wrunipz_nMessageHi,    EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(eb_cycle, wrunipz_nMessageLo,    EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(eb_cycle, wrunipz_dtMax,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(eb_cycle, wrunipz_dtMin,         EB_BIG_ENDIAN|EB_DATA32, &(data[9])); 
  eb_cycle_read(eb_cycle, wrunipz_cycJmpMax,     EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, wrunipz_cycJmpMin,     EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  *ncycles       = data[0];
  *tCycleAvg     = data[1];
  *msgFreqAvg    = data[2];
  *nLate         = data[3];
  *vaccAvg       = data[4];
  *pzAvg         = data[5];
  *nMessages     = (uint64_t)(data[6]) << 32;
  *nMessages    += data[7];
  *dtMax         = data[8];
  *dtMin         = data[9];
  *cycJmpMax     = data[10]; 
  *cycJmpMin     = data[11];

  return COMMON_STATUS_OK;
} // wrunipz_info_read


uint32_t wrunipz_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t printDiag)
{
  eb_status_t eb_status;
  eb_device_t eb_device;

  uint64_t    dummy64a, dummy64b, dummy64c;
  uint32_t    dummy32a, dummy32b, dummy32c, dummy32d, dummy32e;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = comlib_readDiag(eb_device, statusArray, state, version, &dummy64a, &dummy32a, nBadStatus, nBadState, &dummy64b, &dummy64c,
                                   &dummy32b, &dummy32c, &dummy32d, &dummy32e, printDiag)) != COMMON_STATUS_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // wrunipz_status_read
  

void wrunipz_cmd_configure(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
} // wrunipz_cmd_configure


void wrunipz_cmd_startop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP, 0, eb_block);
} // wrunipz_cmd_startop


void wrunipz_cmd_stopop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP, 0, eb_block);
} // wrunipz_cmd_stopop


void wrunipz_cmd_recover(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER, 0, eb_block);
} // wrunipz_cmd_recover


void wrunipz_cmd_idle(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE, 0, eb_block);
} // wrunipz_cmd_idle


void wrunipz_cmd_cleardiag(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
} // wrunipz_cmd_cleardiag


void wrunipz_cmd_submit(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFSUBMIT, 0, eb_block);
} // wrunipz_cmd_submit


void wrunipz_cmd_clearTables(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrunipz_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WRUNIPZ_CMD_CONFCLEAR, 0, eb_block);
} // wrunipz_cmd_submit


uint32_t wrunipz_table_upload(uint64_t ebDevice, uint32_t pz, uint32_t vacc, uint32_t chn, uint32_t *data, uint32_t nData)
{
  int          i;
  uint32_t     newFlag;        // flag: signals that new data are available
  uint32_t     validFlag;      // flag: data[n] is valid
  uint32_t     prepFlag;       // flag: data[n] is prep datum
  uint32_t     evtFlag;        // flag: data[n] is evt
  //uint32_t     *data;          // helper variable: pointer to arrays
  //uint32_t     nData[NKANAL];  // helper variable: number of data for each channel
  uint32_t     tOffset;        // helper variable: offset of duetime of an event within an UNILAC cycle [us]
  uint32_t     addrOffset;     // helper variable: address offset for data     
  /*eb_data_t    eb_data;*/
  eb_cycle_t   cycle;
  
  if (!ebDevice) return COMMON_STATUS_EB;

  if (chn   >= WRUNIPZ_NCHN)  return COMMON_STATUS_OUTOFRANGE;
  if (pz    >= WRUNIPZ_NPZ)   return COMMON_STATUS_OUTOFRANGE;
  if (vacc  >= WRUNIPZ_NVACC) return COMMON_STATUS_OUTOFRANGE;
  if (nData >  WRUNIPZ_NEVT)  return COMMON_STATUS_OUTOFRANGE;
  
  // required for looping over the two channels
  /*  data[0]  = dataChn0;
  data[1]  = dataChn1;
  nData[0] = nDataChn0;
  nData[1] = nDataChn1;*/
  /*
  // check if transaction has been initialized
  if (eb_device_read(device, DPstat, EB_BIG_ENDIAN|EB_DATA32, &eb_data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  if (eb_data != WRUNIPZ_CONFSTAT_INIT) return WRUNIPZ_STATUS_TRANSACTION;

  // pz flag 
  if (eb_device_read(device, DPpz, EB_BIG_ENDIAN|EB_DATA32, &eb_data, 0, eb_block) != EB_OK) return COMMON_STATUS_EB;
  pzFlag = (uint32_t)eb_data | (1 << pz);
  */
  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &cycle) != EB_OK) return COMMON_STATUS_EB;

  //eb_cycle_write(cycle, DPpz, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)pzFlag);
  newFlag   = 0x1;
  validFlag = 0;
  prepFlag  = 0;
  evtFlag   = 0;
    
  // write data
  for (i=0; i < nData; i++) {
    validFlag = validFlag | (1 << i);
    evtFlag   = evtFlag   | (1 << i);      // for now we assume all data are timing events
    tOffset   = (uint32_t)(data[i] >> 16); // get offset of event within UNILAC cycle
    if (tOffset < WRUNIPZ_MAXPREPOFFSET)  prepFlag  = prepFlag | (1 << i); // events early in the cycle are marked as 'prep events'

    addrOffset  = vacc * WRUNIPZ_NEVT * WRUNIPZ_NCHN * WRUNIPZ_NPZ;  // offset for vacc
    addrOffset += pz   * WRUNIPZ_NEVT * WRUNIPZ_NCHN;                // offset for pz
    addrOffset += chn  * WRUNIPZ_NEVT;                               // offset for chn

    eb_cycle_write(cycle, wrunipz_evtData + (eb_address_t)((addrOffset + i) << 2), EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(data[i]));
  } // for i
    
  // write flags
  addrOffset  = vacc * WRUNIPZ_NFLAG * WRUNIPZ_NCHN * WRUNIPZ_NPZ;   // offset for vacc
  addrOffset += pz   * WRUNIPZ_NFLAG * WRUNIPZ_NCHN;                 // offset for pz
  addrOffset += chn  * WRUNIPZ_NFLAG;                                // offset of chn
  
  eb_cycle_write(cycle, wrunipz_evtFlags + (eb_address_t)((addrOffset + 1) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)validFlag);
  eb_cycle_write(cycle, wrunipz_evtFlags + (eb_address_t)((addrOffset + 2) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)prepFlag);
  eb_cycle_write(cycle, wrunipz_evtFlags + (eb_address_t)((addrOffset + 3) << 2),  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)evtFlag);

  if (eb_cycle_close(cycle) != EB_OK) return COMMON_STATUS_EB;

  // after all data has been written, mark them as 'new' 
  eb_device_write(ebDevice, wrunipz_evtFlags + (eb_address_t)((addrOffset + 0) << 2), EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)newFlag, 0, eb_block);
  
  return COMMON_STATUS_OK;
} // wrunipz_table_upload


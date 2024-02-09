/******************************************************************************
 *  wrmillib.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 09-Feb-2024
 *
 * library for wr-mil
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
#include <math.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <common-defs.h>                 // common definitions
#include <common-lib.h>                  // common routines
#include <common-core.h>                 // common core
#include <wrmil_shared_mmap.h>           // FW shared def
#include <wr-mil.h>                      // FW defs
#include <wrmillib.h>                    // x86 library

// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// public variables
eb_socket_t  eb_socket;                   // EB socket
eb_address_t lm32_base;                   // lm32
eb_address_t wrmil_cmd;                   // command, write
eb_address_t wrmil_state;                 // state of state machine
uint32_t     wrmil_flagDebug = 0;         // flag debug

// application specific stuff
// set values
eb_address_t wrmil_set_utcTrigger;        // the MIL event that triggers the generation of UTC events
eb_address_t wrmil_set_utcUtcDelay;       // delay [us] between the 5 generated UTC MIL events
eb_address_t wrmil_set_trigUtcDelay;      // delay [us] between the trigger event and the first UTC (and other) generated events
eb_address_t wrmil_set_gid;               // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
eb_address_t wrmil_set_latency;           // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
eb_address_t wrmil_set_utcOffsetHi;       // delay [ms] between the TAI and the MIL-UTC, high word
eb_address_t wrmil_set_utcOffsetLo;       // delay [ms] between the TAI and the MIL-UTC, low word
eb_address_t wrmil_set_requestFill;       // if this is written to 1, the gateway will send a fill event as soon as possible
eb_address_t wrmil_set_milDev;            // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
eb_address_t wrmil_set_milMon;            // 1: monitor MIL events; 0; don't monitor MIL events
eb_address_t wrmil_get_nEvtsHi;           // number of translated events from WR to MIL, high word
eb_address_t wrmil_get_nEvtsLo;           // number of translated events from WR to MIL, low word
eb_address_t wrmil_get_nLate;             // number of translated events that could not be delivered in time
eb_address_t wrmil_get_comLatency;        // latency for messages received from via ECA (tDeadline - tNow)) [ns]                             

// get values
eb_address_t wrmil_get_gid;               // GID for transfer

#define WAITCMDDONE COMMON_DEFAULT_TIMEOUT * 1000 // use default timeout and convert to us to be sure the command is processed

uint64_t wrmil_getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * (uint64_t)1000000000+ tv.tv_usec * (uint64_t)1000;
} // wrmil_getSysTime()


const char* wrmil_status_text(uint32_t code)
{  
  static char message[256];
  
  switch (code) {
    case WRMIL_STATUS_SAFETYMARGIN         : sprintf(message, "error %d, %s", code, "violation of safety margin for data master and timing network"); break;
    case WRMIL_STATUS_LATEMESSAGE          : sprintf(message, "error %d, %s", code, "late timing message received"); break;
    case WRMIL_STATUS_BADSETTING           : sprintf(message, "error %d, %s", code, "bad setting data"); break;
    default                                : sprintf(message, "%s", comlib_statusText(code)); break;
  } // switch code
  
  return message;
} // wrmil_status_text


const char* wrmil_state_text(uint32_t code)
{
  return comlib_stateText(code);
} // wrmil_state_text


const char* wrmil_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // wrmil_version_text


void wrmil_debug(uint32_t flagDebug)
{
  if (flagDebug > 0) {
    wrmil_flagDebug = 1;
  } // if flagdebug
  else {
    wrmil_flagDebug = 0;
  } // else flagdebug
} // wrmil_debug


uint32_t wrmil_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
{
  eb_status_t         status;
  eb_device_t         eb_device;  
  struct sdb_device   sdbDevice;                           // instantiated lm32 core
  int                 nDevices;                            // number of instantiated cores

  *ebDevice = 0x0;
  if (cpu != 0) return COMMON_STATUS_OUTOFRANGE;           // chk, only support 1st core (this is a quick hack)
  nDevices = 1;

  // open Etherbone device and socket 
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDRX|EB_DATAX, &eb_socket)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_device_open(eb_socket, devName, EB_ADDRX|EB_DATAX, 3, &eb_device)) != EB_OK) return COMMON_STATUS_EB;

  //  get Wishbone address of lm32 
  if ((status = eb_sdb_find_by_identity(eb_device, GSI, LM32_RAM_USER, &sdbDevice, &nDevices)) != EB_OK) return COMMON_STATUS_EB;

  lm32_base            = sdbDevice.sdb_component.addr_first;

  comlib_initShared(lm32_base, SHARED_OFFS);
  wrmil_cmd              = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  wrmil_set_utcTrigger   = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_UTC_TRIGGER;
  wrmil_set_utcUtcDelay  = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_UTC_UTC_DELAY;
  wrmil_set_trigUtcDelay = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_TRIG_UTC_DELAY;
  wrmil_set_gid          = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_GID;
  wrmil_set_latency      = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_LATENCY;
  wrmil_set_utcOffsetHi  = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_UTC_OFFSET_HI;
  wrmil_set_utcOffsetLo  = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_UTC_OFFSET_LO;
  wrmil_set_requestFill  = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_REQUEST_FILL_EVT;
  wrmil_set_milDev       = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_MIL_DEV;
  wrmil_set_milMon       = lm32_base + SHARED_OFFS + WRMIL_SHARED_SET_MIL_MON;
  wrmil_get_nEvtsHi      = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_NUM_EVENTS_HI;       
  wrmil_get_nEvtsLo      = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_NUM_EVENTS_LO;
  wrmil_get_nLate        = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_LATE_EVENTS;         
  wrmil_get_comLatency   = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_COM_LATENCY;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;

  return COMMON_STATUS_OK;
} // wrmil_firmware_open


uint32_t wrmil_firmware_close(uint64_t ebDevice)
{
  eb_status_t status;
  eb_device_t eb_device;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;
  
  // close Etherbone device and socket
  if ((status = eb_device_close(eb_device)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(eb_socket)) != EB_OK) return COMMON_STATUS_EB;

  wrmil_debug(0);

  return COMMON_STATUS_OK;
} // wrmil_firmware_close


uint32_t wrmil_version_firmware(uint64_t ebDevice, uint32_t *version)
{
  uint64_t       dummy64a;
  uint32_t       dummy32a, dummy32b, dummy32c, dummy32d;
  uint32_t       errorCode;

  if (!ebDevice) return COMMON_STATUS_EB;
  *version = 0xffffffff;
  
  errorCode = wrmil_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, version, &dummy32d, 0);

  return errorCode;
} // wrmil_version_firmware


uint32_t wrmil_version_library(uint32_t *version)
{
  *version = (uint32_t)WRMILLIB_VERSION;

  return COMMON_STATUS_OK;
} // wrmil_version_library


void wrmil_printDiag(uint32_t utcTrigger, uint32_t utcDelay, uint32_t trigUtcDelay, uint32_t gid, int32_t latency, uint64_t utcOffset, uint32_t requestFill, uint32_t milDev, uint32_t milMon, uint64_t numEvts, uint32_t lateEvts, uint32_t comLatency)
{
  printf("wrmil: info  ...\n\n");

  printf("GID                          : 0x%015x\n"     , gid);
  printf("UTC trigger evtid            : 0d%015u\n"     , utcTrigger);
  printf("UTC MIL delay [us]           : 0d%015u\n"     , utcDelay);
  printf("UTC trigger event ID         : 0d%015u\n"     , utcTrigger);
  printf("MIL latency [ns]             : 0d%015u\n"     , latency);
  printf("UTC offset [ms]              : 0d%015lu\n"    , utcOffset);
  printf("request fill event           : 0d%015u\n"     , requestFill);
  printf("MIL dev (0: piggy, 1.. :SIO) : 0x%015x\n"     , milDev);
  printf("MIL monitoring               : 0d%015u\n"     , milMon);
  printf("# transmitted events         : 0d%015lu\n"    , numEvts);
  printf("# late events                : 0d%015u\n"     , lateEvts);
  printf("communiation latency         : 0d%015u\n"     , comLatency);
} // wrmil_printDiag


uint32_t wrmil_info_read(uint64_t ebDevice, uint32_t *utcTrigger, uint32_t *utcUtcDelay, uint32_t *trigUtcDelay, uint32_t *gid, int32_t *latency, uint64_t *utcOffset, uint32_t *requestFill, uint32_t *milDev,
                         uint32_t *milMon, uint64_t *numEvts, uint32_t *lateEvts, uint32_t *comLatency, int printFlag)
{
  eb_cycle_t   eb_cycle;
  eb_status_t  eb_status;
  eb_device_t  eb_device;
  eb_data_t    data[30];

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, wrmil_set_utcTrigger  , EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(eb_cycle, wrmil_set_utcUtcDelay , EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(eb_cycle, wrmil_set_trigUtcDelay, EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(eb_cycle, wrmil_set_gid         , EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(eb_cycle, wrmil_set_latency     , EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(eb_cycle, wrmil_set_utcOffsetHi , EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(eb_cycle, wrmil_set_utcOffsetLo , EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(eb_cycle, wrmil_set_requestFill , EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(eb_cycle, wrmil_set_milDev      , EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(eb_cycle, wrmil_set_milMon      , EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsHi     , EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsLo     , EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(eb_cycle, wrmil_get_nLate       , EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(eb_cycle, wrmil_get_comLatency  , EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  *utcTrigger    = data[0];
  *utcUtcDelay   = data[1];
  *trigUtcDelay  = data[2];
  *gid           = data[3];
  *latency       = data[4];
  *utcOffset     = ((uint64_t)data[5] & 0xffffffff) << 32;
  *utcOffset    |= (uint64_t)data[6] & 0xffffffff;
  *requestFill   = data[7];
  *milDev        = data[8];
  *milMon        = data[9];
  *numEvts       = ((uint64_t)data[10] & 0xffffffff) << 32;
  *numEvts      |= (uint64_t)data[11] & 0xffffffff;
  *lateEvts      = data[12];
  *comLatency    = data[13]; 

  if (printFlag) wrmil_printDiag(*utcTrigger, *utcUtcDelay, *trigUtcDelay, *gid, *latency, *utcOffset, *requestFill, *milDev, *milMon, *numEvts, *lateEvts, *comLatency);
  
  return COMMON_STATUS_OK;
} // wrmil_info_read


uint32_t wrmil_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t *nTransfer, int printDiag)
{
  eb_status_t eb_status;
  eb_device_t eb_device;

  uint64_t    dummy64a, dummy64b, dummy64c;
  uint32_t    dummy32a, dummy32c, dummy32d, dummy32e;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = comlib_readDiag(eb_device, statusArray, state, version, &dummy64a, &dummy32a, nBadStatus, nBadState, &dummy64b, &dummy64c,
                                   nTransfer, &dummy32c, &dummy32d, &dummy32e, printDiag)) != COMMON_STATUS_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // wrmil_status_read
  

uint32_t wrmil_upload(uint64_t ebDevice, uint32_t utcTrigger, uint32_t utcUtcDelay, uint32_t trigUtcDelay, uint32_t gid, int32_t latency, uint64_t utcOffset, uint32_t requestFill, uint32_t milDev, uint32_t milMon)
{
  eb_cycle_t   eb_cycle;     // eb cycle
  eb_status_t  eb_status;    // eb status

  if (!ebDevice) return COMMON_STATUS_EB;

  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, wrmil_set_utcTrigger  , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utcTrigger);
  eb_cycle_write(eb_cycle, wrmil_set_utcUtcDelay , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utcUtcDelay);
  eb_cycle_write(eb_cycle, wrmil_set_trigUtcDelay, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)trigUtcDelay);
  eb_cycle_write(eb_cycle, wrmil_set_gid         , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)gid);
  eb_cycle_write(eb_cycle, wrmil_set_latency     , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)latency);
  eb_cycle_write(eb_cycle, wrmil_set_utcOffsetHi , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((utcOffset >> 32) & 0xffffffff));
  eb_cycle_write(eb_cycle, wrmil_set_utcOffsetLo , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(utcOffset & 0xffffffff));
  eb_cycle_write(eb_cycle, wrmil_set_requestFill , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)requestFill);
  eb_cycle_write(eb_cycle, wrmil_set_milDev      , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)milDev);
  eb_cycle_write(eb_cycle, wrmil_set_milMon      , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)milMon);  
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // wrmil_context_ext_upload


void wrmil_cmd_configure(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_configure


void wrmil_cmd_startop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_startop


void wrmil_cmd_stopop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_stopop


void wrmil_cmd_recover(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_recover


void wrmil_cmd_idle(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_idle


void wrmil_cmd_cleardiag(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, wrmil_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // wrmil_cmd_cleardiag

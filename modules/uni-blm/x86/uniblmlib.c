/******************************************************************************
 *  uniblmlib.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 25-Oct-2024
 *
 * library for uni-blm
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
#include <time.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <common-defs.h>                  // common definitions
#include <common-lib.h>                   // common routines
#include <common-core.h>                  // common core
#include <uniblm_shared_mmap.h>           // FW shared def
#include <uni-blm.h>                      // FW defs
#include <uniblmlib.h>                    // x86 library
 
// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// public variables; chk: do we need a separate set of variables for uni-blm? Just in case we use two cores from the same application?
eb_socket_t  eb_socket;                    // EB socket
eb_address_t lm32_base;                    // lm32                  
eb_address_t uniblm_cmd;                   // command, write
uint32_t     uniblm_flagDebug = 0;         // flag debug

// application specific stuff
// set values
eb_address_t uniblm_set_setEventKey;              // bogus set value
eb_address_t uniblm_set_setB;              // bogus set value

// get values
eb_address_t uniblm_get_getReloadCounter;  // bogus get value
eb_address_t uniblm_get_getD;              // bogus get value

#define WAITCMDDONE COMMON_DEFAULT_TIMEOUT * 1000 // use default timeout and convert to us to be sure the command is processed

uint64_t uniblm_getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec * (uint64_t)1000000000+ tv.tv_usec * (uint64_t)1000;
} // uniblm_getSysTime()


void uniblm_t2secs(uint64_t ts, uint32_t *secs, uint32_t *nsecs)
{
  *nsecs = (uint32_t)(ts % 1000000000);
  *secs  = (uint32_t)(ts / 1000000000);
} // uniblm_t2secs


const char* uniblm_status_text(uint32_t code)
{  
  static char message[256];
  
  switch (code) {
    case UNIBLMLIB_STATUS_NODATA             : sprintf(message, "error %d, %s", code, "no data"); break;     
    default                                  : sprintf(message, "%s", comlib_statusText(code)); break;
  } // switch code
  
  return message;
} // uniblm_status_text


const char* uniblm_state_text(uint32_t code)
{
  return comlib_stateText(code);
} // uniblm_state_text


const char* uniblm_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // uniblm_version_text


void uniblm_debug(uint32_t flagDebug)
{
  if (flagDebug > 0) {
    uniblm_flagDebug = 1;
  } // if flagdebug
  else {
    uniblm_flagDebug = 0;
  } // else flagdebug
} // uniblm_debug


uint32_t uniblm_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
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
  uniblm_cmd              = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  uniblm_set_setEventKey  = lm32_base + SHARED_OFFS + UNIBLM_SHARED_SET_EVENT_KEY;
  uniblm_get_getReloadCounter = lm32_base + SHARED_OFFS + UNIBLM_SHARED_GET_RELOAD_COUNTER;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;

  return COMMON_STATUS_OK;
} // uniblm_firmware_open


uint32_t uniblm_firmware_close(uint64_t ebDevice)
{
  eb_status_t status;
  eb_device_t eb_device;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;
  
  // close Etherbone device and socket
  if ((status = eb_device_close(eb_device)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(eb_socket)) != EB_OK) return COMMON_STATUS_EB;

  uniblm_debug(0);

  return COMMON_STATUS_OK;
} // uniblm_firmware_close


uint32_t uniblm_version_firmware(uint64_t ebDevice, uint32_t *version)
{
  uint64_t       dummy64a;
  uint32_t       dummy32a, dummy32b, dummy32c, dummy32d, dummy32e;
  uint32_t       errorCode;

  if (!ebDevice) return COMMON_STATUS_EB;
  *version = 0xffffffff;
  
  errorCode = uniblm_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, version, &dummy32d, &dummy32e, 0);

  return errorCode;
} // uniblm_version_firmware


uint32_t uniblm_version_library(uint32_t *version)
{
  *version = (uint32_t)UNIBLMLIB_VERSION;

  return COMMON_STATUS_OK;
} // uniblm_version_library


void uniblm_printDiag(uint32_t getReloadCounter)
{
  printf("uniblm: info  ...\n\n");
  
  printf("getReloadCounter                    : %15d\n"        , getReloadCounter);
} // uniblm_printDiag


uint32_t uniblm_info_read(uint64_t ebDevice, uint32_t *getReloadCounter, int printFlag)
{
  eb_cycle_t   eb_cycle;
  eb_status_t  eb_status;
  eb_device_t  eb_device;
  eb_data_t    data[30];

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, uniblm_get_getReloadCounter        , EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  *getReloadCounter = data[0];

  if (printFlag) uniblm_printDiag(*getReloadCounter);
  
  return COMMON_STATUS_OK;
} // uniblm_info_read


uint32_t uniblm_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t *nTransfer, uint32_t *nLate, int printDiag)
{
  eb_status_t eb_status;
  eb_device_t eb_device;

  uint64_t    dummy64a, dummy64b, dummy64c;
  uint32_t    dummy32a, dummy32c, dummy32d, dummy32f, dummy32g, dummy32h;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = comlib_readDiag(eb_device, statusArray, state, version, &dummy64a, &dummy32a, nBadStatus, nBadState, &dummy64b, &dummy64c,
                                   nTransfer, &dummy32c, &dummy32d, nLate, &dummy32f, &dummy32g, &dummy32h, printDiag)) != COMMON_STATUS_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // uniblm_status_read
  

uint32_t uniblm_upload(uint64_t ebDevice, uint32_t setEventKey)
{
  eb_cycle_t   eb_cycle;     // eb cycle
  eb_status_t  eb_status;    // eb status

  if (!ebDevice) return COMMON_STATUS_EB;


  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, uniblm_set_setEventKey, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)setEventKey);
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) {printf("uni-blm: upload failed %d \n", eb_status); return eb_status;}

  return COMMON_STATUS_OK;
} // uniblm_upload


void uniblm_cmd_configure(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_configure


void uniblm_cmd_startop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_startop


void uniblm_cmd_stopop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_stopop


void uniblm_cmd_recover(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_recover


void uniblm_cmd_idle(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_idle


void uniblm_cmd_cleardiag(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, uniblm_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // uniblm_cmd_cleardiag

/******************************************************************************
 *  b2blib.c
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 27-January-2021
 *
 * library for b2b
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
#include <b2bcbu_shared_mmap.h>          // FW shared def
#include <b2b.h>                         // FW defs
#include <b2blib.h>                      // x86 library

// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// public variables
eb_socket_t  eb_socket;                 // EB socket
eb_address_t lm32_base;                 // lm32
eb_address_t b2b_cmd;                   // command, write
eb_address_t b2b_state;                 // state of state machine

// application specific stuff
// set values
eb_address_t b2b_set_gid;               // GID for transfer
eb_address_t b2b_set_sid;               // SID for transfer    
eb_address_t b2b_set_mode;              // mode of B2B transfer
eb_address_t b2b_set_TH1ExtHi;          // period of h=1 extraction, high bits
eb_address_t b2b_set_TH1ExtLo;          // period of h=1 extraction, low bits
eb_address_t b2b_set_nHExt;             // harmonic number of extraction RF
eb_address_t b2b_set_TH1InjHi;          // period of h=1 injection, high bits
eb_address_t b2b_set_TH1InjLo;          // period of h=1 injection, low bits
eb_address_t b2b_set_nHInj;             // harmonic number of injection RF
eb_address_t b2b_set_cPhase;            // phase correction
eb_address_t b2b_set_cTrigExt;          // kicker correction extraction
eb_address_t b2b_set_cTrigInj;          // kicker correction injection
eb_address_t b2b_cmd;                   // command, write


// get values
eb_address_t b2b_get_gid;               // GID for transfer
eb_address_t b2b_get_sid;               // SID for transfer    
eb_address_t b2b_get_mode;              // mode of B2B transfer
eb_address_t b2b_get_TH1ExtHi;          // period of h=1 extraction, high bits
eb_address_t b2b_get_TH1ExtLo;          // period of h=1 extraction, low bits
eb_address_t b2b_get_nHExt;             // harmonic number of extraction RF
eb_address_t b2b_get_TH1InjHi;          // period of h=1 injection, high bits
eb_address_t b2b_get_TH1InjLo;          // period of h=1 injection, low bits
eb_address_t b2b_get_nHInj;             // harmonic number of injection RF
eb_address_t b2b_get_TBeatHi;           // period of beating, high bits
eb_address_t b2b_get_TBeatLo;           // period of beating, low bits
eb_address_t b2b_get_cPhase;            // phase correction
eb_address_t b2b_get_cTrigExt;          // kicker correction extraction
eb_address_t b2b_get_cTrigInj;          // kicker correction injection
eb_address_t b2b_get_comLatency;        // latency for message transfer via ECA


#define WAITCMDDONE COMMON_DEFAULT_TIMEOUT * 1000 // use default timeout and convert to us to be sure the command is processed

uint64_t b2b_getSysTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
} // b2b_getSysTime()


const char* b2b_status_text(uint32_t bit)
{  
  static char message[256];
  
  switch (bit) {
    case B2B_STATUS_PHASEFAILED          : sprintf(message, "error %d, %s",    bit, "phase measurement failed"); break;                            
    case B2B_STATUS_TRANSFER             : sprintf(message, "error %d, %s",    bit, "transfer failed"); break;
    case B2B_STATUS_SAFETYMARGIN         : sprintf(message, "error %d, %s",    bit, "violation of safety margin for data master and timing network"); break;
    case B2B_STATUS_NORF                 : sprintf(message, "error %d, %s",    bit, "no RF signal detected"); break;
    case B2B_STATUS_LATEMESSAGE          : sprintf(message, "error %d, %s",    bit, "late timing message received"); break;
    default                              : sprintf(message, "%s", comlib_statusText(bit)); break;
  } // switch bit
  
  return message;
} // b2b_status_text


const char* b2b_state_text(uint32_t code)
{
  return comlib_stateText(code);
} // b2b_state_text


const char* b2b_version_text(uint32_t number)
{
  static char    version[32];

  sprintf(version, "%02x.%02x.%02x", (number & 0x00ff0000) >> 16, (number & 0x0000ff00) >> 8, number & 0x000000ff);

  return version;
} // b2b_version_text


double b2b_flsa2fdds(double flsa)
{
  double twoep32;
  double twoem32;
  double fclk;
  double fdds;

  twoep32 = pow(2,  32);
  twoem32 = pow(2, -32);
  fclk    = (double)B2B_F_CLK;

  fdds   = twoem32 * floor(twoep32 * flsa / fclk) * fclk;

  return fdds;
} // b2b_flsa2fdds


void b2b_t2secs(uint64_t ts, uint32_t *secs, uint32_t *nsecs)
{
  *nsecs = (uint32_t)(ts % 1000000000);
  *secs  = (uint32_t)(ts / 1000000000);
} // b2b_t2secs
  

uint32_t b2b_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
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
  b2b_cmd              = lm32_base + SHARED_OFFS + COMMON_SHARED_CMD;
  b2b_set_gid          = lm32_base + SHARED_OFFS + B2B_SHARED_SET_GID;     
  b2b_set_sid          = lm32_base + SHARED_OFFS + B2B_SHARED_SET_SID;     
  b2b_set_mode         = lm32_base + SHARED_OFFS + B2B_SHARED_SET_MODE;;   
  b2b_set_TH1ExtHi     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTHI;
  b2b_set_TH1ExtLo     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1EXTLO;
  b2b_set_nHExt        = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHEXT;   
  b2b_set_TH1InjHi     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJHI;
  b2b_set_TH1InjLo     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_TH1INJLO;
  b2b_set_nHInj        = lm32_base + SHARED_OFFS + B2B_SHARED_SET_NHINJ;
  b2b_set_cPhase       = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CPHASE;  
  b2b_set_cTrigExt     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGEXT;
  b2b_set_cTrigInj     = lm32_base + SHARED_OFFS + B2B_SHARED_SET_CTRIGINJ;                   
  b2b_get_gid          = lm32_base + SHARED_OFFS + B2B_SHARED_GET_GID;
  b2b_get_sid          = lm32_base + SHARED_OFFS + B2B_SHARED_GET_SID;
  b2b_get_mode         = lm32_base + SHARED_OFFS + B2B_SHARED_GET_MODE;;
  b2b_get_TH1ExtHi     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1EXTHI;
  b2b_get_TH1ExtLo     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1EXTLO;
  b2b_get_nHExt        = lm32_base + SHARED_OFFS + B2B_SHARED_GET_NHEXT;
  b2b_get_TH1InjHi     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1INJHI;
  b2b_get_TH1InjLo     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TH1INJLO;
  b2b_get_nHInj        = lm32_base + SHARED_OFFS + B2B_SHARED_GET_NHINJ;
  b2b_get_TBeatHi      = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TBEATHI;
  b2b_get_TBeatLo      = lm32_base + SHARED_OFFS + B2B_SHARED_GET_TBEATLO;
  b2b_get_cPhase       = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CPHASE;
  b2b_get_cTrigExt     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CTRIGEXT;
  b2b_get_cTrigInj     = lm32_base + SHARED_OFFS + B2B_SHARED_GET_CTRIGINJ;
  b2b_get_comLatency   = lm32_base + SHARED_OFFS + B2B_SHARED_GET_COMLATENCY;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;
  
  return COMMON_STATUS_OK;
} // b2b_firmware_open


uint32_t b2b_firmware_close(uint64_t ebDevice)
{
  eb_status_t status;
  eb_device_t eb_device;

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;
  
  // close Etherbone device and socket
  if ((status = eb_device_close(eb_device)) != EB_OK) return COMMON_STATUS_EB;
  if ((status = eb_socket_close(eb_socket)) != EB_OK) return COMMON_STATUS_EB;

  return COMMON_STATUS_OK;
} // b2b_firmware_close


uint32_t b2b_version_firmware(uint64_t ebDevice, uint32_t *version)
{
  uint64_t       dummy64a;
  uint32_t       dummy32a, dummy32b, dummy32c, dummy32d;
  uint32_t       errorCode;

  if (!ebDevice) return COMMON_STATUS_EB;
  *version = 0xffffffff;
  
  errorCode = b2b_common_read(ebDevice, &dummy64a, &dummy32a, &dummy32b, &dummy32c, version, &dummy32d, 0);

  return errorCode;
} // b2b_version_firmware


uint32_t b2b_version_library(uint32_t *version)
{
  *version = (uint32_t)B2BLIB_VERSION;

  return COMMON_STATUS_OK;
} // b2b_version_library


void b2b_printDiag(uint32_t sid, uint32_t gid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj, uint64_t TBeat, int32_t cPhase, int32_t cTrigExt, int32_t cTrigInj, int32_t comLatency)
{
  printf("b2b: info  ...\n\n");

  printf("SID                   : %012u\n"     , sid);
  printf("GID                   : %012u\n"     , gid);
  printf("mode                  : %012u\n"     , mode);
  printf("period h=1 extraction : %012.6f ns\n", (double)TH1Ext/1000000000.0);
  printf("period h=1 injection  : %012.6f ns\n", (double)TH1Inj/1000000000.0);
  printf("harmonic number extr. : %012d\n"     , nHExt);
  printf("harmonic number inj.  : %012d\n"     , nHInj);
  printf("period of beating     : %012.6f us\n", (double)TBeat/1000000000000.0);
  printf("adjust RF-phase       : %012d ns\n"  , cPhase);
  printf("adjust ext kicker     : %012d ns\n"  , cTrigExt);
  printf("adjust inj kicker     : %012d ns\n"  , cTrigInj);
  printf("communication latency : %012.3f us\n", (double)comLatency/1000.0);
} // b2b_printDiags


uint32_t b2b_info_read(uint64_t ebDevice, uint32_t *sid, uint32_t *gid, uint32_t *mode, uint64_t *TH1Ext, uint32_t *nHExt, uint64_t *TH1Inj, uint32_t *nHInj, uint64_t *TBeat, int32_t *cPhase, int32_t *cTrigExt, int32_t *cTrigInj, int32_t *comLatency, int printFlag)
{
  eb_cycle_t  eb_cycle;
  eb_status_t eb_status;
  eb_device_t eb_device;
  eb_data_t   data[30];

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, b2b_get_gid,           EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(eb_cycle, b2b_get_sid,           EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(eb_cycle, b2b_get_mode,          EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(eb_cycle, b2b_get_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(eb_cycle, b2b_get_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(eb_cycle, b2b_get_nHExt,         EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(eb_cycle, b2b_get_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(eb_cycle, b2b_get_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(eb_cycle, b2b_get_nHInj,         EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(eb_cycle, b2b_get_TBeatHi,       EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(eb_cycle, b2b_get_TBeatLo,       EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, b2b_get_cPhase,        EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(eb_cycle, b2b_get_cTrigExt,      EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(eb_cycle, b2b_get_cTrigInj,      EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(eb_cycle, b2b_get_comLatency,    EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

  *gid           = data[0];
  *sid           = data[1];
  *mode          = data[2];
  *TH1Ext        = (uint64_t)(data[3]) << 32;
  *TH1Ext       += data[4];
  *nHExt         = data[5];
  *TH1Inj        = (uint64_t)(data[6]) << 32;
  *TH1Inj       += data[7];
  *nHInj         = data[8];
  *TBeat         = (uint64_t)(data[9]) << 32;
  *TBeat        += data[10];
  *cPhase        = data[11];
  *cTrigExt      = data[12];
  *cTrigInj      = data[13];
  *comLatency    = data[14];

  if (printFlag) b2b_printDiag(*sid, *gid, *mode, *TH1Ext, *nHExt, *TH1Inj, *nHInj, *TBeat, *cPhase, *cTrigExt, *cTrigInj, *comLatency);
  
  return COMMON_STATUS_OK;
} // b2b_info_read


uint32_t b2b_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t *nTransfer, int printDiag)
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
} // b2b_status_read
  

uint32_t b2b_context_upload(uint64_t ebDevice, uint32_t sid, uint32_t gid, uint32_t mode, uint64_t TH1Ext, uint32_t nHExt, uint64_t TH1Inj, uint32_t nHInj,
                            int32_t  cPhase, int32_t  cTrigExt, int32_t  cTrigInj)
{
  eb_cycle_t   eb_cycle;
  eb_status_t  eb_status;
  
  if (!ebDevice) return COMMON_STATUS_EB;

  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, b2b_set_sid,           EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)sid);
  eb_cycle_write(eb_cycle, b2b_set_gid,           EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)gid);
  eb_cycle_write(eb_cycle, b2b_set_mode,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)mode);
  eb_cycle_write(eb_cycle, b2b_set_TH1ExtHi,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext >> 32));
  eb_cycle_write(eb_cycle, b2b_set_TH1ExtLo,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Ext & 0xffffffff));
  eb_cycle_write(eb_cycle, b2b_set_nHExt,         EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHExt);
  eb_cycle_write(eb_cycle, b2b_set_TH1InjHi,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj >> 32));
  eb_cycle_write(eb_cycle, b2b_set_TH1InjLo,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(TH1Inj & 0xffffffff));
  eb_cycle_write(eb_cycle, b2b_set_nHInj,         EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)nHInj);
  eb_cycle_write(eb_cycle, b2b_set_cPhase,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((uint32_t)cPhase));
  eb_cycle_write(eb_cycle, b2b_set_cTrigExt,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((uint32_t)cTrigExt));
  eb_cycle_write(eb_cycle, b2b_set_cTrigInj,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((uint32_t)cTrigInj));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) {printf("eb-status %d\n", eb_status); getchar(); return COMMON_STATUS_EB;};

  b2b_cmd_submit(ebDevice);

  return COMMON_STATUS_OK;
} // b2b_table_upload

uint32_t b2b_table_download(uint64_t ebDevice, uint32_t pz, uint32_t vacc, uint32_t chn, uint32_t *data, uint32_t *nData)
{
  if (!ebDevice) return COMMON_STATUS_EB;

  /* to be implemented */
  
  return COMMON_STATUS_OK;
} // b2b_table_download


void b2b_cmd_configure(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CONFIGURE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_configure


void b2b_cmd_startop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STARTOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_startop


void b2b_cmd_stopop(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_STOPOP, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_stopop


void b2b_cmd_recover(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_RECOVER, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_recover


void b2b_cmd_idle(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_IDLE, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_idle


void b2b_cmd_cleardiag(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)COMMON_CMD_CLEARDIAG, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_cleardiag


void b2b_cmd_submit(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFSUBMIT, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_submit


void b2b_cmd_clearConfig(uint64_t ebDevice)
{
  eb_device_t eb_device;

  if (!ebDevice) return;
  eb_device = (eb_device_t)ebDevice;
  eb_device_write(eb_device, b2b_cmd, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)B2B_CMD_CONFCLEAR, 0, eb_block);
  usleep(WAITCMDDONE); // wait for command execution to complete
} // b2b_cmd_clearConfig

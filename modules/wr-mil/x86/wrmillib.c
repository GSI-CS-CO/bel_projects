/******************************************************************************
 *  wrmillib.c
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 02-jan-2026
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
#include <time.h>

// etherbone
#include <etherbone.h>

// wr-unipz
#include <common-defs.h>                 // common definitions
#include <common-lib.h>                  // common routines
#include <common-core.h>                 // common core
#include <wrmil_shared_mmap.h>           // FW shared def
#include <wr-mil.h>                      // FW defs
#include <wr-f50.h>                      // FW defs
#include <wrmillib.h>                    // x86 library

// lm32 sdb
#define GSI           0x00000651
#define LM32_RAM_USER 0x54111351

// public variables; chk: do we need a separate set of variables for wr-f50? Just in case we use two cores from the same application?
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

eb_address_t wrf50_set_f50Offset;         // here: offset to TLU signal         
eb_address_t wrf50_set_mode;              // here: mode of 50 Hz synchronization

// get values
eb_address_t wrmil_get_gid;               // GID for transfer
eb_address_t wrmil_get_nEvtsSndHi;        // number of sent MIL telegrams, high word
eb_address_t wrmil_get_nEvtsSndLo;        // number of sent MIL telegrams, low word
eb_address_t wrmil_get_nEvtsRecTHi;       // number of received MIL telegrams (TAI), high word
eb_address_t wrmil_get_nEvtsRecTLo;       // number of received MIL telegrams (TAI), low word
eb_address_t wrmil_get_nEvtsRecDHi;       // number of received MIL telegrams (data), high word
eb_address_t wrmil_get_nEvtsRecDLo;       // number of received MIL telegrams (data), low word
eb_address_t wrmil_get_nEvtsErr;          // number of received MIL 'broken' MIL telegrams detected by VHDL Manchester decoder
eb_address_t wrmil_get_nEvtsBurst;        // number of detected 'high frequency bursts'

eb_address_t wrf50_get_TMainsAct;         // period of mains cycle [ns], actual value                           
eb_address_t wrf50_get_TDMAct;            // period of Data Master cycle [ns], actual value                     
eb_address_t wrf50_get_TDMSet;            // period of Data Master cycle [ns], actual value                     
eb_address_t wrf50_get_offsDMAct;         // offset of cycle start: t_DM_act - t_mains_act; actual value                
eb_address_t wrf50_get_offsDMMin;         // offset of cycle start: t_DM_act - t_mains_act; min value                   
eb_address_t wrf50_get_offsDMMax;         // offset of cycle start: t_DM_act - t_mains_act; max value                   
eb_address_t wrf50_get_dTDMAct;           // change of period: DM_act - DM_previous; actual value
eb_address_t wrf50_get_dTDMMin;           // change of period: DM_act - DM_previous; min value   
eb_address_t wrf50_get_dTDMMax;           // change of period: DM_act - DM_previous; max value   
eb_address_t wrf50_get_offsMainsAct;      // offset of cycle start: t_mains_act - t_mains_predict; actual value     
eb_address_t wrf50_get_offsMainsMin;      // offset of cycle start: t_mains_act - t_mains_predict; min value        
eb_address_t wrf50_get_offsMainsMax;      // offset of cycle start: t_mains_act - t_mains_predict; max value        
eb_address_t wrf50_get_lockState;         // lock state; how DM is locked to mains                              
eb_address_t wrf50_get_lockDateHi;        // time when lock has been achieve [ns], high bits                    
eb_address_t wrf50_get_lockDateLo;        // time when lock has been achieve [ns], low bits                     
eb_address_t wrf50_get_nLocked;           // counts how many locks have been achieved                           
eb_address_t wrf50_get_nCycles;           // number of UNILAC cycles
eb_address_t wrf50_get_nSent;             // number of messages sent to the Data Master (as broadcast)
eb_address_t wrf50_get_nEvtsLate;         // number of translated events that could not be delivered in time
eb_address_t wrf50_get_offsDone;          // offset t_mains_act to time when we are done
eb_address_t wrf50_get_comLatency;        // latency for messages received from via ECA (tDeadline - tNow)) [ns]


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
  wrmil_get_nEvtsSndHi   = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_SND_HI;       
  wrmil_get_nEvtsSndLo   = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_SND_LO;
  wrmil_get_nEvtsRecTHi  = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_RECT_HI;       
  wrmil_get_nEvtsRecTLo  = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_RECT_LO;
  wrmil_get_nEvtsRecDHi  = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_RECD_HI;
  wrmil_get_nEvtsRecDLo  = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_RECD_LO;
  wrmil_get_nEvtsErr     = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_ERR;
  wrmil_get_nEvtsBurst   = lm32_base + SHARED_OFFS + WRMIL_SHARED_GET_N_EVTS_BURST;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;

  return COMMON_STATUS_OK;
} // wrmil_firmware_open


uint32_t wrf50_firmware_open(uint64_t *ebDevice, const char* devName, uint32_t cpu, uint32_t *address)
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
  wrf50_set_f50Offset    = lm32_base + SHARED_OFFS + WRF50_SHARED_SET_F50OFFSET;
  wrf50_set_mode         = lm32_base + SHARED_OFFS + WRF50_SHARED_SET_MODE;
  wrf50_get_TMainsAct    = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_T_MAINS_ACT;
  wrf50_get_TDMAct       = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_T_DM_ACT;
  wrf50_get_TDMSet       = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_T_DM_SET;
  wrf50_get_offsDMAct    = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_DM_ACT;
  wrf50_get_offsDMMin    = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_DM_MIN;
  wrf50_get_offsDMMax    = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_DM_MAX;
  wrf50_get_dTDMAct      = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_DT_DM_ACT;
  wrf50_get_dTDMMin      = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_DT_DM_MIN;
  wrf50_get_dTDMMax      = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_DT_DM_MAX;
  wrf50_get_offsMainsAct = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_MAINS_ACT;
  wrf50_get_offsMainsMin = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_MAINS_MIN;
  wrf50_get_offsMainsMax = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_OFFS_MAINS_MAX;
  wrf50_get_lockState    = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_LOCK_STATE;
  wrf50_get_lockDateHi   = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_LOCK_DATE_HIGH;
  wrf50_get_lockDateLo   = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_LOCK_DATE_LOW;
  wrf50_get_nLocked      = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_N_LOCKED;
  wrf50_get_nCycles      = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_N_CYCLES;
  wrf50_get_nSent        = lm32_base + SHARED_OFFS + WRF50_SHARED_GET_N_SENT;

  // do this just at the very end
  *ebDevice = (uint64_t)eb_device;

  return COMMON_STATUS_OK;
} // wrf50_firmware_open


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


uint32_t wrf50_firmware_close(uint64_t ebDevice)
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
} // wrf50_firmware_close


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


void wrmil_printDiag(uint32_t utcTrigger, uint32_t utcDelay, uint32_t trigUtcDelay, uint32_t gid, int32_t latency, uint64_t utcOffset, uint32_t requestFill, uint32_t milDev,
                     uint32_t milMon, uint64_t nEvtsSnd, uint64_t nEvtsRecT, uint64_t nEvtsRecD, uint32_t nEvtsErr, uint32_t nEvtsBurst)
{
  printf("wrmil: info  ...\n\n");

  printf("GID                                 : 0x%015x\n"     , gid);
  printf("UTC trigger evt ID                  : 0x%015x\n"     , utcTrigger);
  printf("UTC MIL delay [us]                  : 0d%015u\n"     , utcDelay);
  printf("UTC trigger delay [us]              : 0d%015u\n"     , trigUtcDelay);
  printf("MIL latency [ns]                    : 0d%015d\n"     , latency);
  printf("UTC offset [ms]                     : 0d%015lu\n"    , utcOffset);
  printf("request fill event                  : 0d%015u\n"     , requestFill);
  printf("MIL dev (0: piggy, 1.. :SIO)        : 0x%015x\n"     , milDev);
  printf("MIL data monitoring                 : 0d%015u\n"     , milMon);
  printf("# MIL events sent                   : 0d%015lu\n"    , nEvtsSnd);
  printf("# MIL events received (TAI)         : 0d%015lu\n"    , nEvtsRecT);
  printf("# MIL events received (data)        : 0d%015lu\n"    , nEvtsRecD);
  printf("# MIL events received (error)       : 0d%015u\n"     , nEvtsErr);
  printf("# high frequency bursts             : 0d%015u\n"     , nEvtsBurst);
} // wrmil_printDiag


void wrf50_printDiag(int32_t f50Offs, uint32_t mode, uint32_t TMainsAct, uint32_t TDmAct, uint32_t TDmSet, int32_t offsDmAct, int32_t offsDmMin, int32_t offsDmMax, int32_t dTDMAct,
                     int32_t dTDMMin, int32_t dTDMMax,  int32_t offsMainsAct, int32_t offsMainsMin, int32_t offsMainsMax, uint32_t lockState, uint64_t lockDate, uint32_t nLocked,
                     uint32_t nCycles, uint32_t nSent)
{
  const struct tm* tm;
  char             timestr[60];
  time_t           secs;

  secs     = (unsigned long)((double)lockDate / 1000000000.0);
  tm = gmtime(&secs);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S TAI", tm);
  
  printf("wrf50: info  ...\n\n");
  
  printf("offset to 50 Hz mains [us]          : %15d\n"        , f50Offs);
  printf("mode                                : %15u\n"        , mode);
  printf("act period of mains [us]            : %15.3f\n"      , (double)TMainsAct/1000.0);
  printf("act period of DM [us]               : %15.3f\n"      , (double)TDmAct/1000.0);
  printf("set period of DM [us]               : %15.3f\n"      , (double)TDmSet/1000.0);
  printf("act offset DM: DM - mains [us]      : %15.3f\n"      , (double)offsDmAct/1000.0);
  printf("min offset DM: DM - mains [us]      : %15.3f\n"      , (double)offsDmMin/1000.0);
  printf("max offset DM: DM - mains [us]      : %15.3f\n"      , (double)offsDmMax/1000.0);
  printf("act period change DM: act - prev[us]: %15.3f\n"      , (double)dTDMAct/1000.0);
  printf("min period change DM: act - prev[us]: %15.3f\n"      , (double)dTDMMin/1000.0);
  printf("max period change DM: act - prev[us]: %15.3f\n"      , (double)dTDMMax/1000.0);
  printf("act offset mains: act - predict [us]: %15.3f\n"      , (double)offsMainsAct/1000.0);
  printf("min offset mains: act - predict [us]: %15.3f\n"      , (double)offsMainsMin/1000.0);
  printf("max offset mains: act - predict [us]: %15.3f\n"      , (double)offsMainsMax/1000.0);
  printf("DM lock state                       : %15u\n"        , lockState);
  printf("DM lock date                        : %s\n"          , timestr);
  printf("DM # locks                          : %15u\n"        , nLocked);
  printf("DM # cycles                         : %15u\n"        , nCycles);
  printf("DM # sent phase words               : %15u\n"        , nSent);
} // wrf50_printDiag


uint32_t wrmil_info_read(uint64_t ebDevice, uint32_t *utcTrigger, uint32_t *utcUtcDelay, uint32_t *trigUtcDelay, uint32_t *gid, int32_t *latency, uint64_t *utcOffset, uint32_t *requestFill, uint32_t *milDev,
                         uint32_t *milMon, uint64_t *nEvtsSnd, uint64_t *nEvtsRecT, uint64_t *nEvtsRecD, uint32_t *nEvtsErr, uint32_t *nEvtsBurst, int printFlag)
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
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsSndHi  , EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsSndLo  , EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsRecTHi , EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsRecTLo , EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsRecDHi , EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsRecDLo , EB_BIG_ENDIAN|EB_DATA32, &(data[15]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsErr    , EB_BIG_ENDIAN|EB_DATA32, &(data[16]));
  eb_cycle_read(eb_cycle, wrmil_get_nEvtsBurst  , EB_BIG_ENDIAN|EB_DATA32, &(data[17]));
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
  *nEvtsSnd      = ((uint64_t)data[10] & 0xffffffff) << 32;
  *nEvtsSnd     |= (uint64_t)data[11] & 0xffffffff;
  *nEvtsRecT      = ((uint64_t)data[12] & 0xffffffff) << 32;
  *nEvtsRecT     |= (uint64_t)data[13] & 0xffffffff;
  *nEvtsRecD      = ((uint64_t)data[14] & 0xffffffff) << 32;
  *nEvtsRecD     |= (uint64_t)data[15] & 0xffffffff;
  *nEvtsErr      = data[16];
  *nEvtsBurst    = data[17];

  if (printFlag) wrmil_printDiag(*utcTrigger, *utcUtcDelay, *trigUtcDelay, *gid, *latency, *utcOffset, *requestFill, *milDev, *milMon, *nEvtsSnd, *nEvtsRecT, *nEvtsRecD, *nEvtsErr, *nEvtsBurst);
  
  return COMMON_STATUS_OK;
} // wrmil_info_read


uint32_t wrf50_info_read(uint64_t ebDevice, int32_t  *f50Offs, uint32_t *mode, uint32_t *TMainsAct, uint32_t *TDmAct, uint32_t *TDmSet, int32_t *offsDmAct, int32_t *offsDmMin,
                         int32_t *offsDmMax, int32_t *dTDMAct, int32_t *dTDMMin, int32_t *dTDMMax, int32_t *offsMainsAct, int32_t *offsMainsMin, int32_t *offsMainsMax, uint32_t *lockState,
                         uint64_t *lockDate, uint32_t *nLocked, uint32_t *nCycles, uint32_t *nSent, int printFlag)
{
  eb_cycle_t   eb_cycle;
  eb_status_t  eb_status;
  eb_device_t  eb_device;
  eb_data_t    data[30];

  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_read(eb_cycle, wrf50_set_f50Offset   , EB_BIG_ENDIAN|EB_DATA32, &(data[0]));
  eb_cycle_read(eb_cycle, wrf50_set_mode        , EB_BIG_ENDIAN|EB_DATA32, &(data[1]));
  eb_cycle_read(eb_cycle, wrf50_get_TMainsAct   , EB_BIG_ENDIAN|EB_DATA32, &(data[2]));
  eb_cycle_read(eb_cycle, wrf50_get_TDMAct      , EB_BIG_ENDIAN|EB_DATA32, &(data[3]));
  eb_cycle_read(eb_cycle, wrf50_get_TDMSet      , EB_BIG_ENDIAN|EB_DATA32, &(data[4]));
  eb_cycle_read(eb_cycle, wrf50_get_offsDMAct   , EB_BIG_ENDIAN|EB_DATA32, &(data[5]));
  eb_cycle_read(eb_cycle, wrf50_get_offsDMMin   , EB_BIG_ENDIAN|EB_DATA32, &(data[6]));
  eb_cycle_read(eb_cycle, wrf50_get_offsDMMax   , EB_BIG_ENDIAN|EB_DATA32, &(data[7]));
  eb_cycle_read(eb_cycle, wrf50_get_dTDMAct     , EB_BIG_ENDIAN|EB_DATA32, &(data[8]));
  eb_cycle_read(eb_cycle, wrf50_get_dTDMMin     , EB_BIG_ENDIAN|EB_DATA32, &(data[9]));
  eb_cycle_read(eb_cycle, wrf50_get_dTDMMax     , EB_BIG_ENDIAN|EB_DATA32, &(data[10]));
  eb_cycle_read(eb_cycle, wrf50_get_offsMainsAct, EB_BIG_ENDIAN|EB_DATA32, &(data[11]));
  eb_cycle_read(eb_cycle, wrf50_get_offsMainsMin, EB_BIG_ENDIAN|EB_DATA32, &(data[12]));
  eb_cycle_read(eb_cycle, wrf50_get_offsMainsMax, EB_BIG_ENDIAN|EB_DATA32, &(data[13]));
  eb_cycle_read(eb_cycle, wrf50_get_lockState   , EB_BIG_ENDIAN|EB_DATA32, &(data[14]));
  eb_cycle_read(eb_cycle, wrf50_get_lockDateHi  , EB_BIG_ENDIAN|EB_DATA32, &(data[15]));
  eb_cycle_read(eb_cycle, wrf50_get_lockDateLo  , EB_BIG_ENDIAN|EB_DATA32, &(data[16]));
  eb_cycle_read(eb_cycle, wrf50_get_nLocked     , EB_BIG_ENDIAN|EB_DATA32, &(data[17]));
  eb_cycle_read(eb_cycle, wrf50_get_nCycles     , EB_BIG_ENDIAN|EB_DATA32, &(data[18]));
  eb_cycle_read(eb_cycle, wrf50_get_nSent       , EB_BIG_ENDIAN|EB_DATA32, &(data[19]));
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) return COMMON_STATUS_EB;

 *f50Offs       = data[0];
 *mode          = data[1];
 *TMainsAct     = data[2];
 *TDmAct        = data[3];
 *TDmSet        = data[4];
 *offsDmAct     = data[5];
 *offsDmMin     = data[6];
 *offsDmMax     = data[7];
 *dTDMAct       = data[8];
 *dTDMMin       = data[9];
 *dTDMMax       = data[10];
 *offsMainsAct  = data[11];
 *offsMainsMin  = data[12];
 *offsMainsMax  = data[13];
 *lockState     = data[14];
 *lockDate      = ((uint64_t)data[15] & 0xffffffff) << 32;
 *lockDate     |= (uint64_t)data[16] & 0xffffffff;
 *nLocked       = data[17];
 *nCycles       = data[18];
 *nSent         = data[19];

 if (printFlag) wrf50_printDiag(*f50Offs, *mode, *TMainsAct, *TDmAct, *TDmSet, *offsDmAct, *offsDmMin, *offsDmMax, *dTDMAct, *dTDMMin, *dTDMMax, *offsMainsAct, *offsMainsMin,
                                *offsMainsMax, *lockState, *lockDate, *nLocked, *nCycles, *nSent);

  return COMMON_STATUS_OK;
} // wrf50_info_read


uint32_t wrmil_common_read(uint64_t ebDevice, uint64_t *statusArray, uint32_t *state, uint32_t *nBadStatus, uint32_t *nBadState, uint32_t *version, uint32_t *nTransfer, int printDiag)
{
  eb_status_t   eb_status;
  eb_device_t   eb_device;
  comlib_diag_t data;


  if (!ebDevice) return COMMON_STATUS_EB;
  eb_device = (eb_device_t)ebDevice;

  if ((eb_status = comlib_readDiag2(eb_device, state, version, statusArray, &data, printDiag)) != COMMON_STATUS_OK) {
    *nBadStatus = data.nBadStatus;
    *nBadState  = data.nBadState;
    *nTransfer  = data.nTransfer;
    return COMMON_STATUS_EB;
  } // if eb_status

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
  eb_cycle_write(eb_cycle, wrmil_set_latency     , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(uint32_t)latency);          // we must cast a signed integer to a 32bit prior casting to eb_data_t
  eb_cycle_write(eb_cycle, wrmil_set_utcOffsetHi , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((utcOffset >> 32) & 0xffffffff));
  eb_cycle_write(eb_cycle, wrmil_set_utcOffsetLo , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)(utcOffset & 0xffffffff));
  eb_cycle_write(eb_cycle, wrmil_set_requestFill , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)requestFill);
  eb_cycle_write(eb_cycle, wrmil_set_milDev      , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)milDev);
  eb_cycle_write(eb_cycle, wrmil_set_milMon      , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)milMon);  
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) {printf("wr-mil: upload failed %d \n", eb_status); return eb_status;}

  return COMMON_STATUS_OK;
} // wrmil_context_ext_upload


uint32_t wrf50_upload(uint64_t ebDevice, int32_t  phaseOffset, uint32_t mode)
{
  eb_cycle_t   eb_cycle;     // eb cycle
  eb_status_t  eb_status;    // eb status

  if (!ebDevice) return COMMON_STATUS_EB;

  // EB cycle
  if (eb_cycle_open(ebDevice, 0, eb_block, &eb_cycle) != EB_OK) return COMMON_STATUS_EB;
  eb_cycle_write(eb_cycle, wrf50_set_f50Offset, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)phaseOffset);
  eb_cycle_write(eb_cycle, wrf50_set_mode     , EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)mode);
  if ((eb_status = eb_cycle_close(eb_cycle)) != EB_OK) {printf("wr-f50: upload failed %d \n", eb_status); return eb_status;}

  return COMMON_STATUS_OK;
} // wrf50_upload


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

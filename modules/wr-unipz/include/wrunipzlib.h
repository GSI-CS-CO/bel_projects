/******************************************************************************
 *  wrunipzlib.h
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 11-Jul-2024
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
#ifndef _WR_UNIPZ_LIB_H_
#define _WR_UNIPZ_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WRUNIPZLIB_VERSION 0x000218

// (error) codes; duplicated to avoid the need of joining bel_projects and acc git repos
#define  WRUNIPZLIB_STATUS_OK                 0    // OK
#define  WRUNIPZLIB_STATUS_ERROR              1    // an error occured
#define  WRUNIPZLIB_STATUS_TIMEDOUT           2    // a timeout occured
#define  WRUNIPZLIB_STATUS_OUTOFRANGE         3    // some value is out of range
#define  WRUNIPZLIB_STATUS_EB                 4    // an Etherbone error occured
#define  WRUNIPZLIB_STATUS_NOIP               5    // DHCP request via WR network failed
#define  WRUNIPZLIB_STATUS_WRONGIP            6    // IP received via DHCP does not match local config
#define  WRUNIPZLIB_STATUS_EBREADTIMEDOUT     7    // EB read via WR network timed out
#define  WRUNIPZLIB_STATUS_WRBADSYNC          8    // White Rabbit: not in 'TRACK_PHASE'
#define  WRUNIPZLIB_STATUS_AUTORECOVERY       9    // trying auto-recovery from state ERROR
#define  WRUNIPZLIB_STATUS_RESERVEDTILHERE   15    // 00..15 reserved for common error codes

// states; duplicated to avoid the need of joining bel_projects and acc git repos
#define  WRUNIPZLIB_STATE_UNKNOWN             0    // unknown state
#define  WRUNIPZLIB_STATE_S0                  1    // initial state -> IDLE (automatic)
#define  WRUNIPZLIB_STATE_IDLE                2    // idle state -> CONFIGURED (by command "configure")
#define  WRUNIPZLIB_STATE_CONFIGURED          3    // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  WRUNIPZLIB_STATE_OPREADY             4    // gateway in operation -> STOPPING ("stopop")
#define  WRUNIPZLIB_STATE_STOPPING            5    // gateway in operation -> CONFIGURED (automatic)
#define  WRUNIPZLIB_STATE_ERROR               6    // gateway in error -> IDLE ("recover")
#define  WRUNIPZLIB_STATE_FATAL               7    // gateway in fatal error; RIP

  // ---------------------------------
  // helper routines
  // ---------------------------------
  
  // get host system time (us)
  uint64_t wrunipz_getSysTime();

  // convert status code to status text
  const char* wrunipz_status_text(uint32_t code                // status code
                                  );

  // convert state code to state text
  const char* wrunipz_state_text(uint32_t code                 // state code
                                 );
  // convert numeric version number to string
  const char* wrunipz_version_text(uint32_t number             // version number
                                   );

  // ---------------------------------
  // communication with lm32 firmware
  // ---------------------------------

  // open connection to firmware, returns error code
  uint32_t wrunipz_firmware_open(uint64_t       *ebDevice,     // EB device
                                 const char*    device,        // EB device such as 'dev/wbm0'
                                 uint32_t       cpu,           // # of CPU, 0..0xff
                                 uint32_t       *wbAddr        // WB address of firmware
                                 );
  
  // close connection to firmware, returns error code
  uint32_t wrunipz_firmware_close(uint64_t ebDevice            // EB device
                                  );
  
  // get version of firmware, returns error code
  uint32_t wrunipz_version_firmware(uint64_t ebDevice,         // EB device
                                    uint32_t *version          // version number
                                    );

  // get version of library, returns error code
  uint32_t wrunipz_version_library(uint32_t *version           // version number
                                   );
  
  // get info from firmware, returns error code
  uint32_t wrunipz_info_read(uint64_t ebDevice,                // EB device
                             uint32_t *ncycles,                // # of cycles executed
                             uint32_t *tCycleAvg,              // average time per cycle [ns]
                             uint32_t *msgFreqAvg,             // average message rate [Hz]
                             uint32_t *nLate,                  // # of late messages
                             uint32_t *vaccAvg,                // virt accs used, bits 0..15 (normal), 16-31 (verkuerzt)
                             uint32_t *pzAvg,                  // PZ used (past second) bits 0..6
                             uint64_t *nMessages,              // # of messages sent
                             int32_t  *dtMax,                  // delta T max (actTime - deadline)
                             int32_t  *dtMin,                  // delta T min (actTime - deadline)
                             int32_t  *cycJmpMax,              // delta T max (expected and actual start of UNILAC cycle)
                             int32_t  *cycJmpMin               // delta T min (expected and actual start of UNILAC cycle)
                             );
  
  // get common properties from firmware, returns error code
  uint32_t wrunipz_common_read(uint64_t ebDevice,              // EB device
                               uint64_t *statusArray,          // array with status bits
                               uint32_t *state,                // state
                               uint32_t *nBadStatus,           // # of bad status incidents
                               uint32_t *nBadState,            // # of bad state incidents
                               uint32_t *version,              // FW version
                               uint32_t printDiag              // prints info on common firmware properties to stdout
                               );
  
  // uploads (parts of) an event table to the firmware, returns error code
  uint32_t wrunipz_table_upload(uint64_t ebDevice,             // EB device
                                uint32_t pz,                   // # of PZ
                                uint32_t vacc,                 // # of vacc
                                uint32_t chn,                  // # of 'Kanal'; there are max two channels
                                uint32_t *data,                // event data
                                uint32_t nData                 // # of events in data
                                );

  // downloads (parts of) an event table from the firmware, returns error code
  uint32_t wrunipz_table_download(uint64_t ebDevice,           // EB device
                                  uint32_t pz,                 // # of PZ
                                  uint32_t vacc,               // # of vacc
                                  uint32_t chn,                // # of 'Kanal'; there are max two channels
                                  uint32_t *data,              // event data
                                  uint32_t *nData              // # of events in data
                                  );

  // commands requesting state transitions
  void wrunipz_cmd_configure(uint64_t ebDevice);               // to state 'configured'
  void wrunipz_cmd_startop(uint64_t ebDevice);                 // to state 'opready'
  void wrunipz_cmd_stopop(uint64_t ebDevice);                  // back to state 'configured'
  void wrunipz_cmd_recover(uint64_t ebDevice);                 // try error recovery
  void wrunipz_cmd_idle(uint64_t ebDevice);                    // to state idle
  
  // commands for normal operation
  void wrunipz_cmd_cleardiag(uint64_t ebDevice);               // clear diagnostic data
  void wrunipz_cmd_submit(uint64_t ebDevice);                  // submit all pending event tables; useful for testing
  void wrunipz_cmd_clearTables(uint64_t ebDevice);             // clears all event tables; useful for testing

  
#ifdef __cplusplus
}
#endif 

#endif

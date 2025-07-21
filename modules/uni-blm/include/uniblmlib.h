/******************************************************************************
 *  uniblmlib.h
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 06-Nov-2024
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
#ifndef _UNIBLM_LIB_H_
#define _UNIBLM_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNIBLMLIB_VERSION 0x000001

// (error) codes; duplicated to avoid the need of joining bel_projects and acc git repos
#define  UNIBLMLIB_STATUS_OK                  0            // OK
#define  UNIBLMLIB_STATUS_ERROR               1            // an error occured
#define  UNIBLMLIB_STATUS_TIMEDOUT            2            // a timeout occured
#define  UNIBLMLIB_STATUS_OUTOFRANGE          3            // some value is out of range
#define  UNIBLMLIB_STATUS_EB                  4            // an Etherbone error occured
#define  UNIBLMLIB_STATUS_NOIP                5            // DHCP request via WR network failed
#define  UNIBLMLIB_STATUS_WRONGIP             6            // IP received via DHCP does not match local config
#define  UNIBLMLIB_STATUS_EBREADTIMEDOUT      7            // EB read via WR network timed out
#define  UNIBLMLIB_STATUS_WRBADSYNC           8            // White Rabbit: not in 'TRACK_PHASE'
#define  UNIBLMLIB_STATUS_AUTORECOVERY        9            // trying auto-recovery from state ERROR
#define  UNIBLMLIB_COMMON_STATUS_LATEMESSAGE 10            // late timing message received
#define  UNIBLMLIB_STATUS_BADSETTING         11            // bad setting data
#define  UNIBLMLIB_STATUS_RESERVEDTILHERE    15            // 00..15 reserved for common error codes
#define  UNIBLMLIB_STATUS_NODATA             16            // no data

// states; duplicated to avoid the need of joining bel_projects and acc git repos
#define  UNIBLMLIB_STATE_UNKNOWN             0            // unknown state
#define  UNIBLMLIB_STATE_S0                  1            // initial state -> IDLE (automatic)
#define  UNIBLMLIB_STATE_IDLE                2            // idle state -> CONFIGURED (by command "configure")
#define  UNIBLMLIB_STATE_CONFIGURED          3            // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  UNIBLMLIB_STATE_OPREADY             4            // in operation -> STOPPING ("stopop")
#define  UNIBLMLIB_STATE_STOPPING            5            // in operation -> CONFIGURED (automatic)
#define  UNIBLMLIB_STATE_ERROR               6            // in error -> IDLE ("recover")
#define  UNIBLMLIB_STATE_FATAL               7            // in fatal error; RIP
  
  // ---------------------------------

  // helper routines
  // ---------------------------------
  
  // get host system time [ns]
  uint64_t uniblm_getSysTime();

  //convert timestamp [ns] to seconds and nanoseconds
  void uniblm_t2secs(uint64_t ts,                               // timestamp [ns]
                      uint32_t *secs,                           // seconds
                      uint32_t *nsecs                           // nanosecons
                      );

  // convert status code to status text
  const char* uniblm_status_text(uint32_t code                  // status code
                              );

  // convert state code to state text
  const char* uniblm_state_text(uint32_t code                   // state code
                               );

  // convert numeric version number to string
  const char* uniblm_version_text(uint32_t number               // version number
                               );

  // enable debugging to trace library activity (experimental)
  void uniblm_debug(uint32_t flagDebug                          // 1: debug on; 0: debug off
                 );

  // ---------------------------------
  // communication with lm32 firmware
  // ---------------------------------

  // open connection to firmware, returns error code
  uint32_t uniblm_firmware_open(uint64_t     *ebDevice,          // EB device
                               const char*    device,            // EB device such as 'dev/wbm0'
                               uint32_t       cpu,               // # of CPU, 0..0xff
                               uint32_t       *wbAddr            // WB address of firmware
                               );
  
  // close connection to firmware, returns error code
  uint32_t uniblm_firmware_close(uint64_t ebDevice               // EB device
                                );
  
  // get version of firmware, returns error code
  uint32_t uniblm_version_firmware(uint64_t ebDevice,            // EB device
                                  uint32_t *version              // version number
                                  );
  
  // get version of library, returns error code
  uint32_t uniblm_version_library(uint32_t *version              // version number
                                 );
  
  // get info from firmware, returns error code
  uint32_t uniblm_info_read(uint64_t ebDevice,                   // EB device
                            uint32_t *getC,                      // get value C
                            uint32_t *getD,                      // get value D
                            int printFlag                        // print info to screen
                            );

  // get common properties from firmware, returns error code
  uint32_t uniblm_common_read(uint64_t ebDevice,                 // EB device
                               uint64_t *statusArray,            // array with status bits
                               uint32_t *state,                  // state
                               uint32_t *nBadStatus,             // # of bad status incidents
                               uint32_t *nBadState,              // # of bad state incidents
                               uint32_t *version,                // FW version
                               uint32_t *nTransfer,              // # of transfer
                               uint32_t *nLate,                  // number of late events received
                               int      printDiag                // prints info on common firmware properties to stdout
                             );
  
  // uploads configuration, returns error code
  uint32_t uniblm_upload(uint64_t ebDevice,                      // EB device
                          uint32_t setA                          // bogus parameter
                         );

  // commands requesting state transitions
  void uniblm_cmd_configure(uint64_t ebDevice);                  // to state 'configured'
  void uniblm_cmd_startop(uint64_t ebDevice);                    // to state 'opready'
  void uniblm_cmd_stopop(uint64_t ebDevice);                     // back to state 'configured'
  void uniblm_cmd_recover(uint64_t ebDevice);                    // try error recovery
  void uniblm_cmd_idle(uint64_t ebDevice);                       // to state idle
  
  // commands for normal operation
  void uniblm_cmd_cleardiag(uint64_t ebDevice);                  // clear diagnostic data
  void uniblm_cmd_submit(uint64_t ebDevice);                     // submit pending context
  void uniblm_cmd_clearConfig(uint64_t ebDevice);                // clear all context data

  
#ifdef __cplusplus
}
#endif 

#endif

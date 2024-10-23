/******************************************************************************
 *  unichoplib.h
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Oct-2024
 *
 * library for uni-chop
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
#ifndef _UNICHOP_LIB_H_
#define _UNICHOP_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UNICHOPLIB_VERSION 0x000007

// (error) codes; duplicated to avoid the need of joining bel_projects and acc git repos
#define  UNICHOPLIB_STATUS_OK                  0            // OK
#define  UNICHOPLIB_STATUS_ERROR               1            // an error occured
#define  UNICHOPLIB_STATUS_TIMEDOUT            2            // a timeout occured
#define  UNICHOPLIB_STATUS_OUTOFRANGE          3            // some value is out of range
#define  UNICHOPLIB_STATUS_EB                  4            // an Etherbone error occured
#define  UNICHOPLIB_STATUS_NOIP                5            // DHCP request via WR network failed
#define  UNICHOPLIB_STATUS_WRONGIP             6            // IP received via DHCP does not match local config
#define  UNICHOPLIB_STATUS_EBREADTIMEDOUT      7            // EB read via WR network timed out
#define  UNICHOPLIB_STATUS_WRBADSYNC           8            // White Rabbit: not in 'TRACK_PHASE'
#define  UNICHOPLIB_STATUS_AUTORECOVERY        9            // trying auto-recovery from state ERROR
#define  UNICHOPLIB_COMMON_STATUS_LATEMESSAGE 10            // late timing message received
#define  UNICHOPLIB_STATUS_BADSETTING         11            // bad setting data
#define  UNICHOPLIB_STATUS_RESERVEDTILHERE    15            // 00..15 reserved for common error codes
  //#define  UNICHOPLIB_STATUS_NODATA             16            // no data

// states; duplicated to avoid the need of joining bel_projects and acc git repos
#define  UNICHOPLIB_STATE_UNKNOWN             0            // unknown state
#define  UNICHOPLIB_STATE_S0                  1            // initial state -> IDLE (automatic)
#define  UNICHOPLIB_STATE_IDLE                2            // idle state -> CONFIGURED (by command "configure")
#define  UNICHOPLIB_STATE_CONFIGURED          3            // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  UNICHOPLIB_STATE_OPREADY             4            // in operation -> STOPPING ("stopop")
#define  UNICHOPLIB_STATE_STOPPING            5            // in operation -> CONFIGURED (automatic)
#define  UNICHOPLIB_STATE_ERROR               6            // in error -> IDLE ("recover")
#define  UNICHOPLIB_STATE_FATAL               7            // in fatal error; RIP

  enum evtTag{tagHLI, tagHSI};
  typedef enum evtTag evtTag_t;

  // data type monitoring values; data are in 'native units' used by the lm32 firmware
  // added 'dummies' to avoid odd-numbered data types for better compatibility 
  typedef struct{
    uint32_t  cyclesN;                                   // number of cycles
    uint32_t  triggerLen;                                // length of trigger pulse, t0 is rising edge of trigger pulse [us]
    uint32_t  triggerN;                                  // number of detected triggers
    uint32_t  triggerErrN;                               // number of trigger errors
    int32_t   triggerFlag;                               // 1: trigger detected
    int32_t   triggerErr;                                // 1: trigger error
    uint32_t  pulseStartT;                               // rising edge of chopper pulse [us], t0 is ...
    uint32_t  pulseStartN;                               // ... number
    uint32_t  pulseStartErrN;                            // ... number
    int32_t   pulseStartFlag;                            // ... detected
    int32_t   pulseStartErr;                             // ... error
    uint32_t  pulseStopT;                                // falling edge of chopper pulse [us], t0 is ...
    uint32_t  pulseStopN;                                // ... number
    uint32_t  pulseStopErrN;                             // ... number
    int32_t   pulseStopFlag;                             // ... detected
    uint32_t  pulseStopErr;                              // ... error
    uint32_t  pulseLen;                                  // length of chopper pulse
  } monData_t;
    
  // ---------------------------------

  // helper routines
  // ---------------------------------
  
  // get host system time [ns]
  uint64_t unichop_getSysTime();

  //convert timestamp [ns] to seconds and nanoseconds
  void unichop_t2secs(uint64_t ts,                              // timestamp [ns]
                      uint32_t *secs,                           // seconds
                      uint32_t *nsecs                           // nanosecons
                      );

  // convert status code to status text
  const char* unichop_status_text(uint32_t code                 // status code
                              );

  // convert state code to state text
  const char* unichop_state_text(uint32_t code                  // state code
                               );

  // convert numeric version number to string
  const char* unichop_version_text(uint32_t number              // version number
                               );

  // enable debugging to trace library activity (experimental)
  void unichop_debug(uint32_t flagDebug                         // 1: debug on; 0: debug off
                 );

  // ---------------------------------
  // communication with lm32 firmware
  // ---------------------------------

  // open connection to firmware, returns error code
  uint32_t unichop_firmware_open(uint64_t     *ebDevice,         // EB device
                               const char*    device,            // EB device such as 'dev/wbm0'
                               uint32_t       cpu,               // # of CPU, 0..0xff
                               uint32_t       *wbAddr            // WB address of firmware
                               );
  
  // close connection to firmware, returns error code
  uint32_t unichop_firmware_close(uint64_t ebDevice              // EB device
                                );
  
  // get version of firmware, returns error code
  uint32_t unichop_version_firmware(uint64_t ebDevice,           // EB device
                                  uint32_t *version              // version number
                                  );
  
  // get version of library, returns error code
  uint32_t unichop_version_library(uint32_t *version             // version number
                                 );
  
  // get info from firmware, returns error code
  uint32_t unichop_info_read(uint64_t ebDevice,                  // EB device
                             uint32_t *milDev,                   // mil device; 0: piggy, 1..N: SIO1..N
                             uint64_t *nMilSend,                 // # of MIL writes
                             uint32_t *nMilError,                // # of MIL communication errors
                             uint64_t *nEvtsReceived,            // # of received timing messages
                             int printFlag                       // print info to screen
                             );

  // get common properties from firmware, returns error code
  uint32_t unichop_common_read(uint64_t ebDevice,                // EB device
                             uint64_t *statusArray,              // array with status bits
                             uint32_t *state,                    // state
                             uint32_t *nBadStatus,               // # of bad status incidents
                             uint32_t *nBadState,                // # of bad state incidents
                             uint32_t *version,                  // FW version
                             uint32_t *nTransfer,                // # of transfer
                             int      printDiag                  // prints info on common firmware properties to stdout
                             );
  
  // uploads configuration, returns error code
  uint32_t unichop_upload(uint64_t ebDevice,                     // EB device
                          uint32_t milDev                       // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
                          );

  // commands requesting state transitions
  void unichop_cmd_configure(uint64_t ebDevice);                   // to state 'configured'
  void unichop_cmd_startop(uint64_t ebDevice);                     // to state 'opready'
  void unichop_cmd_stopop(uint64_t ebDevice);                      // back to state 'configured'
  void unichop_cmd_recover(uint64_t ebDevice);                     // try error recovery
  void unichop_cmd_idle(uint64_t ebDevice);                        // to state idle
  
  // commands for normal operation
  void unichop_cmd_cleardiag(uint64_t ebDevice);                   // clear diagnostic data
  void unichop_cmd_submit(uint64_t ebDevice);                      // submit pending context
  void unichop_cmd_clearConfig(uint64_t ebDevice);                 // clear all context data

  
#ifdef __cplusplus
}
#endif 

#endif

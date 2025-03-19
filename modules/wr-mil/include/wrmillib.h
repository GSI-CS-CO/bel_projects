/******************************************************************************
 *  wrmillib.h
 *
 *  created : 2024
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 10-Jul-2024
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
#ifndef _WRMIL_LIB_H_
#define _WRMIL_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WRMILLIB_VERSION 0x000014

// (error) codes; duplicated to avoid the need of joining bel_projects and acc git repos
#define  WRMILLIB_STATUS_OK                 0            // OK
#define  WRMILLIB_STATUS_ERROR              1            // an error occured
#define  WRMILLIB_STATUS_TIMEDOUT           2            // a timeout occured
#define  WRMILLIB_STATUS_OUTOFRANGE         3            // some value is out of range
#define  WRMILLIB_STATUS_EB                 4            // an Etherbone error occured
#define  WRMILLIB_STATUS_NOIP               5            // DHCP request via WR network failed
#define  WRMILLIB_STATUS_WRONGIP            6            // IP received via DHCP does not match local config
#define  WRMILLIB_STATUS_EBREADTIMEDOUT     7            // EB read via WR network timed out
#define  WRMILLIB_STATUS_WRBADSYNC          8            // White Rabbit: not in 'TRACK_PHASE'
#define  WRMILLIB_STATUS_AUTORECOVERY       9            // trying auto-recovery from state ERROR
#define  WRMILLIB_STATUS_RESERVEDTILHERE   15            // 00..15 reserved for common error codes

// states; duplicated to avoid the need of joining bel_projects and acc git repos
#define  WRMILLIB_STATE_UNKNOWN             0            // unknown state
#define  WRMILLIB_STATE_S0                  1            // initial state -> IDLE (automatic)
#define  WRMILLIB_STATE_IDLE                2            // idle state -> CONFIGURED (by command "configure")
#define  WRMILLIB_STATE_CONFIGURED          3            // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  WRMILLIB_STATE_OPREADY             4            // in operation -> STOPPING ("stopop")
#define  WRMILLIB_STATE_STOPPING            5            // in operation -> CONFIGURED (automatic)
#define  WRMILLIB_STATE_ERROR               6            // in error -> IDLE ("recover")
#define  WRMILLIB_STATE_FATAL               7            // in fatal error; RIP

  enum evtTag{tagStart, tagStop};
  typedef enum evtTag evtTag_t;

  // data type monitoring values; data are in 'native units' used by the lm32 firmware
  // added 'dummies' to avoid odd-numbered data types for better compatibility 
  typedef struct{
    uint32_t  gid;                                       // GID for which the gateway is active
    uint32_t  cMode;                                     // comparison mode; server option '-c'
    uint64_t  nFwSnd;                                    // firmware # of sent MIL telegrams
    uint64_t  nFwRecD;                                   // firmware # of received MIL telegrams (data)
    uint64_t  nFwRecT;                                   // firmware # of received MIL telegrams (TAI)
    uint32_t  nFwRecErr;                                 // firmware # of received 'broken' MIL telegrams detected by VHDL Manchester decoder
    uint32_t  nFwBurst;                                  // firmware # of detected high frequency bursts
    uint64_t  nStart;                                    // host # of start messages (type depends on comparison mode)
    uint64_t  nStop;                                     // host # of stop messages (type depends on comparison mode)
    uint64_t  nMatch;                                    // host # of matches (start vs stop messages)
    uint32_t  nFailSnd;                                  // host # of mismatches due to start event
    uint32_t  nFailEvt;                                  // host # of mismatches due to event number
    uint32_t  nFailOrder;                                // host # of mismatches due to event order
    double    tAct;                                      // actual deviation offset value, t_stop - t_start [us]
    double    tMin;                                      // minimum offset value [us]
    double    tMax;                                      // maximum offset value [us]
    double    tAve;                                      // average offset value [us]
    double    tSdev;                                     // standard deviation offset value [us]
  } monval_t;

    
  // ---------------------------------
  // helper routines
  // ---------------------------------
  
  // get host system time [ns]
  uint64_t wrmil_getSysTime();

  // convert status code to status text
  const char* wrmil_status_text(uint32_t code                    // status code
                              );

  // convert state code to state text
  const char* wrmil_state_text(uint32_t code                     // state code
                               );

  // convert numeric version number to string
  const char* wrmil_version_text(uint32_t number                 // version number
                               );

  //convert timestamp [ns] to seconds and nanoseconds
  void wrmil_t2secs(uint64_t ts,                                 // timestamp [ns]
                  uint32_t *secs,                              // seconds
                  uint32_t *nsecs                              // nanosecons
                  );

  // enable debugging to trace library activity (experimental)
  void wrmil_debug(uint32_t flagDebug                            // 1: debug on; 0: debug off
                 );

  // ---------------------------------
  // communication with lm32 firmware
  // ---------------------------------

  // open connection to firmware, returns error code
  uint32_t wrmil_firmware_open(uint64_t       *ebDevice,         // EB device
                               const char*    device,            // EB device such as 'dev/wbm0'
                               uint32_t       cpu,               // # of CPU, 0..0xff
                               uint32_t       *wbAddr            // WB address of firmware
                               );
  
  // close connection to firmware, returns error code
  uint32_t wrmil_firmware_close(uint64_t ebDevice                // EB device
                                );
  
  // open connection to firmware, returns error code
  uint32_t wrf50_firmware_open(uint64_t       *ebDevice,         // EB device
                               const char*    device,            // EB device such as 'dev/wbm0'
                               uint32_t       cpu,               // # of CPU, 0..0xff
                               uint32_t       *wbAddr            // WB address of firmware
                               );
  
  // close connection to firmware, returns error code
  uint32_t wrf50_firmware_close(uint64_t ebDevice                // EB device
                                );

  // get version of firmware, returns error code
  uint32_t wrmil_version_firmware(uint64_t ebDevice,             // EB device
                                  uint32_t *version              // version number
                                  );
  
  // get version of library, returns error code
  uint32_t wrmil_version_library(uint32_t *version               // version number
                                 );
  
  // get info from firmware, returns error code
  uint32_t wrmil_info_read(uint64_t ebDevice,                    // EB device
                           uint32_t *utcTrigger,                 // the MIL event that triggers the generation of UTC events
                           uint32_t *utcDelay,                   // delay [us] between the 5 generated UTC MIL events
                           uint32_t *trigUtcDelay,               // delay [us] between the trigger event and the first UTC (and other) generated events
                           uint32_t *gid,                        // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
                           int32_t  *latency,                    // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
                           uint64_t *utcOffset,                  // delay [ms] between the TAI and the MIL-UTC, high word   
                           uint32_t *requestFill,                // if this is written to 1, the gateway will send a fill event as soon as possible
                           uint32_t *milDev,                     // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
                           uint32_t *milMon,                     // 1: monitor MIL events; 0; don't monitor MIL events
                           uint64_t *nEvtsSnd,                   // number of MIL telegrams sent
                           uint64_t *nEvtsRecT,                  // number of MIL telegrams received (TAI)
                           uint64_t *nEvtsRecD,                  // number of MIL telegrams received (data)
                           uint32_t *nEvtsRecErr,                // number of 'broken' MIL telegrams received by VHDL Manchester decoder
                           uint32_t *nEvtsBurst,                 // number of detected high frequency bursts
                           int      printFlag                    // print info to screen 
                           );

  // get info from firmware, returns error code
  uint32_t wrf50_info_read(uint64_t ebDevice,                    // EB device
                           int32_t  *f50Offs,                    // offset to TLU signal
                           uint32_t *mode,                       // mode of 50 Hz synchronization
                           uint32_t *TMainsAct,                  // period of mains cycle [ns], actual value                           
                           uint32_t *TDmAct,                     // period of Data Master cycle [ns], actual value                     
                           uint32_t *TDmSet,                     // period of Data Master cycle [ns], set value calculated by fw for next DM cycle
                           int32_t  *offsDmAct,                  // offset of cycle start: t_DM_act - t_mains_act; actual value                
                           int32_t  *offsDmMin,                  // offset of cycle start: t_DM_act - t_mains_act; min value                   
                           int32_t  *offsDmMax,                  // offset of cycle start: t_DM_act - t_mains_act; max value
                           int32_t  *DTDMAct,                    // change of period: DM_act - DM_previous; actual value 
                           int32_t  *DTDMMin,                    // change of period: DM_act - DM_previous; min value   
                           int32_t  *DTDMMax,                    // change of period: DM_act - DM_previous; max value   
                           int32_t  *offsMainsAct,               // offset of cycle start: t_mains_act - t_mains_predict; actual value     
                           int32_t  *offsMainsMin,               // offset of cycle start: t_mains_act - t_mains_predict; min value        
                           int32_t  *offsMainsMax,               // offset of cycle start: t_mains_act - t_mains_predict; max value        
                           uint32_t *lockState,                  // lock state; how DM is locked to mains                              
                           uint64_t *lockDate,                   // time when lock has been achieved [ns]
                           uint32_t *nLocked,                    // counts how many locks have been achieved                           
                           uint32_t *nCycles,                    // number of UNILAC cycles
                           uint32_t *nSent,                      // number of messages sent to the Data Master (as broadcast)
                           int      printFlag                    // prints info on common firmware properties to stdout
                           );
 
  // get common properties from firmware, returns error code
  uint32_t wrmil_common_read(uint64_t ebDevice,                  // EB device
                             uint64_t *statusArray,              // array with status bits
                             uint32_t *state,                    // state
                             uint32_t *nBadStatus,               // # of bad status incidents
                             uint32_t *nBadState,                // # of bad state incidents
                             uint32_t *version,                  // FW version
                             uint32_t *nTransfer,                // # of transfer
                             int      printDiag                  // prints info on common firmware properties to stdout
                             );
  
  // uploads configuration, returns error code
  uint32_t wrmil_upload(uint64_t ebDevice,                       // EB device
                        uint32_t utcTrigger,                     // the MIL event that triggers the generation of UTC events
                        uint32_t utcUtcDelay,                    // delay [us] between the 5 generated UTC MIL events
                        uint32_t trigUtcDelay,                   // delay [us] between the trigger event and the first UTC (and other) generated events
                        uint32_t gid,                            // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
                        int32_t  latency,                        // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
                        uint64_t utcOffset,                      // delay [ms] between the TAI and the MIL-UTC, high word   
                        uint32_t requestFill,                    // if this is written to 1, the gateway will send a fill event as soon as possible
                        uint32_t milDev,                         // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
                        uint32_t milMon                          // 1: monitor MIL events; 0; don't monitor MIL events          
                        );


  // uploads configuration, returns error code
  uint32_t wrf50_upload(uint64_t ebDevice,                       // EB device
                        int32_t  phaseOffset,                    // offset [us] to the zero transition of the 50 Hz mains signal
                        uint32_t mode                            // see WRF50_MODE_...
                        );

  // commands requesting state transitions
  void wrmil_cmd_configure(uint64_t ebDevice);                   // to state 'configured'
  void wrmil_cmd_startop(uint64_t ebDevice);                     // to state 'opready'
  void wrmil_cmd_stopop(uint64_t ebDevice);                      // back to state 'configured'
  void wrmil_cmd_recover(uint64_t ebDevice);                     // try error recovery
  void wrmil_cmd_idle(uint64_t ebDevice);                        // to state idle
  
  // commands for normal operation
  void wrmil_cmd_cleardiag(uint64_t ebDevice);                   // clear diagnostic data
  void wrmil_cmd_submit(uint64_t ebDevice);                      // submit pending context
  void wrmil_cmd_clearConfig(uint64_t ebDevice);                 // clear all context data

  
#ifdef __cplusplus
}
#endif 

#endif

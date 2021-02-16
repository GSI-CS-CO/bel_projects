/******************************************************************************
 *  b2blib.h
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 08-Feb-2021
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
#ifndef _B2B_LIB_H_
#define _B2B_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#define B2BLIB_VERSION 0x000102

// (error) codes; duplicated to avoid the need of joining bel_projects and acc git repos
#define  B2BLIB_STATUS_OK                 0            // OK
#define  B2BLIB_STATUS_ERROR              1            // an error occured
#define  B2BLIB_STATUS_TIMEDOUT           2            // a timeout occured
#define  B2BLIB_STATUS_OUTOFRANGE         3            // some value is out of range
#define  B2BLIB_STATUS_EB                 4            // an Etherbone error occured
#define  B2BLIB_STATUS_NOIP               5            // DHCP request via WR network failed
#define  B2BLIB_STATUS_WRONGIP            6            // IP received via DHCP does not match local config
#define  B2BLIB_STATUS_EBREADTIMEDOUT     7            // EB read via WR network timed out
#define  B2BLIB_STATUS_WRBADSYNC          8            // White Rabbit: not in 'TRACK_PHASE'
#define  B2BLIB_STATUS_AUTORECOVERY       9            // trying auto-recovery from state ERROR
#define  B2BLIB_STATUS_RESERVEDTILHERE   15            // 00..15 reserved for common error codes

// states; duplicated to avoid the need of joining bel_projects and acc git repos
#define  B2BLIB_STATE_UNKNOWN             0            // unknown state
#define  B2BLIB_STATE_S0                  1            // initial state -> IDLE (automatic)
#define  B2BLIB_STATE_IDLE                2            // idle state -> CONFIGURED (by command "configure")
#define  B2BLIB_STATE_CONFIGURED          3            // configured state -> IDLE ("idle"), CONFIGURED ("configure"), OPREADY ("startop")
#define  B2BLIB_STATE_OPREADY             4            // in operation -> STOPPING ("stopop")
#define  B2BLIB_STATE_STOPPING            5            // in operation -> CONFIGURED (automatic)
#define  B2BLIB_STATE_ERROR               6            // in error -> IDLE ("recover")
#define  B2BLIB_STATE_FATAL               7            // in fatal error; RIP
  
  enum ringMachine{NORING, SIS18, ESR, CRYRING};
  typedef enum ringMachine ring_t;

  enum tuneKnob{NOKNOB, TRIGEXT, TRIGINJ, PHASE};
  typedef enum tuneKnob knob_t;

  enum evtTag{tagPme, tagPmi, tagPre, tagPri, tagKte, tagKti, tagKde, tagKdi, tagPde, tagPdi, tagStart, tagStop};
  typedef enum evtTag evtTag_t;

  typedef struct{                                      // data type set values
    uint32_t flag_nok;                                 // flag: data not ok; bit 0: mode, bit 1: ext_T, ...
    uint32_t mode;                                     // mode of B2B system
    uint64_t ext_T;                                    // extraction: period of h=1 Group DDS [as]
    uint32_t ext_h;                                    // extraction: harmonic number of rf
    int32_t  ext_cTrig;                                // extraction: correction for extraction kicker [ns]
    uint64_t inj_T;                                    // injection : ...
    uint32_t inj_h;
    int32_t  inj_cTrig;
    int32_t  cPhase;                                   // phase correction for b2b mode
  } setval_t;

  typedef struct{                                      // data type get values
    uint32_t flag_nok;                                 // flag: data not ok; bit 0: ext_phase, bit 1: ext_dKickMon ...
    uint64_t ext_phase;                                // extraction: phase of h=1 Group DDS [ns]
    int32_t  ext_dKickMon;                             // extraction: offset electronics monitor signal [ns]
    int32_t  ext_dKickProb;                            // extraction: offset magnet probe signal [ns]
    int32_t  ext_diagPhase;                            // extraction: offset from expected h=1 to actual h=1 signal [ns]
    int32_t  ext_diagMatch;                            // extraction: offset from calculated 'phase match' to actual h=1 signal [ns]
    uint64_t inj_phase;                                // injection : ...
    int32_t  inj_dKickMon;                             
    int32_t  inj_dKickProb;
    int32_t  inj_diagPhase;
    int32_t  inj_diagMatch;
    uint32_t flagEvtRec;                               // flag for events received; pme, pmi, pre, pri, kte, kti, kde, kdi, pde, pdi
    uint32_t flagEvtErr;                               // error flag;               pme, pmi, ...
    uint32_t flagEvtLate;                              // flag for events late;     pme, pmi, ...
    uint64_t tEKS;                                     // EKS deadline
    int32_t  doneOff;                                  // offset from EKS deadline to time when CBU sends KTE
    int32_t  preOff;                                   // offset from EKS to meausured extraction phasee
    int32_t  priOff;                                   // offset from EKS to meausured injection phasee
    int32_t  kteOff;                                   // offset from EKS to KTE deadline
    int32_t  ktiOff;                                   // offset from EKS to KTI deadline
  } getval_t;

  typedef struct{
    int32_t  ext_ddsOffAct;                            // extraction, gDDS measured offset: actual value
    uint32_t ext_ddsOffN;                              // number of values
    double   ext_ddsOffAve;                            // average value
    double   ext_ddsOffSdev;                           // standard deviation
    int32_t  ext_ddsOffMin;                            // minimum value
    int32_t  ext_ddsOffMax;                            // maximum value
    int32_t  inj_ddsOffAct;                            // injection, gDDS measured offset: ...
    uint32_t inj_ddsOffN;
    double   inj_ddsOffAve;
    double   inj_ddsOffSdev;
    int32_t  inj_ddsOffMin;
    int32_t  inj_ddsOffMax;
    int32_t  phaseOffAct;                              // gDDS measured phase offset: ...
    uint32_t phaseOffN;
    double   phaseOffAve;
    double   phaseOffSdev;
    int32_t  phaseOffMin;
    int32_t  phaseOffMax;
    int32_t  ext_rfOffAct;                            // extraction, measured rf offsety
    uint32_t ext_rfOffN;
    double   ext_rfOffAve;
    double   ext_rfOffSdev;
    int32_t  ext_rfOffMin;
    int32_t  ext_rfOffMax;
    int32_t  inj_rfOffAct;                            // injection, measured rf offset
    uint32_t inj_rfOffN;
    double   inj_rfOffAve;
    double   inj_rfOffSdev;
    int32_t  inj_rfOffMin;
    int32_t  inj_rfOffMax;
    uint32_t ext_rfNueN;                              // extraction, measured rf frequency
    double   ext_rfNueAve;
    double   ext_rfNueSdev;
    double   ext_rfNueDiff;
    double   ext_rfNueEst;
    uint32_t inj_rfNueN;                             // injection, measured rf frequency
    double   inj_rfNueAve;
    double   inj_rfNueSdev;
    double   inj_rfNueDiff;
    double   inj_rfNueEst;
  } diagval_t;

  typedef struct {    
    int32_t  eks_doneOffAct;                         // offset from EKS deadline to time when we are done
    uint32_t eks_doneOffN;
    double   eks_doneOffAve;
    double   eks_doneOffSdev;
    int32_t  eks_doneOffMin;
    int32_t  eks_doneOffMax;
    int32_t  eks_preOffAct;                          // offset from EKS to measured extraction phase
    uint32_t eks_preOffN;
    double   eks_preOffAve;
    double   eks_preOffSdev;
    int32_t  eks_preOffMin;
    int32_t  eks_preOffMax;
    int32_t  eks_priOffAct;                          // offset from EKS to measured injection phase
    uint32_t eks_priOffN;
    double   eks_priOffAve;
    double   eks_priOffSdev;
    int32_t  eks_priOffMin;
    int32_t  eks_priOffMax;
    int32_t  eks_kteOffAct;                          // offset from EKS to KTE
    uint32_t eks_kteOffN;
    double   eks_kteOffAve;
    double   eks_kteOffSdev;
    int32_t  eks_kteOffMin;
    int32_t  eks_kteOffMax;
    int32_t  eks_ktiOffAct;                          // offset from EKS to KTE
    uint32_t eks_ktiOffN;
    double   eks_ktiOffAve;
    double   eks_ktiOffSdev;
    int32_t  eks_ktiOffMin;
    int32_t  eks_ktiOffMax;
    int32_t  ext_monOffAct;                          // offset electronics monitor to KTE
    uint32_t ext_monOffN;
    double   ext_monOffAve;
    double   ext_monOffSdev;
    int32_t  ext_monOffMin;
    int32_t  ext_monOffMax;
    int32_t  inj_monOffAct;                          // offset electronics monitor to KTE
    uint32_t inj_monOffN;
    double   inj_monOffAve;
    double   inj_monOffSdev;
    int32_t  inj_monOffMin;
    int32_t  inj_monOffMax;
  } diagstat_t;
    
  // ---------------------------------
  // helper routines
  // ---------------------------------
  
  // get host system time (us)
  uint64_t b2b_getSysTime();

  // convert status code to status text
  const char* b2b_status_text(uint32_t code                    // status code
                              );
  // convert state code to state text
  const char* b2b_state_text(uint32_t code                     // state code
                             );
  // convert numeric version number to string
  const char* b2b_version_text(uint32_t number                 // version number
                               );
  // convert LSA frequency to DDS frequency
  double b2b_flsa2fdds(double flsa                             // LSA frequency [Hz]
                       );
  //convert timestamp to seconds and nanoseconds
  void b2b_t2secs(uint64_t ts,                                 // timestamp
                  uint32_t *secs,                              // seconds
                  uint32_t *nsecs                              // nanosecons
                  );
  
  // ---------------------------------
  // communication with lm32 firmware
  // ---------------------------------

  // open connection to firmware, returns error code
  uint32_t b2b_firmware_open(uint64_t       *ebDevice,         // EB device
                             const char*    device,            // EB device such as 'dev/wbm0'
                             uint32_t       cpu,               // # of CPU, 0..0xff
                             uint32_t       *wbAddr            // WB address of firmware
                             );
  
  // close connection to firmware, returns error code
  uint32_t b2b_firmware_close(uint64_t ebDevice                // EB device
                              );
  
  // get version of firmware, returns error code
  uint32_t b2b_version_firmware(uint64_t ebDevice,             // EB device
                                uint32_t *version              // version number
                                );

  // get version of library, returns error code
  uint32_t b2b_version_library(uint32_t *version               // version number
                               );
  
  // get info from firmware, returns error code
  uint32_t b2b_info_read(uint64_t ebDevice,                    // EB device
                         uint32_t *sid,                        // SID
                         uint32_t *gid,                        // GID
                         uint32_t *mode,                       // mode
                         uint64_t *TH1Ext,                     // period of H=1 extraction [as]
                         uint32_t *nHExt,                      // harmonic number extraction
                         uint64_t *TH1Inj,                     // period of H=1 injection [as]
                         uint32_t *nHInj,                      // harmonic number injection
                         uint64_t *TBeat,                      // period of beating signal [as]
                         int32_t *cPhase,                      // correction of phase [ns]
                         int32_t *cTrigExt,                    // correction of extraction kicker trigger [ns]
                         int32_t *cTrigInj,                    // correction of injection kicker trigger [ns]
                         int32_t *comLatency,                  // communication latency [ns]
                         int     printFlag                     // prints info on b2b firmware properties to stdout
                         );
  
  // get common properties from firmware, returns error code
  uint32_t b2b_common_read(uint64_t ebDevice,                  // EB device
                           uint64_t *statusArray,              // array with status bits
                           uint32_t *state,                    // state
                           uint32_t *nBadStatus,               // # of bad status incidents
                           uint32_t *nBadState,                // # of bad state incidents
                           uint32_t *version,                  // FW version
                           uint32_t *nTransfer,                // # of transfer
                           int      printDiag                  // prints info on common firmware properties to stdout
                           );
  
  // uploads configuration for a SID, returns error code
  uint32_t b2b_context_upload(uint64_t ebDevice,               // EB device
                              uint32_t sid,                    // SID
                              uint32_t gid,                    // GID
                              uint32_t mode,                   // mode
                              uint64_t TH1Ext,                 // h=1 period [as] of extraction machine
                              uint32_t nHExt,                  // harmonic number extraction machine
                              uint64_t TH1Inj,                 // h=1 period [as] of injection machine
                              uint32_t nHInj,                  // harmonic number injection machine
                              int32_t  cPhase,                 // phase correction [ns]
                              int32_t  cTrigExt,               // trigger correction extraction
                              int32_t  cTrigInj                // trigger correction injection
                              );

  // downloads configuration for a SID, returns error code
  uint32_t b2b_context_download(uint64_t ebDevice,             // EB device
                                uint32_t sid,                  // SID
                                uint32_t *gid,                 // GID
                                uint32_t *mode,                // mode
                                uint64_t *TH1Ext,              // h=1 period [as] of extraction machine
                                uint32_t *nHExt,               // harmonic number extraction machine
                                uint64_t *TH1Inj,              // h=1 period [as] of injection machine
                                uint32_t *nHInj,               // harmonic number injection machine
                                int32_t  *cPhase,              // phase correction [ns]
                                int32_t  *cTrigExt,            // trigger correction extraction
                                int32_t  *cTrigInj             // trigger correction injection
                                );

  // commands requesting state transitions
  void b2b_cmd_configure(uint64_t ebDevice);                   // to state 'configured'
  void b2b_cmd_startop(uint64_t ebDevice);                     // to state 'opready'
  void b2b_cmd_stopop(uint64_t ebDevice);                      // back to state 'configured'
  void b2b_cmd_recover(uint64_t ebDevice);                     // try error recovery
  void b2b_cmd_idle(uint64_t ebDevice);                        // to state idle
  
  // commands for normal operation
  void b2b_cmd_cleardiag(uint64_t ebDevice);                   // clear diagnostic data
  void b2b_cmd_submit(uint64_t ebDevice);                      // submit pending context
  void b2b_cmd_clearConfig(uint64_t ebDevice);                 // clear all context data

  
#ifdef __cplusplus
}
#endif 

#endif

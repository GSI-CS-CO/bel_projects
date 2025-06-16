/******************************************************************************
 *  b2blib.h
 *
 *  created : 2020
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 03-jan-2025
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

#define B2BLIB_VERSION 0x000807

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

  enum evtTag{tagPme, tagPmi, tagPre, tagPri, tagPse, tagPsi, tagKte, tagKti, tagKde, tagKdi, tagPde, tagPdi, tagStart, tagStop};
  typedef enum evtTag evtTag_t;

  // data type set values; data are in 'native units' used by the lm32 firmware; NAN of unsigned integers is signaled by all bits set
  typedef struct{                                      
    uint32_t mode;                                     // mode of B2B system
    uint64_t ext_T;                                    // extraction: period of h=1 Group DDS [as]
    uint32_t ext_h;                                    // extraction: harmonic number of rf
    float    ext_cTrig;                                // extraction: correction for extraction kicker [ns]
    uint32_t ext_sid;                                  // extraction: ID of extraction sequence (redundant)
    uint32_t ext_gid;                                  // extraction: GID of extraction machine (redundant)
    uint64_t inj_T;                                    // injection : ...
    uint32_t inj_h;
    float    inj_cTrig;
    uint32_t inj_sid;
    uint32_t inj_gid; 
    float    cPhase;                                   // phase correction for b2b mode
  } setval_t;

  // data type get values; data are in 'native units' used by the lm32 firmware
  typedef struct{                                      
    uint64_t ext_phase;                                // extraction: phase of h=1 Group DDS, ns part
    float    ext_phaseFract;                           // extraction: fractional phase [ps]
    float    ext_phaseErr;                             // extraction: (statistical) uncertainty of phase [ps]
    float    ext_phaseSysmaxErr;                       // extraction: maximum systematic error of phase [ns]
    float    ext_dKickMon;                             // extraction: offset electronics monitor signal [ns]
    float    ext_dKickProb;                            // extraction: offset magnet probe signal [ns]
    float    ext_dKickProbLen;                         // extraction: length of magnet probe signal [ns]
    float    ext_dKickProbLevel;                       // extraction: level of comparator for magent probe signal [%]
    float    ext_diagPhase;                            // extraction: offset from expected h=1 to actual h=1 signal [ns]
    float    ext_diagMatch;                            // extraction: offset from calculated 'phase match' to actual h=1 signal [ns]
    float    ext_phaseShift;                           // phase shift value [ns]
    uint64_t inj_phase;                                // injection : ...
    float    inj_phaseFract;
    float    inj_phaseErr;
    float    inj_phaseSysmaxErr;    
    float    inj_dKickMon;
    float    inj_dKickProb;
    float    inj_dKickProbLen;
    float    inj_dKickProbLevel;
    float    inj_diagPhase;
    float    inj_diagMatch;
    float    inj_phaseShift;
    uint32_t flagEvtRec;                               // flag for events received; pme, pmi, pre, pri, pse, psi, kte, kti, kde, kdi, pde, pdi, start, stop
    uint32_t flagEvtErr;                               // error flag;               pme, pmi, ...
    uint32_t flagEvtLate;                              // flag for events late;     pme, pmi, ...
    uint64_t tCBS;                                     // deadline of CMD_B2B_START [ns]
    float    finOff;                                   // offset from CBS deadline to time when CBU sends KTE [ns]
    float    prrOff;                                   // offset from CBS to time when CBU received all phase results
    float    preOff;                                   // offset from CBS to measured extraction phase [ns]
    float    priOff;                                   // offset from CBS to measured injection phase [ns]
    float    kteOff;                                   // offset from CBS to KTE deadline [ns]
    float    ktiOff;                                   // offset from CBS to KTI deadline [ns]
  } getval_t;

  // data type for diagnostic values
  typedef struct{                                      
    double   ext_ddsOffAct;                            // extraction, gDDS measured offset: actual value
    uint32_t ext_ddsOffN;                              // number of values
    double   ext_ddsOffAve;                            // average value
    double   ext_ddsOffSdev;                           // standard deviation
    double   ext_ddsOffMin;                            // minimum value
    double   ext_ddsOffMax;                            // maximum value
    double   inj_ddsOffAct;                            // injection, gDDS measured offset: ...
    uint32_t inj_ddsOffN;
    double   inj_ddsOffAve;
    double   inj_ddsOffSdev;
    double   inj_ddsOffMin;
    double   inj_ddsOffMax;
    double   phaseOffAct;                              // gDDS measured phase offset: ...
    uint32_t phaseOffN;
    double   phaseOffAve;
    double   phaseOffSdev;
    double   phaseOffMin;
    double   phaseOffMax;
    double   ext_rfOffAct;                             // extraction, measured rf offset
    uint32_t ext_rfOffN;
    double   ext_rfOffAve;
    double   ext_rfOffSdev;
    double   ext_rfOffMin;
    double   ext_rfOffMax;
    double   inj_rfOffAct;                             // injection, measured rf offset
    uint32_t inj_rfOffN;
    double   inj_rfOffAve;
    double   inj_rfOffSdev;
    double   inj_rfOffMin;
    double   inj_rfOffMax;
    double   ext_rfNueAct;                             // extraction, measured rf frequency
    double   ext_rfNueActErr;
    uint32_t ext_rfNueN;
    double   ext_rfNueAve;
    double   ext_rfNueSdev;
    double   ext_rfNueDiff;
    double   ext_rfNueEst;                             // estimated 'true' DDS frequency based on DDS resolution (32bit)
    double   inj_rfNueAct;                             // injection, measured rf frequency
    double   inj_rfNueActErr;
    uint32_t inj_rfNueN;
    double   inj_rfNueAve;
    double   inj_rfNueSdev;
    double   inj_rfNueDiff;
    double   inj_rfNueEst;
  } diagval_t;

  // data type for status information
  typedef struct {                                     
    double   cbs_finOffAct;                            // offset from CBS deadline to time when we are done
    uint32_t cbs_finOffN;
    double   cbs_finOffAve;
    double   cbs_finOffSdev;
    double   cbs_finOffMin;
    double   cbs_finOffMax;
    double   cbs_prrOffAct;                            // offset from CBS deadline to time when we received the PRE message
    uint32_t cbs_prrOffN;
    double   cbs_prrOffAve;
    double   cbs_prrOffSdev;
    double   cbs_prrOffMin;
    double   cbs_prrOffMax;
    double   cbs_preOffAct;                            // offset from CBS to measured extraction phase
    uint32_t cbs_preOffN;
    double   cbs_preOffAve;
    double   cbs_preOffSdev;
    double   cbs_preOffMin;
    double   cbs_preOffMax;
    double   cbs_priOffAct;                            // offset from CBS to measured injection phase
    uint32_t cbs_priOffN;
    double   cbs_priOffAve;
    double   cbs_priOffSdev;
    double   cbs_priOffMin;
    double   cbs_priOffMax;
    double   cbs_kteOffAct;                            // offset from CBS to KTE
    uint32_t cbs_kteOffN;
    double   cbs_kteOffAve;
    double   cbs_kteOffSdev;
    double   cbs_kteOffMin;
    double   cbs_kteOffMax;
    double   cbs_ktiOffAct;                            // offset from CBS to KTE
    uint32_t cbs_ktiOffN;
    double   cbs_ktiOffAve;
    double   cbs_ktiOffSdev;
    double   cbs_ktiOffMin;
    double   cbs_ktiOffMax;
    double   ext_monRemAct;                            // remainder (ext_T, h=1) from phase to electronics monitor; chk: can be removed after 2025 beamtime
    uint32_t ext_monRemN;                              
    double   ext_monRemAve;
    double   ext_monRemSdev;
    double   ext_monRemMin;
    double   ext_monRemMax;
    double   inj_monRemAct;                            // remainder (ext_T, h=1) from phase to electronics monitor; chk: can be removed after 2025 beamtime
    uint32_t inj_monRemN;
    double   inj_monRemAve;
    double   inj_monRemSdev;
    double   inj_monRemMin;
    double   inj_monRemMax;
  } diagstat_t;

  typedef struct {
    double   nueSet;                                   // DDS set value; just a crosscheck [Hz]
    double   nueGet;                                   // DDS measured value [Hz]
    double   nueDiff;                                  // difference nue - nueSet [Hz]
    double   nueErr;                                   // uncertainty of measured nue [Hz]
    double   nuerChi2;                                 // reduced chi square
    double   nueSlope;                                 // slope of measuared values [kHz/s], should be 0
    double   nueSlopeErr;                              // uncertainty of measured slope
    int32_t  nSeries;                                  // # of data series, a series contains multiple timestamps
    int32_t  nTS;                                      // # total number of time stamps used for calculus
    int32_t  nBadTS;                                   // # total number of bad (= dropped) time stamps
  } nueMeas_t;

  typedef struct {
    double   ppsAct;                                   // actual PPS value, fractional part of a second [ns]
    uint32_t ppsN;                                     // number of values
    double   ppsMean;                                  // mean value
    double   ppsSdev;                                  // standard deviation
    double   ppsMin;                                   // min value
    double   ppsMax;                                   // max value
  } jitterChk_t;
    
  // ---------------------------------
  // helper routines
  // ---------------------------------
  
  // get host system time [ns]
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
  //convert timestamp [ns] to seconds and nanoseconds
  void b2b_t2secs(uint64_t ts,                                 // timestamp [ns]
                  uint32_t *secs,                              // seconds
                  uint32_t *nsecs                              // nanosecons
                  );

  // find rising edge of h=1 signal nearest to 0; result [ns]
  double b2b_fixTS(double   tsDiff,                            // timestamp difference to '0' [ns]
                   double   corr,                              // given (trigger) correction [ns]
                   uint64_t TH1As                              // h=1 period [as]
                   );

  // enable debugging to trace library activity (experimental)
  void b2b_debug(uint32_t flagDebug                            // 1: debug on; 0: debug off
                 );

  // returns the maximum systematic deviation of the sub-ns fit [ps]
  uint32_t b2b_calc_max_sysdev_ps(uint64_t TH1_as,             // h=1 period [as]
                                  uint32_t nSamples,           // number of timestamp samples
                                  uint32_t printFlag           // 0: don't print info; >1 print info
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
  // after the 2022 beamtime, data types of cPhase, cTrigExt cTrigInj should change to *double
  uint32_t b2b_info_read(uint64_t ebDevice,                    // EB device
                         uint32_t *sid,                        // SID
                         uint32_t *gid,                        // GID
                         uint32_t *mode,                       // mode
                         uint64_t *TH1Ext,                     // period of h=1 extraction [as]
                         uint32_t *nHExt,                      // harmonic number extraction
                         uint64_t *TH1Inj,                     // period of h=1 injection [as]
                         uint32_t *nHInj,                      // harmonic number injection
                         uint64_t *TBeat,                      // period of beating signal [as]
                         double  *cPhase,                      // correction of phase [ns]
                         double  *cTrigExt,                    // correction of extraction kicker trigger [ns]
                         double  *cTrigInj,                    // correction of injection kicker trigger [ns]
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
  
  // uploads configuration for the extraction machine, returns error code
  // after the 2022 beamtime, data types of cPhase, cTrig should change to double
  uint32_t b2b_context_ext_upload(uint64_t ebDevice,           // EB device
                                  uint32_t sid,                // SID
                                  uint32_t gid,                // GID of ring machine
                                  uint32_t mode,               // mode
                                  double   nueH1,              // h=1 frequency [Hz] of machine
                                  uint32_t fNueConv,           // flag: convert frequency to DDS (default '1')
                                  uint32_t nH,                 // harmonic number of machine
                                  double   cTrig,              // trigger correction
                                  int32_t  nBucket,            // bucket number
                                  double   cPhase,             // phase correction [ns]
                                  uint32_t fFineTune,          // flag: use fine tune (default '1')
                                  uint32_t fMBTune             // flag: use multi-beat tune (default '1')
                                  );

  // uploads configuration for a injection machine, returns error code
  // after the 2022 beamtime, data type of cTrig should change to double
  uint32_t b2b_context_inj_upload(uint64_t ebDevice,           // EB device
                                  uint32_t sidExt,             // SID; NB: this is the SID of the extraction machine!!!
                                  uint32_t gid,                // GID of ring machine (injection machine)
                                  uint32_t sid,                // SID
                                  uint32_t bpid,               // bpid
                                  uint64_t param,              // parameter field
                                  double   nueH1,              // h=1 frequency [Hz] of machine
                                  uint32_t fNueConv,           // flag: convert frequency to DDS (default '1')
                                  uint32_t nH,                 // harmonic number injection machine
                                  double   cTrig,              // trigger correction injection
                                  int32_t  nBucket             // bucket number
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

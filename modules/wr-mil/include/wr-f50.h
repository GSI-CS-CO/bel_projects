#ifndef _WRF50_
#define _WRF50_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define WRF50_STATUS_LATEMESSAGE        16   // late timing message received
#define WRF50_STATUS_BADSETTING         17   // bad setting data
#define WRF50_STATUS_SAFETYMARGIN       18   // violation of safety margin for data master and timing network

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define WRF50_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define WRF50_ECADO_UNKOWN                1  // unkown activity requested (unexpected action by ECA)
#define WRF50_ECADO_F50_DM            0xfc0  // timing message from Data Master, indicates start of UNILAC 50 Hz cycle
#define WRF50_ECADO_F50_TUNE          0xfc1  // timing message sent by THIS fw, carries information for the Data Master
#define WRF50_ECADO_F50_TLU           0xa01  // timing message from TLU, evtno is 0xa01

// commands from the outside

// WR-F50 error flags (in EvtId)

// WR-F50 group ID; relevant messages are sent in this group
#define PZU_F50                       0x4c0    // information related to UNILAC 50 Hz synchronization

// WR-F50 lock states
#define WRF50_SLOCK_UNKWN               0x0    // state: unknonwn
#define WRF50_SLOCK_UNLOCKED            0x1    // state: not locked
#define WRF50_SLOCK_LOCKING             0x2    // state: locking
#define WRF50_SLOCK_LOCKED              0x3    // state: locked

// WR-F50 modes
#define WRF50_MODE_OFF                  0x0    // mode: off
#define WRF50_MODE_LOCK_HARD_SIM        0x2    // mode: lock to 50 Hz mains, follow mains without smoothing, simulation
#define WRF50_MODE_LOCK_SMOOTH_SIM      0x3    // mode: lock to 50 Hz mains, follow mains with smoothing, simulation
#define WRF50_MODE_LOCK_HARD_DM         0x4    // mode: lock to 50 Hz mains, follow mains without smoothing, Data Master
#define WRF50_MODE_LOCK_SMOOTH_DM       0x5    // mode: lock to 50 Hz mains, follow mains with smoothing, Data Master
#define WRF50_MASK_LOCK_SMOOTH          0x1    // mode is 'smooth' locking
#define WRF50_MASK_LOCK_SIM             0x2    // mode is simulation
#define WRF50_MASK_LOCK_DM              0x4    // mode is Data Master

// constants
#define WRF50_POSTTRIGGER_TLU        500000    // posttrigger [ns] for avoiding late messages from the TLU and defining an order (DM vs mains)x
#define WRF50_CYCLELEN_MIN         19800000    // minimum cycle length [ns]
#define WRF50_CYCLELEN_MAX         20400000    // maximum cycle length [ns]
#define WRF50_TUNE_MSG_DELAY        1000000    // delay for deadline for sending WRF50_ECADO_F50_TUNE message
//#define WRF50_CYCLEDIFF_MAX            1000    // maximum difference between cycle length of Data Master und mains [ns], used for set-value
//#define WRF50_LOCK_DIFFCYCLE           2000    // maximum difference between cycle length of Data Master und mains [ns], criterium for lock state
#define WRF50_LOCK_DIFFDM             20000    // maximum difference of WRF50_SHARED_GET_OFFS_DM_ACT, criterium for lock state
#define WRF50_LOCK_DIFFMAINS          20000    // maximum difference of WRF50_SHARED_GET_OFFS_MAINS_ACT, criterium for lock state
#define WRF50_N_STAMPS                   11    // number of timestamps used for averaging; THIS MUST BE AN ODD NUMBER to avoid floating point calculation

// default values
#define WR50_DFLT_F50OFFSET               0    // default value [ns] for offset from 50 Hz signal


// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define WRF50_SHARED_SET_F50OFFSET         (COMMON_SHARED_END                 + _32b_SIZE_)  // offset to TLU signal
#define WRF50_SHARED_SET_MODE              (WRF50_SHARED_SET_F50OFFSET        + _32b_SIZE_)  // mode of 50 Hz synchronization

// get values
#define WRF50_SHARED_GET_T_MAINS_ACT       (WRF50_SHARED_SET_MODE             + _32b_SIZE_)  // period of mains cycle [ns], actual value
#define WRF50_SHARED_GET_T_DM_ACT          (WRF50_SHARED_GET_T_MAINS_ACT      + _32b_SIZE_)  // period of Data Master cycle [ns], actual value
#define WRF50_SHARED_GET_T_DM_SET          (WRF50_SHARED_GET_T_DM_ACT         + _32b_SIZE_)  // period of Data Master cycle [ns], set value calculated by fw for next DM cycle
#define WRF50_SHARED_GET_OFFS_DM_ACT       (WRF50_SHARED_GET_T_DM_SET         + _32b_SIZE_)  // offset of cycle start: t_DM_act - t_DM_set; actual value
#define WRF50_SHARED_GET_OFFS_DM_MIN       (WRF50_SHARED_GET_OFFS_DM_ACT      + _32b_SIZE_)  // offset of cycle start: t_DM_act - t_DM_set; min value
#define WRF50_SHARED_GET_OFFS_DM_MAX       (WRF50_SHARED_GET_OFFS_DM_MIN      + _32b_SIZE_)  // offset of cycle start: t_DM_act - t_DM_set; max value
#define WRF50_SHARED_GET_OFFS_MAINS_ACT    (WRF50_SHARED_GET_OFFS_DM_MAX      + _32b_SIZE_)  // offset of cycle start: t_mains_act - t_mains_predict; actual value
#define WRF50_SHARED_GET_OFFS_MAINS_MIN    (WRF50_SHARED_GET_OFFS_MAINS_ACT   + _32b_SIZE_)  // offset of cycle start: t_mains_act - t_mains_predict; min value
#define WRF50_SHARED_GET_OFFS_MAINS_MAX    (WRF50_SHARED_GET_OFFS_MAINS_MIN   + _32b_SIZE_)  // offset of cycle start: t_mains_act - t_mains_predict; max value
#define WRF50_SHARED_GET_LOCK_STATE        (WRF50_SHARED_GET_OFFS_MAINS_MAX   + _32b_SIZE_)  // lock state; how DM is locked to mains
#define WRF50_SHARED_GET_LOCK_DATE_HIGH    (WRF50_SHARED_GET_LOCK_STATE       + _32b_SIZE_)  // time when lock has been achieved [ns], high bits
#define WRF50_SHARED_GET_LOCK_DATE_LOW     (WRF50_SHARED_GET_LOCK_DATE_HIGH   + _32b_SIZE_)  // time when lock has been achieved [ns], low bits
#define WRF50_SHARED_GET_N_LOCKED          (WRF50_SHARED_GET_LOCK_DATE_LOW    + _32b_SIZE_)  // counts how many locks have been achieved
#define WRF50_SHARED_GET_N_CYCLES          (WRF50_SHARED_GET_N_LOCKED         + _32b_SIZE_)  // number of UNILAC cycles
#define WRF50_SHARED_GET_N_EVTS_LATE       (WRF50_SHARED_GET_N_CYCLES         + _32b_SIZE_)  // number of translated events that could not be delivered in time
#define WRF50_SHARED_GET_COM_LATENCY       (WRF50_SHARED_GET_N_EVTS_LATE      + _32b_SIZE_)  // latency for messages received from via ECA (tDeadline - tNow)) [ns]

// diagnosis: end of used shared memory
#define WRF50_SHARED_END                   (WRF50_SHARED_GET_COM_LATENCY      + _32b_SIZE_) 

#endif

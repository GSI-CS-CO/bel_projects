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
#define WRF50_ECADO_F50_TLU           0x0a1  // timing message from TLU, evtno is 0xa01

// commands from the outside

// WR-F50 error flags (in EvtId)

// WR-F50 group ID; relevant messages are sent in this group
#define PZU_F50                       0x4c0    // information related to UNILAC 50 Hz synchronization 

// constants
#define WRF50_POSTTRIGGER_TLU         20000    // posttrigger [ns] for avoiding late messages from the TLU

// default values
#define WR50_DFLT_F50OFFSET               0    // default value [ns] for offset from 50 Hz signal


// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define WRF50_SHARED_SET_F50OFFSET         (COMMON_SHARED_END                 + _32b_SIZE_)  // offset to TLU signal
#define WRF50_SHARED_SET_UTC_UTC_DELAY     (WRF50_SHARED_SET_UTC_TRIGGER      + _32b_SIZE_)  // delay [us] between the 5 generated UTC MIL events
#define WRF50_SHARED_SET_TRIG_UTC_DELAY    (WRF50_SHARED_SET_UTC_UTC_DELAY    + _32b_SIZE_)  // delay [us] between the trigger event and the first UTC (and other) generated events
#define WRF50_SHARED_SET_GID               (WRF50_SHARED_SET_TRIG_UTC_DELAY   + _32b_SIZE_)  // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
#define WRF50_SHARED_SET_LATENCY           (WRF50_SHARED_SET_GID              + _32b_SIZE_)  // [us] MIL event is generated 100us+latency after the WR event. The value of latency can be negative
#define WRF50_SHARED_SET_UTC_OFFSET_HI     (WRF50_SHARED_SET_LATENCY          + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, high word 
#define WRF50_SHARED_SET_UTC_OFFSET_LO     (WRF50_SHARED_SET_UTC_OFFSET_HI    + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, low  word
#define WRF50_SHARED_SET_REQUEST_FILL_EVT  (WRF50_SHARED_SET_UTC_OFFSET_LO    + _32b_SIZE_)  // if this is written to 1, the gateway will send a fill event as soon as possible
#define WRF50_SHARED_SET_MIL_DEV           (WRF50_SHARED_SET_REQUEST_FILL_EVT + _32b_SIZE_)  // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
#define WRF50_SHARED_SET_MIL_MON           (WRF50_SHARED_SET_MIL_DEV          + _32b_SIZE_)  // 1: monitor MIL event data; 0; don't monitor MIL event data

// get values
#define WRF50_SHARED_GET_N_EVTS_SND_HI     (WRF50_SHARED_SET_MIL_MON          + _32b_SIZE_)  // number of sent MIL telegrams, high word
#define WRF50_SHARED_GET_N_EVTS_SND_LO     (WRF50_SHARED_GET_N_EVTS_SND_HI    + _32b_SIZE_)  // number of sent MIL telegrams, low word
#define WRF50_SHARED_GET_N_EVTS_RECT_HI    (WRF50_SHARED_GET_N_EVTS_SND_LO    + _32b_SIZE_)  // number of received MIL telegrams (TAI), high word
#define WRF50_SHARED_GET_N_EVTS_RECT_LO    (WRF50_SHARED_GET_N_EVTS_RECT_HI   + _32b_SIZE_)  // number of received MIL telegrams (TAI), low word
#define WRF50_SHARED_GET_N_EVTS_RECD_HI    (WRF50_SHARED_GET_N_EVTS_RECT_LO   + _32b_SIZE_)  // number of received MIL telegrams (data), high word
#define WRF50_SHARED_GET_N_EVTS_RECD_LO    (WRF50_SHARED_GET_N_EVTS_RECD_HI   + _32b_SIZE_)  // number of received MIL telegrams (data), low word
#define WRF50_SHARED_GET_N_EVTS_ERR        (WRF50_SHARED_GET_N_EVTS_RECD_LO   + _32b_SIZE_)  // number of received MIL telegrams with errors, detected by VHDL manchester decoder
#define WRF50_SHARED_GET_N_EVTS_LATE       (WRF50_SHARED_GET_N_EVTS_ERR       + _32b_SIZE_)  // number of translated events that could not be delivered in time
#define WRF50_SHARED_GET_COM_LATENCY       (WRF50_SHARED_GET_N_EVTS_LATE      + _32b_SIZE_)  // latency for messages received from via ECA (tDeadline - tNow)) [ns]

// diagnosis: end of used shared memory
#define WRF50_SHARED_END                   (WRF50_SHARED_GET_COM_LATENCY      + _32b_SIZE_) 

#endif

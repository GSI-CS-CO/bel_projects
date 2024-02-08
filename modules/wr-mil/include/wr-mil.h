#ifndef _WRMIL_
#define _WRMIL_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define WRMIL_STATUS_LATEMESSAGE        16   // late timing message received
#define WRMIL_STATUS_BADSETTING         17   // bad setting data
#define WRMIL_STATUS_SAFETYMARGIN       18   // violation of safety margin for data master and timing network

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define WRMIL_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define WRMIL_ECADO_UNKOWN              1   // unkown activity requested (unexpected action by ECA)
#define WRMIL_ECADO_MIL_EVT          0xff   // MIL event, evtno is 0x00..0xff

// commands from the outside

// WR-MIL error flags (in EvtId)

// the WR-MIL gateway can run at SIS18,  ESR, UNILAC  
// and has to be configured to one these; 
// at startup it is unconfigured.
#define GID_INVALID                     0x0   // invalid GID
#define SIS18_RING                    0x12c   // LSA GID
#define ESR_RING                      0x154   // LSA GID

// specialities
// interrupts
//#define WRMIL_GW_MSI_LATE_EVENT         0x1
//#define WRMIL_GW_MSI_STATE_CHANGED      0x2
//#define WRMIL_GW_MSI_EVENT              0x3  // ored with (mil_event_number << 8)

// default values
#define WRMIL_DFLT_UTC_TRIGGER         0xf6    // EVT_BEGIN_CMD_EXEC
#define WRMIL_DFLT_UTC_UTC_DELAY         30    // [us]
#define WRMIL_DFLT_TRIG_UTC_DELAY         0    // [us]
#define WRMIL_DFLT_LATENCY              100    // [us]
#define WRMIL_DFLT_UTC_OFFSET 1199142000000    // [ms], year 2008

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define WRMIL_SHARED_SET_UTC_TRIGGER       (COMMON_SHARED_END                 + _32b_SIZE_)  // the MIL event that triggers the generation of UTC events
#define WRMIL_SHARED_SET_UTC_UTC_DELAY     (WRMIL_SHARED_SET_UTC_TRIGGER      + _32b_SIZE_)  // delay [us] between the 5 generated UTC MIL events
#define WRMIL_SHARED_SET_TRIG_UTC_DELAY    (WRMIL_SHARED_SET_UTC_UTC_DELAY    + _32b_SIZE_)  // delay [us] between the trigger event and the first UTC (and other) generated events
#define WRMIL_SHARED_SET_GID               (WRMIL_SHARED_SET_TRIG_UTC_DELAY   + _32b_SIZE_)  // timing group ID for which the gateway is generating MIL events (example: 0x12c is SIS18)
#define WRMIL_SHARED_SET_LATENCY           (WRMIL_SHARED_SET_GID              + _32b_SIZE_)  // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
#define WRMIL_SHARED_SET_UTC_OFFSET_HI     (WRMIL_SHARED_SET_LATENCY          + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, high word 
#define WRMIL_SHARED_SET_UTC_OFFSET_LO     (WRMIL_SHARED_SET_UTC_OFFSET_HI    + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, low  word
#define WRMIL_SHARED_SET_REQUEST_FILL_EVT  (WRMIL_SHARED_SET_UTC_OFFSET_LO    + _32b_SIZE_)  // if this is written to 1, the gateway will send a fill event as soon as possible
#define WRMIL_SHARED_SET_MIL_DEV           (WRMIL_SHARED_SET_REQUEST_FILL_EVT + _32b_SIZE_)  // MIL device; 0: MIL Piggy; 1..: SIO in slot 1..

// get values
#define WRMIL_SHARED_GET_NUM_EVENTS_HI     (WRMIL_SHARED_SET_MIL_DEV          + _32b_SIZE_)  // number of translated events from WR to MIL, high word
#define WRMIL_SHARED_GET_NUM_EVENTS_LO     (WRMIL_SHARED_GET_NUM_EVENTS_HI    + _32b_SIZE_)  // number of translated events from WR to MIL, low word
#define WRMIL_SHARED_GET_LATE_EVENTS       (WRMIL_SHARED_GET_NUM_EVENTS_LO    + _32b_SIZE_)  // number of translated events that could not be delivered in time
#define WRMIL_SHARED_GET_COM_LATENCY       (WRMIL_SHARED_GET_LATE_EVENTS      + _32b_SIZE_)  // latency for messages received from via ECA (tDeadline - tNow)) [ns]
//#define WRMIL_SHARED_GET_LATE_HISTOGRAM    (WRMIL_SHARED_GET_COM_LATENCY      + _32b_SIZE_)  // dummy register to indicate position after the last valid register
//#define WRMIL_SHARED_GET_MIL_HISTOGRAM     (WRMIL_SHARED_GET_LATE_HISTOGRAM   + _32b_SIZE_)  // dummy register to indicate position after the last valid register
//#define WRMIL_SHARED_GET_MSI_SLOT          (WRMIL_SHARED_GET_MIL_HISTOGRAM    + _32b_SIZE_)  // MSI slot is stored here

// diagnosis: end of used shared memory
#define WRMIL_SHARED_END                   (WRMIL_SHARED_GET_COM_LATENCY      + _32b_SIZE_) 

#endif

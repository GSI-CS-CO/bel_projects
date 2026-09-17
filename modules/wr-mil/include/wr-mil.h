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
#define WRMIL_STATUS_MIL                19   // an error on MIL hardware occured (MIL piggy etc...)

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define WRMIL_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define WRMIL_ECADO_UNKOWN               1   // unkown activity requested (unexpected action by ECA)
#define WRMIL_ECADO_MIL_EVT           0xff   // timing message "EVT_...." received, evtno is 0x00..0xff
#define WRMIL_ECADO_MIL_TLU          0xa01   // timing message from TLU, evtno is 0xa01

// commands from the outside

// WR-MIL error flags (in EvtId)

// the WR-MIL gateway can run at SIS18,  ESR, UNILAC  
// and has to be configured to one these; 
// at startup it is unconfigured.
#define GID_INVALID                     0x0   // invalid GID
#define PZU_QR                        0x250   // LSA UNILAC Source Right                           ; new UR_TO_GUH1MU2
#define PZU_QL                        0x251   // LSA UNILAC Source Left                            ; new UL_TO_GUH1MU2
#define PZU_QN                        0x253   // LSA UNILAC Source High Charge State Injector (HLI); new UN_TO_GUN3BC1L
#define PZU_UN                        0x270   // LSA UNILAC High Charge State Injector (HLI)       ; new GUN3BC1L_TO_GUN6MU2
#define PZU_UH                        0x261   // LSA UNILAC High Current Injector (HSI)            ; new GUH2BC1L_TO_GUS3MK1
#define PZU_AT                        0x280   // LSA UNILAC Alvarez Cavities                       ; new GUS4MK6_TO_GUT1MK0
#define PZU_TK                        0x290   // LSA UNILAC Transfer Line                          ; new GUT1MK1_TO_GTK3MV1
#define PZU_F50                       0x4c0   // TOS internal: UNILAC 50 Hz synchronization
#define SIS18_RING                    0x12c   // LSA SIS18 ring
#define ESR_RING                      0x154   // LSA ESR ring
#define LOC_MIL_SEND                  0xff0   // internal: MIL telegrams to be sent
#define LOC_MIL_REC                   0xff1   // internal: MIL telegrams received (data)
#define LOC_TLU                       0xfe1   // internal: MIL telegrams received (timestamp only)

// specialities
// interrupts
//#define WRMIL_GW_MSI_LATE_EVENT         0x1
//#define WRMIL_GW_MSI_STATE_CHANGED      0x2
//#define WRMIL_GW_MSI_EVENT              0x3  // ored with (mil_event_number << 8)

// constants
#define WRMIL_MILSEND_LATENCY         23530    // latency [ns] from pushing to mil piggy/sio queue to last transition on the mil bus
#define WRMIL_MILSEND_MININTERVAL     25000    // min interval between sending two MIL telegrams
#define WRMIL_PRETRIGGER_DM          500000    // pretrigger [ns] for messages from the Data Master
#define WRMIL_POSTTRIGGER_TLU         20000    // posttrigger [ns] for avoiding late messages from the TLU
#define WRMIL_N_UTC_EVTS                  5    // number of generated EVT_UTC telegrams


// default values
#define WRMIL_DFLT_EVT_UTC_TRIGGER     0xf6    // EVT_BEGIN_CMD_EXEC
#define WRMIL_DFLT_EVT_UTC_1           0xe0    // EVT_UTC_1
#define WRMIL_DFLT_EVT_UTC_2           0xe1    // EVT_UTC_2
#define WRMIL_DFLT_EVT_UTC_3           0xe2    // EVT_UTC_3
#define WRMIL_DFLT_EVT_UTC_4           0xe3    // EVT_UTC_4
#define WRMIL_DFLT_EVT_UTC_5           0xe4    // EVT_UTC_5
#define WRMIL_DFLT_MIL_EVT_FILL        0xc7    // EVT_INTERNAL_FILL
#define WRMIL_DFLT_UTC_UTC_DELAY         30    // [us]
#define WRMIL_DFLT_TRIG_UTC_DELAY         0    // [us]
#define WRMIL_DFLT_LATENCY                0    // [us], can be used for tuning
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
#define WRMIL_SHARED_SET_LATENCY           (WRMIL_SHARED_SET_GID              + _32b_SIZE_)  // [us] MIL event is generated 100us+latency after the WR event. The value of latency can be negative
#define WRMIL_SHARED_SET_UTC_OFFSET_HI     (WRMIL_SHARED_SET_LATENCY          + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, high word 
#define WRMIL_SHARED_SET_UTC_OFFSET_LO     (WRMIL_SHARED_SET_UTC_OFFSET_HI    + _32b_SIZE_)  // offset [ms] between the TAI and the MIL-UTC, low  word
#define WRMIL_SHARED_SET_REQUEST_FILL_EVT  (WRMIL_SHARED_SET_UTC_OFFSET_LO    + _32b_SIZE_)  // if this is written to 1, the gateway will send a fill event as soon as possible
#define WRMIL_SHARED_SET_MIL_DEV           (WRMIL_SHARED_SET_REQUEST_FILL_EVT + _32b_SIZE_)  // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..
#define WRMIL_SHARED_SET_MIL_MON           (WRMIL_SHARED_SET_MIL_DEV          + _32b_SIZE_)  // 1: monitor MIL event data; 0; don't monitor MIL event data

// get values
#define WRMIL_SHARED_GET_N_EVTS_SND_HI     (WRMIL_SHARED_SET_MIL_MON          + _32b_SIZE_)  // number of sent MIL telegrams, high word
#define WRMIL_SHARED_GET_N_EVTS_SND_LO     (WRMIL_SHARED_GET_N_EVTS_SND_HI    + _32b_SIZE_)  // number of sent MIL telegrams, low word
#define WRMIL_SHARED_GET_N_EVTS_RECT_HI    (WRMIL_SHARED_GET_N_EVTS_SND_LO    + _32b_SIZE_)  // number of received MIL telegrams (TAI), high word
#define WRMIL_SHARED_GET_N_EVTS_RECT_LO    (WRMIL_SHARED_GET_N_EVTS_RECT_HI   + _32b_SIZE_)  // number of received MIL telegrams (TAI), low word
#define WRMIL_SHARED_GET_N_EVTS_RECD_HI    (WRMIL_SHARED_GET_N_EVTS_RECT_LO   + _32b_SIZE_)  // number of received MIL telegrams (data), high word
#define WRMIL_SHARED_GET_N_EVTS_RECD_LO    (WRMIL_SHARED_GET_N_EVTS_RECD_HI   + _32b_SIZE_)  // number of received MIL telegrams (data), low word
#define WRMIL_SHARED_GET_N_EVTS_ERR        (WRMIL_SHARED_GET_N_EVTS_RECD_LO   + _32b_SIZE_)  // number of received MIL telegrams with errors, detected by VHDL manchester decoder
#define WRMIL_SHARED_GET_N_EVTS_BURST      (WRMIL_SHARED_GET_N_EVTS_ERR       + _32b_SIZE_)  // number of occurences of 'nonsense high frequency bursts' 

// diagnosis: end of used shared memory
#define WRMIL_SHARED_END                   (WRMIL_SHARED_GET_N_EVTS_BURST     + _32b_SIZE_) 

#endif

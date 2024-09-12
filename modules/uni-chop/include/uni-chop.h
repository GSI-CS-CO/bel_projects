#ifndef _UNICHOP_
#define _UNICHOP_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define UNICHOP_STATUS_LATEMESSAGE        16   // late timing message received
#define UNICHOP_STATUS_BADSETTING         17   // bad setting data
#define UNICHOP_STATUS_MIL                18   // an error on MIL hardware occured (MIL piggy etc...)

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define UNICHOP_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define UNICHOP_ECADO_UNKOWN               1   // unkown activity requested (unexpected action by ECA)
#define UNICHOP_ECADO_STRAHLWEG_REC    0xfa0   // timing message received with 'Strahlweg information'; param 0..15: Strahlwegregister; param 16..31: Strahlwegmaske
#define UNICHOP_ECADO_MIL_SEND         0xfa8   // diagnostic timing message sent by firmware for each MIL write; param: 24..31 addr, 16..23: func code, 0..15 data

// commands from the outside

#define GID_INVALID                      0x0   // invalid GID
#define GID_LOCAL                      0xff0   // group for local timing messages received/sent by firmware

// specialities
// part below provided by Ludwig Hechler and Stefan Rauch
// interface boards: common function codes
#define IFB_FC_ADDR_BUS_W               0x11   // function code, address bus write
#define IFB_FC_DATA_BUS_W               0x10   // function code, data bus write
#define IFB_FC_DATA_BUS_R               0x90   // function code, data bus read

// interface board: main chopoper control
#define IFB_ADDR_CU                     0x68   // MIL address of chopper IF; FG 380.221
#define MOD_LOGIC1_ADDR                 0x09   // logic module 1 (Strahlwege et al),  module bus address,  FG 450.410/411
#define MOD_LOGIC1_REG_STRAHLWEG_REG    0x60   // logic module 1, register/address for Strahlwegregister
#define MOD_LOGIC1_REG_STRAHLWEG_MASK   0x62   // logic module 1, register/address for Strahlwegmaske
#define MOD_LOGIC1_REG_STATUSGLOBAL     0x66   // logic module 1, register/address for global status, contains version number (2024: 0x14)

#define IFB_ADDR_RPG_0                  0x60   // MIL address; Rahmenpulsgenerator 0
#define IFB_ADDR_RPG_1                  0x61   // MIL address; Rahmenpulsgenerator 1
#define IFB_ADDR_RPG_2                  0x62   // MIL address; Rahmenpulsgenerator 2
#define IFB_ADDR_RPG_3                  0x63   // MIL address; Rahmenpulsgenerator 3


// interrupts
//#define UNICHOP_GW_MSI_LATE_EVENT         0x1
//#define UNICHOP_GW_MSI_STATE_CHANGED      0x2
//#define UNICHOP_GW_MSI_EVENT              0x3  // ored with (mil_event_number << 8)

// constants

// default values

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define UNICHOP_SHARED_SET_MIL_DEV           (COMMON_SHARED_END                   + _32b_SIZE_)  // MIL device for sending MIL messages; 0: MIL Piggy; 1..: SIO in slot 1..

// get values
#define UNICHOP_SHARED_GET_N_MIL_SND_HI      (UNICHOP_SHARED_SET_MIL_DEV          + _32b_SIZE_)  // number of sent MIL telegrams, high word
#define UNICHOP_SHARED_GET_N_MIL_SND_LO      (UNICHOP_SHARED_GET_N_MIL_SND_HI     + _32b_SIZE_)  // number of sent MIL telegrams, low word
#define UNICHOP_SHARED_GET_N_MIL_SND_ERR     (UNICHOP_SHARED_GET_N_MIL_SND_LO     + _32b_SIZE_)  // number of failed MIL writes
#define UNICHOP_SHARED_GET_N_EVTS_REC_HI     (UNICHOP_SHARED_GET_N_MIL_SND_ERR    + _32b_SIZE_)  // number of received timing messages with Strahlweg data, high word
#define UNICHOP_SHARED_GET_N_EVTS_REC_LO     (UNICHOP_SHARED_GET_N_EVTS_REC_HI    + _32b_SIZE_)  // number of received timing messages with Strahlweg data, low word

// diagnosis: end of used shared memory
#define UNICHOP_SHARED_END                   (UNICHOP_SHARED_GET_N_EVTS_REC_LO    + _32b_SIZE_) 

#endif

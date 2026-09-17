#ifndef _UNIBLM_
#define _UNIBLM_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define UNIBLM_STATUS_BLM                    16   // an error on BLM hardware occured

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define UNIBLM_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define UNIBLM_ECADO_UNKOWN                   1   // unkown activity requested (unexpected action by ECA)

// commands from the outside

// GIDs
#define GID_INVALID                         0x0   // invalid GID

// constants

// default values

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// BLM gateware does support this amount of counter groups
#define UNIBLM_NUMBER_OF_COUNTER_GROUPS 16

// offsets
// set values
#define UNIBLM_SHARED_SET_EVENT_KEY         (COMMON_SHARED_END                  + _32b_SIZE_)  // set event key

// get values of the 16 groups
//                   _________
#define UNIBLM_SHARED_GET_RELOAD_COUNTER_0  (UNIBLM_SHARED_SET_EVENT_KEY        + _32b_SIZE_)  // get counter of reload events


// diagnosis: end of used shared memory
#define UNIBLM_SHARED_END                   (UNIBLM_SHARED_GET_RELOAD_COUNTER_0 + (UNIBLM_NUMBER_OF_COUNTER_GROUPS*_32b_SIZE_))

#endif

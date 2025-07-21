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
#define UNIBLM_ECADO_EVT_BEAM_ON          0x006   // EVT_BEAM_ON

// commands from the outside

// GIDs
#define GID_INVALID                         0x0   // invalid GID
#define GID_PZU_QR                        0x250   // UNILAC Timing - Source Right

// constants

// default values

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values
#define UNIBLM_SHARED_SET_A                 (COMMON_SHARED_END                  + _32b_SIZE_)  // set value A
#define UNIBLM_SHARED_SET_B                 (UNIBLM_SHARED_SET_A                + _32b_SIZE_)  // set value B


// get values
#define UNIBLM_SHARED_GET_C                 (UNIBLM_SHARED_SET_B                + _32b_SIZE_)  // get value C
#define UNIBLM_SHARED_GET_D                 (UNIBLM_SHARED_GET_C                + _32b_SIZE_)  // get value D

// diagnosis: end of used shared memory
#define UNIBLM_SHARED_END                   (UNIBLM_SHARED_GET_D                + _32b_SIZE_) 

#endif

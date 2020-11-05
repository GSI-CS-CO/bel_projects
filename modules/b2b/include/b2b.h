#ifndef _B2B_
#define _B2B_

#include <common-defs.h>

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define  B2B_STATUS_PHASEFAILED      16    // phase measurement failed
#define  B2B_STATUS_TRANSFER         17    // transfer failed
#define  B2B_STATUS_SAFETYMARGIN     18    // violation of safety margin for data master and timing network

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define  B2B_ECADO_TIMEOUT         COMMON_ECADO_TIMEOUT
#define  B2B_ECADO_UNKOWN             1    // unkown activity requested (unexpected action by ECA)
#define  B2B_ECADO_TLUINPUT           2    // event from input (TLU)
#define  B2B_ECADO_KICKSTART        100    // SIS18 extraction: EVT_KICK_START1; ESR extraction: EVT_KICK_START2
#define  B2B_ECADO_B2B_PMEXT       2048    // command: perform phase measurement (extraction)
#define  B2B_ECADO_B2B_PMINJ       2049    // command: perform phase measurement (injection)
#define  B2B_ECADO_B2B_PREXT       2050    // command: result of phase measurement (extraction)
#define  B2B_ECADO_B2B_PRINJ       2051    // command: result of phase measurement (injecton)
#define  B2B_ECADO_B2B_TRIGGEREXT  2052    // command: trigger kicker (extraction)
#define  B2B_ECADO_B2B_TRIGGERINJ  2053    // command: trigger kicker (injection)
#define  B2B_ECADO_B2B_DIAGMATCH   2054    // command: time, when h=1 phases of extraction and injection will match, includes phase corrections
#define  B2B_ECADO_B2B_DIAGEXT     2055    // command: projects measured phase 100000 periods into the future (extraction), includes phase corrections
#define  B2B_ECADO_B2B_DIAGINJ     2056    // command: projects measured phase 100000 periods into the future (injection), includes phase corrections

// status flags
#define  B2B_FLAG_TRANSACTIVE       0x1    // flag: transfer active
#define  B2B_FLAG_TRANSPEXT         0x2    // flag: got measured phase from extraction
#define  B2B_FLAG_TRANSPINJ         0x4    // flag: got measured phase from injection

// B2B modes
#define  B2B_MODE_NA                0x0    // N/A: do nothing
#define  B2B_MODE_B2B               0x1    // bunch to bucket transfer
#define  B2B_MODE_B2C               0x2    // bunch to coasting transfer
#define  B2B_MODE_B2E               0x3    // simple bunch extraction
#define  B2B_MODE_KSB               0x4    // EVT_KICK_START: trigger both (extraction and injection) kickers
#define  B2B_MODE_KSE               0x5    // EVT_KICK_START: trigger extraction kicker
#define  B2B_MODE_KSI               0x6    // EVT_KICK_START: trigger injection kicker

// B2B action flags
#define  B2B_ACTION_TRIGEXT         0x1    // trigger extraction
#define  B2B_ACTION_TRIGINJ         0x2    // trigger injection
#define  B2B_ACTION_PEXT            0x4    // consider phase of extraction only
#define  B2B_ACTION_PMATCH          0x8    // match phase of extraction and injection

// group IDs
#define  SIS18_B2B_EXTRACT        0x3a0    // GID: SIS18 simple extraction
#define  SIS18_B2B_ESR            0x3a1    // GID: SIS18 to ESR
#define  SIS18_B2B_SIS100         0x3a2    // GID: SIS18 to CRYRING
#define  ESR_B2B_EXTRACT          0x3a5    // GID: ESR simple extraction
#define  ESR_B2B_CRYRING          0x3a6    // GID: ESR to CRYRING
#define  CRYRING_B2B_EXTRACT      0x3a0    // GID: CRYRING simple extraction

// specialities
#define  B2B_AHEADOFFSET            500    // offset [us] used for receiving KICK_START or for sending messages
                                           // this value is very special and should be about of the value used by the DM


// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
#define B2B_SHARED_GIDEXT         (COMMON_SHARED_END        + _32b_SIZE_)       // GID of B2B Transfer (
#define B2B_SHARED_SIDEXT         (B2B_SHARED_GIDEXT        + _32b_SIZE_)       // sequence ID for B2B transfer 
#define B2B_SHARED_MODE           (B2B_SHARED_SIDEXT        + _32b_SIZE_)       // mode of B2B transfer
#define B2B_SHARED_TH1EXTHI       (B2B_SHARED_MODE          + _32b_SIZE_)       // period of h=1 extraction, high bits
#define B2B_SHARED_TH1EXTLO       (B2B_SHARED_TH1EXTHI      + _32b_SIZE_)       // period of h=1 extraction, low bits
#define B2B_SHARED_NHEXT          (B2B_SHARED_TH1EXTLO      + _32b_SIZE_)       // harmonic number of extraction RF
#define B2B_SHARED_TH1INJHI       (B2B_SHARED_NHEXT         + _32b_SIZE_)       // period of h=1 injection, high bits 
#define B2B_SHARED_TH1INJLO       (B2B_SHARED_TH1INJHI      + _32b_SIZE_)       // period of h=1 injection, low bits
#define B2B_SHARED_NHINJ          (B2B_SHARED_TH1INJLO      + _32b_SIZE_)       // harmonic number of injection RF
#define B2B_SHARED_TBEATHI        (B2B_SHARED_NHINJ         + _32b_SIZE_)       // period of beating, high bits
#define B2B_SHARED_TBEATLO        (B2B_SHARED_TBEATHI       + _32b_SIZE_)       // period of beating, low bits
#define B2B_SHARED_CPHASE         (B2B_SHARED_TBEATLO       + _32b_SIZE_)       // correction for phase matching ('phase knob') [ns]
#define B2B_SHARED_CTRIGEXT       (B2B_SHARED_CPHASE        + _32b_SIZE_)       // correction for trigger extraction ('extraction kicker knob') [ns]
#define B2B_SHARED_CTRIGINJ       (B2B_SHARED_CTRIGEXT      + _32b_SIZE_)       // correction for trigger injection ('injction kicker knob') [ns]

// diagnosis: end of used shared memory
#define B2B_SHARED_END            (B2B_SHARED_CTRIGINJ      + _32b_SIZE_) 

#endif

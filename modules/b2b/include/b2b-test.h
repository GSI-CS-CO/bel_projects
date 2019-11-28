#ifndef _B2B_TEST_
#define _B2B_TEST_

#include <b2b-common.h>

// !!!!!
// experimental: let's try the same header and DP RAM layout for ALL B2B firmwares....
// !!!!!

// this file is structured in two parts
// 1st: definitions of general things like error messages
// 2nd: definitions for data exchange via DP RAM

// ****************************************************************************************
// general things
// ****************************************************************************************

// (error) status, each status will be represented by one bit, bits will be ORed into a 32bit word
#define  B2BTEST_STATUS_PHASEFAILED      16    // phase measurement failed
#define  B2BTEST_STATUS_TRANSFER         17    // transfer failed
#define  B2BTEST_STATUS_SAFETYMARGIN     18    // violation of safety margin for data master and timing network

// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define  B2BTEST_ECADO_TIMEOUT         COMMON_ECADO_TIMEOUT
#define  B2BTEST_ECADO_UNKOWN             1    // unkown activity requested (unexpected action by ECA)
#define  B2BTEST_ECADO_TLUINPUT           2    // event from input (TLU)
#define  B2BTEST_ECADO_B2B_START       2048    // command: start B2B transfer
#define  B2BTEST_ECADO_B2B_PMEXT       2049    // command: perform phase measurement (extraction)
#define  B2BTEST_ECADO_B2B_PMINJ       2050    // command: perform phase measurement (injection)
#define  B2BTEST_ECADO_B2B_PREXT       2051    // command: result of phase measurement (extraction)
#define  B2BTEST_ECADO_B2B_PRINJ       2052    // command: result of phase measurement (injecton)
#define  B2BTEST_ECADO_B2B_DIAGEXT     2053    // command: projects measured phase 100000 periods into the future (extraction)
#define  B2BTEST_ECADO_B2B_DIAGINJ     2054    // command: projects measured phase 100000 periods into the future (extraction)
#define  B2BTEST_ECADO_B2B_DIAGMATCH   2055    // command: time, when h=1 phases of extraction and injection will match

#define  B2BTEST_FLAG_TRANSACTIVE       0x1    // flag: transfer active
#define  B2BTEST_FLAG_TRANSPEXT         0x2    // flag: got measured phase from extraction
#define  B2BTEST_FLAG_TRANSPINJ         0x4    // flag: got measured phase from injection

// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
#define B2BTEST_SHARED_TH1EXTHI       (COMMON_SHARED_END            + _32b_SIZE_)       // period of h=1 extraction, high bits
#define B2BTEST_SHARED_TH1EXTLO       (B2BTEST_SHARED_TH1EXTHI      + _32b_SIZE_)       // period of h=1 extraction, low bits
#define B2BTEST_SHARED_NHEXT          (B2BTEST_SHARED_TH1EXTLO      + _32b_SIZE_)       // harmonic number of extraction RF
#define B2BTEST_SHARED_TH1INJHI       (B2BTEST_SHARED_NHEXT         + _32b_SIZE_)       // period of h=1 injection, high bits 
#define B2BTEST_SHARED_TH1INJLO       (B2BTEST_SHARED_TH1INJHI      + _32b_SIZE_)       // period of h=1 injection, low bits
#define B2BTEST_SHARED_NHINJ          (B2BTEST_SHARED_TH1INJLO      + _32b_SIZE_)       // harmonic number of injection RF
#define B2BTEST_SHARED_TBEATHI        (B2BTEST_SHARED_NHINJ         + _32b_SIZE_)       // period of beating, high bits
#define B2BTEST_SHARED_TBEATLO        (B2BTEST_SHARED_TBEATHI       + _32b_SIZE_)       // period of beating, low bits
#define B2BTEST_SHARED_INTCALIB       (B2BTEST_SHARED_TBEATLO       + _32b_SIZE_)       // internal calibration, value is added to published B2B_DIAGMATCH
#define B2BTEST_SHARED_EXTCALIB       (B2BTEST_SHARED_INTCALIB      + _32b_SIZE_)       // calibration of extraction, value will be added to received B2B_PREXT
#define B2BTEST_SHARED_INJCALIB       (B2BTEST_SHARED_EXTCALIB      + _32b_SIZE_)       // calibration of injection, value will be added to received B2B_PRINJ

// diagnosis: end of used shared memory
#define B2BTEST_SHARED_END            (B2BTEST_SHARED_INJCALIB      + _32b_SIZE_) 

#endif

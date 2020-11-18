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
#define  B2B_ECADO_UNKOWN              1   // unkown activity requested (unexpected action by ECA)
#define  B2B_ECADO_TLUINPUT            2   // event from input (TLU)
#define  B2B_ECADO_KICKSTART        0x31   // SIS18 extraction: EVT_KICK_START1; ESR extraction: EVT_KICK_START2
#define  B2B_ECADO_B2B_PMEXT       0x800   // command: perform phase measurement (extraction)
#define  B2B_ECADO_B2B_PMINJ       0x801   // command: perform phase measurement (injection)
#define  B2B_ECADO_B2B_PREXT       0x802   // command: result of phase measurement (extraction)
#define  B2B_ECADO_B2B_PRINJ       0x803   // command: result of phase measurement (injecton)
#define  B2B_ECADO_B2B_TRIGGEREXT  0x804   // command: trigger kicker (extraction)
#define  B2B_ECADO_B2B_TRIGGERINJ  0x805   // command: trigger kicker (injection)
#define  B2B_ECADO_B2B_DIAGMATCH   0x806   // command: optional diagnostic, indicates when phases match
#define  B2B_ECADO_B2B_DIAGEXT     0x807   // command: optional diagnostic (extraction)
#define  B2B_ECADO_B2B_DIAGINJ     0x808   // command: optional diagnostic (injection)
#define  B2B_ECADO_B2B_DIAGKICKEXT 0x809   // command: optional kick diagnostic (extraction)
#define  B2B_ECADO_B2B_DIAGKICKINJ 0x80a   // command: optional kick diagnostic (injection)

// status flags
//#define  B2B_FLAG_TRANSACTIVE       0x1    // flag: transfer active
//#define  B2B_FLAG_TRANSPEXTS        0x2    // flag: phase measurement extraction, request has been sent
//#define  B2B_FLAG_TRANSPEXTR        0x4    // flag: phase measurement extraction, data has been received
//#define  B2B_FLAG_TRANSPINJS        0x8    // flag: phase measurement injection, request has been sent
//#define  B2B_FLAG_TRANSPINJR        0x10   // flag: phase measurement injection, data has been received

// B2B mode flags                          //                                            | ext trig | ext phase | inj trig | inj phase |
#define  B2B_MODE_KSE                 1    // EVT_KICK_START: trigger extraction kicker  |     x    |           |          |           |
#define  B2B_MODE_B2E                 2    // simple bunch extraction                    |     x    |     x     |          |           | 
#define  B2B_MODE_B2C                 3    // bunch to coasting transfer                 |     x    |     x     |    x     |           | 
#define  B2B_MODE_B2B                 4    // bunch to bucket transfer                   |     x    |     x     |    x     |     x     |

// B2B todo flags
#define B2B_TODO_NOTHING            0x0    // nothing to do
#define B2B_TODO_EXTPS              0x1    // phase measurement extraction, send request to PM
#define B2B_TODO_EXTPR              0x2    // phase measurement extraction, receive data from PM
#define B2B_TODO_EXTKST             0x4    // calculate time for immediate extraction at EVT_KICK_START
#define B2B_TODO_EXTBGT             0x8    // calculate time for extraction at next bunch gap
#define B2B_TODO_EXTMATCHT         0x10    // calculate time for phase matching 
#define B2B_TODO_EXTTRIG           0x20    // trigger extraction kicker
#define B2B_TODO_INJPS             0x40    // phase measurement injection, send request to PM
#define B2B_TODO_INJPR             0x80    // phase measurement injection, receive data from PM
#define B2B_TODO_INJTRIG          0x100    // trigger injection kicker

// B2B action flags
//#define  B2B_ACTION_TRIGEXT         0x1    // trigger extraction
//#define  B2B_ACTION_PEXT            0x2    // consider phase of extraction
//#define  B2B_ACTION_TRIGINJ         0x4    // trigger injection
//#define  B2B_ACTION_PINJ            0x8    // consider phase of injection (= matching/alignment)

// group IDs
#define  GID_INVALID                0x0    // invalid GID
#define  SIS18_RING               0x12c    // LSA GID
#define  SIS18_B2B_EXTRACT        0x3a0    // GID: SIS18 simple extraction
#define  SIS18_B2B_ESR            0x3a1    // GID: SIS18 to ESR
#define  SIS18_B2B_SIS100         0x3a2    // GID: SIS18 to CRYRING
#define  ESR_B2B_EXTRACT          0x3a5    // GID: ESR simple extraction
#define  ESR_B2B_CRYRING          0x3a6    // GID: ESR to CRYRING
#define  CRYRING_B2B_EXTRACT      0x3aa    // GID: CRYRING simple extraction
#define  SIS100_B2B_EXTRACT       0x3b0    // GID: CRYRING simple extraction

// specialities
#define  B2B_DMOFFSET           1000000    // offset [ns] used for receiving KICK_START or for sending messages
                                           // this value is very special and should equal the value used by the DM


// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
#define B2B_SHARED_GID            (COMMON_SHARED_END        + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define B2B_SHARED_SID            (B2B_SHARED_GID           + _32b_SIZE_)       // sequence ID for B2B transfer 
#define B2B_SHARED_MODE           (B2B_SHARED_SID           + _32b_SIZE_)       // mode of B2B transfer
#define B2B_SHARED_TH1EXTHI       (B2B_SHARED_MODE          + _32b_SIZE_)       // period [as] of h=1 extraction, high bits
#define B2B_SHARED_TH1EXTLO       (B2B_SHARED_TH1EXTHI      + _32b_SIZE_)       // period of h=1 extraction, low bits
#define B2B_SHARED_NHEXT          (B2B_SHARED_TH1EXTLO      + _32b_SIZE_)       // harmonic number of extraction RF
#define B2B_SHARED_TH1INJHI       (B2B_SHARED_NHEXT         + _32b_SIZE_)       // period [as] of h=1 injection, high bits
#define B2B_SHARED_TH1INJLO       (B2B_SHARED_TH1INJHI      + _32b_SIZE_)       // period of h=1 injection, low bits
#define B2B_SHARED_NHINJ          (B2B_SHARED_TH1INJLO      + _32b_SIZE_)       // harmonic number of injection RF
#define B2B_SHARED_TBEATHI        (B2B_SHARED_NHINJ         + _32b_SIZE_)       // period of beating, high bits
#define B2B_SHARED_TBEATLO        (B2B_SHARED_TBEATHI       + _32b_SIZE_)       // period of beating, low bits
#define B2B_SHARED_CPHASE         (B2B_SHARED_TBEATLO       + _32b_SIZE_)       // correction for phase matching ('phase knob') [ns]
#define B2B_SHARED_CTRIGEXT       (B2B_SHARED_CPHASE        + _32b_SIZE_)       // correction for trigger extraction ('extraction kicker knob') [ns]
#define B2B_SHARED_CTRIGINJ       (B2B_SHARED_CTRIGEXT      + _32b_SIZE_)       // correction for trigger injection ('injction kicker knob') [ns]
#define B2B_SHARED_DMLATENCY      (B2B_SHARED_CTRIGINJ      + _32b_SIZE_)       // latency for messages received from DM (prio Q + network) [ns]

// diagnosis: end of used shared memory
#define B2B_SHARED_END            (B2B_SHARED_DMLATENCY     + _32b_SIZE_) 

#endif

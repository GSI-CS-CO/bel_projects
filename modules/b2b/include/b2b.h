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
#define B2B_STATUS_PHASEFAILED        16   // phase measurement failed
#define B2B_STATUS_TRANSFER           17   // transfer failed
#define B2B_STATUS_SAFETYMARGIN       18   // violation of safety margin for data master and timing network
#define B2B_STATUS_NORF               19   // no RF signal detected
#define B2B_STATUS_LATEMESSAGE        20   // late timing message received
#define B2B_STATUS_NOKICK             21   // no kicker signal detected
#define B2B_STATUS_BADSETTING         22   // bad setting data


// activity requested by ECA Handler, the relevant codes are also used as "tags"
#define B2B_ECADO_TIMEOUT    COMMON_ECADO_TIMEOUT
#define B2B_ECADO_UNKOWN               1   // unkown activity requested (unexpected action by ECA)
#define B2B_ECADO_TLUINPUT1        0xa01   // event from IO1 (TLU)
#define B2B_ECADO_TLUINPUT2        0xa02   // event from IO2 (TLU)
#define B2B_ECADO_TLUINPUT3        0xa03   // event from IO3 (TLU)
#define B2B_ECADO_TLUINPUT4        0xa04   // event from IO4 (TLU)
#define B2B_ECADO_KICKSTART1        0x31   // SIS18 extraction: EVT_KICK_START1; ESR extraction: EVT_KICK_START2
#define B2B_ECADO_KICKSTART2        0x45   // SIS18 extraction: EVT_KICK_START1; ESR extraction: EVT_KICK_START2
#define B2B_ECADO_B2B_PMEXT        0x800   // command: perform phase measurement (extraction)
#define B2B_ECADO_B2B_PMINJ        0x801   // command: perform phase measurement (injection)
#define B2B_ECADO_B2B_PREXT        0x802   // command: result of phase measurement (extraction)
#define B2B_ECADO_B2B_PRINJ        0x803   // command: result of phase measurement (injection)
#define B2B_ECADO_B2B_TRIGGEREXT   0x804   // command: trigger kicker (extraction)
#define B2B_ECADO_B2B_TRIGGERINJ   0x805   // command: trigger kicker (injection)
#define B2B_ECADO_B2B_DIAGKICKEXT  0x806   // command: kick diagnostic (extraction)
#define B2B_ECADO_B2B_DIAGKICKINJ  0x807   // command: kick diagnostic (injection)
#define B2B_ECADO_B2B_DIAGEXT      0x808   // command: result of diagnostic (extraction)
#define B2B_ECADO_B2B_DIAGINJ      0x809   // command: result of diagnostic (injection)
#define B2B_ECADO_B2B_START        0x81f   // command: start b2b procedure
#define B2B_ECADO_B2B_PDEXT        0x820   // internal command: perform phase diagnostic (extraction)
#define B2B_ECADO_B2B_PDINJ        0x821   // internal command: perform phase diagnostic (injection)

// commands from the outside
#define B2B_CMD_CONFSUBMIT            11   // submit data written to DP RAM
#define B2B_CMD_CONFCLEAR             12   // this will clear all event tables

// B2B error flags (in EvtId)
#define B2B_ERRFLAG_PMEXT            0x1   // error phase measurement extraction
#define B2B_ERRFLAG_KDEXT            0x2   // error kick diagnostic extraction
#define B2B_ERRFLAG_PMINJ            0x4   // error phase measurement injection
#define B2B_ERRFLAG_KDINJ            0x8   // error kick diagnostic injection
#define B2B_ERRFLAG_CBU             0x10   // error central b2b unit

// B2B mode flags                          //                                            | ext trig | ext phase | inj trig | inj phase |
#define B2B_MODE_BSE                   1   // CMD_B2B_START: trigger extraction kicker   |     x    |           |          |           |
#define B2B_MODE_B2E                   2   // simple bunch extraction 'fast extraction'  |     x    |     x     |          |           | 
#define B2B_MODE_B2C                   3   // bunch to coasting transfer                 |     x    |     x     |    x     |           | 
#define B2B_MODE_B2B                   4   // bunch to bucket transfer                   |     x    |     x     |    x     |     x     |

// B2B states of 'miniFSM'
#define B2B_MFSM_S0                  0x1   // start state
#define B2B_MFSM_EXTPS               0x2   // phase measurement extraction, send request to PM
#define B2B_MFSM_EXTPR               0x4   // phase measurement extraction, receive data from PM
#define B2B_MFSM_EXTKICK             0x8   // calculate time for earliest kick (~ EVT_KICK_START)
#define B2B_MFSM_EXTBGT             0x10   // calculate time for extraction at next bunch gap
#define B2B_MFSM_EXTMATCHT          0x20   // calculate time for phase matching 
#define B2B_MFSM_EXTTRIG            0x40   // trigger extraction kicker
#define B2B_MFSM_INJPS              0x80   // phase measurement injection, send request to PM
#define B2B_MFSM_BOTHPR            0x100   // phase measurement of extraction and injection, receive data from PM
#define B2B_MFSM_INJTRIG           0x200   // trigger injection kicker
#define B2B_MFSM_NOTHING        0x100000   // nothing to do

// group IDs
#define GID_INVALID                  0x0   // invalid GID
#define SIS18_RING                 0x12c   // LSA GID
#define ESR_RING                   0x154   // LSA GID
#define CRYRING_RING               0x0d2   // LSA GID
#define SIS18_B2B_EXTRACT          0x3a0   // GID: SIS18 simple extraction
#define SIS18_B2B_ESR              0x3a1   // GID: SIS18 to ESR
#define SIS18_B2B_SIS100           0x3a2   // GID: SIS18 to CRYRING
#define ESR_B2B_EXTRACT            0x3a5   // GID: ESR simple extraction
#define ESR_B2B_CRYRING            0x3a6   // GID: ESR to CRYRING
#define CRYRING_B2B_EXTRACT        0x3aa   // GID: CRYRING simple extraction
#define SIS100_B2B_EXTRACT         0x3b0   // GID: SIS100 simple extraction

// specialities
#define B2B_PMOFFSET              500000    // offset [ns] for deadline of PMEXT/PMINJ events relative to B2BS event
#define B2B_KICKOFFSET           2000000    // offset [ns] for earliest deadline of kicker trigger events
#define B2B_PRETRIGGERPR          250000    // offset [ns] used as pre-trigger on the PRINJ/PREXT event
#define B2B_PRETRIGGERTR           20000    // offset [ns] used as pre-trigger on the trigger event
#define B2B_ACCEPTKMON             10000    // timewindow [us]!!! in which monitor signal  from kicker electronics is expected
#define B2B_ACCEPTKPROBE             100    // timewindow [us]!!! in which signals from kicker magnet probe are expected
#define B2B_NSID                      16    // max number of SID settings
#define B2B_F_CLK              200000000    // clock for DDS, here: BuTiS 200 MHz


// ****************************************************************************************
// DP RAM
// ****************************************************************************************

// offsets
// set values for data supply (extraction ring)
#define B2B_SHARED_SET_SIDEEXT    (COMMON_SHARED_END         + _32b_SIZE_)       // sequence ID for B2B transfer at extraction
#define B2B_SHARED_SET_GIDEXT     (B2B_SHARED_SET_SIDEEXT    + _32b_SIZE_)       // b2b GID of extraction ring
#define B2B_SHARED_SET_MODE       (B2B_SHARED_SET_GIDEXT     + _32b_SIZE_)       // mode of B2B transfer
#define B2B_SHARED_SET_TH1EXTHI   (B2B_SHARED_SET_MODE       + _32b_SIZE_)       // period [as] of h=1 extraction, high bits
#define B2B_SHARED_SET_TH1EXTLO   (B2B_SHARED_SET_TH1EXTHI   + _32b_SIZE_)       // period of h=1 extraction, low bits
#define B2B_SHARED_SET_NHEXT      (B2B_SHARED_SET_TH1EXTLO   + _32b_SIZE_)       // harmonic number of extraction RF
#define B2B_SHARED_SET_CTRIGEXT   (B2B_SHARED_SET_NHEXT      + _32b_SIZE_)       // correction for trigger extraction ('extraction kicker knob') [ns]
#define B2B_SHARED_SET_NBUCKEXT   (B2B_SHARED_SET_CTRIGEXT   + _32b_SIZE_)       // bucket number of extraction
#define B2B_SHARED_SET_CPHASE     (B2B_SHARED_SET_NBUCKEXT   + _32b_SIZE_)       // correction for phase matching ('phase knob') [ns]
#define B2B_SHARED_SET_FFINTUNE   (B2B_SHARED_SET_CPHASE     + _32b_SIZE_)       // flag: use fine tune
#define B2B_SHARED_SET_FMBTUNE    (B2B_SHARED_SET_FFINTUNE   + _32b_SIZE_)       // flag: use multi-beat tune

// set values for data supply (injection ring)
#define B2B_SHARED_SET_SIDEINJ    (B2B_SHARED_SET_FMBTUNE    + _32b_SIZE_)       // sequence ID for B2B transfer at extraction (!) ring (required for joining the data)
#define B2B_SHARED_SET_GIDINJ     (B2B_SHARED_SET_SIDEINJ    + _32b_SIZE_)       // b2b GID offset of injection ring
#define B2B_SHARED_SET_TH1INJHI   (B2B_SHARED_SET_GIDINJ     + _32b_SIZE_)       // period [as] of h=1 injection, high bits
#define B2B_SHARED_SET_TH1INJLO   (B2B_SHARED_SET_TH1INJHI   + _32b_SIZE_)       // period of h=1 injection, low bits
#define B2B_SHARED_SET_NHINJ      (B2B_SHARED_SET_TH1INJLO   + _32b_SIZE_)       // harmonic number of injection RF
#define B2B_SHARED_SET_CTRIGINJ   (B2B_SHARED_SET_NHINJ      + _32b_SIZE_)       // correction for trigger injection ('injction kicker knob') [ns]
#define B2B_SHARED_SET_NBUCKINJ   (B2B_SHARED_SET_CTRIGINJ   + _32b_SIZE_)       // bucket number of injection

//get values
#define B2B_SHARED_GET_SID        (B2B_SHARED_SET_NBUCKINJ   + _32b_SIZE_)       // sequence ID for B2B transfer 
#define B2B_SHARED_GET_GID        (B2B_SHARED_GET_SID        + _32b_SIZE_)       // GID of B2B Transfer ('EXTRING_B2B_...')
#define B2B_SHARED_GET_MODE       (B2B_SHARED_GET_GID        + _32b_SIZE_)       // mode of B2B transfer
#define B2B_SHARED_GET_TH1EXTHI   (B2B_SHARED_GET_MODE       + _32b_SIZE_)       // period [as] of h=1 extraction, high bits
#define B2B_SHARED_GET_TH1EXTLO   (B2B_SHARED_GET_TH1EXTHI   + _32b_SIZE_)       // period of h=1 extraction, low bits
#define B2B_SHARED_GET_NHEXT      (B2B_SHARED_GET_TH1EXTLO   + _32b_SIZE_)       // harmonic number of extraction RF
#define B2B_SHARED_GET_TH1INJHI   (B2B_SHARED_GET_NHEXT      + _32b_SIZE_)       // period [as] of h=1 injection, high bits
#define B2B_SHARED_GET_TH1INJLO   (B2B_SHARED_GET_TH1INJHI   + _32b_SIZE_)       // period of h=1 injection, low bits
#define B2B_SHARED_GET_NHINJ      (B2B_SHARED_GET_TH1INJLO   + _32b_SIZE_)       // harmonic number of injection RF
#define B2B_SHARED_GET_CPHASE     (B2B_SHARED_GET_NHINJ      + _32b_SIZE_)       // correction for phase matching ('phase knob') [ns]
#define B2B_SHARED_GET_CTRIGEXT   (B2B_SHARED_GET_CPHASE     + _32b_SIZE_)       // correction for trigger extraction ('extraction kicker knob') [ns]
#define B2B_SHARED_GET_CTRIGINJ   (B2B_SHARED_GET_CTRIGEXT   + _32b_SIZE_)       // correction for trigger injection ('injction kicker knob') [ns]
#define B2B_SHARED_GET_TBEATHI    (B2B_SHARED_GET_CTRIGINJ   + _32b_SIZE_)       // period of beating, high bits
#define B2B_SHARED_GET_TBEATLO    (B2B_SHARED_GET_TBEATHI    + _32b_SIZE_)       // period of beating, low bits
#define B2B_SHARED_GET_COMLATENCY (B2B_SHARED_GET_TBEATLO    + _32b_SIZE_)       // latency for messages received from via ECA (tDealine - tNow)) [ns]
#define B2B_SHARED_GET_TKTRIGHI   (B2B_SHARED_GET_COMLATENCY + _32b_SIZE_)       // time of kicker trigger signal, high bits [ns]
#define B2B_SHARED_GET_TKTRIGLO   (B2B_SHARED_GET_TKTRIGHI   + _32b_SIZE_)       // time of kicker trigger signal, low bits [ns]
#define B2B_SHARED_GET_DKMON      (B2B_SHARED_GET_TKTRIGLO   + _32b_SIZE_)       // delay of kicker monitor signal [ns], delay is measured from kicker trigger signal
#define B2B_SHARED_GET_DKPROBE    (B2B_SHARED_GET_DKMON      + _32b_SIZE_)       // delay of kicker probe signal [ns], delay is measured from kicker trigger signal

// diagnosis: end of used shared memory
#define B2B_SHARED_END            (B2B_SHARED_GET_DKPROBE    + _32b_SIZE_) 

#endif

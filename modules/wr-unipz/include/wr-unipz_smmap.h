#ifndef _WRUNIPZ_REGS_
#define _WRUNIPZ_REGS_

#include "wrunipz_shared_mmap.h"

// sizes
#define _32b_SIZE_                    4                                                 // size of 32bit value [bytes]
#define WRUNIPZ_DATA4EBSIZE          (_32b_SIZE_ * 20)                                  // size of shared memory used to receive EB return values [bytes]
#define WRUNIPZ_NVACC                 16                                                // #  vAcc
#define WRUNIPZ_NSET                  2                                                 // # of sets for each vAcc ("normal", "verkuerzt")
#define WRUNIPZ_NPZ                   7                                                 // # of Pulszentralen
#define WRUNIPZ_NFLAG                 4                                                 // # flags per virtAcc
#define WRUNIPZ_NCONFDATA            (WRUNIPZ_NEVT  * WRUNIPZ_NPZ * WRUNIPZ_NSET)       // # of config data words for one virtual accelerator
#define WRUNIPZ_NCONFFLAG            (WRUNIPZ_NFLAG * WRUNIPZ_NPZ * WRUNIPZ_NSET)       // # of config flag words for one virtual accelerator

// offsets
// simple values
#define WRUNIPZ_SHARED_STATUS         0x0                                               // error status                       
#define WRUNIPZ_SHARED_CMD            (WRUNIPZ_SHARED_STATUS     + _32b_SIZE_)          // input of 32bit command
#define WRUNIPZ_SHARED_STATE          (WRUNIPZ_SHARED_CMD        + _32b_SIZE_)          // state of state machine
#define WRUNIPZ_SHARED_TCYCLEAVG      (WRUNIPZ_SHARED_STATE      + _32b_SIZE_)          // period of UNILAC cycleft [us] (average over one second)
#define WRUNIPZ_SHARED_VERSION        (WRUNIPZ_SHARED_TCYCLEAVG  + _32b_SIZE_)          // version of firmware
#define WRUNIPZ_SHARED_MACHI          (WRUNIPZ_SHARED_VERSION    + _32b_SIZE_)          // WR MAC of wrunipz, bits 31..16 unused
#define WRUNIPZ_SHARED_MACLO          (WRUNIPZ_SHARED_MACHI      + _32b_SIZE_)          // WR MAC of wrunipz
#define WRUNIPZ_SHARED_IP             (WRUNIPZ_SHARED_MACLO      + _32b_SIZE_)          // IP of wrunipz
#define WRUNIPZ_SHARED_NBADSTATUS     (WRUNIPZ_SHARED_IP         + _32b_SIZE_)          // # of bad status (=error) incidents
#define WRUNIPZ_SHARED_NBADSTATE      (WRUNIPZ_SHARED_NBADSTATUS + _32b_SIZE_)          // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define WRUNIPZ_SHARED_NCYCLE         (WRUNIPZ_SHARED_NBADSTATE  + _32b_SIZE_)          // # of UNILAC cycles
#define WRUNIPZ_SHARED_NMESSAGEHI     (WRUNIPZ_SHARED_NCYCLE     + _32b_SIZE_)          // # of messsages, high bits
#define WRUNIPZ_SHARED_NMESSAGELO     (WRUNIPZ_SHARED_NMESSAGEHI + _32b_SIZE_)          // # of messsages, low bits
#define WRUNIPZ_SHARED_MSGFREQAVG     (WRUNIPZ_SHARED_NMESSAGELO + _32b_SIZE_)          // message rate (average over one second)
#define WRUNIPZ_SHARED_DTAVG          (WRUNIPZ_SHARED_MSGFREQAVG + _32b_SIZE_)          // delta T between message time of dispatching and deadline
#define WRUNIPZ_SHARED_DTMAX          (WRUNIPZ_SHARED_DTAVG      + _32b_SIZE_)          // delta T max
#define WRUNIPZ_SHARED_DTMIN          (WRUNIPZ_SHARED_DTMAX      + _32b_SIZE_)          // delta T min

// shared memory for EB return values
#define WRUNIPZ_SHARED_DATA_4EB       (WRUNIPZ_SHARED_DTMIN      + _32b_SIZE_)   

// shared memory for submitting new 'event tables'                                      
#define WRUNIPZ_SHARED_CONF_VACC      (WRUNIPZ_SHARED_DATA_4EB   + WRUNIPZ_DATA4EBSIZE) // vACC for config data
#define WRUNIPZ_SHARED_CONF_STAT      (WRUNIPZ_SHARED_CONF_VACC  + _32b_SIZE_)          // status of config transaction
// config submit flag layout: least significant bit is PZ0; '1': data shall be submitted
#define WRUNIPZ_SHARED_CONF_SUBMIT    (WRUNIPZ_SHARED_CONF_STAT  + _32b_SIZE_)
// config data layout
// ==================
// note: all config data is valid for the SAME virtual accelerator defined in WRUNIPZ_SHARED_CONF_VACC
// (there are 32 words per virtual accelerator for "normal" and another 32 words for "verkuerzt" operation)
// [norm0 of PZ0]..[norm31 of PZ0][kurz0 of PZ0]..[kurz31 of PZ0][norm0 of PZ1].....[kurz31 of PZ6] 
#define WRUNIPZ_SHARED_CONF_DATA      (WRUNIPZ_SHARED_CONF_SUBMIT + _32b_SIZE_) 
// config flag layout
// ==================
// (there are 4 words per virtual accelerator for "normal" and another 4 words for "verkuerzt" operation)
// [norm0 of PZ0]..[norm3 of PZ0][kurz0 of PZ0]..[kurz3 of PZ0][norm0 of PZ1].....[kurz32 of PZ6] 
#define WRUNIPZ_SHARED_CONF_FLAG      (WRUNIPZ_SHARED_CONF_DATA   + (WRUNIPZ_NCONFDATA << 2))  

// diagnosis
#define WRUNIPZ_SHARED_SIZEUSED       (WRUNIPZ_SHARED_CONF_FLAG   + _32b_SIZE_) // used size of shared area

#endif

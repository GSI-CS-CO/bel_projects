#ifndef _WRUNIPZ__REGS_
#define _WRUNIPZ_REGS_

#include "wrunipz_shared_mmap.h"

#define WRUNIPZ_SHARED_DATA_4EB_SIZE  68     // size of shared memory used to receive EB return values; size is in bytes

/* the following values must be added to the offset of the shared memory */
#define WRUNIPZ_SHARED_STATUS         0x00   // error status                       
#define WRUNIPZ_SHARED_CMD            0x04   // input of 32bit command
#define WRUNIPZ_SHARED_STATE          0x08   // state of state machine
#define WRUNIPZ_SHARED_TCYCLE         0x0C   // perdiod of UNILAC cycle [us]
#define WRUNIPZ_SHARED_VERSION        0x10   // version of firmware
#define WRUNIPZ_SHARED_MACHI          0x14   // WR MAC of wrunipz, bits 31..16 unused
#define WRUNIPZ_SHARED_MACLO          0x18   // WR MAC of wrunipz
#define WRUNIPZ_SHARED_IP             0x20   // IP of wrunipz
#define WRUNIPZ_SHARED_NBADSTATUS     0x24   // # of bad status (=error) incidents
#define WRUNIPZ_SHARED_NBADSTATE      0x28   // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
#define WRUNIPZ_SHARED_NCYCLE         0x2C   // # of UNILAC cycles
#define WRUNIPZ_SHARED_NMESSAGEHI     0x30   // # of messsages, high bits
#define WRUNIPZ_SHARED_NMESSAGELO     0x34   // # of messsages, low bits


// 0x8a-0x8f: reserved
#define WRUNIPZ_SHARED_DATA_4EB_START 0x90    // start of shared memory for EB return values
#define WRUNIPZ_SHARED_DATA_4EB_END   WRUNIPZ_SHARED_DATA_4EB_START + WRUNIPZ_SHARED_DATA_4EB_SIZE // end of shared memory area for EB return values

#endif

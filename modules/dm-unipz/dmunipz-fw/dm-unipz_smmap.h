#ifndef _DMUNIPZ__REGS_
#define _DMUNIPZ_REGS_

#include "dmunipz_shared_mmap.h"

#define DMUNIPZ_SHARED_DATA_4EB_SIZE  40     // size of shared memory used to receive EB return values; size is in bytes

/* the following values must be added to the offset of the shared memory */
#define DMUNIPZ_SHARED_STATUS         0x00   // error status                       
#define DMUNIPZ_SHARED_CMD            0x04   // input of 32bit command
#define DMUNIPZ_SHARED_STATE          0x08   // state of state machine
#define DMUNIPZ_SHARED_NITERMAIN      0x0C   // # of iterations of main loop
#define DMUNIPZ_SHARED_VERSION        0x10   // version of firmware
#define DMUNIPZ_SHARED_SRCMAC         0x14   // WR MAC of dmunipz, bits 63..48 unused
#define DMUNIPZ_SHARED_SRCIP          0x1C   // IP of dmunipz
#define DMUNIPZ_SHARED_DSTMAC         0x20   // WR MAC of data master, bits 63..48 unused
#define DMUNIPZ_SHARED_DSTIP          0x28   // IP of data master
// 0x2C-0x2F: reserved
#define DMUNIPZ_SHARED_TRANSN         0x30   // # N of transfers
#define DMUNIPZ_SHARED_INJECTN        0x34   // # N of injections (of current transfer)
#define DMUNIPZ_SHARED_TRANSVIRTACC   0x38   // # requested virtual accelerator 0..F
#define DMUNIPZ_SHARED_TRANSSTATUS    0x3C   // # status of transfer
// 0x40-0x4F: reserved
#define DMUNIPZ_SHARED_DATA_4EB_START 0x50    // start of shared memory for EB return values
#define DMUNIPZ_SHARED_DATA_4EB_END   DMUNIPZ_SHARED_DATA_4EB_START + DMUNIPZ_SHARED_DATA_4EB_SIZE // end of shared memory area for EB return values

#endif

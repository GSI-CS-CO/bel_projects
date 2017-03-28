#ifndef _DMUNIPZ__REGS_
#define _DMUNIPZ_REGS_

#include "dmunipz_shared_mmap.h"

#define DMUNIPZ_SHARED_DATA_4EB_SIZE  40     // size of shared memory used to receive EB return values; size is in bytes

/* the following values must be added to the offset of the shared memory */
#define DMUNIPZ_SHARED_STATUS         0x00   // status                       
#define DMUNIPZ_SHARED_CMD            0x04   // input of 32bit command
#define DMUNIPZ_SHARED_NITERMAIN      0x08   // # of iterations of main loop
// 0x0C-0x2F: reserved
#define DMUNIPZ_SHARED_NRECMILEVT     0x30   // # of received MIL Events
// 0x34-0x4F: reserved
#define DMUNIPZ_SHARED_DATA_4EB_START 0x50    // start of shared memory for EB return values
#define DMUNIPZ_SHARED_DATA_4EB_END   DMUNIPZ_SHARED_DATA_4EB_START + DMUNIPZ_SHARED_DATA_4EB_SIZE // end of shared memory area for EB return values

#endif

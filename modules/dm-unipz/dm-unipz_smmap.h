#ifndef _DMUNIPZ__REGS_
#define _DMUNIPZ_REGS_

/* the following values must be added to the offset of the shared memory */
#define DMUNIPZ_SHARED_STATUS    0x0    /* status                        */
#define DMUNIPZ_SHARED_CMD       0x4    /* input of 32bit command        */
#define DMUNIPZ_SHARED_DATA1     0x8    /* 32bits for data exchange      */
#define DMUNIPZ_SHARED_DATA2     0xC    /* 32bits for data exchange      */
#define DMUNIPZ_SHARED_DATA3     0x10   /* 32bits for data exchange      */
#define DMUNIPZ_SHARED_DATA4     0x14   /* 32bits for data exchange      */

#endif

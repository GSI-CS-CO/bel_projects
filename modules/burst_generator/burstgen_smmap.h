#ifndef _SHARED_MEM_OFFSETS_
#define _SHARED_MEM_OFFSETS_

#include "burstgen_shared_mmap.h"

// the following values must be added to the offset of the shared memory
#define SHARED_COUNTER   0x0    // publish iteration counter
#define SHARED_INPUT     0x4    // input of 32bit data
#define SHARED_CMD       0x8    // input of 32bit command

#endif

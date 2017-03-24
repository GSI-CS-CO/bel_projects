#ifndef _EXAMPLE_REGS_
#define _EXAMPLE_REGS_

#include "example_shared_mmap.h"

// the following values must be added to the offset of the shared memory
#define EXAMPLE_SHARED_COUNTER   0x0    // publish iteration counter
#define EXAMPLE_SHARED_INPUT     0x4    // input of 32bit data
#define EXAMPLE_SHARED_CMD       0x8    // input of 32bit command

#endif

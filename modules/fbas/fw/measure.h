#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <stdint.h>

#include "dbg.h"
#include "fbas.h"

#define _64b_SIZE  8

void storeTimestamp(uint32_t* reg, uint32_t offset, uint64_t ts);
int64_t getElapsedTime(uint32_t* reg, uint32_t offset, uint64_t now);
void storeTsMeasureDelays(uint32_t* base, uint32_t offset, uint64_t tsEca, uint64_t tsTx);
void measureDelays(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca);

#endif

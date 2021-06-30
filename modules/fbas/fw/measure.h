#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"
#include "fbas.h"

#define _64b_SIZE  8

typedef struct msrCnt msrCnt_t;
struct msrCnt {
  uint32_t val;    // counter value
};

void storeTimestamp(uint32_t* reg, uint32_t offset, uint64_t ts);
int64_t getElapsedTime(uint32_t* reg, uint32_t offset, uint64_t now);
void storeTsMeasureDelays(uint32_t* base, uint32_t offset, uint64_t tsEca, uint64_t tsTx);
void measureDelays(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca);
/**
 * \brief count events
 *
 * \param enable indicates if the counter is incremented (=true) or initialized with a given value
 * \param value  used to increment/initialize the counter
 *
 * \ret counter value
 **/
uint32_t doCnt(bool enable, uint32_t value);

#endif

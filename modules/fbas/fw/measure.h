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

typedef struct msrPerfStats msrPerfStats_t;
struct msrPerfStats {
  uint64_t avgNwDelay;    // cumulative moving average network delay
  uint64_t avgSgLatency;  // cumulative moving average signal latency
  int64_t minNwDelay;     // minimum network delay
  int64_t minSgLatency;   // minimum signal latency
  uint64_t maxNwDelay;    // maximum network delay
  uint64_t maxSgLatency;  // maximum signal latency
  uint32_t cntNwDelay;    // number of delay measurement
  uint32_t cntSgLatency;  // number of latency measurement
  uint32_t cntTotal;      // total number of measurement
};

typedef struct msrOwDelay msrOwDelay_t;
struct msrOwDelay {
  uint64_t avg;
  int64_t  min;
  uint64_t max;
  uint32_t cntValid;
  uint32_t cntTotal;
};

void storeTimestamp(uint32_t* reg, uint32_t offset, uint64_t ts);
int64_t getElapsedTime(uint32_t* reg, uint32_t offset, uint64_t now);
void storeTsMeasureDelays(uint32_t* base, uint32_t offset, uint64_t tsEca, uint64_t tsTx);
void measureNwPerf(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca, bool verbose);
void printMeasureNwDelay(uint32_t* base, uint32_t offset);
void printMeasureSgLatency(uint32_t* base, uint32_t offset);
void measureOwDelay(uint64_t now, uint64_t ts, bool verbose);
void printMeasureOwDelay(uint32_t* base, uint32_t offset);

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

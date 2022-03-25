#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"  // DBPRINTx()
#include "aux.h"  // getSysTime()
#include "fbas.h"
#include "fbas_common.h"

#define _64b_SIZE  8

enum MSR_CNT {
  RX_EVT_CNT,
  TX_EVT_CNT,
  ECA_VLD_ACT,
  ECA_OVF_ACT,
  N_MSR_CNT,
};

typedef struct msrCnt msrCnt_t;
struct msrCnt {
  uint32_t val;    // counter value
};

typedef struct msrSumStats msrSumStats_t;
struct msrSumStats {
  uint64_t avg;          // cumulative moving average
  int64_t  min;          // minimum value
  uint64_t max;          // maximum value
  uint32_t cntValid;     // number of valid measurement
  uint32_t cntTotal;     // number of total measurement
};

enum {
  msr_tx_dly,  // transmission delay
  msr_sg_lty,  // signalling latency
  msr_ow_dly,  // one-way delay
  msr_ttl,     // TTL threshold/interval
  msr_all,
};

void storeTimestamp(uint32_t* reg, uint32_t offset, uint64_t ts);
int64_t getElapsedTime(uint32_t* reg, uint32_t offset, uint64_t now);
void storeTsMeasureDelays(uint32_t* base, uint32_t offset, uint64_t tsEca, uint64_t tsTx);
void measureNwPerf(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca, bool verbose);
void printMeasureTxDelay(uint32_t* base, uint32_t offset);
void printMeasureSgLatency(uint32_t* base, uint32_t offset);
void measureOwDelay(uint64_t now, uint64_t ts, bool verbose);
void printMeasureOwDelay(uint32_t* base, uint32_t offset);
void measureTtlInterval(mpsTimParam_t* buf);
void printMeasureTtl(uint32_t* base, uint32_t offset);
uint32_t calculateSumStats(int64_t value, msrSumStats_t* pStats);
void wrSumStats(msrSumStats_t* pStats, uint64_t* pSharedReg64);

/**
 * \brief Count events
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \ret counter  Value
 **/
uint32_t msrCnt(unsigned name, uint32_t value);

/**
 * \brief Set event counter
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \ret counter  Value
 **/
uint32_t msrSetCnt(unsigned name, uint32_t value);

#endif

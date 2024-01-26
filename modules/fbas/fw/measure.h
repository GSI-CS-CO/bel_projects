#ifndef _MEASUREMENT_H_
#define _MEASUREMENT_H_

#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"  // DBPRINTx()
#include "aux.h"  // getSysTime()
#include "fbas.h"
#include "fbas_common.h"

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

/**
 * \brief List of measured items
*/
typedef enum MSR_ITEMS {
  MSR_TX_DLY,  // transmission delay (requires feedback via LEMO cabling)
  MSR_SG_LTY,  // signalling latency (requires feedback via LEMO cabling)
  MSR_MSG_DLY, // messaging delay
  MSR_TTL,     // TTL threshold/interval
  MSR_TX_LTY,  // MPS transmit latency
  MSR_TX_MPS_HANDLE, // MPS event handling delay at TX node
  N_MSR_ITEMS,
} msrItem_t;

void storeTimestamp(uint32_t* reg, uint32_t offset, uint64_t ts);
int64_t getElapsedTime(uint32_t* reg, uint32_t offset, uint64_t now);
void measureClearAverage(verbosity_t verbose);
void measureAverage(msrItem_t item, uint64_t from, uint64_t now, verbosity_t verbose);
void measurePrintAverage(msrItem_t item, uint32_t* base, uint32_t offset);

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

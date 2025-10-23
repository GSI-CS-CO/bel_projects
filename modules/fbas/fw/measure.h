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
  BAD_MSG_CNT,
  N_MSR_CNT,
};

typedef struct msrCnt msrCnt_t;
struct msrCnt {
  uint32_t val;    // counter value
};

typedef struct msrSumStats msrSumStats_t;
struct msrSumStats {
  uint64_t ts;           // start time of measurement
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
  MSR_ECA_HANDLE,    // ECA handling delay
  MSR_MAIN_LOOP_PRD, // period of the main loop
  N_MSR_ITEMS,
} msrItem_t;

void measurePutTimestamp(msrItem_t item, uint64_t ts);
uint64_t measureGetTimestamp(msrItem_t item);
void measureClearSummary(verbosity_t verbose);
void measureSummarize(msrItem_t item, uint64_t from, uint64_t now, verbosity_t verbose);
void measureExportSummary(msrItem_t item, uint32_t* base, uint32_t offset);
void measurePrintSummary(msrItem_t item);

uint32_t measureCountEvt(unsigned name, uint32_t value);
uint32_t measureSetCounter(unsigned name, uint32_t value);

#endif

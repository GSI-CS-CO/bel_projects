/********************************************************************************************
 *  measurement.c
 *
 *  created : 2021
 *  author  : Enkhbold Ochirsuren, GSI-Darmstadt
 *  version : 04-June-2021
 *
 *  Functions to measure timing performance, packet loss etc
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2021  Enkhbold Ochirsuren
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: e.ochirsuren@gsi.de
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For all questions and ideas contact: e.ochirsuren@gsi.de
 * Last update: 04-June-2021
 ********************************************************************************************/

#include "measure.h"

struct outlierStat_s {
  uint32_t threshold; // threshold, ns
  uint32_t cnt;       // counts
};

static struct outlierStat_s outlierStat[N_MSR_ITEMS] = {
  {100000, 0},    // transmission delay, ns
  {100000, 0},    // signalling latency, ns
  {100000, 0},    // messaging delay, ns
  {101000000, 0}, // TTL, ns
  {10000, 0}      // ECA event handling, ns
};

static msrSumStats_t sumStats[N_MSR_ITEMS];  // buffer for summary statistics
static msrCnt_t      cnt[N_MSR_CNT];         // event and action counters

/**
 * \brief Count events
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \return Counter value
 **/
uint32_t measureCountEvt(unsigned name, uint32_t value)
{
  cnt[name].val += value;

  return cnt[name].val;
}

/**
 * \brief Set the specified event counter
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \return Counter value
 **/
uint32_t measureSetCounter(unsigned name, uint32_t value)
{
  cnt[name].val = value;

  return cnt[name].val;
}

/**
 * \brief calculate summary statistics
 *
 * Calculate the moving average, minimum, and maximum values.
 *
 * \param value  measured value for calculation
 * \param pStats pointer to summary statistics buffer
 * \ret   count  total number of measurements
 **/
static uint32_t calculateSumStats(const int64_t value, msrSumStats_t *const pStats) {

    if (value > 0) {
      pStats->avg = (value + (pStats->cntValid * pStats->avg)) / (pStats->cntValid + 1);
      ++pStats->cntValid;

      if (value > pStats->max)
        pStats->max = value;

      if (value < pStats->min || !pStats->min)
        pStats->min = value;
    }

    return ++pStats->cntTotal;
}

/**
 * \brief Keep the start timestamp to measure the given item
 *
 * Given timestamp is kept to measure the elapsed time later.
 *
 * \param item   measured item
 * \param ts     timestamp
 **/
void measurePutTimestamp(msrItem_t item, uint64_t ts)
{
  sumStats[item].ts = ts;
}

/**
 * \brief Return the start timestamp to measure the given item
 *
 * \param item   measured item
 * \return timestamp
 **/
uint64_t measureGetTimestamp(msrItem_t item)
{
  return sumStats[item].ts;
}

/**
 * \brief Summarize the measured item
 *
 * Summarize the measured item (calculate moving averag, find the minimum,
 * maximum, outlier values, and count measurements).
 *
 * \param item    measured item
 * \param from    start timestamp
 * \param now     actual timestamp (or system time)
 * \param verbose verbosity flag
*/
void measureSummarize(msrItem_t item, uint64_t from, uint64_t now, verbosity_t verbose) {

  int64_t period = now - from;  // calculate the time period
  if (verbose)
    DBPRINT2("%d: %lli\n", item, period);

  // calculate and store the summed average
  calculateSumStats(period, &sumStats[item]);

  // count outliers
  if (period > outlierStat[item].threshold)
    ++outlierStat[item].cnt;
}

/**
 * \brief Print the measurement statistics of the given item
 *
 * Print the measurement statistics of the given item to the WR console
 *
 * \param item    measured item
*/
void measurePrintSummary(msrItem_t item) {
  msrSumStats_t* pStats = &sumStats[item];

  DBPRINT2("%d avg=%llu min=%lli max=%llu cnt=%lu/%lu\n",
    item,
    pStats->avg, pStats->min, pStats->max, pStats->cntValid, pStats->cntTotal);

  DBPRINT2("lmt=%lu\n", outlierStat[item].cnt);
}

/**
 * \brief Export the measurement summary statistics to the shared memory
 *
 * Export the measurement summary statistics (avg, min, max) to the given
 * location of the shared memory.
 *
 * \param item    measured item
 * \param base    base memory address
 * \param offset  offset memory address
*/
void measureExportSummary(msrItem_t item, uint32_t* base, uint32_t offset) {
  uint64_t *pSharedReg64 = (uint64_t *)(base + (offset >> 2));
  uint32_t *pSharedReg32;
  msrSumStats_t* pStats = &sumStats[item];

  *pSharedReg64 = pStats->avg;
  *(++pSharedReg64) = pStats->min;
  *(++pSharedReg64) = pStats->max;
  ++pSharedReg64;

  pSharedReg32 = (uint32_t *)pSharedReg64;
  *pSharedReg32 = pStats->cntValid;
  *(++pSharedReg32) = pStats->cntTotal;
}

/**
 * \brief Clear the summary statistics
 *
 * \param verbose verbosity
 *
 * \return none
*/
void measureClearSummary(verbosity_t verbose) {
  msrItem_t item;

  for (item = 0; item < N_MSR_ITEMS; ++item) {
    memset(&sumStats[item], 0, sizeof(msrSumStats_t));
    outlierStat[item].cnt = 0;

    if (verbose)
      // implement in 2 calls, otherwise 'max' has garbage
      DBPRINT2("%d @0x%p ", item, &sumStats[item]);
      DBPRINT2("avg=%llu min=%lli max=%llu val=%lu all=%lu\n",
        sumStats[item].avg, sumStats[item].min, sumStats[item].max,
        sumStats[item].cntValid, sumStats[item].cntTotal);
  }
}
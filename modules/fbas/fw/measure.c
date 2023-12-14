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

static msrSumStats_t sumStats[N_MSR_ITEMS];  // buffer for summary statistics
static msrCnt_t      cnt[N_MSR_CNT];         // event and action counters

/**
 * \brief store a timestamp
 *
 * Given timestamp is stored in shared memory location pointed by base + offset.
 *
 * \param base   base address of the user-defined u32 register set
 * \param offset offset in the register set that will store given timestamp
 * \param ts     timestamp
 *
 * \ret none
 **/
void storeTimestamp(uint32_t* base, uint32_t offset, uint64_t ts)
{
  uint64_t* pSharedTs = (uint64_t *)(base + (offset >> 2));

  *pSharedTs = ts;
}

/**
 * \brief get the elapsed time
 *
 * Return an elapsed time, which is the difference of given system time and
 * timestamp stored in shared memory (pointed by base + offset).
 * The timestamp is updated then with the given system time.
 *
 * \param base   base address of the user-defined u32 register set
 * \param offset offset in the register set that stores timestamp needed for calculation
 * \param now    actual system time
 *
 * \ret time     elapsed time in nanosecond since last timestamp
 **/
int64_t getElapsedTime(uint32_t* base, uint32_t offset, uint64_t now)
{
  uint64_t* pSharedTs = (uint64_t *)(base + (offset >> 2));

  int64_t elapsed = now - *pSharedTs;

  *pSharedTs = now;

  return elapsed;
}

/**
 * \brief Count events
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \ret counter  Value
 **/
uint32_t msrCnt(unsigned name, uint32_t value)
{
  cnt[name].val += value;

  return cnt[name].val;
}

/**
 * \brief Set event counter
 *
 * \param name   Counter name (listed in MSR_CNT)
 * \param value  Used to increment/initialize the counter
 *
 * \ret counter  Value
 **/
uint32_t msrSetCnt(unsigned name, uint32_t value)
{
  cnt[name].val = value;

  return cnt[name].val;
}

/**
 * \brief calculate summary statistics
 *
 * \param value  measured value for calculation
 * \param pStats pointer to summary statistics buffer
 * \ret   count  total number of measurements
 **/
static uint32_t calculateSumStats(int64_t value, msrSumStats_t* pStats) {

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
 * \brief write a specified summary statistics to a given memory location
 *
 * \param pStats       pointer to summary statistics (avg, min, max) buffer
 * \param pSharedReg64 address of the shared memory location (64-bit)
 * \ret none
 **/
static void wrSumStats(msrSumStats_t* pStats, uint64_t* pSharedReg64) {

  uint32_t *pSharedReg32;

  *pSharedReg64 = pStats->avg;
  *(++pSharedReg64) = pStats->min;
  *(++pSharedReg64) = pStats->max;
  ++pSharedReg64;

  pSharedReg32 = (uint32_t *)pSharedReg64;
  *pSharedReg32 = pStats->cntValid;
  *(++pSharedReg32) = pStats->cntTotal;
}

/**
 * \brief measure the average of the given item
 *
 * Average time period of a specified item is measured.
 *
 * \param item    measured item
 * \param from    start timestamp
 * \param now     actual timestamp (or system time)
 * \param verbose verbosity flag
*/
void measureAverage(msrItem_t item, uint64_t from, uint64_t now, verbosity_t verbose) {
  msrSumStats_t* pStats = &sumStats[item];
  int64_t period = now - from;  // calculate the time period
  if (verbose)
    DBPRINT2("period=%lli\n", period);

  // calculate and store the summed average
  calculateSumStats(period, pStats);
}

/**
 * \brief print the measured average of the given item
 *
 * Print the measured average of the given item
 * to the console and dedicated shared memory location
 *
 * \param item    measured item
 * \param base    base memory address
 * \param offset  offset memory address
*/
void measurePrintAverage(msrItem_t item, uint32_t* base, uint32_t offset) {
  uint64_t *pSharedReg64 = (uint64_t *)(base + (offset >> 2));
  msrSumStats_t* pStats = &sumStats[item];

  DBPRINT2("%d @0x%8p avg=%llu min=%lli max=%llu cnt=%d/%d\n",
    item,
    pSharedReg64,
    pStats->avg, pStats->min, pStats->max, pStats->cntValid, pStats->cntTotal);

  wrSumStats(pStats, pSharedReg64);
}

/**
 * \brief Clear the summary statistics
 *
 * \param verbose verbosity
 *
 * \return none
*/
void measureClearAverage(verbosity_t verbose) {
  msrItem_t item;

  for (item = 0; item < N_MSR_ITEMS; ++item) {
    memset(&sumStats[item], 0, sizeof(msrSumStats_t));

    if (verbose)
      DBPRINT2("%d avg=%llu min=%lli max=%llu val=%d all=%d\n",
        item,
        sumStats[item].avg, sumStats[item].min, sumStats[item].max,
        sumStats[item].cntValid, sumStats[item].cntTotal);
  }
}
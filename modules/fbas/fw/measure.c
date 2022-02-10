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

msrPerfStats_t perfStats = {0};
msrOwDelay_t   owDelay = {0};
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
 * \brief store timestamps to measure delays
 *
 * Store time points, at which an MPS event was detected and forwarded, in
 * shared memory:
 * - timestamp of MPS event detection is stored in a location pointed by base + offset
 * - timestamp of MPS event transmission is stored in next location.
 *
 * \param base   base address of the user-defined u32 register set
 * \param offset offset in the register set that will store given timestamps
 * \param tsEca  timestamp of MPS event detection (ECA event deadline)
 * \param tsTx   timestamp of MPS event transmission (actual system time)
 *
 * \ret none
 **/
void storeTsMeasureDelays(uint32_t* base, uint32_t offset, uint64_t tsEca, uint64_t tsTx)
{
  uint32_t next = offset + _64b_SIZE;
  uint64_t *pSharedTs = (uint64_t *)(base + (offset >> 2));
  *pSharedTs = tsEca;
  *(pSharedTs + (next >> 2)) = tsTx;
}

/**
 * \brief measure network performance
 *
 * Network delay on transmission of an MPS event (broadcast from TX node to RX node) and
 * signalling latency to forward an MPS event (from MPS event generation at TX node
 * to IO event detection at RX node) are measured using timestamps.
 * The measurement results are output as debug msg.
 *
 * The timestamps required to calculate elapsed time are stored in shared memory:
 * - timestamp of MPS event detection is stored in a location pointed by base + offset
 * - timestamp of MPS event transmission is stored in next location.
 *
 * \param base   base address of the user-defined u32 register set
 * \param offset offset in register set that stores timestamps
 * \param tag    ECA condition tag
 * \param flag   ECA late event flag
 * \param now    actual system time
 * \param tsEca  timestamp of IO event detection (ECA event deadline)
 *
 * \ret none
 **/
void measureNwPerf(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca, bool verbose)
{
  uint64_t *pSharedTs = (uint64_t *)(base + (offset >> 2));
  uint64_t tmp64 = *(pSharedTs + ((offset + _64b_SIZE) >> 2));

  int64_t nwDelay = tsEca - tmp64;       // network delay
  int64_t sgLatency = now - *pSharedTs;  // signal latency
  if (verbose)
    DBPRINT2("nwDly=%lli, sgLty=%lli\n", nwDelay, sgLatency);

  if (nwDelay > 0) {
    perfStats.avgNwDelay = (nwDelay + (perfStats.cntNwDelay * perfStats.avgNwDelay)) / (perfStats.cntNwDelay + 1);
    ++perfStats.cntNwDelay;
    if (nwDelay > perfStats.maxNwDelay)
      perfStats.maxNwDelay = nwDelay;
  }

  if (nwDelay < perfStats.minNwDelay || !perfStats.minNwDelay)
    perfStats.minNwDelay = nwDelay;

  if (sgLatency > 0) {
    perfStats.avgSgLatency = (sgLatency + (perfStats.cntSgLatency * perfStats.avgSgLatency)) / (perfStats.cntSgLatency + 1);
    ++perfStats.cntSgLatency;
    if (sgLatency > perfStats.maxSgLatency)
      perfStats.maxSgLatency = sgLatency;
  }

  if (sgLatency < perfStats.minSgLatency || !perfStats.minSgLatency)
    perfStats.minSgLatency = sgLatency;

  ++perfStats.cntTotal;

  // for details, elapsed time of other actions are also calculated
  int64_t poll = now - tsEca;      // elapsed time to detect IO (TLU) event (RX->TX)
  DBPRINT3("IO evt (tag %x, flag %x, ts %llu, now %llu, poll %lli)\n",
      tag, flag, tsEca, now, poll);

  poll = tmp64 - *pSharedTs;       // elapsed time to send MPS event (TX->RX)
  DBPRINT3("MSP evt (detect %llu, send %llu, poll %lli)\n",
      *pSharedTs, tmp64, poll);
}

/**
 * \brief print result of network performance measurement - network delay
 *
 * Average, minimum and maximum of network delay are
 * printed to debug output (invoke eb-console $dev to get the debug output)
 *
 * \param none
 * \ret none
 **/
void printMeasureNwDelay(uint32_t* base, uint32_t offset) {

  uint64_t *pSharedReg64 = (uint64_t *)(base + (offset >> 2));
  uint32_t *pSharedReg32;

  *pSharedReg64 = perfStats.avgNwDelay;
  *(++pSharedReg64) = perfStats.minNwDelay;
  *(++pSharedReg64) = perfStats.maxNwDelay;
  ++pSharedReg64;

  pSharedReg32 = (uint32_t *)pSharedReg64;
  *pSharedReg32 = perfStats.cntNwDelay;
  *(++pSharedReg32) = perfStats.cntTotal;

  DBPRINT2("nwDly @0x%08x, avg=%llu, min=%lli, max=%llu, cnt=%d/%d\n",
      pSharedReg64 - 3,
      perfStats.avgNwDelay, perfStats.minNwDelay, perfStats.maxNwDelay,
      perfStats.cntNwDelay, perfStats.cntTotal);
}

/**
 * \brief print result of network performance measurement - signaling latency
 *
 * Average, minimum and maximum of signaling latency are
 * printed to debug output (invoke eb-console $dev to get the debug output)
 *
 * \param none
 * \ret none
 **/
void printMeasureSgLatency(uint32_t* base, uint32_t offset) {

  uint64_t *pSharedReg64 = (uint64_t *)(base + (offset >> 2));
  uint32_t *pSharedReg32;

  *pSharedReg64 = perfStats.avgSgLatency;
  *(++pSharedReg64) = perfStats.minSgLatency;
  *(++pSharedReg64) = perfStats.maxSgLatency;
  ++pSharedReg64;

  pSharedReg32 = (uint32_t *)pSharedReg64;
  *pSharedReg32 = perfStats.cntSgLatency;
  *(++pSharedReg32) = perfStats.cntTotal;

  DBPRINT2("sgLty @0x%08x, avg=%llu, min=%lli, max=%llu, cnt=%d/%d\n",
      pSharedReg64 - 3,
      perfStats.avgSgLatency, perfStats.minSgLatency, perfStats.maxSgLatency,
      perfStats.cntSgLatency, perfStats.cntTotal);
}

/**
 * \brief count events
 *
 * \param enable indicates if the counter is incremented (=true) or initialized with a given value
 * \param value  used to increment/initialize the counter
 *
 * \ret counter value
 **/
uint32_t doCnt(bool enable, uint32_t value)
{
  static msrCnt_t cnt = {0};

  if (enable) {
    cnt.val += value;
  } else {
    cnt.val = value;
  }

  return cnt.val;
}

/**
 * \brief measure one-way delay
 *
 * The one-way delay (or end-to-end) is the time taken for a timing message
 * (with a MPS flag) to be transmitted across a network (a WRS switch) from
 * a TX node to a RX node.
 *
 * \param now   actual system time
 * \param ts    timestamp of MPS flag
 *
 * \ret none
 **/
void measureOwDelay(uint64_t now, uint64_t ts, bool verbose)
{
  int64_t owd = now - ts;  // one-way (end-to-end) delay
  if (verbose)
    DBPRINT2("owd=%lli\n", owd);

  if (owd > 0) {
    owDelay.avg = (owd + (owDelay.cntValid * owDelay.avg)) / (owDelay.cntValid + 1);
    ++owDelay.cntValid;
    if (owd > owDelay.max)
      owDelay.max = owd;
  }

  if (owd < owDelay.min || !owDelay.min)
    owDelay.min = owd;

  ++owDelay.cntTotal;
}

/**
 * \brief print result of one-way delay measurement
 *
 * \param none
 * \ret none
 **/
void printMeasureOwDelay(uint32_t* base, uint32_t offset) {

  uint64_t *pSharedReg64 = (uint64_t *)(base + (offset >> 2));
  uint32_t *pSharedReg32;

  *pSharedReg64 = owDelay.avg;
  *(++pSharedReg64) = owDelay.min;
  *(++pSharedReg64) = owDelay.max;
  ++pSharedReg64;

  pSharedReg32 = (uint32_t *)pSharedReg64;
  *pSharedReg32 = owDelay.cntValid;
  *(++pSharedReg32) = owDelay.cntTotal;

  DBPRINT2("owd @0x%08x, avg=%llu, min=%lli, max=%llu, cnt=%d/%d\n",
      pSharedReg64 - 3,
      owDelay.avg, owDelay.min, owDelay.max, owDelay.cntValid, owDelay.cntTotal);
}

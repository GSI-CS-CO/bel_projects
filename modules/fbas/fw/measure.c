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
 * \brief measure delays
 *
 * Network delay on transmission of an MPS event (broadcast from TX node to RX node) and
 * time spent to forward an MPS event (from MPS event generation at TX node
 * to IO event detection at RX node) are measured.
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
void measureDelays(uint32_t* base, uint32_t offset, uint32_t tag, uint32_t flag, uint64_t now, uint64_t tsEca)
{
  uint64_t *pSharedTs = (uint64_t *)(base + (offset >> 2));
  uint64_t tmp64 = *(pSharedTs + ((offset + _64b_SIZE) >> 2));

  int64_t delayNw = tsEca - tmp64;     // network delay
  int64_t delayFwd = now - *pSharedTs; // forward duration
  DBPRINT2("nw=%lli, fwd=%lli\n", delayNw, delayFwd);

  // for details, elapsed time of other actions are also calculated
  int64_t poll = now - tsEca;      // elapsed time to detect IO (TLU) event (RX->TX)
  DBPRINT3("IO evt (tag %x, flag %x, ts %llu, now %llu, poll %lli)\n",
      tag, flag, tsEca, now, poll);

  poll = tmp64 - *pSharedTs;       // elapsed time to send MPS event (TX->RX)
  DBPRINT3("MSP evt (detect %llu, send %llu, poll %lli)\n",
      *pSharedTs, tmp64, poll);
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

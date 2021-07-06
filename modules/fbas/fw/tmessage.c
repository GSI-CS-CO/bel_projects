/********************************************************************************************
 *  tmessage.c
 *
 *  created : 2021
 *  author  : Enkhbold Ochirsuren, GSI-Darmstadt
 *  version : 04-June-2021
 *
 *  Functions to send and receive the MPS flags using timing message
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

#include "tmessage.h"

// application-specific variables
mpsTimParam_t bufMpsFlag[N_MPS_CHANNELS] = {0};   // buffer for MPS flags
timedItr_t rdItr = {0};                           // read-access iterator for MPS flags

/**
 * \brief initialize iterator
 *
 * Initialize an iterator that is used to specify a next MPS flag to send.
 *
 * \param itr   pointer to an iterator
 * \param total max. number of iterator indices
 * \param now   timestamp of latest iterator access
 * \param freq  iteration period
 *
 * \ret none
 **/
void initItr(timedItr_t* itr, uint8_t total, uint64_t now, uint32_t freq)
{
  itr->idx = 0;
  itr->total = total;
  itr->last = now;
  itr->period = TIM_1000_MS;
  if (freq && itr->total) {
    itr->period /=(freq * itr->total); // for 30Hz it's 33312 us (30.0192 Hz)

    //itr->period /= 1000ULL; // granularity in 1 us
    //itr->period *= 1000ULL;
  }
}

/**
 * \brief reset iterator
 *
 * Reset an iterator that is used to specify a next MPS flag to send.
 *
 * \param itr pointer to an iterator
 * \param now timestamp of last iterator access
 *
 * \ret none
 **/
void resetItr(timedItr_t* itr, uint64_t now)
{
  itr->last = now;

  ++itr->idx;
  if (itr->idx >= itr->total)
    itr->idx = 0;
}

/**
 * \brief send a block of MPS flags
 *
 * Send a specified number of the MPS flags
 *
 * \param len   block length
 * \param itr   read-access iterator that specifies next MPS flag to send
 * \param evtId event ID for timing messages
 *
 * \ret status
 **/
status_t sendMpsFlagBlock(size_t len, timedItr_t* itr, uint64_t evtId)
{
  uint32_t res, tef;                // temporary variables for bit shifting etc
  uint32_t deadlineLo, deadlineHi;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  uint64_t now = getSysTime();
  uint64_t deadline = itr->last + itr->period;

  if (len > N_MAX_TIMMSG)
    return COMMON_STATUS_OUTOFRANGE;

  if (!itr->last)
    deadline = now;  // initial transmission

  // send timing messages if deadline is over
  if (deadline <= now) {
    // pack Ethernet frame with messages
    idHi       = (uint32_t)((evtId >> 32)    & 0xffffffff);
    idLo       = (uint32_t)(evtId            & 0xffffffff);
    tef        = 0x00000000;
    res        = 0x00000000;
    deadlineHi = (uint32_t)((deadline >> 32) & 0xffffffff);
    deadlineLo = (uint32_t)(deadline         & 0xffffffff);

    // start EB operation
    ebm_hi(COMMON_ECA_ADDRESS);

    // send a block of MPS flags
    atomic_on();
    for (size_t i = 0; i < len; ++i) {
      // get MPS flag
      paramHi  = (uint32_t)((bufMpsFlag[itr->idx].param >> 32) & 0xffffffff);
      paramLo  = (uint32_t)(bufMpsFlag[itr->idx].param         & 0xffffffff);

      // update iterator
      resetItr(itr, deadline);

      // build a timing message
      ebm_op(COMMON_ECA_ADDRESS, idHi,       EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, idLo,       EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, paramHi,    EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, paramLo,    EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, tef,        EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, res,        EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, deadlineHi, EBM_WRITE);
      ebm_op(COMMON_ECA_ADDRESS, deadlineLo, EBM_WRITE);

    }
    atomic_off();

    // send timing messages
    ebm_flush();
  }
  else
    return COMMON_STATUS_ERROR;

  return COMMON_STATUS_OK;
}

/**
 * \brief send an MPS flag
 *
 * MPS flags are sent at specified period. [MPS_FS_530]
 *
 * \param itr   read-access iterator that specifies next flag to send
 * \param evtid event ID used to send a timing message
 *
 * \ret status
 **/
status_t sendMpsFlag(timedItr_t* itr, uint64_t evtid)
{
  uint64_t now = getSysTime();
  uint64_t deadline = itr->last + itr->period;
  if (!itr->last)
    deadline = now;       // initial transmission

  // send next MPS flag if deadline is over
  if (deadline <= now) {

    // send MPS flag with current timestamp, which varies around deadline
    fwlib_ebmWriteTM(now, evtid, bufMpsFlag[itr->idx].param, 1);

    // update iterator with deadline
    resetItr(itr, deadline);
  }
  else
    return COMMON_STATUS_ERROR;

  return COMMON_STATUS_OK;
}

/**
 * \brief send an MPS event
 *
 * Upon flag change to NOK, there shall be 2 extra events within 50 us. [MPS_FS_530]
 * If the read iterator is blocked by new cycle, then do not send any MPS event. [MPS_FS_630]
 *
 * \param itr   read-access iterator that specifies next flag to send
 * \param buf   pointer to MPS event buffer
 * \param evtid event ID used to send a timing message
 * \param extra number of extra events
 *
 * \ret status
 **/
status_t sendMpsEvent(timedItr_t* itr, mpsTimParam_t* buf, uint64_t evtid, uint8_t extra)
{
  uint64_t now = getSysTime();

  if (itr->last >= now) // delayed by a new cycle
    return COMMON_STATUS_ERROR;

  // send specified MPS event
  fwlib_ebmWriteTM(now, evtid, buf->param, 1);

  // NOK flag shall be sent as extra events
  if (buf->prot.flag == MPS_FLAG_NOK) {
    for (uint8_t i = 0; i < extra; ++i) {
      now = getSysTime();
      fwlib_ebmWriteTM(now, evtid, buf->param, 1);
    }
  }

  return COMMON_STATUS_OK;
}

/**
 * \brief alter lifetime of MPS flags [MPS_FS_600]
 *
 * \param itr iterator used to access MPS flags in pre-defined period
 *
 * \ret ptr pointer to expired MPS flag
 **/
mpsTimParam_t* expireMpsFlag(timedItr_t* itr)
{
  uint64_t now = getSysTime();
  uint64_t deadline = itr->last + itr->period;

  if (!itr->last)
    deadline = now;       // initial check

  // check lifetime of next MPS flag
  if (deadline <= now) {

    // decrement TTL counter
    if (bufMpsFlag[itr->idx].prot.ttl) {
      --bufMpsFlag[itr->idx].prot.ttl;

      if (!bufMpsFlag[itr->idx].prot.ttl) {
        bufMpsFlag[itr->idx].prot.flag = MPS_FLAG_NOK;
        return &bufMpsFlag[itr->idx];  // expired MPS flag
      }
    }

    // update iterator with deadline
    resetItr(itr, deadline);
  }

  return 0;
}

/**
 * \brief update MPS flag with recieved MPS event
 *
 * \param buf pointer to MPS flags buffer
 * \param evt raw event data (bits 31-24 = flag, 23-16 = grpId, 15-0 = evtId)
 *
 * \ret ptr pointer to the updated MPS flag
 **/
mpsTimParam_t* updateMpsFlag(mpsTimParam_t* buf, uint64_t evt)
{
  // evaluate MPS channel and its flag
  uint8_t flag = evt >> 24;
  uint8_t grpId = evt >> 16;
  uint16_t evtId = evt & 0xFFFF;

  if (evtId >= N_MPS_CHANNELS)
    return 0;

  // update MPS flag
  buf += evtId;
  buf->prot.flag = flag;
  return buf;
}

/**
 * \brief store recieved MPS flag
 *
 * \param buf pointer to MPS flags buffer
 * \param raw raw protocol data (bits 63-56 = flag, 57-48 = grpId, 47-32 = evtId)
 *
 * \ret ptr pointer to the stored MPS flag
 **/
mpsTimParam_t* storeMpsFlag(mpsTimParam_t* buf, uint64_t raw)
{
  // evaluate MPS channel and its flag
  uint8_t flag = raw >> 56;
  uint8_t grpId = raw >> 48;
  uint16_t evtId = raw >> 32;

  if (evtId >= N_MPS_CHANNELS)
    return 0;

  // store MPS flag
  buf += evtId;
  buf->prot.pending = buf->prot.flag ^ flag;
  buf->prot.flag = flag;
  buf->prot.ttl = 10; // die after 10 iterations
  return buf;
}

/**
 * \brief reset MPS flag
 *
 * It is used to reset the CMOS input virtually to high voltage in TX [MPS_FS_620] or
 * reset effective logic input to HIGH bit in RX [MPS_FS_630].
 *
 * \param buf pointer to MPS flag buffer
 *
 **/
void resetMpsFlag(size_t len, mpsTimParam_t* buf)
{
  uint8_t flag = MPS_FLAG_OK;

  for (size_t i = 0; i < len; ++i) {
    (buf + i)->prot.pending = (buf + i)->prot.flag ^ flag;
    (buf + i)->prot.flag  = flag;
    (buf + i)->prot.ttl = 10; // time-out for 10 iterations
  }
}


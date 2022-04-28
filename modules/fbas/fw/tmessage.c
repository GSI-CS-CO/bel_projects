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
mpsMsg_t   bufMpsMsg[N_MPS_CHANNELS] = {0};       // buffer for MPS timing messages
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

    itr->ttl = itr->total * freq / 15; // TTL value of the MPS protocol data (2x lost msgs)

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
 * \brief Send a block of MPS messages
 *
 * Send a specified number of the MPS messages
 *
 * \param len   Block length
 * \param itr   Read-access iterator that specifies next MPS flag to send
 * \param evtId Event ID for timing messages
 *
 * \ret status
 **/
status_t sendMpsMsgBlock(size_t len, timedItr_t* itr, uint64_t evtId)
{
  uint32_t res, tef;                // temporary variables for bit shifting etc
  uint32_t deadlineLo, deadlineHi;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;
  uint64_t param;

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
    deadlineHi = (uint32_t)((now >> 32) & 0xffffffff);
    deadlineLo = (uint32_t)(now         & 0xffffffff);

    // start EB operation
    ebm_hi(COMMON_ECA_ADDRESS);

    // send a block of MPS flags
    atomic_on();
    for (size_t i = 0; i < len; ++i) {
      // get MPS protocol
      memcpy(&param, &bufMpsMsg[itr->idx].prot, sizeof(uint64_t));
      paramHi  = (uint32_t)((param >> 32) & 0xffffffff);
      paramLo  = (uint32_t)(param         & 0xffffffff);

      // update iterator
      resetItr(itr, now);

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
 * \brief Send MPS messages periodically
 *
 * MPS flags are sent at specified period. [MPS_FS_530]
 *
 * \param itr   Read-access iterator that specifies next MPS message to send
 * \param evtid Event ID used to send a timing message
 *
 * \ret status  Zero on success, otherwise non-zero
 **/
status_t sendMpsMsgPeriodic(timedItr_t* itr, uint64_t evtid)
{
  uint64_t now = getSysTime();
  uint64_t deadline = itr->last + itr->period;
  if (!itr->last)
    deadline = now;       // initial transmission

  // send next MPS message if deadline is over
  if (deadline <= now) {
    uint64_t param;
    mpsProtocol_t* prot = &bufMpsMsg[itr->idx].prot;
    memcpy(&param, prot, sizeof(mpsProtocol_t));

    // send MPS message with current timestamp, which varies around deadline
    fwlib_ebmWriteTM(now, evtid, param, 1);

    // update iterator with deadline
    resetItr(itr, now);
  }
  else
    return COMMON_STATUS_ERROR;

  return COMMON_STATUS_OK;
}

/**
 * \brief Send a specific MPS message
 *
 * Upon flag change to NOK, there shall be 2 extra events within 50 us. [MPS_FS_530]
 * If the read iterator is blocked by new cycle, then do not send any MPS event. [MPS_FS_630]
 *
 * \param itr   Read-access iterator that points to MPS message buffer
 * \param buf   Pointer to a specific MPS message
 * \param evtid Event ID used to send a timing message
 * \param extra Number of extra messages
 *
 * \ret status  Zero on success, otherwise non-zero
 **/
status_t sendMpsMsgSpecific(timedItr_t* itr, mpsMsg_t* buf, uint64_t evtid, uint8_t extra)
{
  uint64_t now = getSysTime();

  if (itr->last >= now) // delayed by a new cycle
    return COMMON_STATUS_ERROR;

  uint64_t param;
  memcpy(&param, &buf->prot, sizeof(buf->prot));

  // send specified MPS event
  fwlib_ebmWriteTM(now, evtid, param, 1);

  // NOK flag shall be sent as extra events
  if (buf->prot.flag == MPS_FLAG_NOK) {
    for (uint8_t i = 0; i < extra; ++i) {
      fwlib_ebmWriteTM(now, evtid, param, 1);
    }
  }

  return COMMON_STATUS_OK;
}

/**
 * \brief alter lifetime of received MPS messages [MPS_FS_600]
 *
 * \param itr iterator used to access MPS messages in pre-defined period
 *
 * \ret ptr pointer to expired MPS message
 **/
mpsMsg_t* expireMpsMsg(timedItr_t* itr)
{
  uint64_t now = getSysTime();
  uint64_t deadline = itr->last + itr->period;
  mpsMsg_t* buf = 0;

  if (!itr->last)
    deadline = now;       // initial check

  // check lifetime of next MPS flag
  if (deadline <= now) {

    if (bufMpsMsg[itr->idx].ttl) {

      if (now - bufMpsMsg[itr->idx].ts > TIM_100_MS) {
        //DBPRINT2("%llu %llu\n", now, bufMpsMsg[itr->idx].ts);
        bufMpsMsg[itr->idx].prot.flag = MPS_FLAG_NOK;
        bufMpsMsg[itr->idx].ttl = 0;
        buf = &bufMpsMsg[itr->idx];  // expired MPS message
      }
    }

    // update iterator with deadline
    resetItr(itr, now);
  }

  return buf;
}

/**
 * \brief update MPS message with a given MPS event
 *
 * \param buf Pointer to MPS message buffer
 * \param evt Raw event data (bits 15-8 = index, 7-0 = flag)
 *
 * \ret ptr Pointer to the updated MPS message buffer
 **/
mpsMsg_t* updateMpsMsg(mpsMsg_t* buf, uint64_t evt)
{
  // evaluate MPS channel and its flag
  uint8_t idx = evt >> 8;
  uint8_t flag = evt;

  // update MPS message
  buf->prot.idx = idx;
  buf->prot.flag = flag;
  return buf;
}

/**
 * \brief store recieved MPS message
 *
 * \param raw Raw MPS protocol (bits 63-16 = addr, 15-8 = index, 7-0 = flag)
 * \param ts  Timestamp
 * \param itr Read-access iterator
 *
 * \ret ptr pointer to the stored MPS flag
 **/
mpsMsg_t* storeMpsMsg(uint64_t raw, uint64_t ts, timedItr_t* itr)
{
  uint8_t flag = raw;
  uint8_t idx = raw >> 8;
  uint8_t addr[ETH_ALEN];
  mpsMsg_t* buf = &bufMpsMsg[0];

  memcpy(addr, &raw, ETH_ALEN);

  for (int i = 0; i < N_MPS_CHANNELS; ++i)
    if (addr_equal(addr, buf->prot.addr)) {
      if (buf->prot.idx == idx) {
        buf->pending = buf->prot.flag ^ flag;
        buf->prot.flag = flag;
        buf->ttl = itr->ttl;
        buf->ts = ts;
        return buf;
      }
    }
    else
      ++buf;

  return 0;
}

/**
 * \brief reset MPS message buffer
 *
 * It is used to reset the CMOS input virtually to high voltage in TX [MPS_FS_620] or
 * reset effective logic input to HIGH bit in RX [MPS_FS_630].
 *
 * \param buf Pointer to MPS message buffer
 *
 **/
void resetMpsMsg(size_t len, mpsMsg_t* buf)
{
  uint8_t flag = MPS_FLAG_OK;

  for (size_t i = 0; i < len; ++i) {
    (buf + i)->pending = (buf + i)->prot.flag ^ flag;
    (buf + i)->prot.flag  = flag;
    (buf + i)->ttl = 0;
    (buf + i)->ts = 0;
  }
}

/**
 * \brief Set the sender ID to the MPS message buffer
 *
 * RX node evaluates sender ID of the received MPS message.
 *
 * \param msg     MPS message buffer
 * \param raw     Sender ID (MAC address)
 * \param verbose Non-zero enables verbosity
 *
 * \ret none
 **/
void setMpsMsgSenderId(mpsMsg_t* msg, uint64_t raw, uint8_t verbose)
{
  uint8_t bits = 0;
  for (int i = ETH_ALEN - 1; i >= 0; i--) {
    msg->prot.addr[i] = raw >> bits;
    bits += 8;
  }

  if (verbose) {
    DBPRINT1("tmessage: sender ID: ");
    for (int i = 0; i < ETH_ALEN; i++)
      DBPRINT1("%x ", msg->prot.addr[i]);
    DBPRINT1("\n");
  }
}

/**
 * \brief Check if given MAC addresses are equal
 *
 * \param a  MAC address
 * \param b  MAC address
 *
 * \ret  Return 1 if both addresses are equal, otherwise 0.
 **/
int addr_equal(uint8_t a[ETH_ALEN], uint8_t b[ETH_ALEN])
{
  return !memcmp(a, b, ETH_ALEN);
}

/**
 * \brief Copy source MAC address into the destination MAC address
 *
 * \param src Source MAC address
 * \param dst Destination MAC addres
 *
 * \ret  Pointer to the destination MAC address
 **/
uint8_t *addr_copy(uint8_t dst[ETH_ALEN], uint8_t src[ETH_ALEN])
{
  return memcpy(dst, src, ETH_ALEN);
}

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
uint8_t    nodeIds[N_MAX_TX_NODES][ETH_ALEN];   // sender node ID list
mpsMsg_t   bufMpsMsg[N_MPS_CHANNELS];       // buffer for MPS timing messages
mpsMsg_t *const headBufMps = &bufMpsMsg[0]; // head of the MPS message buffer
timedItr_t rdItr;                           // read-access iterator for MPS flags

static int addr_equal(uint8_t a[ETH_ALEN], uint8_t b[ETH_ALEN]); // wr-switch-sw/userspace/libwr
static uint8_t *addr_copy(uint8_t dst[ETH_ALEN], uint8_t src[ETH_ALEN]);

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
void initItr(timedItr_t *const itr, const uint8_t total, const uint64_t now, const uint32_t freq)
{
  itr->idx = 0;
  itr->total = total;
  itr->last = now;
  itr->period = TIM_1000_MS;

  // set the iteration period
  if (freq && itr->total) {
    itr->period /=(freq * itr->total); // for 30Hz it's 33312 us (30.0192 Hz)

    itr->ttl = TIM_100_MS/TIM_1_MS + 1; // TTL value = 101 milliseconds

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
void resetItr(timedItr_t* itr, const uint64_t now)
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
 * \ret count   Number of sent messages
 **/
uint32_t sendMpsMsgBlock(size_t len, timedItr_t* itr, uint64_t evtId)
{
  uint32_t count = 0;
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
    deadline   = now + FBAS_AHEAD_TIME;
    deadlineHi = (uint32_t)((deadline >> 32) & 0xffffffff);
    deadlineLo = (uint32_t)(deadline         & 0xffffffff);

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
    ++count;
  }

  return count;
}

/**
 * \brief Send MPS messages periodically
 *
 * MPS flags are sent at specified period. [MPS_FS_530]
 *
 * \param itr   Read-access iterator that specifies next MPS message to send
 * \param evtid Event ID used to send a timing message
 *
 * \ret count   Number of sent messages
 **/
uint32_t msgSendPeriodicMps(timedItr_t* itr, const uint64_t evtid)
{
  uint32_t count = 0;
  uint32_t tef = 0;
  uint64_t deadline = itr->last + itr->period;
  uint64_t now = getSysTime();

  if (!itr->last)
    deadline = now;       // initial transmission

  // send next MPS message if deadline is over
  if (deadline <= now) {
    // send MPS message with ahead timestamp
    deadline = now + FBAS_AHEAD_TIME;
    if (fwlib_ebmWriteTM(deadline, evtid, bufMpsMsg[itr->idx].param, tef, 1) == COMMON_STATUS_OK)
      ++count;

    // update iterator with deadline
    resetItr(itr, now);
  }

  return count;
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
 * \ret count   Number of sent messages
 **/
uint32_t msgSendSpecificMps(const timedItr_t* itr, mpsMsg_t *const buf, const uint64_t evtid, const uint8_t extra)
{
  uint32_t count = 0;
  uint32_t tef = 0;
  uint64_t now = getSysTime();

  if (itr->last >= now) // delayed by a new cycle
    return count;

  // send a specified MPS event with ahead timestamp
  uint64_t deadline = now + FBAS_AHEAD_TIME;
  if (fwlib_ebmWriteTM(deadline, evtid, buf->param, tef, 1) == COMMON_STATUS_OK)
    ++count;

  // NOK flag shall be sent as extra events
  if (buf->prot.flag == MPS_FLAG_NOK) {
    for (uint8_t i = 0; i < extra; ++i) {
      if (fwlib_ebmWriteTM(deadline, evtid, buf->param, tef, 1) == COMMON_STATUS_OK)
        ++count;
    }
  }

  return count;
}

/**
 * \brief fetch MPS event
 *
 * MPS event is fetched from ECA and stored in the dedicated buffer.
 *
 * \param idx Base index for TX node
 * \param evt Raw event data (bits 63-16 = node ID, 15-8 = channel, 7-0 = flag)
 *
 * \return Pointer to the message buffer with the MPS event/flag
 **/
mpsMsg_t* msgFetchMps(const uint8_t idx, uint64_t evt)
{
  // evaluate MPS channel and MPS flag
  uint8_t ch = evt >> 8;
  uint8_t flag = evt;

  // store MPS channel and MPS flag
  headBufMps->prot.idx = idx + ch;
  headBufMps->prot.flag = flag;
  return headBufMps;
}

/**
 * \brief store recieved MPS message
 *
 * Store a received MPS message only if its timestamp is actual.
 * The reason is that the NOK flag is transmitted 3 times
 * with the same timestamp.
 *
 * \param raw Raw MPS protocol (bits 63-16 = node ID, 15-8 = index, 7-0 = flag)
 * \param ts  Timestamp of the MPS protocol
 * \param itr Read-access iterator
 *
 * \return Offset to the MPS msg buffer on reception of an actual MPS msg,
 * or N_MPS_CHANNELS on reception of a repeated MPS msg, otherwise negative integer.
 **/
int msgStoreMpsMsg(const uint64_t *raw, const uint64_t *ts, const timedItr_t* itr)
{
  uint8_t idx, flag;

  for (int i = 0; i < N_MPS_CHANNELS; ++i) {
    // node ID match
    if (!memcmp(raw, (headBufMps+i)->prot.addr, ETH_ALEN)) {
      idx = (uint8_t)(*raw >> 8);
      // MPS channel match
      if ((headBufMps+i)->prot.idx == idx) {
        // new MPS msg
        if (*ts != (headBufMps+i)->tsRx) {
          flag = (uint8_t)*raw;
          (headBufMps+i)->pending = (headBufMps+i)->prot.flag ^ flag;
          (headBufMps+i)->prot.flag = flag;
          (headBufMps+i)->ttl = itr->ttl;
          (headBufMps+i)->tsRx = *ts;
        }
        else {
          // repeated MPS msg
          return N_MPS_CHANNELS;
        }
        return i;
      }
    }
  }

  return -1;
}

/**
 * \brief Evaluate the lifetime of received MPS protocols [MPS_FS_600]
 *
 * \param idx Index of the MPS protocol
 *
 * \ret   ptr Pointer to expired MPS protocol
 **/
mpsMsg_t* evalMpsMsgTtl(uint64_t now, int idx) {
  mpsMsg_t* buf = 0;

  if (bufMpsMsg[idx].ttl) {
    --bufMpsMsg[idx].ttl;

    if (!bufMpsMsg[idx].ttl) {
      bufMpsMsg[idx].prot.flag = MPS_FLAG_NOK;
      buf = &bufMpsMsg[idx];
    }
  }

  return buf;
}

/**
 * \brief Initialize MPS message buffer
 *
 * \param id Pointer to node ID (MAC address)
 *
 * \return none
*/
void msgInitMpsMsg(const uint64_t *id)
{
  uint8_t *mac = (uint8_t *)id;
  mac+=2;

  for (int i = 0; i < N_MPS_CHANNELS; ++i)
  {
    msgResetMpsBuf(i, mac, MPS_FLAG_TEST);
    DBPRINT1("%x: mac=%x:%x:%x:%x:%x:%x idx=%x flag=%x @0x%8p\n",
             i, bufMpsMsg[i].prot.addr[0], bufMpsMsg[i].prot.addr[1], bufMpsMsg[i].prot.addr[2],
             bufMpsMsg[i].prot.addr[3], bufMpsMsg[i].prot.addr[4], bufMpsMsg[i].prot.addr[5],
             bufMpsMsg[i].prot.idx, bufMpsMsg[i].prot.flag, &bufMpsMsg[i]);
  }
}

/**
 * \brief Force input virtually to high
 *
 * It is used to set the CMOS input virtually to high voltage in TX [MPS_FS_620] or
 * set effective logic input to HIGH bit in RX [MPS_FS_630].
 *
 * \param buf Pointer to MPS message buffer
 *
 **/
void msgForceHigh(mpsMsg_t *const buf)
{
  uint8_t flag = MPS_FLAG_OK;

  for (int i = 0; i < N_MAX_TX_NODES; ++i) {
    (buf + i)->pending = (buf + i)->prot.flag ^ flag;
    (buf + i)->prot.flag  = flag;
    (buf + i)->ttl = 0;
    (buf + i)->tsRx = 0;
  }
}

/**
 * \brief reset MPS message buffer
 *
 * \param idx  Index of the MPS message buffer
 * \param flag MPS flag
 *
 * \return None
 *
 **/
void msgResetMpsBuf(const uint8_t idx, const uint8_t *pId, const uint8_t flag)
{
  if (pId)
    memcpy(bufMpsMsg[idx].prot.addr, pId, ETH_ALEN);
  else
    memset(bufMpsMsg[idx].prot.addr, 0, ETH_ALEN);

  bufMpsMsg[idx].prot.flag = flag;
  bufMpsMsg[idx].prot.idx = 0;
  bufMpsMsg[idx].ttl = 0;
  bufMpsMsg[idx].tsRx = 0;
}

/**
 * \brief Update the node ID array and MPS message buffer
 *
 * \param pId  Pointer to the full node ID (idx + reserved + MAC address)
 *
 * \return None
 **/
void msgUpdateMpsBuf(const uint64_t *pId)
{
  uint8_t idx = (uint8_t)(*pId >> 56);  // index
  uint8_t *id = (uint8_t*)pId;          // point to sender node ID (lower 6 bytes)
  id+=2;

  // if the same ID exists, remove it from the node ID array and MPS message buffer
  for (int i = 0; i < N_MAX_TX_NODES; ++i) {
    if (!(memcmp(&nodeIds[i][0], id, ETH_ALEN))) {
      memset(&nodeIds[i][0], 0, ETH_ALEN);
    }

    if (!(memcmp(bufMpsMsg[i].prot.addr, id, ETH_ALEN))) {
      msgResetMpsBuf(i, 0, MPS_FLAG_TEST);
    }
  }

  // update the node ID array and MPS message buffer
  memcpy(&nodeIds[idx][0], id, ETH_ALEN);

  msgResetMpsBuf(idx, id, MPS_FLAG_OK);
  bufMpsMsg[idx].prot.idx = idx;

  // node ID array and MPS message buffer must match
  if (!(memcmp(&nodeIds[idx][0], &bufMpsMsg[idx].prot.addr[0], ETH_ALEN))) {
    DBPRINT1("tmessage: sender %x: ", idx);
    for (int i = 0; i < ETH_ALEN; i++)
      DBPRINT1("%02x", bufMpsMsg[idx].prot.addr[i]);
    DBPRINT1(" (id: %016llx)\n", *pId);
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
static int addr_equal(uint8_t a[ETH_ALEN], uint8_t b[ETH_ALEN])
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
static uint8_t *addr_copy(uint8_t dst[ETH_ALEN], uint8_t src[ETH_ALEN])
{
  return memcpy(dst, src, ETH_ALEN);
}

/**
 * \brief Send the node registration request/response
 *
 * TX nodes send the registration request (in form of the MPS protocol) to
 * register them to the designated RX node. The transmission type should be broadcast.
 *
 * RX node responds with a special MPS message on reception of the registration
 * request from the TX nodes.
 *
 * The info field contains additional information regarding the MPS channels:
 * - TX node delivers the number of the MPS channels that are managed by itself
 * - RX node informs the position index that is reserved to a target TX node
 *
 * \param id   Node ID
 * \param cmd  Registration command
 * \param info Additional information (TX: number of MPS channels, RX: position index)
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t msgRegisterNode(const uint64_t id, const regCmd_t cmd, const uint8_t info)
{
  uint32_t tef = 0;
  uint32_t forceLate = 1;
  uint64_t param = (id << 16) | (cmd << 8) | info;
  uint64_t deadline = getSysTime() + FBAS_AHEAD_TIME;

  status_t status = fwlib_ebmWriteTM(deadline, FBAS_REG_EID, param, tef, forceLate);
  if (status != COMMON_STATUS_OK)
    DBPRINT1("Err - failed to send reg.rsp!\n");

  return status;
}

/**
 * \brief Get the index of the given sender node ID
 *
 * An array of TX node IDs is provided to the RX node during setup.
 * This function searches the ID of a given sender node in
 * that array and returns its index if the ID is found.
 *
 * \param pId   Pointer to the sender node ID (MAC address)
 *
 * \return  Returns the list index, otherwise negative value
 **/
int8_t msgGetNodeIndex(const uint64_t *pId)
{
  uint8_t *p = (uint8_t*)pId; // lower 6 bytes hold the sender ID
  p+=2;                       // seek the start of the sender ID

  int i = 0;
  int unknown = true;

  while (unknown && i < N_MAX_TX_NODES) {
    unknown = memcmp(&nodeIds[i][0], p, ETH_ALEN);
    if (unknown)
      DBPRINT3("cmp: %x%x%x%x%x%x - %x%x%x%x%x%x\n",
        nodeIds[i][0], nodeIds[i][1], nodeIds[i][2],
        nodeIds[i][3], nodeIds[i][4], nodeIds[i][5],
        *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5));
    ++i;
  }

  if (unknown)
    return -1;
  else
    return --i;
}

/**
 * \brief Print the MPS message buffer
 *
 * MPS message buffer contains MPS protocols
 *
 **/
void ioPrintMpsBuf(void)
{
  DBPRINT2("bufMpsMsg\n");
  DBPRINT2("buf_idx: protocol (MAC - idx - flag), msg (tsRx - ttl - pending)\n");

  for (int i = 0; i < N_MPS_CHANNELS; ++i)
     DBPRINT2("%x: %02x%02x%02x%02x%02x%02x - %x - %x, %llx - %x - %x\n",
        i,
        bufMpsMsg[i].prot.addr[0], bufMpsMsg[i].prot.addr[1],
        bufMpsMsg[i].prot.addr[2], bufMpsMsg[i].prot.addr[3],
        bufMpsMsg[i].prot.addr[4], bufMpsMsg[i].prot.addr[5],
        bufMpsMsg[i].prot.idx,
        bufMpsMsg[i].prot.flag,
        bufMpsMsg[i].tsRx,
        bufMpsMsg[i].ttl,
        bufMpsMsg[i].pending);
}

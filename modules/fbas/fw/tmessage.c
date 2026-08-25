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
uint8_t    senders[N_MAX_TX_NODES][ETH_ALEN];   // sender list
mpsMsg_t   bufMpsMsg[N_MAX_MPS_CHANNELS];       // buffer for the C2 message
mpsMsg_t *const bufPcEvent = &bufMpsMsg[0];     // head of the C2 message buffer (used as the PC event buffer for TX node)
msgCtrl_t  mpsMsgCtrl;                          // C2 messaging control structure
const uint32_t txMsgRates[N_TX_RATES] = {       // TX messaging rates, [us]
              33333, 100000, 80000, 50000,      // 30, 10, 12.5, 20 [Hz]
              20000, 10000, 5000, 2000,         // 50, 100, 200, 500 [Hz]
              1000, 500, 200, 100};             // 1000, 2000, 5000, 10000 [Hz]

static int addr_equal(uint8_t a[ETH_ALEN], uint8_t b[ETH_ALEN]); // wr-switch-sw/userspace/libwr
static uint8_t *addr_copy(uint8_t dst[ETH_ALEN], uint8_t src[ETH_ALEN]);

/**
 * \brief Initialize the C2 messaging controller
 *
 * Control structure for messaging the C2 (class 2) protocol periodically.
 * Emitter nodes use it.
 *
 * \param ctrl  Pointer to the C2 messaging controller
 * \param total Total number of the MPS channels
 * \param now   Timestamp of access
 * \param period Messaging period, [us]
 *
 * \return None
 **/
void msgInitMsgCtrl(msgCtrl_t *const ctrl, const uint8_t total, const uint64_t now, const uint32_t period)
{
  ctrl->total = total;
  ctrl->last = now;
  ctrl->period = txMsgRates[0];          // default period of 33,3 ms (30 Hz)

  // set the iteration period
  if (period && ctrl->total) {
    ctrl->period = period * 1000;        // us -> ns

    ctrl->ttl = TIM_100_MS/TIM_1_MS + 1; // TTL value = 101 milliseconds
  }
}

/**
 * \brief Send stored the PC flag
 *
 * Send the PC (Power Converter) flag stored in the PC event buffer
 *
 * \param ctrl  Pointer to the C2 messaging controller
 * \param evtId Event ID for timing messages
 *
 * \return Number of sent messages
 **/
uint32_t msgSendPcFlag(msgCtrl_t* ctrl, uint64_t evtId)
{
  uint32_t count = 0;
  uint32_t res, tef;                // temporary variables for bit shifting etc
  uint32_t deadlineLo, deadlineHi;
  uint32_t idLo, idHi;
  uint32_t paramLo, paramHi;

  uint64_t now = getSysTime();
  uint64_t deadline = ctrl->last + ctrl->period;

  if (!ctrl->last)
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

    atomic_on();

    paramHi  = (uint32_t)((bufPcEvent->param >> 32) & 0xffffffff);
    paramLo  = (uint32_t)(bufPcEvent->param         & 0xffffffff);

    // build a timing message
    ebm_op(COMMON_ECA_ADDRESS, idHi,       EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, idLo,       EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, paramHi,    EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, paramLo,    EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, tef,        EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, res,        EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, deadlineHi, EBM_WRITE);
    ebm_op(COMMON_ECA_ADDRESS, deadlineLo, EBM_WRITE);

    atomic_off();

    ++count;

    // send timing messages
    ebm_flush();

    // update the messaging controller
    ctrl->last = now;
  }

  return count;
}

/**
 * \brief Send the PC event
 *
 * Send an PC (Power Converter) event immediatelly.
 *
 * Upon flag change to NOK, there shall be 2 extra events within 50 us. [MPS_FS_530]
 * In case of new cycle, do not send any PC event. [MPS_FS_630]
 *
 * \param ctrl  Pointer to the messaging controller
 * \param buf   Location of the PC event buffer
 * \param evtid Event ID for a timing message
 * \param extra Number of extra messages
 *
 * \return   Number of sent messages
 **/
uint32_t msgSendPcEvent(const msgCtrl_t* ctrl, mpsMsg_t *const buf, const uint64_t evtid, const uint8_t extra)
{
  uint32_t count = 0;
  uint32_t tef = 0;
  uint64_t now = getSysTime();

  if (ctrl->last >= now) // delayed by a new cycle
    return count;

  // send a specified PC event with ahead timestamp
  uint64_t deadline = buf->tsRx + FBAS_AHEAD_TIME;
  if (fwlib_ebmWriteTM(deadline, evtid, buf->param, tef, 1) == COMMON_STATUS_OK)
    ++count;

  // NOK flag shall be sent as burst
  if (buf->prot.flag == MPS_FLAG_NOK) {
    for (uint8_t i = 0; i < extra; ++i) {
      if (fwlib_ebmWriteTM(deadline, evtid, buf->param, tef, 1) == COMMON_STATUS_OK)
        ++count;
    }
  }

  return count;
}

/**
 * \brief Store the fetched PC event
 *
 * Store the PC (Power Converter) event fetched from ECA in the dedicated buffer.
 *
 * \param evt Raw ECA data (bits 63-16 = event ID, 0 = flag)
 * \param ts  Timestamp
 *
 * \return Pointer to the PC event buffer
 **/
mpsMsg_t* msgStorePcEvent(const uint64_t evt, const uint64_t ts)
{
  // parse the PC flag
  uint8_t flag = (uint8_t)evt;

  // PC events simulated by TLU can only have values 1 and 0, therefore
  // map these values into the valid PC flags: 0->OK, 1->NOK, other->TEST
  switch ((uint8_t)evt) {
    case MPS_FLAG_OK :  flag = MPS_FLAG_OK;   break;
    case MPS_FLAG_NOK:  flag = MPS_FLAG_NOK;  break;
    default:            flag = MPS_FLAG_TEST;
  }

  // keep the PC flag and timestamp
  bufPcEvent->prot.flag = flag;
  bufPcEvent->tsRx = ts;

  // return the message buffer
  return bufPcEvent;
}

/**
 * \brief Store the recieved C2 message
 *
 * Store a received C2 message only if its timestamp is actual.
 * The reason is that the NOK flag is transmitted 3 times with the same timestamp.
 *
 * \param raw Raw C2 protocol data (bits 63-16 = node ID, 15-8 = bic_id, 7-4 = ch_id, 3-0 = flag)
 * \param ts  Timestamp of the C2 message
 * \param ctrl Read-access iterator
 *
 * \return Channel ID of a sender node on reception of the new/actual C2 message,
 * or N_MAX_MPS_CHANNELS on reception of a repeated C2 message, otherwise negative integer.
 **/
int msgStoreMpsMsg(const uint64_t *raw, const uint64_t *ts, const msgCtrl_t* ctrl)
{
  uint8_t ch_id  = (uint8_t)(*raw >> 4) & CH_MSK;    // channel ID of the sender
  uint8_t flag = (uint8_t)*raw & FLAG_MSK;

  if (ch_id >= N_MAX_MPS_CHANNELS)
    return -1;

  // sender ID match
  if (!memcmp(raw, (bufMpsMsg+ch_id)->prot.addr, ETH_ALEN)) {
    // node channel match
    if ((bufMpsMsg+ch_id)->prot.ch_id == ch_id) {
      // actual C2 protocol
      if (*ts != (bufMpsMsg+ch_id)->tsRx) {
        (bufMpsMsg+ch_id)->pending = (bufMpsMsg+ch_id)->prot.flag ^ flag;
        (bufMpsMsg+ch_id)->prot.flag = flag;
        (bufMpsMsg+ch_id)->ttl = ctrl->ttl;
        (bufMpsMsg+ch_id)->tsRx = *ts;
      }
      else {
        // or repeated C2 protocol
        return N_MAX_MPS_CHANNELS;
      }
      return ch_id;
    }
  }

  return -1;
}

/**
 * \brief Evaluate the lifetime of received C2 messages [MPS_FS_600]
 *
 * \param idx Index of the C2 message buffer
 *
 * \ret   ptr Pointer to expired C2 message buffer
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
 * \brief Initialize the C2 message buffer
 *
 * \param id Pointer to the sender ID (MAC address)
 *
 * \return None
*/
void msgInitMpsMsgBuf(const uint64_t *id)
{
  uint8_t *mac = (uint8_t *)id;
  mac+=2;                         // lower 6-byte is MAC address

  for (int i = 0; i < N_MAX_MPS_CHANNELS; ++i)
  {
    msgResetMpsBuf(i, mac, MPS_FLAG_TEST);
    DBPRINT1("%x: mac=%x:%x:%x:%x:%x:%x bic_id=%x ch_id=%x flag=%x @0x%8p\n",
             i, bufMpsMsg[i].prot.addr[0], bufMpsMsg[i].prot.addr[1], bufMpsMsg[i].prot.addr[2],
             bufMpsMsg[i].prot.addr[3], bufMpsMsg[i].prot.addr[4], bufMpsMsg[i].prot.addr[5],
             bufMpsMsg[i].prot.bic_id, bufMpsMsg[i].prot.ch_id, bufMpsMsg[i].prot.flag, &bufMpsMsg[i]);
  }
}

/**
 * \brief Initialize the PC event buffer
 *
 * \param id    Pointer to the sender ID (MAC address)
 * \param ch_id PC event channel
 *
 * \return None
*/
void msgInitPcEventBuf(const uint64_t *id, uint8_t ch_id)
{
  uint8_t *mac = (uint8_t *)id;
  mac+=2;                         // lower 6-byte is MAC address
  memcpy(bufPcEvent->prot.addr, mac, ETH_ALEN);

  bufPcEvent->prot.flag = MPS_FLAG_TEST;
  bufPcEvent->prot.bic_id = 0;
  bufPcEvent->prot.ch_id = ch_id;
  bufPcEvent->ttl = 0;
  bufPcEvent->tsRx = 0;
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
 * \brief Reset the C2 message buffer
 *
 * \param ch_id Channel ID of a sender node
 * \param pId   Pointer to the sender ID (MAC address)
 * \param flag  PC flag
 *
 * \return None
 *
 **/
void msgResetMpsBuf(const uint8_t ch_id, const uint8_t *pId, const uint8_t flag)
{
  if (pId)
    memcpy(bufMpsMsg[ch_id].prot.addr, pId, ETH_ALEN);
  else
    memset(bufMpsMsg[ch_id].prot.addr, 0, ETH_ALEN);

  bufMpsMsg[ch_id].prot.flag = flag;
  bufMpsMsg[ch_id].prot.bic_id = 0;
  bufMpsMsg[ch_id].prot.ch_id = ch_id;
  bufMpsMsg[ch_id].ttl = 0;
  bufMpsMsg[ch_id].tsRx = 0;
}

/**
 * \brief Update the sender ID array and C2 message buffer.
 *
 * Update the sender ID array and C2 message buffer, when
 * a valid node identification provided by user.
 *
 * \param pId  Pointer to the shared memory location,
 * which holds user input of a valid node (ch_id + reserved + MAC address)
 *
 * \return None
 **/
void msgUpdateMpsBuf(const uint64_t *pId)
{
  uint8_t ch_id = (uint8_t)(*pId >> 56);  // channel ID (for senders[], 0..15)
  uint8_t *node_id = (uint8_t*)pId;       // point to sender ID (lower 6 bytes)
  node_id+=2;
  uint8_t offset;                         // offset to the C2 message buffer

  // if the same sender ID already exists, then remove it
  for (int i = 0; i < N_MAX_TX_NODES; ++i) {
    if (!(memcmp(senders[i], node_id, ETH_ALEN))) {
      memset(senders[i], 0, ETH_ALEN);
    }

    if (!(memcmp(bufMpsMsg[i].prot.addr, node_id, ETH_ALEN))) {
        msgResetMpsBuf(i, 0, MPS_FLAG_TEST);
    }
  }

  // update the sender ID array and C2 message buffer
  memcpy(senders[ch_id], node_id, ETH_ALEN);
  msgResetMpsBuf(ch_id, node_id, MPS_FLAG_OK);
  bufMpsMsg[ch_id].prot.ch_id = ch_id;

  // print node ID array index and MPS message buffer content
  DBPRINT1("sender: ch_id=%x: ", ch_id);
  for (int i = 0; i < ETH_ALEN; i++)
    DBPRINT1("%02x", bufMpsMsg[ch_id].prot.addr[i]);

  // node ID array and MPS message buffer must match
  if (memcmp(senders[ch_id], bufMpsMsg[ch_id].prot.addr, ETH_ALEN)) {
    // mismatch
    DBPRINT1(" ! ");
  } else {
    // match
    DBPRINT1(" = ", *pId);
  }

  // valid node identification in the shared memory (provided by user)
  DBPRINT1("(id: %016llx)\n", *pId);
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
 * Emitter nodes send the registration request, where bic_id, ch_id and flag are set
 * to their maximum value (0xF, 0xFF). The transmission type should be broadcast.
 *
 * Collector node responds with its bic_id and emitter's channel ID. The transmission
 * should be unicast.
 *
 *
 * \param node_id  Node ID
 * \param bic_id   BIC ID
 * \param ch_id    C2 channel ID
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t msgRegisterNode(const uint64_t node_id, const uint8_t bic_id, const uint8_t ch_id, const uint8_t flag)
{
  uint32_t tef = 0;
  uint32_t forceLate = 1;
  uint64_t param = (node_id << 16) | (bic_id << 8) | (ch_id << 4) | flag;
  uint64_t deadline = getSysTime() + FBAS_AHEAD_TIME;

  status_t status = fwlib_ebmWriteTM(deadline, FBAS_REG_EID, param, tef, forceLate);
  if (status != COMMON_STATUS_OK)
    DBPRINT1("Err - failed to send reg.rsp!\n");

  return status;
}

/**
 * \brief Get the index of the given sender node
 *
 * An array of senders is provided to a collector node during setup.
 * This function searches the ID (MAC address) of a given sender node in
 * that array and returns its index if the ID is found.
 *
 * \param pId   Pointer to the sender ID (MAC address)
 *
 * \return  Returns the array index, otherwise negative value
 **/
int8_t msgGetSenderIndex(const uint64_t *pId)
{
  uint8_t *p = (uint8_t*)pId; // lower 6 bytes hold the sender ID
  p+=2;                       // seek the start of the sender ID

  int i = 0;
  int unknown = true;

  while (unknown && i < N_MAX_TX_NODES) {
    unknown = memcmp(&senders[i][0], p, ETH_ALEN);
    if (unknown)
      DBPRINT3("cmp: %x%x%x%x%x%x - %x%x%x%x%x%x\n",
        senders[i][0], senders[i][1], senders[i][2],
        senders[i][3], senders[i][4], senders[i][5],
        *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5));
    ++i;
  }

  if (unknown)
    return -1;
  else
    return --i;
}

/**
 * \brief Print the contents of the C2 message buffer
 *
 * Output the content of the C2 message buffer to console.
 *
 **/
void msgPrintMpsBuf(void)
{
  DBPRINT2("bufMpsMsg\n");
  DBPRINT2("offset: protocol (MAC - ch_id - flag), msg (tsRx - ttl - pending)\n");

  for (int i = 0; i < N_MAX_MPS_CHANNELS; ++i)
     DBPRINT2("%x: %02x%02x%02x%02x%02x%02x - %x - %x, %llx - %x - %x\n",
        i,
        bufMpsMsg[i].prot.addr[0], bufMpsMsg[i].prot.addr[1],
        bufMpsMsg[i].prot.addr[2], bufMpsMsg[i].prot.addr[3],
        bufMpsMsg[i].prot.addr[4], bufMpsMsg[i].prot.addr[5],
        bufMpsMsg[i].prot.ch_id,
        bufMpsMsg[i].prot.flag,
        bufMpsMsg[i].tsRx,
        bufMpsMsg[i].ttl,
        bufMpsMsg[i].pending);
}

/**
 * \brief Build a bit-wise representation of the PC flags
 *
 * Build a simple data representing the current PC (Power Converter) flags.
 * PC flags are stored in the C2 message buffer.
 * Each bit represents a PC flag from each emitter:
 * - bit 0 corresponds to emitter 1
 * - logic 1 = NOK, logic 0 = OK
 *
 * Up to 16 emitters are supported, then lower 16 bits are
 * effectively present the PC flags.
 *
 * \return Returns data representing the PC flags
 *
 **/
uint32_t msgRepresentMpsFlags(void)
{
  int i, j, step = 1;
  uint32_t flags = 0;

  for (i = 0, j = 0; i < N_MAX_TX_NODES; i++, j++) {
    if (bufMpsMsg[i].prot.flag == MPS_FLAG_NOK)
      flags|= (1 << j);
  }

  return flags;
}

/********************************************************************************************
 *  ioctl.c
 *
 *  created : 2021
 *  author  : Enkhbold Ochirsuren, GSI-Darmstadt
 *  version : 05-June-2021
 *
 *  Functions to control the IO ports
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
 * Last update: 05-June-2021
 ********************************************************************************************/

#include "ioctl.h"

volatile uint32_t *pIOCtrl;             // WB address of IO Control
io_port_t EffLogOut[N_MPS_CHANNELS];    // mapping between MPS message buffer and IO output port

/**
 * \brief Set IO output enable
 *
 * \param channel Output channel type (GPIO, LVDS)
 * \param idx     Output channel index (0..31)
 * \param val     Value to enable/disable (true/false) output
 *
 * \ret   status  COMMON_STATUS_OK on success, otherwise ERROR
 **/
status_t setIoOe(uint32_t channel, uint32_t idx, bool val)
{
  uint32_t reg = 0;
  if (channel == IO_CFG_CHANNEL_GPIO)      // GPIO channel
    reg = val ? IO_GPIO_OE_SETLOW : IO_GPIO_OE_RESETLOW;
  else if (channel == IO_CFG_CHANNEL_LVDS) // LVDS channel
    reg = val ? IO_LVDS_OE_SETLOW : IO_LVDS_OE_RESETLOW;
  else
    return COMMON_STATUS_ERROR;

  if (idx > 31)
    return COMMON_STATUS_ERROR;

  if (reg)
    *(pIOCtrl + (reg >> 2)) = (1 << idx);

  return COMMON_STATUS_OK;
}

// get IO output enable
uint32_t getIoOe(uint32_t channel)
{
  uint32_t reg = 0;
  if (channel == IO_CFG_CHANNEL_GPIO) // GPIO channel
    reg = IO_GPIO_OE_SETLOW;
  else if (channel == IO_CFG_CHANNEL_LVDS) // LVDS channel
    reg = IO_LVDS_OE_SETLOW;
  else
    return 0xFFFF;

  if (reg)
    return *(pIOCtrl + (reg >> 2));
}

/**
 * \brief Drive the chosen output port
 *
 * \param channel  Output channel type
 * \param idx      Output channel index
 * \param value    Logic value for high/low signal
 *
 * \ret   none
 **/
void driveOutPort(uint32_t channel, uint8_t idx, uint8_t value)
{
  uint32_t reg = 0;
  uint32_t outVal = 0;

  if (value == MPS_SIGNAL_INVALID)
    return;

  if (channel == IO_CFG_CHANNEL_GPIO) { // GPIO channel
    reg = IO_GPIO_SET_OUTBEGIN;
    if (value)
      outVal = 0x01;
  } else if (channel == IO_CFG_CHANNEL_LVDS) { // LVDS channel
    reg = IO_LVDS_SET_OUTBEGIN;
    if (value)
      outVal = 0xff;
  }
  else
    return;

  if (reg)
    *(pIOCtrl + (reg >> 2) + idx) = outVal;
}

/**
 * \brief Drive the effective logic output [MPS_FS_640]
 *
 * Drive internal signal based on MPS flag:
 * - high if MPS flag is OK
 * - low if MPS flag is NOK or TEST
 *
 * Generate error (internal signal), if lifetime of MPS flag is expired.
 *
 * \param buf Pointer to MPS message buffer
 *
 * \ret none
 **/
void driveEffLogOut(mpsMsg_t* buf)
{
  uint8_t ioVal = MPS_SIGNAL_INVALID;

  // handle MPS flag if it's changed or expired
  if (buf->pending) {
    buf->pending = 0;
    DBPRINT3("pend: %x %x %x\n", buf->prot.addr[0], buf->prot.idx, buf->prot.flag);
    if (buf->prot.flag == MPS_FLAG_OK)
      ioVal = MPS_SIGNAL_HIGH;
    else
      ioVal = MPS_SIGNAL_LOW;
  } else if (!buf->ttl) {
    ioVal = MPS_SIGNAL_LOW;
    DBPRINT3("ttl: %x %x %x\n", buf->prot.addr[0], buf->prot.idx, buf->prot.flag);
  }

  io_port_t out_port;
  if (getEffLogOut(buf->prot.idx, &out_port) == COMMON_STATUS_OK)
    driveOutPort(out_port.type, out_port.idx, ioVal);     // drive the assigned output port
}

/**
 * \brief Set up the output port to the MPS message buffer
 *
 * For testing purpose, the direct mapping of the MPS buffer index and
 * output port is set up.
 * For example, this mapping can be used to measure the MPS signaling latency
 * for multiple TX nodes.
 *
 * \param buf_idx  MPS message buffer index
 * \param ch_type  Channel type (of output port)
 * \param ch_idx   Channel index (of output port)
 *
 * \ret none
 *
 **/
void setupEffLogOut(uint8_t buf_idx, uint32_t ch_type, uint8_t ch_idx)
{
  if (buf_idx > N_MPS_CHANNELS)
    return;

  if ((ch_type != IO_CFG_CHANNEL_GPIO) &&
      (ch_type != IO_CFG_CHANNEL_LVDS))
    return;

  if ((ch_type == IO_CFG_CHANNEL_GPIO) &&
      (ch_idx > N_LEMO_OUT_SCU))
    return;

  if ((ch_type == IO_CFG_CHANNEL_LVDS) &&
      (ch_idx > N_LEMO_OUT_PEXARIA))
    return;

  EffLogOut[buf_idx].type = ch_type;
  EffLogOut[buf_idx].idx  = ch_idx;
}

/**
 * \brief Get the output port of the MPS message buffer
 *
 * \param buf_idx  MPS message buffer index
 * \param port     Pointer to structure with port channel type and index (assigned to the given MPS message buffer)
 *
 * \ret   status   OK for success, otherwise ERROR
 *
 **/
status_t getEffLogOut(uint8_t buf_idx, io_port_t* port)
{
  if (buf_idx > N_MPS_CHANNELS)
    return COMMON_STATUS_ERROR;

  switch (EffLogOut[buf_idx].type) {
    case IO_CFG_CHANNEL_GPIO:
    case IO_CFG_CHANNEL_LVDS:
      port->type = EffLogOut[buf_idx].type;
      port->idx = EffLogOut[buf_idx].idx;
      return COMMON_STATUS_OK;
    default:
      break;
  }

  return COMMON_STATUS_ERROR;
}

void qualifyInput(size_t len, mpsMsg_t* buf) {
}

void testOutput(size_t len, mpsMsg_t* buf) {
}

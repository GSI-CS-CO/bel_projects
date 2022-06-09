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

// set IO output enable
uint32_t setIoOe(uint32_t channel, uint32_t idx)
{
  uint32_t reg = 0;
  if (channel == IO_CFG_CHANNEL_GPIO) // GPIO channel
    reg = IO_GPIO_OE_SETLOW;
  else if (channel == IO_CFG_CHANNEL_LVDS) // LVDS channel
    reg = IO_LVDS_OE_SETLOW;
  else
    return 0xFFFF;

  if (reg)
    *(pIOCtrl + (reg >> 2)) = (1 << idx);
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

// toggle IO output
void driveIo(uint32_t channel, uint32_t idx, uint8_t value)
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
void driveEffLogOut(uint32_t channel, mpsMsg_t* buf)
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

  driveIo(channel, 0, ioVal); // drive the IO1 (B1) port
}

void qualifyInput(size_t len, mpsMsg_t* buf) {
}

void testOutput(size_t len, mpsMsg_t* buf) {
}

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

out_port_config_t outPortCfg =
  {IO_CFG_CHANNEL_GPIO, N_OUT_LEMO_SCU};    // default output port configuration for SCU3
static io_port_t EffLogOut[N_MAX_TX_NODES]; // mapping between MPS message buffer and IO output port

status_t findOutPort(const uint8_t bufIdx, io_port_t* port);

/**
 * \brief Init IO port map
 *
 * \return None
*/
void ioInitPortMap(void)
{
  for (uint8_t i = 0; i < N_MAX_TX_NODES; ++i)
    EffLogOut[i].idx = N_MAX_TX_NODES;
}

/**
 * \brief Set port 'output enable'
 *
 * \param index   Output port index (0..31)
 * \param enable  Control value (enable=true, disable=false)
 *
 * \return  COMMON_STATUS_OK on success, otherwise ERROR
 **/
status_t ioSetOutEnable(const uint8_t index, const bool enable)
{
  uint32_t reg = 0;

  if (index >= outPortCfg.total)
    return COMMON_STATUS_ERROR;

  if (outPortCfg.type == IO_CFG_CHANNEL_GPIO) // GPIO channel
    reg = enable ? IO_GPIO_OE_SETLOW : IO_GPIO_OE_RESETLOW;
  else if (outPortCfg.type == IO_CFG_CHANNEL_LVDS)  // LVDS channel
    reg = enable ? IO_LVDS_OE_SETLOW : IO_LVDS_OE_RESETLOW;
  else
    return COMMON_STATUS_ERROR;

  if (reg) {
    *(pIOCtrl + (reg >> 2)) = (1 << index);
    return COMMON_STATUS_OK;
  } else
    return COMMON_STATUS_ERROR;
}

/**
 * \brief Check if output enabled for the given port
 *
 * \param index  Port internal index
 * \param pReg   Pointer to the register value
 *
 * \return COMMON_STATUS_OK on success, otherwise COMMON_STATUS_ERROR
*/
status_t ioIsOutEnabled(const uint8_t index, uint32_t *pReg)
{
  uint32_t reg = 0;

  if (index >= outPortCfg.total)
    return COMMON_STATUS_ERROR;

  if (outPortCfg.type == IO_CFG_CHANNEL_GPIO)      // GPIO channel
    reg = IO_GPIO_OE_SETLOW;
  else if (outPortCfg.type == IO_CFG_CHANNEL_LVDS) // LVDS channel
    reg = IO_LVDS_OE_SETLOW;
  else
    return COMMON_STATUS_ERROR;

  if (reg) {
    *pReg = *(pIOCtrl + (reg >> 2)) & (1 << index);
    return COMMON_STATUS_OK;
  }
  else
    return COMMON_STATUS_ERROR;
}

/**
 * \brief Drive the chosen output port
 *
 * \param pOutPort  Pointer to output port
 * \param value     Logic value for high/low signal
 *
 * \return COMMON_STATUS_OK on success, otherwise COMMON_STATUS_ERROR
 **/
status_t driveOutPort(io_port_t *const pOutPort, const uint8_t value)
{
  uint32_t reg = 0;
  uint32_t outVal = 0;

  if (value == MPS_SIGNAL_INVALID)
    return COMMON_STATUS_ERROR;

  if (pOutPort->type == IO_CFG_CHANNEL_GPIO) {
    reg = IO_GPIO_SET_OUTBEGIN;
    if (value)
      outVal = 0x01;
  } else if (pOutPort->type == IO_CFG_CHANNEL_LVDS) {
    reg = IO_LVDS_SET_OUTBEGIN;
    if (value)
      outVal = 0xff;
  }
  else
    return COMMON_STATUS_ERROR;

  if (reg) {
    *(pIOCtrl + (reg >> 2) + pOutPort->idx) = outVal;
    return COMMON_STATUS_OK;
  } else
    return COMMON_STATUS_ERROR;
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
 * \param pBuf   Pointer to MPS message buffer
 * \param bufIdx Port map index
 *
 * \return COMMON_STATUS_OK on success, otherwise COMMON_STATUS_ERROR
 **/
status_t ioDriveOutput(mpsMsg_t *const pBuf, const uint8_t bufIdx)
{
  if (pBuf == 0)
    return COMMON_STATUS_ERROR;

  uint8_t ioVal = MPS_SIGNAL_INVALID;

  // handle MPS flag if it's changed or expired
  if (pBuf->pending) {
    pBuf->pending = 0;
    DBPRINT3("pend: %x %x %x\n", pBuf->prot.addr[0], pBuf->prot.idx, pBuf->prot.flag);
    if (pBuf->prot.flag == MPS_FLAG_OK)
      ioVal = MPS_SIGNAL_HIGH;
    else
      ioVal = MPS_SIGNAL_LOW;
  } else if (!pBuf->ttl) {
    ioVal = MPS_SIGNAL_LOW;
    DBPRINT3("ttl: %x %x %x\n", pBuf->prot.addr[0], pBuf->prot.idx, pBuf->prot.flag);
  }

  if (ioVal == MPS_SIGNAL_INVALID)
    return COMMON_STATUS_ERROR;

  io_port_t out_port;
  if (findOutPort(bufIdx, &out_port) == COMMON_STATUS_OK)
    return driveOutPort(&out_port, ioVal);
  else
    return COMMON_STATUS_ERROR;
}

/**
 * \brief Map the MPS msg buffer to the output port
 *
 * For testing purpose, the direct mapping of the MPS msg buffer index and
 * output port is set up.
 * For example, this mapping can be used to measure the MPS signaling latency
 * for multiple TX nodes.
 *
 * \param bufIdx  MPS message buffer index
 * \param portIdx Output port index
 *
 * \return COMMON_STATUS_OK on success, otherwise COMMON_STATUS_ERROR
 *
 **/
status_t ioMapOutput(const uint8_t bufIdx, const uint8_t portIdx)
{
  if (bufIdx >= N_MAX_TX_NODES)
    return COMMON_STATUS_ERROR;

  if (portIdx >= outPortCfg.total)
    return COMMON_STATUS_ERROR;

  EffLogOut[bufIdx].type = outPortCfg.type;
  EffLogOut[bufIdx].idx  = portIdx;
  return COMMON_STATUS_OK;
}

/**
 * \brief Find the output port mapped to the given MPS message buffer
 *
 * \param bufIdx  MPS message buffer index
 * \param port    Pointer to structure with port channel type and index (assigned to the given MPS message buffer)
 *
 * \return COMMON_STATUS_OK for success, otherwise COMMON_STATUS_ERROR
 *
 **/
status_t findOutPort(const uint8_t bufIdx, io_port_t* port)
{
  if (bufIdx > N_MAX_TX_NODES)
    return COMMON_STATUS_ERROR;

  switch (EffLogOut[bufIdx].type) {
    case IO_CFG_CHANNEL_GPIO:
    case IO_CFG_CHANNEL_LVDS:
      if (EffLogOut[bufIdx].idx < outPortCfg.total) {
        port->type = EffLogOut[bufIdx].type;
        port->idx = EffLogOut[bufIdx].idx;
        return COMMON_STATUS_OK;
      }
    default:
      break;
  }

  return COMMON_STATUS_ERROR;
}

/**
 * \brief Print the mapping between output and MPS message buffer
 *
 * \return None
 **/
void ioPrintPortMap(void)
{
  DBPRINT2("EffLogOut\n");
  DBPRINT2("buf_idx: IO port (type - idx)\n");

  for (int i = 0; i < N_MAX_TX_NODES; ++i)
    DBPRINT2("%x: %x - %x\n",
        i,
        EffLogOut[i].type,
        EffLogOut[i].idx);
}

void qualifyInput(size_t len, mpsMsg_t* buf) {
}

void testOutput(size_t len, mpsMsg_t* buf) {
}

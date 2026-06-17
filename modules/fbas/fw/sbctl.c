/********************************************************************************************
 *  sbctl.c
 *
 *  created : June 2026
 *  author  : Enkhbold Ochirsuren, GSI-Darmstadt
 *  version : 15 June 2026
 *
 *  SCU bus control sub-module
 *
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2018  Dietrich Beck
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: d.beck@gsi.de
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
 * For all questions and ideas contact: d.beck@gsi.de
 * Last update: 22-November-2018
 ********************************************************************************************/

#include <sbctl.h>

// global variables
volatile uint16_t *pSbMaster;           // pointer to the SCU bus master

// shared memory layout
uint32_t *pSharedSetSbSlaves;           // "user defined" u32 reg: SCU bus slaves (bit1=slot1)
uint32_t *pSharedGetSbSlaves;           // "user defined" u32 reg: SCU bus slaves (bit1=slot1)
uint32_t *pSharedGetSbStd;              // "user defined" u32 reg: standard registers of a SCU bus slave

// SCU bus specific variables
uint32_t sbSlaves = 0;                  // SCU bus slaves (bit1=slot1)
uint32_t sbDiobs = 0;                   // DIOB cards in SCU bus (bit1=slot1)
uint16_t configDiob[N_DIOB_CFG] = {0};  // configuration registers of DIOB
uint16_t statusDiob[N_DIOB_STS] = {0};  // status registers of DIOB
uint16_t configUser[N_USR_CFG] = {0};   // configuration registers of user interface (extension) card
uint16_t statusUser[N_USR_STS] = {0};   // status registers of user interface (extension) card
uint16_t outputUser[N_USR_OUT] = {0};   // output registers of user interface (extension) card
uint16_t inputUser[N_USR_IN] = {0};     // input registers of user interface (extension) card

enum regSetNum {
  DIOB_CFG = 0,   // configuration register set of DIOB
  DIOB_STS,
  USR_CFG,        // configuration register set of an user interface card
  USR_STS,
  USR_OUT,        // output register set of an user interface card
  USR_IN,
  N_REGSET
};

regset_t regSet[N_REGSET] = {
// user register base, offset, number of registers
  {STD_REG_BASE, DIOB_Config_Reg1, N_DIOB_CFG},
  {STD_REG_BASE, DIOB_Status_Reg1, N_DIOB_STS},
  {USR_REG_BASE, Usr_Config_Reg1,  N_USR_CFG},
  {USR_REG_BASE, Usr_Status_Reg1,  N_USR_STS},
  {USR_REG_BASE, Usr_Out_Reg1,     N_USR_OUT},
  {USR_REG_BASE, Usr_In_Reg1,      N_USR_IN},
};

status_t readSbSlaveReg(volatile uint16_t* pSlave, regset_t* regset, uint16_t *pData);
status_t writeSbSlaveReg(volatile uint16_t* pSlave, regset_t* regset, uint16_t *pData);
status_t probeSbSlaveExt(volatile uint16_t* pMaster, uint32_t slot, uint32_t* pDst);
status_t probeSbSlaves(volatile uint16_t* pMaster, uint16_t sysId, uint16_t grpId, uint32_t* slaves);
void exportSbSlaveConfig(volatile uint16_t* pMaster, const uint32_t sbSlaves);

/**
 * \brief Initialize the pointer to the SCU bus master
 *
 * Get the WB address of the SCU bus master.
 *
 * \param none
 *
 * \return none
 **/
void sbInit(void)
{
  // init the SCU bus master
  pSbMaster = (uint16_t*)fwlib_getSbMaster();

  // probe all DIOB cards
  probeSbSlaves(pSbMaster, CID_SYS_DIOB, CID_GRP_DIOB, &sbDiobs);
}

/**
 * \brief Initialize the user-defined registers
 *
 * Registers for the SCU bus control are located in the shared memory.
 *
 * \param pSharedApp  Pointer to app-specific shared memory range
 *
 * \return none
 **/
void sbInitSharedMemory(const uint32_t* pSharedApp)
{
  // locations in the shared memory for the SCU bus control
  pSharedSetSbSlaves = (uint32_t *)(pSharedApp + (FBAS_SHARED_SET_SBSLAVES >> 2));
  pSharedGetSbSlaves = (uint32_t *)(pSharedApp + (FBAS_SHARED_GET_SBSLAVES >> 2));
  pSharedGetSbStd = (uint32_t *)(pSharedApp + (FBAS_SHARED_GET_SBSTDBEGIN >> 2));
  DBPRINT2("sbctl: SHARED_SET_SBSLAVES 0x%08x\n", pSharedSetSbSlaves);
  DBPRINT2("sbctl: SHARED_GET_SBSLAVES 0x%08x\n", pSharedGetSbSlaves);
  DBPRINT2("sbctl: SHARED_GET_SBSTDBEGIN 0x%08x\n", pSharedGetSbStd);
}

/**
 * \brief Read the register set of a given SCU bus slave
 *
 * \param pSlave Pointer to a SCU bus slave
 * \param regset Pointer to a register set of the SCU bus slave
 * \param pData  Data buffer for the register set
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t readSbSlaveReg(volatile uint16_t* pSlave, regset_t* regset, uint16_t *pData)
{
  uint16_t i;

  if (!pSlave || !pData)
    return COMMON_STATUS_ERROR;

  for (i = 0; i < regset->len; ++i)
    *(pData + i) = *(pSlave + regset->base + regset->offset + i);

  DBPRINT3("sbctl: sb=%x, base=%x, off=%x, len=%x\n",
      pSlave, regset->base, regset->offset, regset->len);
  return COMMON_STATUS_OK;
}

/**
 * \brief Write data to the register set of a given SCU bus slave
 *
 * \param pSlave Pointer to a SCU bus slave
 * \param regset Pointer to a register set of the SCU bus slave
 * \param pData  Data buffer for the register set
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t writeSbSlaveReg(volatile uint16_t* pSlave, regset_t* regset, uint16_t *pData)
{
  uint16_t i;

  if (!pSlave || !pData)
    return COMMON_STATUS_ERROR;

  for (i = 0; i < regset->len; ++i)
    *(pSlave + regset->base + regset->offset + i) = *(pData + i);

  return COMMON_STATUS_OK;
}

/**
 * \brief Probe an extension card of a SCU bus slave
 *
 * \param pMaster Pointer to the SCU bus master
 * \param slot    SCU bus slot number
 * \param pSharedDst destination location in the shared memory
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t probeSbSlaveExt(volatile uint16_t* pMaster, uint32_t slot, uint32_t* pDst)
{
  uint16_t u16val, u16val2;
  uint32_t u32val, u32val2;

  if (!pMaster || !pDst)
    return COMMON_STATUS_ERROR;

  u32val = (0x1) << slot;              // encode slot to bit (slot1=bit1, bits=0..31)
  u32val <<= 16;                       // offset for a SCU bus slave
  u16val  = *(pMaster + u32val + SBS_EXT_CID_SYS); // read extension CID system ID
  u16val2 = *(pMaster + u32val + SBS_EXT_CID_GRP); // read extension CID group ID

  if (u16val != SBS_CID_NO_EXT) {
    u32val = u16val;
    u32val <<= 16;
    u32val |= u16val2;

    *(pDst + SBS_EXT_CID_SYS + slot - 1) = u32val;
    DBPRINT1("\text=yes (sys=0x%04x, grp=0x%04x)\n", u16val, u16val2);

  } else {
    DBPRINT1("\text=no (sys=0x%04x, grp=0x%04x)\n", u16val, u16val2);
  }
}

/**
 * \brief Probe SCU bus slaves
 *
 * \param pMaster Pointer to the SCU bus master
 * \param sysId   CID system ID of a SCU bus slave
 * \param grpId   CID group ID of a SCU bus slave
 * \param slaves  available SCU bus slaves with the given CIDs (bit1=slot1)
 *
 * \return status   Returns zero on success, otherwise non-zero
 **/
status_t probeSbSlaves(volatile uint16_t* pMaster, uint16_t sysId, uint16_t grpId, uint32_t* slaves)
{
  int slot;
  uint16_t cidSys, cidGrp, u16val;

  if (!pMaster || !slaves || !sysId || !grpId) {
    DBPRINT1("sbctl: bad arguments to probe SCU bus slaves: 0x%08x, 0x%04x, 0x%04x, 0x%08x\n",
      pMaster, sysId, grpId, slaves);
    return COMMON_STATUS_ERROR;
  }

  for (slot = 1; slot <= N_SB_SLOTS; slot++) {
    cidSys = *(pMaster + (slot << 16) + SBS_CID_SYS); // get CID system ID of a SCU bus slave
    cidGrp = *(pMaster + (slot << 16) + SBS_CID_GRP); // get CID group ID of a SCU bus slave
    if (cidSys == sysId && cidGrp == grpId) {
      *slaves |= (uint32_t)(0x1) << slot;

      DBPRINT1("sbctl: slot=%d, sys=0x%x, grp=0x%x detected\n",
          slot, sysId, grpId);

      // base address of the current slot
      DBPRINT1("\t base adr=0x%08x\n", (pMaster + (slot << 16)));

      // standard register values of a selected SCU bus slave
      u16val  = *(pMaster + (slot << 16) + SBS_SLAVE_ID); // get slave ID
      DBPRINT1("\t       id=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_FW_VER);  // get FW version
      DBPRINT1("\t   fw ver=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_FW_REL);  // get FW release
      DBPRINT1("\t   fw rel=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_MACRO_VER); // get version and release of macro
      DBPRINT1("\tmacro ver=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_CLK_10K); // get clock frequency of macro
      DBPRINT1("\tmacro clk=%d [MHz]\n", u16val/100);

      u16val  = *(pMaster + (slot << 16) + SBS_EXT_CID_SYS); // get extension system ID
      DBPRINT1("\t  ext sys=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_EXT_CID_GRP); // get extension group ID
      DBPRINT1("\t  ext grp=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_ECHO); // get echo register
      DBPRINT1("\t     echo=0x%x\n", u16val);

      u16val  = *(pMaster + (slot << 16) + SBS_STATUS); // get status register
      DBPRINT1("\t   status=0x%x\n", u16val);

      // probe extension card of a SCU bus slave
      probeSbSlaveExt(pMaster, slot, pSharedGetSbStd);

    } else {
      DBPRINT1("sbctl: slot=%d, sys=0x%x, grp=0x%x not found\n",
          slot, sysId, grpId);
    }
  }

  DBPRINT1("\nsbctl: slaves=%08x (bit1=slot1)\n", *slaves);
  return COMMON_STATUS_OK;
}

/**
 * \brief Export the configuration of SCU bus slaves
 *
 * Write the configuration of SCU bus slaves to a dedicated location in the shared memory.
 *
 * \param pMaster  Pointer to the SCU bus master
 * \param sbSlaves Available SCU bus slaves (bit1=slot1)
 *
 * \return none
 **/
void exportSbSlaveConfig(volatile uint16_t* pMaster, const uint32_t sbSlaves)
{
  uint32_t u32val, i, j;
  status_t retval;
  volatile uint16_t *pSlave;

  if (!pMaster || !sbSlaves)
    return;

  for (i = 1; i < N_SB_SLOTS; ++i) {
    u32val = (sbSlaves >> i) & 0x01;

    if (u32val) {
      pSlave = pMaster + (i << 16);  // slave base address on the SCU bus

      retval = readSbSlaveReg(pSlave, &regSet[DIOB_CFG], configDiob);  // get the DIOB configuration
      retval |= readSbSlaveReg(pSlave, &regSet[DIOB_STS], statusDiob);  // get the DIOB status

      if (retval == COMMON_STATUS_OK) {
        for (j = 0; j < regSet[DIOB_CFG].len; ++j)
          *(pSharedGetSbStd + (FBAS_SHARED_GET_SBCFGDIOB >> 2) + j) = configDiob[j];

        for (j = 0; j < regSet[DIOB_STS].len; ++j)
          *(pSharedGetSbStd + (FBAS_SHARED_GET_SBSTSDIOB >> 2) + j) = statusDiob[j];

        break; // FIXME: consider only 1st slave device
      }
    }
  }
}

/**
 * \brief Handle user-defined commands
 *
 * \param cmd  User-defined command code
 *
 * \return none
 **/
void sbCmdHandler(const uint32_t cmd)
{
  uint16_t cid_sys_id = 0;
  uint16_t cid_grp_id = 0;
  uint32_t u32val;

  if (!cmd || !pSbMaster)
    return;

  switch (cmd) {
    case FBAS_CMD_PROBE_SB_DIOB:    // probe DIOB card on SCU bus
      cid_sys_id = CID_SYS_DIOB;
      cid_grp_id = CID_GRP_DIOB;
      break;
    case FBAS_CMD_PROBE_SB_USER:    // probe a slave card (CID must be provided by user)
      u32val = *pSharedSetSbSlaves;
      cid_sys_id = (uint16_t)(u32val);
      cid_grp_id = (uint16_t)(u32val >> 16);
      break;
    default:
      return;
  }

  if (cid_sys_id && cid_grp_id) {
    if (probeSbSlaves(pSbMaster, cid_sys_id, cid_grp_id, &sbSlaves) == COMMON_STATUS_OK) {
      *pSharedGetSbSlaves = sbSlaves;

      exportSbSlaveConfig(pSbMaster, sbSlaves);

      DBPRINT1("sbctl: DIOB cfg %08x, sts %08x\n",
        (pSharedGetSbStd + (FBAS_SHARED_GET_SBCFGDIOB >> 2)),
        (pSharedGetSbStd + (FBAS_SHARED_GET_SBSTSDIOB >> 2)));

    } else {
      DBPRINT1("sbctl: probe failed!\n");
    }
  } else {
    DBPRINT1("sbctl: invalid CIDs (sys=%x, grp=%x)\n", cid_sys_id, cid_grp_id);
  }
}

/**
 * \brief Write data to a given DIOB register
 *
 * 16-bit data contains the current PC signal state from 16 emitters.
 * - "0"=OK, "1"=NOK
 * - bit0 is for emitter1 (or channel1)
 *
 * \param pData   Pointer to the 16-bit data buffer
 * \param reg     given DIOB register (offset)
 *
 * \return status Returns zero on success, otherwise non-zero
 **/
status_t sbWriteDiob(const uint16_t* pData, const uint16_t reg)
{
  int i;
  uint32_t u32val;
  volatile uint16_t *pDiob;

  if (!pData || !sbDiobs)
    return COMMON_STATUS_ERROR;

  for (int i = 1; i < N_SB_SLOTS; ++i) {
    u32val = (sbDiobs >> i) & 0x01;

    if (u32val) {
      pDiob = pSbMaster + (i << 16); // DIOB base address on the SCU bus

      *(pDiob + reg) = *pData;       // write data to the given DIOB register
    }
  }

  return COMMON_STATUS_OK;
}

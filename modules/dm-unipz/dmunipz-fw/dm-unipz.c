/********************************************************************************************
 *  dm-unipz.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 21-September-2017
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and FAIR-style Data Master
 * 
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Dietrich Beck
 * GSI Helmholtzzentrum für Schwerionenforschung GmbH
 * Planckstraße 1
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
 * Last update: 25-April-2015
 ********************************************************************************************/
#define DMUNIPZ_FW_VERSION 0x000008                     // make this consistent with makefile

/* standard includes */
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"

/* includes specific for bel_projects */
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "../../../top/gsi_scu/scu_mil.h"
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h" // register layout ECA queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA control
#include "../../../ip_cores/saftlib/drivers/eca_flags.h"              // definitions for ECA queue
#include "../../ftm/include/ftm_common.h"                             // defs and regs for data master
#include "../../ftm/ftmfw/ftm_shared_mmap.h"                          // info on shared map for data master lm32 cluster

uint32_t dmExt2BaseAddr(uint32_t extAddr) // data master external address -> external base address
{
  uint32_t r;
  uint32_t baseAddr;

  // round ram size up to next 2^n value
  r = RAM_SIZE;
  r--;
  r |= r >> 1;
  r |= r >> 2;
  r |= r >> 4;
  r |= r >> 8;
  r |= r >> 16;
  r++;

  // decrement to obtain mask for 'lower' address bit
  r--;

  // apply inverse mask to get rid of bits for external address range
  baseAddr = extAddr & ~r;

  return baseAddr;
} //dmExt2BaseAddr


uint32_t dmExt2IntAddr(uint32_t extAddr) // data master external address -> internal address
{
  uint32_t intAddr;
  uint32_t extBaseAddr;

  extBaseAddr = dmExt2BaseAddr(extAddr);

  intAddr     = extAddr - extBaseAddr;
  intAddr     = intAddr + INT_BASE_ADR;

  return intAddr;
} //dmExt2IntAddr


uint32_t dmInt2ExtAddr(uint32_t intAddr, uint32_t extBaseAddr) // data master interanl address -> external address
{
  uint32_t extAddr;

  extAddr = intAddr - INT_BASE_ADR;
  extAddr = extAddr + extBaseAddr;

  return extAddr;
} //dmInt2ExtAddr


#include "dm-unipz.h"
#include "dm-unipz_smmap.h"            // shared memory map for communication via Wishbone

// stuff required for environment
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

// global variables 
volatile uint32_t *pECAQ;               // WB address of ECA queue
volatile uint32_t *pMILPiggy;           // WB address of MIL device bus (MIL piggy)                              
volatile uint32_t *pShared;             // pointer to begin of shared memory region                              
uint32_t *pSharedVersion;               // pointer to a "user defined" u32 register; here: publish version
uint32_t *pSharedStatus;                // pointer to a "user defined" u32 register; here: publish status
uint32_t *pSharedNIterMain;             // pointer to a "user defined" u32 register; here: publish # of iterations of main loop
uint32_t *pSharedNTransfer;             // pointer to a "user defined" u32 register; here: publish # of transfers
uint32_t *pSharedNInject;               // pointer to a "user defined" u32 register; here: publish # of injections (of current transfer)
uint32_t *pSharedVirtAcc;               // pointer to a "user defined" u32 register; here: publish # of virtual accelerator
uint32_t *pSharedStatTrans;             // pointer to a "user defined" u32 register; here: publish status of ongoing transfer
volatile uint32_t *pSharedCmd;          // pointer to a "user defined" u32 register; here: get command from host
uint32_t *pSharedState;                 // pointer to a "user defined" u32 register; here: publish status
volatile uint32_t *pSharedData4EB;      // pointer to a n x u32 register; here: memory region for receiving EB return values
volatile uint32_t *pSharedSrcMacHi;     // pointer to a "user defined" u64 register; here: get MAC of dmunipz WR interface from host
volatile uint32_t *pSharedSrcMacLo;     // pointer to a "user defined" u64 register; here: get MAC of dmunipz WR interface from host
volatile uint32_t *pSharedSrcIP;        // pointer to a "user defined" u32 register; here: get IP of dmunipz WR interface from host
volatile uint32_t *pSharedDstMacHi;     // pointer to a "user defined" u64 register; here: get MAC of the Data Master WR interface from host
volatile uint32_t *pSharedDstMacLo;     // pointer to a "user defined" u64 register; here: get MAC of the Data Master WR interface from host
volatile uint32_t *pSharedDstIP;        // pointer to a "user defined" u32 register; here: get IP of Data Master WR interface from host
volatile uint32_t *pSharedFlexOffset;   // pointer to a "user defined" u32 register; here: TS_FLEXWAIT = OFFSETFLEX + TS_MILEVENT; values in ns
volatile uint32_t *pSharedUniTimeout;   // pointer to a "user defined" u32 register; here: timeout value for UNIPZ

uint32_t *pCpuRamExternal;              // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalData4EB;       // external address (seen from host bridge) of this CPU's RAM: field for EB return values

WriteToPZU_Type  writePZUData;          // Modulbus SIS, I/O-Modul 1, Bits 0..15

uint32_t flexOffset;                    // offset added to obtain timestamp for "flex wait"
uint32_t uniTimeout;                    // timeout value for UNIPZ

#define DM_NBLOCKS       3              // max number of blocks withing the Data Master to be treated
dmComm  dmData[DM_NBLOCKS];             // data for treatment of blocks
#define REQTK            0              // 1st block: handles DM for TK request, flow command
#define REQBEAMA         1              // 2nd block: handles DM for beam request, flow command
#define REQBEAMB         2              // 3rd block: handles DM for beam request, flex wait

/*
void show_msi()
{
  DBPRINT3(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);
}

void isr0()
{
  DBPRINT3("ISR0\n");   
  show_msi();
}
*/


uint32_t ebmInit(uint32_t msTimeout) // intialize Etherbone master
{
  uint64_t timeoutT;
  uint64_t dstMac, srcMac;

  timeoutT = getSysTime() + msTimeout * 1000000;
  while (timeoutT < getSysTime()) {
    if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) asm("nop");
    else break;
  } // while no IP via DHCP

  // check IP
  if (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) return DMUNIPZ_STATUS_NOIP;
  if (*(pEbCfg + (EBC_SRC_IP>>2)) != *pSharedSrcIP)  return DMUNIPZ_STATUS_WRONGIP;

  // init ebm
  ebm_init();

  ebm_config_meta(1500, 42, 0x00000000 );

  dstMac = ((uint64_t)(*pSharedDstMacHi) << 32) + (uint64_t)(*pSharedDstMacLo);
  srcMac = ((uint64_t)(*pSharedSrcMacHi) << 32) + (uint64_t)(*pSharedSrcMacLo);

  ebm_config_if(DESTINATION, dstMac, *pSharedDstIP, 0xebd0); 
  ebm_config_if(SOURCE,      srcMac, *pSharedSrcIP, 0xebd0); 

  ebm_clr();

  return DMUNIPZ_STATUS_OK;
} // ebminit


void ebmClearSharedMem() // clear shared memory used for EB return values
{
  uint32_t i;

  for (i=0; i< (DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2); i++) pSharedData4EB[i] = 0x0;
} //ebmClearSharedMem


uint32_t ebmReadN(uint32_t msTimeout, uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  uint64_t timeoutT;
  int      i;
  uint32_t handshakeIdx;

  handshakeIdx = n32BitWords + 1;

  if (n32BitWords >= (DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2)) return DMUNIPZ_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                                   return DMUNIPZ_STATUS_OUTOFRANGE;

  for (i=0; i< n32BitWords; i++) data[i] = 0x0;

  ebmClearSharedMem();                                                                               // clear shared data for EB return values
  pSharedData4EB[handshakeIdx] = DMUNIPZ_EB_HACKISH;                                                 // see below

  ebm_hi(address);                                                                                   // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address, (uint32_t)(&(pCpuRamExternalData4EB[i])), EBM_READ); // put data into EB cycle
                                ebm_op(address, (uint32_t)(&(pCpuRamExternalData4EB[handshakeIdx])), EBM_READ); // handshake data
  ebm_flush();                                                                                       // commit EB cycle via the network
  
  timeoutT = getSysTime() + msTimeout * 1000000;                                                     
  while (getSysTime() < timeoutT) {                                                                  // wait for received data until timeout
    if (pSharedData4EB[handshakeIdx] != DMUNIPZ_EB_HACKISH) {                                        // hackish solution to determine if a reply value has been received
      for (i=0; i<n32BitWords; i++) data[i] = pSharedData4EB[i];
      //mprintf("dm-unipz: ebmReadN EB_address 0x%08x, nWords %d, data[0] 0x%08x\n", address, n32BitWords, data[0]);
      return DMUNIPZ_STATUS_OK;
    }
  } //while not timed out

  return DMUNIPZ_STATUS_EBREADTIMEDOUT; 
} //ebmReadN


uint32_t ebmWriteN(uint32_t address, uint32_t *data, uint32_t n32BitWords)
{
  int i;

  if (n32BitWords > (DMUNIPZ_SHARED_DATA_4EB_SIZE >> 2)) return DMUNIPZ_STATUS_OUTOFRANGE;
  if (n32BitWords == 0)                                  return DMUNIPZ_STATUS_OUTOFRANGE;

  ebmClearSharedMem();                                                      // clear my shared memory used for EB replies

  ebm_hi(address);                                                          // EB operation starts here
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, data[i], EBM_WRITE);  // put data into EB cycle
  if (n32BitWords == 1)         ebm_op(address      , data[0], EBM_WRITE);  // workaround runt frame issue
  ebm_flush();                                                              // commit EB cycle via the network
  
  return DMUNIPZ_STATUS_OK;
} // ebmWriteN


uint32_t dmPrepCmdCommon(uint32_t idx) // prepare data common to all commands
{
  // simplified memory layout at DM
  //
  // blockAddr -> |wrIdx    |
  //              |rdIdx    |
  //              |IL       |
  //              |HI       |
  //              |QLoPrio--|--buffListAddr--> |buf0  |
  //                                           |buf1  |
  //                                           |buf2--|--cmdListAddr-->|cmd0  |
  //                                           |buf3  |                |cmd1  |                    
  //                                                                   |cmd2  |
  //                                                                   |cmd3--|--cmdAddr-->|TS valid Hi  |
  //                                                                                       |TS valid Lo  |
  //                                                                                       |action(type) |
  //                                                                                       |type specific|

  uint32_t blockAddr;                                          // address of begin of control block
  uint32_t qLowPrioAddr;                                       // address of QLoPrio within control block
  uint32_t wrIdxAddr;                                          // address of wrIdx within control block

  uint32_t wrIdx;                                              // write indices for all prios
  uint32_t wrIdxLo;                                            // write index for low priority. Low priority Q is used here.

  uint32_t buffListAddr;                                       // address of  buffer list (here: of low prio Q)
  uint32_t buffListAddrOffs;                                   // where to find the relevant command buffer within the buffer list
  uint32_t buffAddr;                                           // address of relevant command buffer; buffListAdd + buffListAddOffs
  
  uint32_t cmdListAddr;                                        // address of command list 
  uint32_t cmdListAddrOffs;                                    // where to find the relevant command  within the command list
  uint32_t cmdAddr;                                            // address of relevant command; cmdListAddr + cmdListAddrOffs
  
  //uint32_t help1;
  //uint32_t help2;

  uint32_t cmdValidTSHi;                                       // time when command becomes valid, high32 bit
  uint32_t cmdValidTSLo;                                       // time when command becomes valid, low32 bit
  
  uint32_t intBaseAddr;                                        // internal base address of dm; seen from dm lm32 perspective
  uint32_t extBaseAddr;                                        // external base address of dm; seen from 'world' perspective

  uint32_t status;

  // set important external addresses of block
  blockAddr        = dmData[idx].dynpar0;  
  qLowPrioAddr     = blockAddr + BLOCK_CMDQ_LO_PTR;                 
  wrIdxAddr        = blockAddr + BLOCK_CMDQ_WR_IDXS;
  
  // set Data Master (of relevant lm32) basse addresss from internal and external perspective
  intBaseAddr      = INT_BASE_ADR;                         
  extBaseAddr      = dmExt2BaseAddr(blockAddr);

  // read value for writeIdx and calculate indices for buffer list and command list
  if ((status = ebmReadN(2000, wrIdxAddr, &wrIdx, 1)) != DMUNIPZ_STATUS_OK) return status;
  wrIdxLo          = ((wrIdx >> (PRIO_LO * 8)) &  Q_IDX_MAX_OVF_MSK);
  buffListAddrOffs = (wrIdxLo & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_ ) * _PTR_SIZE_;
  cmdListAddrOffs  = (wrIdxLo & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_ ) * _T_CMD_SIZE_; 

  // read address of buffer list and calculate address of command buffer
  //if ((status = ebmReadN(2000, qLowPrioAddr, &help1, 1)) != DMUNIPZ_STATUS_OK) return status;
  //buffListAddr     = help1;
  if ((status = ebmReadN(2000, qLowPrioAddr, &buffListAddr, 1)) != DMUNIPZ_STATUS_OK) return status;
  buffListAddr     = dmInt2ExtAddr(buffListAddr, extBaseAddr);
  buffAddr         = buffListAddr + buffListAddrOffs;

  // read address of command list and calculate address of command
  //if ((status = ebmReadN(2000, buffAddr, &help2, 1)) != DMUNIPZ_STATUS_OK) return status;
  //cmdListAddr      = help2;
  if ((status = ebmReadN(2000, buffAddr, &cmdListAddr, 1)) != DMUNIPZ_STATUS_OK) return status;
  cmdListAddr      = dmInt2ExtAddr(cmdListAddr, extBaseAddr);
  cmdAddr          = cmdListAddr + cmdListAddrOffs;

  // set timestamp when command shall become valid, here: as soon as possible
  cmdValidTSHi     = 0x0;
  cmdValidTSLo     = 0x0;

  // update value for write index
  wrIdx            = wrIdx & ~(0xff << (PRIO_LO * 8));                // clear current value of write index for low priority
  wrIdx            = wrIdx | ((wrIdxLo + 1) & Q_IDX_MAX_OVF_MSK);     // update value of write index for low priority

  //DBPRINT2("dm-unipz: prep dmPepCmdCommon for idx %d, block address 0x%08x, wrIdx %d, wrIdxLo %d, buffLAddrHelp 0x%08x, buffLAddr 0x%08x, buffAddr 0x%08x, cmdLAddrHelp 0x%08x, cmdLAddr 0x%08x, cmdAddr 0x%08x\n", idx, blockAddr, wrIdx, wrIdxLo, help1, buffListAddr, buffAddr, help2, cmdListAddr, cmdAddr);
  DBPRINT2("dm-unipz: prep dmPepCmdCommon for idx %d, block address 0x%08x, wrIdx %d, wrIdxLo %d, buffLAddr 0x%08x, buffAddr 0x%08x, cmdLAddr 0x%08x, cmdAddr 0x%08x\n", idx, blockAddr, wrIdx, wrIdxLo, buffListAddr, buffAddr, cmdListAddr, cmdAddr);
  DBPRINT3("dm-unipz: prep cmdValidTSHi 0x%08x, index %d\n", cmdValidTSHi, T_CMD_TIME >> 2);
  DBPRINT3("dm-unipz: prep cmdValidTSLo 0x%08x\n", cmdValidTSLo);
  DBPRINT3("dm-unipz: prep wrIdx %d\n", wrIdx);
  DBPRINT3("dm-unipz: prep wrIdxAddr 0x%08x\n", wrIdxAddr);
  
  // assign prepared values for later use;
  dmData[idx].cmdAddr                        = cmdAddr;
  dmData[idx].cmdData[(T_CMD_TIME >> 2) + 0] = cmdValidTSHi;  
  dmData[idx].cmdData[(T_CMD_TIME >> 2) + 1] = cmdValidTSLo;  
  dmData[idx].blockWrIdx                     = wrIdx;
  dmData[idx].blockWrIdxAddr                 = wrIdxAddr;  

  return DMUNIPZ_STATUS_OK;
} //dmPrepCmdCommon


uint32_t dmPrepCmdFlow(uint32_t idx) // prepare flow CMD for DM - need to call dmPrepCmdCommon first
{
  // simplified memory layout of flow command
  //
  // dmCmdAddr-->|TS valid Hi|
  //             |TS valid Lo|
  //             |action     |
  //             |flow dest. |
  //             |reserved   |               

  uint32_t cmdAction;                                          // action flags of command
  uint32_t cmdFlowDestAddr;                                    // address of flow destination
  
  // set command action type to flow
  cmdAction        = (ACT_TYPE_FLOW & ACT_TYPE_MSK) << ACT_TYPE_POS;  // set type to "flow"
  cmdAction       |= (            1 & ACT_QTY_MSK)  << ACT_QTY_POS;   // set quantity to "1"
  cmdAction       |= (      PRIO_LO & ACT_PRIO_MSK) << ACT_PRIO_POS;  // set prio to "Low"

  // set address of flow destination
  cmdFlowDestAddr  = dmExt2IntAddr(dmData[idx].dynpar1);

  DBPRINT3("dm-unipz: prep  dmPrepFLowCmd for idx %d\n", idx);
  DBPRINT3("dm-unipz: prep  cmdAction 0x%08x, index %d\n", cmdAction, T_CMD_ACT >> 2);
  DBPRINT3("dm-unipz: prep  cmdFlowDestAddr 0x%08x, index %d\n", cmdFlowDestAddr, T_CMD_FLOW_DEST >> 2);
  DBPRINT3("dm-unipz: prep  cmdFlowReserved 0x%08x, index %d\n", 0x0, T_CMD_RES >> 2);
  
  // assign prepared values for later use;
  dmData[idx].cmdData[T_CMD_ACT >> 2]        = cmdAction;
  dmData[idx].cmdData[T_CMD_FLOW_DEST >> 2]  = cmdFlowDestAddr; 
  dmData[idx].cmdData[T_CMD_RES >> 2]        = 0x0;

  return DMUNIPZ_STATUS_OK;
} //dmPrepFlowCmd


uint32_t dmPrepFlexWaitCmd(uint32_t idx, uint64_t timestamp) // prepare flexible waiting CMD for DM - need to call dmPrepCmdCommon first
{
  // simplified memory layout of flexwait command
  //
  // dmCmdAddr-->|TS valid Hi|
  //             |TS valid Lo|
  //             |action     |
  //             |TS wait Hi |
  //             |TS wait Lo | 

  uint32_t cmdAction;                                          // action flags of command
  uint32_t cmdWaitTimeHi;                                      // waiting time, hi 32 bits
  uint32_t cmdWaitTimeLo;                                      // waiting time, lo 32 bits
  
  // set command action type
  cmdAction        = (ACT_TYPE_WAIT & ACT_TYPE_MSK) << ACT_TYPE_POS;     // set type to "wait"
  cmdAction       |= (            ACT_WAIT_ABS_MSK) << ACT_WAIT_ABS_POS; // set type of timestamp to "absolute"
  cmdAction       |= (            1 & ACT_QTY_MSK)  << ACT_QTY_POS;      // set quantity to "1"
  cmdAction       |= (      PRIO_LO & ACT_PRIO_MSK) << ACT_PRIO_POS;     // set prio to "Low"

  // set waiting time
  cmdWaitTimeHi    = (uint32_t)(timestamp >> 32);
  cmdWaitTimeLo    = (uint32_t)(timestamp & 0xffffffff); 

  DBPRINT3("dm-unipz: prep  dmPrepFlexWaitCmd for idx %d\n", idx);
  DBPRINT3("dm-unipz: prep  cmdAction 0x%08x, index %d\n", cmdAction, T_CMD_ACT >> 2);
  DBPRINT3("dm-unipz: prep  cmdWaitTimeHi %09u, index %d\n", cmdWaitTimeHi, T_CMD_WAIT_TIME >> 2);
  DBPRINT3("dm-unipz: prep  cmdWaitTimeLo %09u, index %d\n", cmdWaitTimeLo, (T_CMD_WAIT_TIME >> 2) + 1);
  
  // assign prepared values for later use;
  dmData[idx].cmdData[T_CMD_ACT >> 2]             = cmdAction;
  dmData[idx].cmdData[T_CMD_WAIT_TIME >> 2]       = cmdWaitTimeHi;
  dmData[idx].cmdData[(T_CMD_WAIT_TIME >> 2) + 1] = cmdWaitTimeLo;
  
  return DMUNIPZ_STATUS_OK;
} //dmPrepFlexWaitCmd


void dmChangeBlock(uint32_t idx)     // alter a block within the Data Master on-the fly
{
  ebmWriteN(dmData[idx].cmdAddr, dmData[idx].cmdData, (_T_CMD_SIZE_ >> 2));  
  ebmWriteN(dmData[idx].blockWrIdxAddr, &dmData[idx].blockWrIdx, 1);             
  DBPRINT2("dm-unipz: dmChangeBlock idx %d, cmdAddr 0x%08x, cmdData[0] 0x%08x, cmdData[1] 0x%08x\n", idx, dmData[idx].cmdAddr, dmData[idx].cmdData[0], dmData[idx].cmdData[1]);
} // dmChangeBlock


void init() // typical init for lm32
{
  discoverPeriphery();           // mini-sdb ...
  uart_init_hw();                
  cpuId = getCpuIdx();

  // set MSI IRQ handler
  isr_table_clr();
  //irq_set_mask(0x01);
  irq_disable(); 
} // init


void initSharedMem() // determine address and clear shared mem
{
  uint32_t idx;
  const uint32_t c_Max_Rams = 10;
  sdb_location found_sdb[c_Max_Rams];
  sdb_location found_clu;
  
  // get pointer to shared memory
  pShared           = (uint32_t *)_startshared;

  // get address to data
  pSharedVersion     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_VERSION >> 2));
  pSharedStatus      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd         = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));
  pSharedState       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATE >> 2));
  pSharedNIterMain   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NITERMAIN >> 2));
  pSharedNTransfer   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSN >> 2));
  pSharedNInject     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_INJECTN >> 2));
  pSharedVirtAcc     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSVIRTACC >> 2));
  pSharedStatTrans   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSSTATUS >> 2));
  pSharedData4EB     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA_4EB_START >> 2));
  pSharedSrcMacHi    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCMACHI >> 2));
  pSharedSrcMacLo    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCMACLO >> 2));
  pSharedSrcIP       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCIP >> 2));
  pSharedDstMacHi    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTMACHI >> 2));
  pSharedDstMacLo    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTMACLO >> 2));
  pSharedDstIP       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTIP >> 2));
  pSharedFlexOffset  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_OFFSETFLEX >> 2));
  pSharedUniTimeout  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_UNITIMEOUT>> 2));

  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
  }

  // set initial values;
  ebmClearSharedMem();
  *pSharedVersion    = DMUNIPZ_FW_VERSION; // of all the shared variabes, only VERSION is a constant. Set it now!
  *pSharedFlexOffset = DMUNIPZ_OFFSETFLEX; // initialize with default value
  *pSharedUniTimeout = DMUNIPZ_UNITIMEOUT; // initialize with default value
} // initSharedMem 


uint32_t findMILPiggy() //find WB address of MIL Piggy
{
  pMILPiggy = 0x0;
  
  // get Wishbone address for MIL Piggy
  pMILPiggy = find_device_adr(GSI, SCU_MIL);

  if (!pMILPiggy) {DBPRINT1("dm-unipz: can't find MIL piggy\n"); return DMUNIPZ_STATUS_ERROR;}
  else                                                           return DMUNIPZ_STATUS_OK;
} // initMILPiggy


uint32_t findECAQueue() // find WB address of ECA channel for LM32
{
#define ECAQMAX           4     // max number of ECA channels in the system
#define ECACHANNELFORLM32 2     // this is a hack! suggest implementing finding via sdb-record and info

  // stuff below needed to get WB address of ECA queue
  sdb_location ECAQ_base[ECAQMAX];
  uint32_t ECAQidx = 0;         
  uint32_t *tmp;                
  int i;

  // get Wishbone address of ECA queue 
  // get list of ECA queues
  find_device_multi(ECAQ_base, &ECAQidx, ECAQMAX, ECA_QUEUE_SDB_VENDOR_ID, ECA_QUEUE_SDB_DEVICE_ID);
  pECAQ = 0x0;

  // find ECA queue connected to ECA chanel for LM32
  for (i=0; i < ECAQidx; i++) {
    tmp = (uint32_t *)(getSdbAdr(&ECAQ_base[i]));  
    if ( *(tmp + (ECA_QUEUE_QUEUE_ID_GET >> 2)) == ECACHANNELFORLM32) pECAQ = tmp;
  }

  if (!pECAQ) {DBPRINT1("dm-unipz: can't find ECA queue\n"); return DMUNIPZ_STATUS_ERROR;}
  else                                                      return DMUNIPZ_STATUS_OK;
} // findECAQueue


uint32_t wait4ECAEvent(uint32_t msTimeout, uint32_t *virtAcc, uint32_t *dryRunFlag)  // 1. query ECA for actions, 2. trigger activity
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID   
  uint32_t evtIdLow;            // low 32bit of eventID    
  uint32_t evtDeadlHigh;        // high 32bit of deadline  
  uint32_t evtDeadlLow;         // low 32bit of deadline   
  uint32_t evtParamHigh;        // high 32 bit of parameter field
  uint32_t evtParamLow ;        // low 32 bit of parameter field
  uint32_t actTag;              // tag of action           
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out

  *virtAcc    = 0xff;           // 0xff: virt acc is not yet set
  *dryRunFlag = 0xff;           // 0xff: "dry run flag" not yet set
  pECAFlag     = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag
  timeoutT = getSysTime() + msTimeout * 1000000;

  while (getSysTime() < timeoutT) {
    if (*pECAFlag & (0x0001 << ECA_VALID)) {                         // if ECA data is valid
      
      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      evtParamHigh = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      evtParamLow  = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));
    
      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      switch (actTag) 
        {
        case DMUNIPZ_ECADO_REQTK :
          nextAction  = DMUNIPZ_ECADO_REQTK;
          *virtAcc    = evtIdLow & 0xf;  
          *dryRunFlag = (evtIdLow & 0x10) != 0;
          dmData[REQBEAMA].dynpar0  = evtParamHigh;                  // address of block B_LOOP1
          dmData[REQBEAMA].dynpar1  = evtParamLow;                   // address of block B_FLEXWAIT0
          dmData[REQBEAMB].dynpar0  = evtParamLow;                   // address of block B_FLEXWAIT0
          dmData[REQBEAMB].dynpar1  = 0x0;
          DBPRINT3("dm-unipz: received ECA event request TK\n");
          break;
        case DMUNIPZ_ECADO_REQBEAM :
          nextAction = DMUNIPZ_ECADO_REQBEAM;
          *virtAcc = evtIdLow & 0xf;   
          DBPRINT3("dm-unipz: received ECA event request beam\n");
          break;
        case DMUNIPZ_ECADO_RELTK :
          nextAction = DMUNIPZ_ECADO_RELTK;
          *virtAcc = evtIdLow & 0xf; 
          break;
        case DMUNIPZ_ECADO_PREPDM :
          nextAction = DMUNIPZ_ECADO_PREPDM;
          // get DM dynpar for waiting on TKREQ
          dmData[REQTK].dynpar0 = evtParamHigh;
          dmData[REQTK].dynpar1 = evtParamLow;
          break;

        default: 
          nextAction = DMUNIPZ_ECADO_UNKOWN;
        } // switch

      return nextAction;

    } // if data is valid
  } // while not timed out

  return  DMUNIPZ_ECADO_TIMEOUT;
} // wait for ECA event


void initCmds() // init stuff for handling commands, trivial for now, will be extended
{
  //  initalize command value: 0x0 means 'no command'
  *pSharedCmd     = 0x0;
} // initCmds


int16_t writeToPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t data) // write bit field to module bus output (linked to UNI PZ)
{
  uint16_t wData     = 0x0;     // data to write
  int16_t  busStatus = 0;       // status of bus operation
  
  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData)) != MIL_STAT_OK) return busStatus;

  // write data word
  wData     = data;
  busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_W, wData);
  // DBPRINT1("dm-unipz: writeToPZU, wrote wData %d\n", wData);

  return (busStatus);
} // writeToPZU


int16_t readFromPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t *data) // read bit field from module bus input (linked to UNI PZ)
{
  uint16_t wData      = 0x0;    // data to write
  uint16_t rData      = 0x0;    // data to read
  int16_t  busStatus  = 0;      // status of bus operation

  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData))  != MIL_STAT_OK) return busStatus;

  // read data
  if ((busStatus = readDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_R, &rData)) == MIL_STAT_OK) *data = rData;

  return(busStatus);
} // readFromPZU 


uint32_t checkClearReqNotOk(uint32_t msTimeout)      // check for 'Req not OK' flag from UNILAC. If the flag is set, try to clear it
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint64_t         timeoutT;     // when to time out

  timeoutT = getSysTime() + msTimeout * 1000000;

  if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;    
  if (readPZUData.bits.Req_not_ok == true) {                                                                                            // check for 'req not ok'

    writePZUData.uword               = 0x0;
    writePZUData.bits.Req_not_ok_Ack = true;
    if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;      // request to clear not_ok flag

    while (getSysTime() < timeoutT) {                                                                                                   // check for timeout
      if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR; 
      if (readPZUData.bits.Req_not_ok == false) {                                                                                       // remove acknowledgement for 'req not ok'
        writePZUData.bits.Req_not_ok_Ack = false;                     
        writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword); 
        return DMUNIPZ_STATUS_REQNOTOK;
      } // if UNILAC cleared 'req not ok' flag
    } // while not timed out
  
    return DMUNIPZ_STATUS_REQNOTOK;
  } // if 'req_no_ok'

  return DMUNIPZ_STATUS_OK;
} // checkClearReqNotOk


uint32_t requestTK(uint32_t msTimeout, uint32_t virtAcc, uint32_t dryRunFlag)
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint64_t         timeoutT;     // when to time out

  if (virtAcc > 0xf) return DMUNIPZ_STATUS_OUTOFRANGE;  

  timeoutT = getSysTime() + msTimeout * 1000000;

  // send request to modulbus I/O (UNIPZ)
  writePZUData.uword               = 0x0;
  writePZUData.bits.TK_Request     = true;
  writePZUData.bits.SIS_Acc_Select = virtAcc;
  writePZUData.bits.ReqNoBeam      = dryRunFlag;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;

  // for for acknowledgement, 'request not ok' or timeout
  while (getSysTime() < timeoutT) {                                                                                                   // check for timeout
    if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR; // read from modulbus I/O (UNIPZ)
    if (readPZUData.bits.TK_Req_Ack == true) return DMUNIPZ_STATUS_OK;                                                                // check for acknowledgement
    if ((status = checkClearReqNotOk(msTimeout)) == DMUNIPZ_STATUS_REQNOTOK) return DMUNIPZ_STATUS_REQTKFAILED;                       // check for 'request not ok'
  } // while not timed out

  return DMUNIPZ_STATUS_TIMEDOUT;
} // requestTK


uint32_t releaseTK()
{
  int16_t          status;       // status MIL device bus operation

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.TK_Request     = false;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
  
  return DMUNIPZ_STATUS_OK;
} // releaseTK


uint32_t requestBeam(uint32_t msTimeout)
{
  int16_t          status;       // status MIL device bus operation

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.SIS_Request  = true;
  
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
  
  // this is all we can do for now. We _must not_ check any ACK or NOT_ACK from UNIPZ to reduce jitter when timestamping the MIL event

  return DMUNIPZ_STATUS_OK;
} // requestBeam


uint32_t releaseBeam()
{
  int16_t          status;       // status MIL device bus operation

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.SIS_Request  = false;
  writePZUData.bits.ReqNoBeam    = false;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
 
  return DMUNIPZ_STATUS_OK;
} // releaseBeam

 
uint32_t configMILEvent(uint16_t evtCode) // configure SoC to receive events via MIL bus
{
  uint32_t i;

  // initialize status and command register with initial values; disable event filtering; clear filter RAM
  if (writeCtrlStatRegEvtMil(pMILPiggy, MIL_CTRL_STAT_ENDECODER_FPGA | MIL_CTRL_STAT_INTR_DEB_ON) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  // clean up 
  if (disableLemoEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableLemoEvtMil(pMILPiggy, 2) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  if (disableFilterEvtMil(pMILPiggy)  != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR; 
  if (clearFilterEvtMil(pMILPiggy)    != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR; 

  for (i=0; i < (0xf+1); i++) {
    // set filter (FIFO and LEMO1 pulsing) for all possible virtual accelerators
    if (setFilterEvtMil(pMILPiggy,  evtCode, i, MIL_FILTER_EV_TO_FIFO | MIL_FILTER_EV_PULS1_S) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  }

  // configure LEMO1 for pulse generation
  if (configLemoPulseEvtMil(pMILPiggy, 1) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;

  return DMUNIPZ_STATUS_OK;
} // configMILEvent


uint16_t wait4MILEvt(uint16_t evtCode, uint16_t virtAcc, uint32_t msTimeout)  // wait for MIL event or timeout
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint32_t virtAccRec;         // virtual accelerator
  uint64_t timeoutT;           // when to time out

  timeoutT = getSysTime() + msTimeout * 10000000;      

  mprintf("dm-unipz: huhu evtCode 0x%04x, virtAcc 0x%04x\n", evtCode, virtAcc);

  while(getSysTime() < timeoutT) {              // while not timed out...
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // while fifo contains data
      popFifoEvtMil(pMILPiggy, &evtDataRec);    
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccRec  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)

      if ((evtCodeRec == evtCode) && (virtAccRec == virtAcc)) return DMUNIPZ_STATUS_OK;

      // chck mprintf("dm-unipz: virtAcc %03d, evtCode %03d\n", virtAccRec, evtCodeRec);

    } // while fifo contains data
    asm("nop");                                 // wait a bit...
  } // while not timed out

  return DMUNIPZ_STATUS_TIMEDOUT;
} //wait4MILEvent


void pulseLemo2() //for debugging with scope
{
  uint32_t i;

  setLemoOutputEvtMil(pMILPiggy, 2, 1);
  for (i=0; i< 10 * DMUNIPZ_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 2, 0);
} // pulseLemo2


uint32_t doActionS0()
{
  uint32_t status = DMUNIPZ_STATUS_OK;

  if (findECAQueue() != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_ERROR; 
  if (findMILPiggy() != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_ERROR; 
  initCmds();                    

  return status;
} // entryActionS0


uint32_t entryActionConfigured()
{
  uint32_t status = DMUNIPZ_STATUS_OK;
  uint32_t virtAcc;
  uint32_t dryRunFlag;
  uint32_t i;
  uint32_t data;

  // configure EB master (SRC and DST MAC/IP are set from host)
  if ((status = ebmInit(2000)) != DMUNIPZ_STATUS_OK) {
    DBPRINT1("dm-unipz: ERROR - init of EB master failed! %d\n", status);
    return status;
  } 

  // test if DM is reachable by reading from ECA input
  if ((status = ebmReadN(2000, DMUNIPZ_ECA_ADDRESS, &data, 1)) != DMUNIPZ_STATUS_OK) {
    DBPRINT1("dm-unipz: ERROR - Data Master unreachable! %d\n", status);
    return status;
  }

  DBPRINT1("dm-unipz: connection to DM ok - 0x%08x\n", data);
      
  // check if modulbus I/O is ok
  if ((status = echoTestDevMil(pMILPiggy, IFB_ADDRESS_SIS, 0xbabe)) != DMUNIPZ_STATUS_OK) {
    DBPRINT1("dm-unipz: ERROR - modulbus SIS IFK not available!\n");
    return DMUNIPZ_STATUS_DEVBUSERROR;
  }

  DBPRINT1("dm-unipz: connection to UNIPZ (devicebus) ok\n");  

  // configure MIL piggy for timing events for all 16 virtual accelerators
  if ((status = configMILEvent(DMUNIPZ_EVT_READY2SIS)) != DMUNIPZ_STATUS_OK) {
    DBPRINT1("dm-unipz: ERROR - failed to configure MIL piggy for receiving timing events! %d\n", status);
    return status;
  } 

  DBPRINT1("dm-unipz: MIL piggy configured for receving events (eventbus)\n");

  configLemoOutputEvtMil(pMILPiggy, 2);    // used to see a blinking LED (and optionally connect a scope) for debugging
  checkClearReqNotOk(uniTimeout);          // in case a 'req_not_ok' flag has been set at UNIPZ, try to clear it

  // clear bits for modulbus I/O to UNILAC
  writePZUData.uword = 0x0;
  writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword);

  // empty ECA queue for lm32
  i = 0;
  while (wait4ECAEvent(1, &virtAcc, & dryRunFlag) !=  DMUNIPZ_ECADO_TIMEOUT) {i++;}
  DBPRINT1("dm-unipz: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  flexOffset  = *pSharedFlexOffset;
  uniTimeout  = *pSharedUniTimeout;

  return status;
} // entryActionConfigured


uint32_t entryActionOperation()
{
  return DMUNIPZ_STATUS_OK;
} // entryActionOperation


uint32_t exitActionOperation()
{
  if (disableFilterEvtMil(pMILPiggy) != MIL_STAT_OK) return DMUNIPZ_STATUS_ERROR;
  
  return DMUNIPZ_STATUS_OK;
} // exitActionOperation

uint32_t exitActionError()
{
  return DMUNIPZ_STATUS_OK;
} // exitActionError


void cmdHandler(uint32_t *reqState) // handle commands from the outside world
{
  uint32_t cmd;

  cmd = *pSharedCmd;
  // check, if the command is valid and request state change
  if (cmd) {
    switch (cmd) {
    case DMUNIPZ_CMD_CONFIGURE :
      *reqState =  DMUNIPZ_STATE_CONFIGURED;
      DBPRINT3("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STARTOP :
      *reqState = DMUNIPZ_STATE_OPERATION;
      DBPRINT3("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STOPOP :
      *reqState = DMUNIPZ_STATE_STOPPING;
      DBPRINT3("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_IDLE :
      *reqState = DMUNIPZ_STATE_IDLE;
      DBPRINT3("received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_RECOVER :
      *reqState = DMUNIPZ_STATE_IDLE;
      DBPRINT3("received cmd %d\n", cmd);
      break;
    default:
      DBPRINT3("cmdHandler: unknown command '0x%08x'\n",*pSharedCmd);
    } // switch 
    *pSharedCmd = 0x0; // reset cmd value in shared memory 
  } // if command 
} // cmdHandler


uint32_t changeState(uint32_t *actState, uint32_t *reqState, uint32_t actStatus)   //state machine; see dm-unipz.h for possible states and transitions
{
  uint32_t statusTransition= DMUNIPZ_STATUS_OK;
  uint32_t status;
  uint32_t nextState;                   

  // if something severe happened, perform implicitely allowed transition to ERROR or FATAL states
  // else                        , handle explicitcely allowed transitions

  if ((*reqState == DMUNIPZ_STATE_ERROR) || (*reqState == DMUNIPZ_STATE_FATAL)) {statusTransition = actStatus; nextState = *reqState;}
  else {
    nextState = *actState;                       // per default: remain in actual state without exit or entry action
    switch (*actState) {                         // check for allowed transitions: 1. determine next state, 2. perform exit or entry actions if required
    case DMUNIPZ_STATE_S0:
      if      (*reqState == DMUNIPZ_STATE_IDLE)       {                                            nextState = *reqState;}      
      break;
    case DMUNIPZ_STATE_IDLE:
      if      (*reqState == DMUNIPZ_STATE_CONFIGURED)  {statusTransition = entryActionConfigured(); nextState = *reqState;}
      break;
    case DMUNIPZ_STATE_CONFIGURED:
      if      (*reqState == DMUNIPZ_STATE_IDLE)        {                                            nextState = *reqState;}
      else if (*reqState == DMUNIPZ_STATE_CONFIGURED)  {statusTransition = entryActionConfigured(); nextState = *reqState;}
      else if (*reqState == DMUNIPZ_STATE_OPERATION)   {statusTransition = entryActionOperation();  nextState = *reqState;}
      break;
    case DMUNIPZ_STATE_OPERATION:
      if      (*reqState == DMUNIPZ_STATE_STOPPING)    {statusTransition = exitActionOperation();   nextState = *reqState;}
      break;
    case DMUNIPZ_STATE_STOPPING:
      nextState = DMUNIPZ_STATE_CONFIGURED;      //automatic transition but without entryActionConfigured
    case DMUNIPZ_STATE_ERROR:
      if      (*reqState == DMUNIPZ_STATE_IDLE)        {statusTransition = exitActionError();       nextState = *reqState;}
      break;
    default: 
      nextState = DMUNIPZ_STATE_S0;
    } // switch actState
  }  // else something severe happened
  
  // if the transition failed, transit to error state (except we are already in FATAL state)
  if ((statusTransition != DMUNIPZ_STATUS_OK) && (nextState != DMUNIPZ_STATE_FATAL)) nextState = DMUNIPZ_STATE_ERROR;

  // if the state changes
  if (*actState != nextState) {                   
    mprintf("dm-unipz: changed to state %d\n", nextState);
    *actState = nextState;                      
    status = statusTransition;
  } // if state change
  else  status = actStatus;

  *reqState = DMUNIPZ_STATE_UNKNOWN;             // reset requested state (= no change state requested)  

  return status;
} //changeState


uint32_t doActionOperation(uint32_t *statusTransfer, uint32_t *virtAcc, uint32_t *nTransfer, uint32_t *nInject, uint32_t actStatus)
{
  uint32_t status, dmStatus;
  uint32_t nextAction;
  uint32_t virtAccTmp;
  uint32_t dryRunFlag;
  uint64_t timestamp;
  uint64_t sendT;
  uint32_t sendTsecs;
  uint32_t sendTnsecs;
  uint64_t tempT;

  uint64_t ts1, ts2, ts3; //chk

  status = actStatus; 

  nextAction = wait4ECAEvent(DMUNIPZ_DEFAULT_TIMEOUT, &virtAccTmp, &dryRunFlag);   // do action is driven by actions issued by the ECA

  switch (nextAction) 
    {
    case DMUNIPZ_ECADO_REQTK :                                                     // received command "REQ_TK" from data master

      *virtAcc        = virtAccTmp;                                                // number of virtual accelerator is set when DM requests TK
      *statusTransfer = DMUNIPZ_TRANS_REQTK;                                       // update status of transfer
      (*nTransfer)++;                                                              // increment number of transfers
      *nInject        = 0;                                                         // number of injections is reset when DM requests TK

      status = requestTK(uniTimeout, virtAccTmp, dryRunFlag);                      // request TK from UNIPZ

      if ((dmStatus = dmPrepCmdCommon(REQTK)) != DMUNIPZ_STATUS_OK)                // prepare common part of command for later use, here: continue after TK request
        return dmStatus;                                                           // failure of preparation is a severe error!
        
      dmPrepCmdFlow(REQTK);                                                        // prepare flow command for later use, here: continue after TK request
      dmChangeBlock(REQTK);                                                        // modify block within DM for execution of a flow command, here: continue after TK request


      if (status == DMUNIPZ_STATUS_OK) *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQTKOK; // update status of transfer

      break;
    case DMUNIPZ_ECADO_REQBEAM :                                                   // received command "REQ_BEAM" from data master

      *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQBEAM;                   // update status of transfer
      (*nInject)++;                                                                // increment number of injections (of current transfer)

      if ((dmStatus = dmPrepCmdCommon(REQBEAMA)) != DMUNIPZ_STATUS_OK)             // prepare common part of command for later use, here: continue after beam request 
        return dmStatus;                                                           // failure of preparation is a severe error!
      dmPrepCmdFlow(REQBEAMA);                                                     // prepare flow command for later use, here: continue after beam request

      if ((dmStatus = dmPrepCmdCommon(REQBEAMB)) != DMUNIPZ_STATUS_OK)             // prepare common part of command for later use, here: flex wait
        return dmStatus;                                                           // failure of preparation is a severe error!      
      // NB: we can't prepare the flex wait yet, as we need to timestamp the MIL event from UNIPZ first
    
      enableFilterEvtMil(pMILPiggy);                                               // enable filter @ MIL piggy
      clearFifoEvtMil(pMILPiggy);                                                  // get rid of junk in FIFO @ MIL piggy
      //      ts1 = getSysTime();
      requestBeam(uniTimeout);                                                     // request beam from UNIPZ, note that we can't check for REQ_NOT_OK from here
      //ts2 = getSysTime();
      //ts3 = ts2 - ts1;
      //      ts3 = ts3 >> 10;

      //ts2 = ts2 >> 30;

      //mprintf("dm-unipz: dt %d %d %d [ns] \n", (uint32_t)ts1, (uint32_t)ts2, (uint32_t)ts3);
      

      status = wait4MILEvt(DMUNIPZ_EVT_READY2SIS, virtAccTmp, uniTimeout);         // wait for MIL Event
      timestamp = getSysTime();                                                    // get timestamp for MIL event
      sendT     = timestamp + (uint64_t)flexOffset;                                // add offset to obtain time for "flex wait"

      pulseLemo2();                                                                // for hardware debugging with scope

      dmPrepFlexWaitCmd(REQBEAMB, sendT);                                          // prepare flex wait command, here: wait until absolute time
      dmChangeBlock(REQBEAMB);                                                     // modify the "flex wait" block within DM
      dmChangeBlock(REQBEAMA);                                                     // modify block within DM for execution of a flow command, here: continue after beam request towards "flex wait"

      // tempT = getSysTime();
      
      // sendTsecs  = (uint32_t)(sendT / (uint64_t)1000000000);
      // sendTnsecs = (uint32_t)(sendT - (uint64_t)sendTsecs);
       
      // mprintf("dm-unipz: sendT %u.%u\n", sendTsecs, sendTnsecs);
      // mprintf("dm-unipz: timestamp - tempT (DM) %u\n", (uint32_t)(tempT - timestamp));
      // mprintf("dm-unipz: timestamp - sendT (DM) %u\n", (uint32_t)(sendT - timestamp));

      if (status == DMUNIPZ_STATUS_TIMEDOUT) {                                     // discriminate between 'timeout' and 'REQ_NOT_OK'
        if (checkClearReqNotOk(uniTimeout) != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_REQBEAMFAILED;
        else                                                     status = DMUNIPZ_STATUS_REQBEAMTIMEDOUT;
      } // if status 


      releaseBeam();                                                               // release beam request

      disableFilterEvtMil(pMILPiggy);                                              // disable filter @ MIL piggy

      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELBEAM;                  // update status of transfer
      if (status == DMUNIPZ_STATUS_OK)                                           
        *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQBEAMOK; 

      break;
    case DMUNIPZ_ECADO_RELTK :                                                     // received command "REL_TK" from data master

      releaseTK();                                                                 // release TK
      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELTK;                    // update status of transfer 

      break;
    case DMUNIPZ_ECADO_PREPDM:                                                     // received command "PREP_DM" from data master

      break;
    default: ;
    } // switch nextAction

  return status;
} //doActionOperation


void main(void) {
 
  uint32_t j;
 
  uint32_t i;                                   // counter for iterations of main loop
  uint32_t status;                              // (error) status
  uint32_t actState;                            // actual FSM state
  uint32_t reqState;                            // requested FSM state

  uint32_t statusTransfer;                      // status of transfer
  uint32_t nTransfer;                           // number of transfers
  uint32_t nInject;                             // number of injections within current transfer
  uint32_t virtAcc;                             // number of virtual accelerator

  mprintf("dm-unipz: ***** firmware v %06d started from scratch *****\n", DMUNIPZ_FW_VERSION);
  
  // init local variables
  i              = 0;
  nTransfer      = 0;                           
  nInject        = 0;                           
  virtAcc        = 0xff;                        
  statusTransfer = DMUNIPZ_TRANS_UNKNOWN;       
  reqState       = DMUNIPZ_STATE_S0;
  actState       = DMUNIPZ_STATE_UNKNOWN;
  status         = DMUNIPZ_STATUS_UNKNOWN;      

  init();                                                                   // initialize stuff for lm32
  initSharedMem();                                                          // initialize shared memory
  mprintf("dm-unipz: quack\n");
  while (1) {
    cmdHandler(&reqState);                                                  // check for commands and possibly request state changes
    status = changeState(&actState, &reqState, status);                     // handle requested state changes
    
    switch(actState)                                                        // state specific do actions
      {
      case DMUNIPZ_STATE_S0 :
        status = doActionS0();                                              // important initialization that must succeed!
        if (status != DMUNIPZ_STATUS_OK) reqState = DMUNIPZ_STATE_FATAL;    // failed:  -> FATAL
        else                             reqState = DMUNIPZ_STATE_IDLE;     // success: -> IDLE
        break;
      case DMUNIPZ_STATE_OPERATION :
        status = doActionOperation(&statusTransfer, &virtAcc, &nTransfer, &nInject, status);
        if (status == DMUNIPZ_STATUS_DEVBUSERROR)    reqState = DMUNIPZ_STATE_ERROR;
        if (status == DMUNIPZ_STATUS_ERROR)          reqState = DMUNIPZ_STATE_ERROR;
        if (status == DMUNIPZ_STATUS_EBREADTIMEDOUT) reqState = DMUNIPZ_STATE_ERROR;
        break;
      case DMUNIPZ_STATE_FATAL :
        *pSharedState  = actState;
        *pSharedStatus = status;
        mprintf("dm-unipz: a FATAL error has occured. Good bye.\n");
        while (1) asm("nop"); // RIP!
        break;
      default :                                                             // avoid flooding WB bus with unnecessary activity
        for (j = 0; j < (DMUNIPZ_DEFAULT_TIMEOUT * DMUNIPZ_MS_ASMNOP); j++) { asm("nop"); }
      } // switch 

    // update shared memory
    *pSharedStatus    = status;
    *pSharedState     = actState;
    i++; *pSharedNIterMain = i;
    *pSharedStatTrans = statusTransfer;
    *pSharedVirtAcc   = virtAcc;
    *pSharedNTransfer = nTransfer;
    *pSharedNInject   = nInject;
  } // while
} // main

/********************************************************************************************
 *  dm-unipz.c
 *
 *  created : 2017
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 26-Jul-2018
 *
 *  lm32 program for gateway between UNILAC Pulszentrale and FAIR-style Data Master
 * 
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2017  Dietrich Beck
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
 * Last update: 25-April-2015
 ********************************************************************************************/
#define DMUNIPZ_FW_VERSION 0x000403                                   // make this consistent with makefile

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
#include "../../../top/gsi_scu/scu_mil.h"                             // register layout of 'MIL macro'
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_queue_regs.h" // register layout ECA queue
#include "../../../ip_cores/wr-cores/modules/wr_eca/eca_regs.h"       // register layout ECA control
#include "../../oled_display/oled_regs.h"                             // register layout of OLED display
#include "../../../ip_cores/saftlib/drivers/eca_flags.h"              // definitions for ECA queue
#include "../../ftm/include/ftm_common.h"                             // defs and regs for data master
#include "../../../syn/gsi_pexarria5/ftm/ftm_shared_mmap.h"           // info on shared map for data master lm32 cluster

#include "dm-unipz.h"                                                 // defs
#include "dm-unipz_smmap.h"                                           // shared memory map for communication via Wishbone

const char* dmunipz_status_text(uint32_t code) {
  switch (code) {
  case DMUNIPZ_STATUS_UNKNOWN          : return "unknown   ";
  case DMUNIPZ_STATUS_OK               : return "OK        ";
  case DMUNIPZ_STATUS_ERROR            : return "error     ";
  case DMUNIPZ_STATUS_TIMEDOUT         : return "timeout   ";
  case DMUNIPZ_STATUS_OUTOFRANGE       : return "bad range ";
  case DMUNIPZ_STATUS_REQTKFAILED      : return "bad TK req";
  case DMUNIPZ_STATUS_REQTKTIMEOUT     : return "req timout";
  case DMUNIPZ_STATUS_REQBEAMFAILED    : return "bad BM req";
  case DMUNIPZ_STATUS_RELTKFAILED      : return "bad TK rel";
  case DMUNIPZ_STATUS_RELBEAMFAILED    : return "bad BM rel";
  case DMUNIPZ_STATUS_DEVBUSERROR      : return "devbus err";
  case DMUNIPZ_STATUS_REQNOTOK         : return "bad req.  ";
  case DMUNIPZ_STATUS_REQBEAMTIMEDOUT  : return "req timout";
  case DMUNIPZ_STATUS_NOIP             : return "bad DHCP  ";
  case DMUNIPZ_STATUS_WRONGIP          : return "bad IP    ";
  case DMUNIPZ_STATUS_NODM             : return "no DM     ";                     
  case DMUNIPZ_STATUS_EBREADTIMEDOUT   : return "EB timeout";
  case DMUNIPZ_STATUS_WRONGVIRTACC     : return "bad vAcc  ";
  case DMUNIPZ_STATUS_SAFETYMARGIN     : return "margin exc";
  case DMUNIPZ_STATUS_NOTIMESTAMP      : return "no TLU TS ";
  case DMUNIPZ_STATUS_BADTIMESTAMP     : return "bad TLU TS";
  case DMUNIPZ_STATUS_DMQNOTEMPTY      : return "DMQ nEmpty";
  case DMUNIPZ_STATUS_LATEEVENT        : return "ECA late  ";
  case DMUNIPZ_STATUS_TKNOTRESERVED    : return "NO TK     ";
  case DMUNIPZ_STATUS_DMTIMEOUT        : return "DM timoeut";
  case DMUNIPZ_STATUS_BADSYNC          : return "bad sync  ";
  case DMUNIPZ_STATUS_WAIT4UNIEVENT    : return "no MIL evt";
  case DMUNIPZ_STATUS_BADSCHEDULEA     : return "bad sched ";
  case DMUNIPZ_STATUS_BADSCHEDULEB     : return "evt unexpt";
  case DMUNIPZ_STATUS_INVALIDBLKADDR   : return "invld addr";
  default                              : return "undef err ";
  }
}

const char* dmunipz_state_text(uint32_t code) {
  switch (code) {
  case DMUNIPZ_STATE_UNKNOWN      : return "UNKNOWN   ";
  case DMUNIPZ_STATE_S0           : return "S0        ";
  case DMUNIPZ_STATE_IDLE         : return "IDLE      ";                                       
  case DMUNIPZ_STATE_CONFIGURED   : return "CONFIGURED";
  case DMUNIPZ_STATE_OPREADY      : return "opReady   ";
  case DMUNIPZ_STATE_STOPPING     : return "STOPPING  ";
  case DMUNIPZ_STATE_ERROR        : return "ERROR     ";
  case DMUNIPZ_STATE_FATAL        : return "FATAL(RIP)";
  default                         : return "undefined ";
  }
}


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


uint32_t dmInt2ExtAddr(uint32_t intAddr, uint32_t extBaseAddr) // data master internal address -> external address
{
  uint32_t extAddr;

  extAddr = intAddr - INT_BASE_ADR;
  extAddr = extAddr + extBaseAddr;

  return extAddr;
} //dmInt2ExtAddr


// stuff required for environment
extern uint32_t*       _startshared[];
unsigned int cpuId, cpuQty;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;

// global variables 
volatile uint32_t *pECAQ;               // WB address of ECA queue
volatile uint32_t *pMILPiggy;           // WB address of MIL device bus (MIL piggy)
volatile uint32_t *pOLED;               // WB address of OLED
volatile uint32_t *pShared;             // pointer to begin of shared memory region                              
uint32_t *pSharedVersion;               // pointer to a "user defined" u32 register; here: publish version
uint32_t *pSharedStatus;                // pointer to a "user defined" u32 register; here: publish status
uint32_t *pSharedNIterMain;             // pointer to a "user defined" u32 register; here: publish # of iterations of main loop
uint32_t *pSharedNTransfer;             // pointer to a "user defined" u32 register; here: publish # of transfers
uint32_t *pSharedNInject;               // pointer to a "user defined" u32 register; here: publish # of injections (of current transfer)
uint32_t *pSharedVirtAcc;               // pointer to a "user defined" u32 register; here: publish # of virtual accelerator requested by Data Master
uint32_t *pSharedVirtAccRec;            // pointer to a "user defined" u32 register; here: publish # of virtual accelerator received from UNIPZ
uint32_t *pSharedNoBeam;                // pointer to a "user defined" u32 register; here: publish 'no beam' flag requested by Data Master
uint32_t *pSharedDtStart;               // pointer to a "user defined" u32 register; here: publish difference between actual time and flextime @ DM
uint32_t *pSharedDtSync;                // pointer to a "user defined" u32 register; here: publish time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER
uint32_t *pSharedDtInject;              // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER
uint32_t *pSharedDtTransfer;            // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER
uint32_t *pSharedDtTkreq;               // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_TKREQ and reply from UNIPZ
uint32_t *pSharedDtBreq;                // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_BREQ and reply from UNIPZ
uint32_t *pSharedDtBprep;               // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_BREQ and begin of request to UNIPZ
uint32_t *pSharedDtReady2Sis;           // pointer to a "user defined" u32 register; here: publish time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS
uint32_t *pSharedNR2sTransfer;          // pointer to a "user defined" u32 register; here: publish # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
uint32_t *pSharedNR2sCycle;             // pointer to a "user defined" u32 register; here: publish # of EVT_READY_TO_SIS events in between CMD_UNI_TKREL and the following CMD_UNI_TKREL
uint32_t *pSharedStatTrans;             // pointer to a "user defined" u32 register; here: publish status of ongoing transfer
uint32_t *pSharedNBadStatus;            // pointer to a "user defined" u32 register; here: publish # of bad status (=error) incidents
uint32_t *pSharedNBadState;             // pointer to a "user defined" u32 register; here: publish # of bad state (=FATAL, ERROR, UNKNOWN) incidents
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
volatile uint32_t *pSharedTkTimeout;    // pointer to a "user defined" u32 register; here: timeout value for TK (via UNIPZ)

uint32_t *pCpuRamExternal;              // external address (seen from host bridge) of this CPU's RAM            
uint32_t *pCpuRamExternalData4EB;       // external address (seen from host bridge) of this CPU's RAM: field for EB return values

WriteToPZU_Type  writePZUData;          // Modulbus SIS, I/O-Modul 1, Bits 0..15

uint32_t flexOffset;                    // offset added to obtain timestamp for "flex wait"
uint32_t uniTimeout;                    // timeout value for UNIPZ
uint32_t tkTimeout;                     // timeout value for TK (via UNIPZ)
uint32_t nBadStatus;                    // # of bad status (=error) incidents
uint32_t nBadState;                     // # of bad state (=FATAL, ERROR, UNKNOWN) incidents
uint64_t tReady2Sis;                    // time, when EVT_READY_TO_SIS was received
uint64_t tBreq;                         // time, when CMD_UNI_BREQ was received
uint64_t tTkreq;                        // time, when CMD_UNI_TCREQ was received
uint32_t flagTkReq;                     // flag for diagnostics: set to '1', when CMD_UNI_TCREQ is received; set to '0', when CMD_UNI_TCREL is received
uint32_t nR2sTransfer;                  // # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
uint32_t nR2sTotal;                     // total # of EVT_READY_TO_SIS events
uint32_t nR2sLastTkrel;                 // # of EVT_READY_TO_SIS events at last CMD_UNI_TKREL
uint32_t nR2sCycle;                     // # of EVT_READY_TO_SIS events since last CMD_UNI_TKREL


#define DM_NBLOCKS       2              // max number of blocks changed within the Data Master
dmComm  dmData[DM_NBLOCKS];             // data for treatment of blocks
#define REQBEAMA         0              // 1st block: handles DM for beam request, flow command
#define REQBEAMB         1              // 2nd block: handles DM for beam request, flex wait

/* disabled MSI usage, keep for later!
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

  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
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
  for (i=0; i<n32BitWords; i++) ebm_op(address + i*4, (uint32_t)(&(pCpuRamExternalData4EB[i])), EBM_READ); // put data into EB cycle
                                ebm_op(address      , (uint32_t)(&(pCpuRamExternalData4EB[handshakeIdx])), EBM_READ); // handshake data
  ebm_flush();                                                                                       // commit EB cycle via the network
  
  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;                                                     
  while (getSysTime() < timeoutT) {                                                                  // wait for received data until timeout
    if (pSharedData4EB[handshakeIdx] != DMUNIPZ_EB_HACKISH) {                                        // hackish solution to determine if a reply value has been received
      for (i=0; i<n32BitWords; i++) data[i] = pSharedData4EB[i];
      // dbg mprintf("dm-unipz: ebmReadN EB_address 0x%08x, nWords %d, data[0] 0x%08x, hackish 0x%08x, return 0x%08x\n", address, n32BitWords, data[0], DMUNIPZ_EB_HACKISH, pSharedData4EB[handshakeIdx]);
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


uint32_t dmCheckCmds(uint32_t blk1, uint32_t blk2) // check hashes of commands
{
  // two checks are run on the data of the commands, which we intend to send to the Data Maser later
  // A. --> verify, the addresses of the blocks are non-zero
  // B. - each block has a hash of its node names
  //    - when sending the addresses of the two blocks via a timing message, the Data Master XORs the hashes of
  //      both node names and sends the XORs result within the TEF field of the timing message
  //    --> verify, the data of both blocks contain equal TEF field values
  //    --> verify, the XORed hashes of both blocks match the value in the TEF field

  uint32_t xOredHashes; 

  // exclude non-zero block addresses (don't test dynpar1 as it is not always used)
  if (!(dmData[blk1].dynpar0))                  return DMUNIPZ_STATUS_INVALIDBLKADDR;
  if (!(dmData[blk2].dynpar0))                  return DMUNIPZ_STATUS_INVALIDBLKADDR;

  // exclude non-zero command addresses
  if (!(dmData[blk1].cmdAddr))                  return DMUNIPZ_STATUS_INVALIDBLKADDR;
  if (!(dmData[blk2].cmdAddr))                  return DMUNIPZ_STATUS_INVALIDBLKADDR;

  // exclude non identical TEF fields
  if ((dmData[blk1].tef) != (dmData[blk2].tef)) return DMUNIPZ_STATUS_INVALIDBLKADDR;

  // exclude non-fitting hashes
  xOredHashes = ((dmData[blk1].hash) ^ (dmData[blk2].hash));

  if ((dmData[blk1].tef) != xOredHashes)        return DMUNIPZ_STATUS_INVALIDBLKADDR;
  
  return DMUNIPZ_STATUS_OK;
} // dmCheckCmds


uint32_t dmClearCmd(uint32_t blk) // clear command data
{
  // - command data is received with every CMD_UNI_TCREQ event from the Data Master
  // - this data is only valid for one transfer
  // - a transfer is terminated via a CMD_UNI_TRREL event
  // - from then on, the command data are no longer valid
  // ==> the command data should be cleared with every CMD_UNI_TCREL command
  // - this prevents that commands are sent to the Data Master in case the schedule
  //   is corrupt

  dmData[blk].dynpar0                        = 0x0;
  dmData[blk].dynpar0                        = 0x0;
  dmData[blk].hash                           = 0x0;
  dmData[blk].tef                            = 0x0;
  dmData[blk].cmdAddr                        = 0x0;
  dmData[blk].blockWrIdxs                    = 0x0;
  dmData[blk].blockWrIdxsAddr                = 0x0;

  return DMUNIPZ_STATUS_OK;
} // dmCheckCmds


uint32_t dmPrepCmdCommon(uint32_t blk, uint32_t prio, uint32_t checkEmptyQ, uint64_t cmdValidTime) // prepare data common to all commands
{
  // simplified memory layout at DM
  //
  // blockAddr -> |...      |
  //              |IL       |
  //              |HI       |
  //              |Lo-------|--buffListAddr--> |buf0  |
  //              |wrIdx    |                  |buf1--|--cmdListAddr-->|cmd0  |                           
  //              |rdIdx    |                                          |cmd1--|--cmdAddr-->|TS valid Hi  |                           
  //              |...      |                                                              |TS valid Lo  |                           
  //                                                                                       |action(type) |
  //                                                                                       |type specific|
  //                                                                           
  //                                                                           
  //                                                   

  uint32_t blockAddr;                                          // address of begin of control block
  uint32_t blockData[(_MEM_BLOCK_SIZE >> 2)];                  // buffer for reading the info of an entire block
  
  uint32_t wrIdxs;                                             // write indices for all prios (8 bit N/A, 8 bit IL, 8 bit Hi, 8 bit Lo)
  uint8_t  wrIdx;                                              // write index for relevant priority
  uint32_t rdIdxs;                                             // read indices for all prios (8 bit N/A, 8 bit IL, 8 bit Hi, 8 bit Lo)
  uint8_t  rdIdx;                                              // read index for relevant priority
  uint32_t hash;                                               // hash of node name

  uint32_t buffListAddrIL;                                     // address of buffer list (of interlock priority Q)
  uint32_t buffListAddrHi;                                     // address of buffer list (of high priority Q)
  uint32_t buffListAddrLo;                                     // address of buffer list (of low priority Q)
  
  uint32_t buffListAddr;                                       // address of  buffer list (of relevant priority)
  uint32_t buffListAddrOffs;                                   // where to find the relevant command buffer within the buffer list
  uint32_t buffAddr;                                           // address of relevant command buffer; buffListAdd + buffListAddOffs
  
  uint32_t cmdListAddr;                                        // address of command list 
  uint32_t cmdListAddrOffs;                                    // where to find the relevant command  within the command list
  uint32_t cmdAddr;                                            // address of relevant command; cmdListAddr + cmdListAddrOffs
  
  uint32_t cmdValidTSHi;                                       // time when command becomes valid, high32 bit
  uint32_t cmdValidTSLo;                                       // time when command becomes valid, low32 bit
  
  uint32_t extBaseAddr;                                        // external base address of dm; seen from 'world' perspective

  uint32_t status;

  if (prio > 2) {
    DBPRINT1("dm-unipz: prep cmd error: illegal value for priority: %d\n", prio);    
    return DMUNIPZ_STATUS_OUTOFRANGE;
  } // if prio illegal
  
  // set Data Master (of relevant lm32) base address from external perspective
  //intBaseAddr      = INT_BASE_ADR;       
  blockAddr      = dmData[blk].dynpar0;
  extBaseAddr    = dmExt2BaseAddr(blockAddr);
  DBPRINT3("dm-unipz: prep cmd for block address0x%08x, extBaseAddr 0x%08x\n", blockAddr, extBaseAddr);

  
  // get data of block from DM
   if ((status = ebmReadN(2000, blockAddr, blockData, _MEM_BLOCK_SIZE >> 2)) != DMUNIPZ_STATUS_OK) return status;

  // assign to local variables 
  buffListAddrLo = blockData[BLOCK_CMDQ_LO_PTR >> 2];      // address of buffer list of low prio Q
  buffListAddrHi = blockData[BLOCK_CMDQ_HI_PTR >> 2];      // address of buffer list of high prio Q
  buffListAddrIL = blockData[BLOCK_CMDQ_IL_PTR >> 2];      // address of buffer list of interlock prio Q
  wrIdxs         = blockData[BLOCK_CMDQ_WR_IDXS >> 2];     // write indices for all prios
  rdIdxs         = blockData[BLOCK_CMDQ_RD_IDXS >> 2];     // read indices for all prios
  hash           = blockData[NODE_HASH >> 2];              // hash of node

  // write and read index bytes
  wrIdx        = (uint8_t)(wrIdxs >> (prio * 8));
  rdIdx        = (uint8_t)(rdIdxs >> (prio * 8));

  //mprintf("debug: blk %d, blkaddr 0x%x, wrIdx %d, rdIdx %d %d\n", blk, blockAddr, wrIdx, rdIdx, prio);

  // check if Q is empty
  if (checkEmptyQ) {
    if (wrIdx != rdIdx) return DMUNIPZ_STATUS_DMQNOTEMPTY;
  } // if checkEmptyQ

  // don't mess up DM: verify queue of relevant prio is _not_ full. In that case we are not allowed to write to queue and we have to give up
  if (((wrIdx & Q_IDX_MAX_OVF_MSK) != (rdIdx & Q_IDX_MAX_OVF_MSK)) && ((wrIdx & Q_IDX_MAX_MSK) == (rdIdx & Q_IDX_MAX_MSK))) {
    DBPRINT1("dm-unipz: prep cmd error: queue for priority %d full\n", prio);
    return DMUNIPZ_STATUS_OUTOFRANGE;
  } // if prio q full
    
  // offset in buffer list and command list
  buffListAddrOffs = (wrIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_ ) * _PTR_SIZE_;
  cmdListAddrOffs  = (wrIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_ ) * _T_CMD_SIZE_; 
  
  // buffer list according to prio, convert to external perspective, add offset
  switch (prio) {
  case 2 :
    buffListAddr   = buffListAddrIL;
    break;
  case 1 :
    buffListAddr   = buffListAddrHi;
    break;
  default:
    buffListAddr   = buffListAddrLo;
  }
  buffListAddr     = dmInt2ExtAddr(buffListAddr, extBaseAddr);
  buffAddr         = buffListAddr + buffListAddrOffs;
  DBPRINT2("dm-unipz: prep cmd buffListAddr 0x%08x, buffAddr  0x%08x\n", buffListAddr, buffAddr);
  
  // read address of command list and calculate address of relevant command
  if ((status = ebmReadN(2000, buffAddr, &cmdListAddr, 1)) != DMUNIPZ_STATUS_OK) return status;
  cmdListAddr      = dmInt2ExtAddr(cmdListAddr, extBaseAddr);
  cmdAddr          = cmdListAddr + cmdListAddrOffs;

  // timestamp when command shall become valid
  cmdValidTSHi     = (uint32_t)(cmdValidTime >> 32);
  cmdValidTSLo     = (uint32_t)(cmdValidTime & 0xffffffff); 

  // update value for write index
  wrIdxs            = wrIdxs & ~(0xff << (prio * 8));                                 // clear current value of write index for relevant priority
  wrIdxs            = wrIdxs | (((wrIdx + 1) & Q_IDX_MAX_OVF_MSK) << (prio * 8));     // update value of write index for relevant priority

  DBPRINT3("dm-unipz: prep cmd validTSHi 0x%08x\n", cmdValidTSHi);
  DBPRINT3("dm-unipz: prep cmd validTSLo 0x%08x\n", cmdValidTSLo);
  
  // assign prepared values for later use;
  dmData[blk].hash                           = hash;
  dmData[blk].cmdAddr                        = cmdAddr;
  dmData[blk].cmdData[(T_CMD_TIME >> 2) + 0] = cmdValidTSHi;  
  dmData[blk].cmdData[(T_CMD_TIME >> 2) + 1] = cmdValidTSLo;  
  dmData[blk].blockWrIdxs                    = wrIdxs;
  dmData[blk].blockWrIdxsAddr                = blockAddr + BLOCK_CMDQ_WR_IDXS;  
  DBPRINT2("dm-unipz: prep cmd wrIdxAddr 0x%08x, wrIdxs 0x%08x\n", dmData[blk].blockWrIdxsAddr, wrIdxs);
  
  return DMUNIPZ_STATUS_OK;
} //dmPrepCmdCommon


uint32_t dmPrepCmdFlow(uint32_t blk) // prepare flow CMD for DM - need to call dmPrepCmdCommon first
{
  // simplified memory layout of flow command
  //
  // dmCmdAddr-->|TS valid Hi|
  //             |TS valid Lo|
  //             |action     |
  //             |flow dest. |
  //             |res        | 
  //             |...        |               

  uint32_t cmdAction;                                          // action flags of command
  uint32_t cmdFlowDestAddr;                                    // address of flow destination
  
  // set command action type to flow
  cmdAction        = (ACT_TYPE_FLOW & ACT_TYPE_MSK) << ACT_TYPE_POS;  // set type to "flow"
  cmdAction       |= (            1 & ACT_QTY_MSK)  << ACT_QTY_POS;   // set quantity to "1"
  cmdAction       |= (      PRIO_LO & ACT_PRIO_MSK) << ACT_PRIO_POS;  // set prio to "Low"

  // set address of flow destination
  cmdFlowDestAddr  = dmExt2IntAddr(dmData[blk].dynpar1);

  DBPRINT3("dm-unipz: prep  dmPrepFLowCmd for blk %d\n", blk);
  DBPRINT3("dm-unipz: prep  cmdAction 0x%08x, index %d\n", cmdAction, T_CMD_ACT >> 2);
  DBPRINT3("dm-unipz: prep  cmdFlowDestAddr 0x%08x, index %d\n", cmdFlowDestAddr, T_CMD_FLOW_DEST >> 2);
  DBPRINT3("dm-unipz: prep  cmdFlowReserved 0x%08x, index %d\n", 0x0, T_CMD_RES >> 2);
  
  // assign prepared values for later use;
  dmData[blk].cmdData[T_CMD_ACT >> 2]        = cmdAction;
  dmData[blk].cmdData[T_CMD_FLOW_DEST >> 2]  = cmdFlowDestAddr; 
  dmData[blk].cmdData[T_CMD_RES >> 2]        = 0x0;

  return DMUNIPZ_STATUS_OK;
} //dmPrepFlowCmd


uint32_t dmPrepCmdFlush(uint32_t blk) // prepare flush CMD for DM - need to call dmPrepCmdCommon first
{
  // simplified memory layout of flexwait command
  //
  // dmCmdAddr-->|TS valid Hi  |
  //             |TS valid Lo  |
  //             |action       |
  //             |...          | 


  uint32_t cmdAction;                                          // action flags of command

  // set command action type to flush
  cmdAction        = (      ACT_TYPE_FLUSH & ACT_TYPE_MSK) << ACT_TYPE_POS;       // set type to "flush"
  cmdAction       |= (                    1 & ACT_QTY_MSK) << ACT_QTY_POS;        // set quantity to "1"
  cmdAction       |= (             PRIO_HI & ACT_PRIO_MSK) << ACT_PRIO_POS;       // set prio to "1"
  cmdAction       |= ((1 << PRIO_LO) & ACT_FLUSH_PRIO_MSK) << ACT_FLUSH_PRIO_POS; // flush lo prio q

  dmData[blk].cmdData[T_CMD_ACT >> 2]            = cmdAction;

  
  return DMUNIPZ_STATUS_OK;  
} //cmdPrepCmdFlush


uint32_t dmPrepFlexWaitCmd(uint32_t blk, uint64_t timestamp) // prepare flexible waiting CMD for DM - need to call dmPrepCmdCommon first
{
  // simplified memory layout of flexwait command
  //
  // dmCmdAddr-->|TS valid Hi|
  //             |TS valid Lo|
  //             |action     |
  //             |TS wait Hi |
  //             |TS wait Lo |
  //             |...        | 

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

  DBPRINT3("dm-unipz: prep  dmPrepFlexWaitCmd for blk %d\n", blk);
  DBPRINT3("dm-unipz: prep  cmdAction 0x%08x, index %d\n", cmdAction, T_CMD_ACT >> 2);
  DBPRINT3("dm-unipz: prep  cmdWaitTimeHi %09u, index %d\n", cmdWaitTimeHi, T_CMD_WAIT_TIME >> 2);
  DBPRINT3("dm-unipz: prep  cmdWaitTimeLo %09u, index %d\n", cmdWaitTimeLo, (T_CMD_WAIT_TIME >> 2) + 1);
  
  // assign prepared values for later use;
  dmData[blk].cmdData[T_CMD_ACT >> 2]             = cmdAction;
  dmData[blk].cmdData[T_CMD_WAIT_TIME >> 2]       = cmdWaitTimeHi;
  dmData[blk].cmdData[(T_CMD_WAIT_TIME >> 2) + 1] = cmdWaitTimeLo;
  
  return DMUNIPZ_STATUS_OK;
} //dmPrepFlexWaitCmd


void dmChangeBlock(uint32_t blk)     // alter a block within the Data Master on-the fly
{
  ebmWriteN(dmData[blk].cmdAddr, dmData[blk].cmdData, (_T_CMD_SIZE_ >> 2));  
  ebmWriteN(dmData[blk].blockWrIdxsAddr, &dmData[blk].blockWrIdxs, 1);             
  DBPRINT2("dm-unipz: dmChangeBlock blk %d, cmdAddr 0x%08x, cmdData[0] 0x%08x, cmdData[1] 0x%08x\n", blk, dmData[blk].cmdAddr, dmData[blk].cmdData[0], dmData[blk].cmdData[1]);
} // dmChangeBlock


void init() // typical init for lm32
{
  discoverPeriphery();        // mini-sdb ...
  uart_init_hw();             // needed by WR console   
  cpuId = getCpuIdx();

  timer_init(1);              // needed by usleep_init() 
  usleep_init();              // needed by scu_mil.c

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
  pSharedVersion      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_VERSION >> 2));
  pSharedStatus       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATUS >> 2));
  pSharedCmd          = (uint32_t *)(pShared + (DMUNIPZ_SHARED_CMD >> 2));
  pSharedState        = (uint32_t *)(pShared + (DMUNIPZ_SHARED_STATE >> 2));
  pSharedNIterMain    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NITERMAIN >> 2));
  pSharedNTransfer    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSN >> 2));
  pSharedNInject      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_INJECTN >> 2));
  pSharedVirtAcc      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSVIRTACC >> 2));
  pSharedNoBeam       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSNOBEAM >> 2));
  pSharedDtStart      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTSTART >> 2));
  pSharedDtSync       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTSYNC >> 2));
  pSharedDtInject     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTINJECT >> 2));
  pSharedDtTransfer   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTTRANSFER >> 2));
  pSharedDtTkreq      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTTKREQ >> 2));
  pSharedDtBreq       = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTBREQ >> 2));
  pSharedDtBprep      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTBPREP >> 2));
  pSharedDtReady2Sis  = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DTREADY2SIS >> 2));
  pSharedNR2sTransfer = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NR2STRANSFER >> 2));
  pSharedNR2sCycle    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NR2SCYCLE >> 2));
  pSharedVirtAccRec   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_RECVIRTACC >> 2));
  pSharedStatTrans    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TRANSSTATUS >> 2));
  pSharedData4EB      = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DATA_4EB_START >> 2));
  pSharedSrcMacHi     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCMACHI >> 2));
  pSharedSrcMacLo     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCMACLO >> 2));
  pSharedSrcIP        = (uint32_t *)(pShared + (DMUNIPZ_SHARED_SRCIP >> 2));
  pSharedDstMacHi     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTMACHI >> 2));
  pSharedDstMacLo     = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTMACLO >> 2));
  pSharedDstIP        = (uint32_t *)(pShared + (DMUNIPZ_SHARED_DSTIP >> 2));
  pSharedFlexOffset   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_OFFSETFLEX >> 2));
  pSharedUniTimeout   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_UNITIMEOUT >> 2));
  pSharedTkTimeout    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_TKTIMEOUT >> 2));
  pSharedNBadStatus   = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NBADSTATUS >> 2));
  pSharedNBadState    = (uint32_t *)(pShared + (DMUNIPZ_SHARED_NBADSTATE >> 2));  
  
  // find address of CPU from external perspective
  idx = 0;
  find_device_multi(&found_clu, &idx, 1, GSI, LM32_CB_CLUSTER);	
  idx = 0;
  find_device_multi_in_subtree(&found_clu, &found_sdb[0], &idx, c_Max_Rams, GSI, LM32_RAM_USER);
  if(idx >= cpuId) {
    pCpuRamExternal           = (uint32_t *)(getSdbAdr(&found_sdb[cpuId]) & 0x7FFFFFFF); // CPU sees the 'world' under 0x8..., remove that bit to get host bridge perspective
    pCpuRamExternalData4EB    = (uint32_t *)(pCpuRamExternal + ((DMUNIPZ_SHARED_DATA_4EB_START + SHARED_OFFS) >> 2));
  }

  DBPRINT2("dm-unipz: CPU RAM External 0x%8x, begin shared 0x%08x\n", pCpuRamExternal, SHARED_OFFS);

  // set initial values;
  ebmClearSharedMem();
  *pSharedVersion    = DMUNIPZ_FW_VERSION; // of all the shared variabes, only VERSION is a constant. Set it now!
  *pSharedFlexOffset = DMUNIPZ_OFFSETFLEX; // initialize with default value
  *pSharedUniTimeout = DMUNIPZ_UNITIMEOUT; // initialize with default value
  *pSharedTkTimeout  = DMUNIPZ_TKTIMEOUT;  // initialize with default value
  *pSharedNBadStatus = 0;
  *pSharedNBadState  = 0;
} // initSharedMem 


uint32_t findMILPiggy() //find WB address of MIL Piggy
{
  pMILPiggy = 0x0;
  
  // get Wishbone address for MIL Piggy
  pMILPiggy = find_device_adr(GSI, SCU_MIL);

  if (!pMILPiggy) {DBPRINT1("dm-unipz: can't find MIL piggy\n"); return DMUNIPZ_STATUS_ERROR;}
  else                                                           return DMUNIPZ_STATUS_OK;
} // findMILPiggy


uint32_t findOLED() //find WB address of OLED
{
  pOLED = 0x0;
  
  // get Wishbone address for OLED
  pOLED = find_device_adr(OLED_SDB_VENDOR_ID, OLED_SDB_DEVICE_ID);

  if (!pOLED) {DBPRINT1("dm-unipz: can't find OLED\n"); return DMUNIPZ_STATUS_ERROR;}
  else                                                  return DMUNIPZ_STATUS_OK;
} // findOLED


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
  else                                                       return DMUNIPZ_STATUS_OK;
} // findECAQueue


uint32_t wait4ECAEvent(uint32_t msTimeout, uint32_t *virtAcc, uint32_t *dryRunFlag, uint64_t *deadline, uint32_t *isLate)  // 1. query ECA for actions, 2. trigger activity
{
  uint32_t *pECAFlag;           // address of ECA flag
  uint32_t evtIdHigh;           // high 32bit of eventID   
  uint32_t evtIdLow;            // low 32bit of eventID    
  uint32_t evtDeadlHigh;        // high 32bit of deadline  
  uint32_t evtDeadlLow;         // low 32bit of deadline   
  uint32_t evtParamHigh;        // high 32 bit of parameter field
  uint32_t evtParamLow ;        // low 32 bit of parameter field
  uint32_t evtTef;              // 32 bit TEF field
  uint32_t actTag;              // tag of action           
  uint32_t nextAction;          // describes what to do next
  uint64_t timeoutT;            // when to time out

  *virtAcc    = 42;             // 42: indicate 'no value'
  *dryRunFlag = 2;              //  2: indicate 'no value'
  pECAFlag    = (uint32_t *)(pECAQ + (ECA_QUEUE_FLAGS_GET >> 2));   // address of ECA flag
  timeoutT    = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;

  while (getSysTime() < timeoutT) {
    if (*pECAFlag & (0x0001 << ECA_VALID)) {                        // if ECA data is valid

      // read data
      evtIdHigh    = *(pECAQ + (ECA_QUEUE_EVENT_ID_HI_GET >> 2));
      evtIdLow     = *(pECAQ + (ECA_QUEUE_EVENT_ID_LO_GET >> 2));
      evtDeadlHigh = *(pECAQ + (ECA_QUEUE_DEADLINE_HI_GET >> 2));
      evtDeadlLow  = *(pECAQ + (ECA_QUEUE_DEADLINE_LO_GET >> 2));
      actTag       = *(pECAQ + (ECA_QUEUE_TAG_GET >> 2));
      evtParamHigh = *(pECAQ + (ECA_QUEUE_PARAM_HI_GET >> 2));
      evtParamLow  = *(pECAQ + (ECA_QUEUE_PARAM_LO_GET >> 2));
      evtTef       = *(pECAQ + (ECA_QUEUE_TEF_GET >> 2));
      *isLate      = *pECAFlag & (0x0001 << ECA_LATE);

      *deadline    = ((uint64_t)evtDeadlHigh << 32) + (uint64_t)evtDeadlLow;
      
      // pop action from channel
      *(pECAQ + (ECA_QUEUE_POP_OWR >> 2)) = 0x1;

      // here: do s.th. according to tag
      switch (actTag) 
        {
        case DMUNIPZ_ECADO_REQTK :
          nextAction  = DMUNIPZ_ECADO_REQTK;
          *virtAcc    = evtIdLow & 0xf;  
          *dryRunFlag = (evtIdLow & 0x10) != 0;
          dmData[REQBEAMA].dynpar0  = evtParamLow;                   // address of block (slow wait with timeout)
          dmData[REQBEAMA].dynpar1  = evtParamHigh;                  // address of block (fast flex wait)
          dmData[REQBEAMA].tef      = evtTef;                        // TEF field
          dmData[REQBEAMB].dynpar0  = evtParamHigh;                  // address of block (fast flex wait)
          dmData[REQBEAMB].dynpar1  = 0x0;
          dmData[REQBEAMB].tef      = evtTef;                        // TEF field
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
        case DMUNIPZ_ECADO_READY2SIS :
          nextAction = DMUNIPZ_ECADO_READY2SIS;
          break;
        case  DMUNIPZ_ECADO_MBTRIGGER :
          nextAction = DMUNIPZ_ECADO_MBTRIGGER;
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
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData)) != MIL_STAT_OK) {
    DBPRINT1("dm-unipz: writeToPZU failed (address), MIL error code %d\n", busStatus);
    return busStatus;
  } // if busStatus not ok

  // write data word
  wData     = data;
  busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_W, wData);
  if (busStatus != MIL_STAT_OK) DBPRINT1("dm-unipz: writeToPZU failed (data), MIL error code %d\n", busStatus);
  
  return (busStatus);
} // writeToPZU


int16_t readFromPZU(uint16_t ifbAddr, uint16_t modAddr, uint16_t *data) // read bit field from module bus input (linked to UNI PZ)
{
  uint16_t wData      = 0x0;    // data to write
  uint16_t rData      = 0x0;    // data to read
  int16_t  busStatus  = 0;      // status of bus operation
  
  // select module
  wData     = (modAddr << 8) | C_IO32_KANAL_0;
  if ((busStatus = writeDevMil(pMILPiggy, ifbAddr, IFB_ADR_BUS_W, wData))  != MIL_STAT_OK) {
    DBPRINT1("dm-unipz: readFromPZU failed (address), MIL error code %d\n", busStatus);
    return busStatus;
  } // if busStatus not ok

  // read data
  if ((busStatus = readDevMil(pMILPiggy, ifbAddr, IFB_DATA_BUS_R, &rData)) == MIL_STAT_OK) *data = rData;
  if (busStatus != MIL_STAT_OK) DBPRINT1("dm-unipz: readFromPZU failed (data), MIL error code %d\n", busStatus);
  
  return(busStatus);
} // readFromPZU 


uint32_t checkClearReqNotOk(uint32_t msTimeout)      // check for 'Req not OK' flag from UNILAC. If the flag is set, try to clear it
{
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  int16_t          status;       // status MIL device bus operation
  uint64_t         timeoutT;     // when to time out

  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;

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

  if (virtAcc > 0xf) {
    DBPRINT3("dm-unipz: illegal value for virtual accelerator %d\n", virtAcc);
    return DMUNIPZ_STATUS_OUTOFRANGE;
  } // if number of virtual accelerator illegal

  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;

  // send request to modulbus I/O (UNIPZ)
  writePZUData.uword               = 0x0;
  writePZUData.bits.TK_Request     = true;
  writePZUData.bits.SIS_Acc_Select = virtAcc;
  writePZUData.bits.ReqNoBeam      = dryRunFlag;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;

  // check for acknowledgement, 'request not ok' or timeout
  while (getSysTime() < timeoutT) {                                                                                                   // check for timeout
    if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR; // read from modulbus I/O (UNIPZ)
    if (readPZUData.bits.TK_Req_Ack == true) return DMUNIPZ_STATUS_OK;                                                                // check for acknowledgement
    if (readPZUData.bits.Req_not_ok == true) return DMUNIPZ_STATUS_REQTKFAILED;                                                       // check for 'request not ok'
  } // while not timed out

  return DMUNIPZ_STATUS_REQTKTIMEOUT;
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
  ReadFromPZU_Type readPZUData;  // Modulbus SIS, I/O-Modul 3, Bits 0..15
  uint64_t         timeoutT;     // when to time out

  // send request to modulbus I/O (UNIPZ)
  writePZUData.bits.SIS_Request  = true;
  if ((status = writeToPZU(IFB_ADDRESS_SIS, IO_MODULE_1, writePZUData.uword)) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;
  
  // query UNIPZ for 'req Ack'
  
  timeoutT = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;

  while (getSysTime() < timeoutT) {                                                                                                    // check for timeout
    if ((status = readFromPZU(IFB_ADDRESS_SIS, IO_MODULE_3, &(readPZUData.uword))) != MIL_STAT_OK) return DMUNIPZ_STATUS_DEVBUSERROR;   
    if (readPZUData.bits.SIS_Req_Ack == true)                                                      return DMUNIPZ_STATUS_OK;           
    if (readPZUData.bits.Req_not_ok == true)                                                       return DMUNIPZ_STATUS_REQBEAMFAILED; 
  } // while not timed out

  return DMUNIPZ_STATUS_REQBEAMTIMEDOUT;
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


uint32_t wait4MILEvent(uint16_t evtCode, uint16_t virtAccReq, uint32_t *virtAccRec, uint32_t msTimeout)  // wait for MIL event or timeout
{
  uint32_t evtDataRec;         // data of one MIL event
  uint32_t evtCodeRec;         // "event number"
  uint64_t timeoutT;           // when to time out
  uint32_t virtAccTmp;         // temp value for virtAcc

  timeoutT    = getSysTime() + (uint64_t)msTimeout * (uint64_t)1000000;
  *virtAccRec = 0;             // last two digits: virtacc; count other junk in FIFO by increasing by 100;

  while(getSysTime() < timeoutT) {              // while not timed out...
    while (fifoNotemptyEvtMil(pMILPiggy)) {     // while fifo contains data
      popFifoEvtMil(pMILPiggy, &evtDataRec);    
      evtCodeRec  = evtDataRec & 0x000000ff;    // extract event code
      virtAccTmp  = (evtDataRec >> 8) & 0x0f;   // extract virtual accelerator (assuming event message)

      *virtAccRec += 100;                       // increase by 100 for each FIFO entry
      
      if (evtCodeRec == evtCode) {
        *virtAccRec += virtAccTmp;
        if (virtAccTmp == virtAccReq) return DMUNIPZ_STATUS_OK;
        else                          return DMUNIPZ_STATUS_WRONGVIRTACC;
      } // if evtCode

      // chck mprintf("dm-unipz: virtAcc %03d, evtCode %03d\n", virtAcc, evtCodeRec);
    } // while fifo contains data
    asm("nop");                                 // wait a bit...
  } // while not timed out

  // up to here: no matching evtCode AND matching virtAcc
  *virtAccRec += 42;                            // mark with illegal virtAcc number
  return DMUNIPZ_STATUS_TIMEDOUT;  
} //wait4MILEvent


void pulseLemo2() //for debugging with scope
{
  uint32_t i;

  setLemoOutputEvtMil(pMILPiggy, 2, 1);
  for (i=0; i< 10 * DMUNIPZ_US_ASMNOP; i++) asm("nop");
  setLemoOutputEvtMil(pMILPiggy, 2, 0);
} // pulseLemo2


void printOLED(char *chars)
{
  uint32_t i;
  
  for (i=0;i<strlen(chars);i++) *(pOLED + (OLED_UART_OWR >> 2)) = chars[i];
} // printOLED

void updateOLED(uint32_t statusTransfer, uint32_t virtAcc, uint32_t nTransfer, uint32_t nInject, uint32_t status, uint32_t actState)
{
  char     c[32];
  
  if (!pOLED) return;                         // no OLED: just return

  *(pOLED + (OLED_UART_OWR >> 2)) = 0xc;      // clear display

  sprintf(c, "%s\n", dmunipz_state_text(actState)); printOLED(c);
  sprintf(c, "%s\n", dmunipz_status_text(status));  printOLED(c);
  sprintf(c, "%vA %7d\n", virtAcc);                 printOLED(c);
  sprintf(c, "nT %7d\n", nTransfer);                printOLED(c);
  sprintf(c, "sT  %d%d%d%d%d%d\n",
          ((statusTransfer & DMUNIPZ_TRANS_REQTK    ) > 0),  
          ((statusTransfer & DMUNIPZ_TRANS_REQTKOK  ) > 0), 
          ((statusTransfer & DMUNIPZ_TRANS_RELTK    ) > 0),
          ((statusTransfer & DMUNIPZ_TRANS_REQBEAM  ) > 0),
          ((statusTransfer & DMUNIPZ_TRANS_REQBEAMOK) > 0),
          ((statusTransfer & DMUNIPZ_TRANS_RELBEAM  ) > 0)
          );                                        printOLED(c);
} // updateOLED


uint32_t doActionS0()
{
  uint32_t status = DMUNIPZ_STATUS_OK;

  if (findECAQueue() != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_ERROR; 
  if (findMILPiggy() != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_ERROR;
  if (findOLED()     != DMUNIPZ_STATUS_OK) status = DMUNIPZ_STATUS_OK;     // in case SCU has no OLED: ignore
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
  uint64_t timestamp;
  uint32_t isLate;
  
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

  // reset MIL piggy and wait
  if ((status = resetPiggyDevMil(pMILPiggy))  != MIL_STAT_OK) {
    DBPRINT1("dm-unipz: ERROR - can't reset MIL Piggy\n");
    return DMUNIPZ_STATUS_DEVBUSERROR;
  } 
  
  // check if modulbus I/O is ok
  if ((status = echoTestDevMil(pMILPiggy, IFB_ADDRESS_SIS, 0xbabe)) != MIL_STAT_OK) {
    DBPRINT1("dm-unipz: ERROR - modulbus SIS IFK not available at (ext) base address 0x%08x! Error code is %d\n", ((uint32_t)pMILPiggy & 0x7FFFFFFF), status);
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
  while (wait4ECAEvent(1, &virtAcc, &dryRunFlag, &timestamp, &isLate) !=  DMUNIPZ_ECADO_TIMEOUT) {i++;}
  DBPRINT1("dm-unipz: ECA queue flushed - removed %d pending entries from ECA queue\n", i);

  flexOffset  = *pSharedFlexOffset;
  uniTimeout  = *pSharedUniTimeout;
  tkTimeout   = *pSharedTkTimeout;

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


void cmdHandler(uint32_t *reqState, uint32_t *statusTransfer) // handle commands from the outside world
{
  uint32_t cmd;

  cmd = *pSharedCmd;
  // check, if the command is valid and request state change
  if (cmd) {
    switch (cmd) {
    case DMUNIPZ_CMD_CONFIGURE :
      *reqState =  DMUNIPZ_STATE_CONFIGURED;
      DBPRINT3("dm-unipz: received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STARTOP :
      *reqState = DMUNIPZ_STATE_OPREADY;
      DBPRINT3("dm-unipz: received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_STOPOP :
      *reqState = DMUNIPZ_STATE_STOPPING;
      DBPRINT3("dm-unipz: received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_IDLE :
      *reqState = DMUNIPZ_STATE_IDLE;
      DBPRINT3("dm-unipz: received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_RECOVER :
      *reqState = DMUNIPZ_STATE_IDLE;
      DBPRINT3("dm-unipz: received cmd %d\n", cmd);
      break;
    case DMUNIPZ_CMD_RELEASETK :
      releaseTK();   // force release of TK request independently of state or status
      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELTK;
      DBPRINT1("dm-unipz: received cmd %d, forcing release of TK request\n", cmd);
      break;
    case DMUNIPZ_CMD_RELEASEBEAM :
      releaseBeam(); // force release of beam request indpendently of state or status
      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELBEAM;   
      DBPRINT1("dm-unipz: received cmd %d, forcing release of beam request\n", cmd);
      break;
    default:
      DBPRINT3("dm-unipz: received unknown command '0x%08x'\n", cmd);
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
      if      (*reqState == DMUNIPZ_STATE_IDLE)       {                                            nextState = *reqState;}
      else if (*reqState == DMUNIPZ_STATE_CONFIGURED) {statusTransition = entryActionConfigured(); nextState = *reqState;}
      else if (*reqState == DMUNIPZ_STATE_OPREADY)    {statusTransition = entryActionOperation();  nextState = *reqState;}
      break;
    case DMUNIPZ_STATE_OPREADY:
      if      (*reqState == DMUNIPZ_STATE_STOPPING)   {statusTransition = exitActionOperation();   nextState = *reqState;}
      break;
    case DMUNIPZ_STATE_STOPPING:
      nextState = DMUNIPZ_STATE_CONFIGURED;      //automatic transition but without entryActionConfigured
    case DMUNIPZ_STATE_ERROR:
      if      (*reqState == DMUNIPZ_STATE_IDLE)       {statusTransition = exitActionError();       nextState = *reqState;}
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


uint32_t doActionOperation(uint32_t *statusTransfer,          // status bits indicating status of transfer
                           uint32_t *virtAccReq,              // virtual accelerator requested from Data Master
                           uint32_t *virtAccRec,              // virtual accelerator received from UNIPZ
                           uint32_t *noBeam,                  // 'no beam' flag from Data Master: UNILAC requested but without beam
                           uint64_t *dtStart,                 // remaining time budget for DM after 'flex command' has been sent, minimum value is 1ms
                           uint64_t *dtSync,                  // time difference between EVT_READY_2_SIS and EVT_MB_TRIGGER, should be 10ms exactly
                           uint64_t *dtInject,                // time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER, must be larger than 10ms
                           uint64_t *dtTransfer,              // time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER, for diagnostics only
                           uint64_t *dtTkreq,                 // time difference between CMD_UNI_TKREQ and reply from UNIPZ
                           uint64_t *dtBreq,                  // time difference between CMD_UNI_BREQ and reply from UNIPZ
                           uint64_t *dtBprep,                 // time difference between CMD_UNI_BREQ and begin of request to UNIPZ
                           uint64_t *dtReady2Sis,             // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS
                           uint32_t *nTransfer,               // total number of transfers since start of firmware
                           uint32_t *nInject,                 // number of injections withing ongoing transfer
                           uint32_t actStatus)                // actual status of firmware
{
  uint32_t status, dmStatus, milStatus;                                            // status returned by routines
  uint32_t flagEBTimeout;                                                          // flag indicating EB communication with DM timed out
  uint32_t flagMilEvtValid;                                                        // flag indicating that we recevied a valid MIL event
  uint32_t flagDryRun;                                                             // flag indicating that UNILAC is requested but without beam
  uint32_t flagIsLate;                                                             // flag indicating that a 'late event' was received from data master
  uint32_t flagNoCmd;                                                              // flag indicating we should not send a command to DM
  uint32_t nextAction;                                                             // action triggered by event received from ECA
  uint32_t virtAcc;                                                                // # of virtual accelerator in event received from ECA
  uint32_t dummy1, dummy2;                                                         // dummy variables
  uint64_t deadline;                                                               // deadline of event received via ECA
  uint64_t tDmTimeout;                                                             // time, when beam request at DM will timeout
  uint64_t tCmdFlex;                                                               // time, when DM is requested to continue its schedule after flex wait
  uint64_t tCmdValid;                                                              // time, when commands sent to DM shall become valid
  uint32_t ecaInjAction;                                                           // action received by ECA during injection

  status = actStatus; 

  nextAction = wait4ECAEvent(DMUNIPZ_DEFAULT_TIMEOUT, &virtAcc, &flagDryRun, &deadline, &flagIsLate);  // 'do action' is driven by actions issued by the ECA

  switch (nextAction) 
    {
    case DMUNIPZ_ECADO_REQTK :                                                     // received command "REQ_TK" from data master

      if (flagIsLate) return DMUNIPZ_STATUS_LATEEVENT;                             // never request TK in case of a late event

      (*nTransfer)++;                                                              // diagnostics: increment number of transfers

      //---- init values
      *virtAccReq     = virtAcc;                                                   // number of virtual accelerator is set when DM requests TK
      *virtAccRec     = 42;
      *noBeam         = flagDryRun;                                                // UNILAC requested without beam
      *statusTransfer = DMUNIPZ_TRANS_REQTK;                                       // update status of transfer
      *nInject        = 0;                                                         // number of injections is reset when DM requests TK
      *dtSync         = 0xffffffffffffffff;                                        // time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER
      *dtTransfer     = 0xffffffffffffffff;                                        // time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER
      *dtInject       = 0xffffffffffffffff;                                        // time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER
      *dtTkreq        = 0xffffffffffffffff;                                        // time difference between CMD_UNI_TKREQ and reply from UNIPZ
      *dtBreq         = 0xffffffffffffffff;                                        // time difference between CMD_UNI_BREQ and reply from UNIPZ
      *dtBprep        = 0xffffffffffffffff;                                        // time difference between CMD_UNI_BREQ and begin to request at UNIPZ
      *dtReady2Sis    = 0xffffffffffffffff;                                        // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS
      *dtStart        = 0xffffffffffffffff;                                        // remaining time budget for DM after 'flex command' has been sent, minimum value is 1ms
      flagTkReq       = 1;                                                         // used to diagnose number of EVT_READY_TO_SIS events
      tTkreq          = deadline;
      nR2sTransfer    = 0;

      //---- reserve TK
      status   = requestTK(tkTimeout, virtAcc, flagDryRun);                        // request TK from UNIPZ
      checkClearReqNotOk(uniTimeout);                                              // check and possibly clear 'req not ok' flag at UNIPZ

      *dtTkreq = getSysTime() - deadline;                                          // diagnostics: time difference between CMD_UNI_TKREQ and reply from UNIPZ

      if (status == DMUNIPZ_STATUS_OK)
        *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQTKOK;                 // update status of transfer

      break;

    case DMUNIPZ_ECADO_REQBEAM :                                                   // received command "CMD_UNI_BREQ" from data master

      if (flagIsLate) return DMUNIPZ_STATUS_LATEEVENT;                             // error: never request beam in case of a 'late event'

      (*nInject)++;                                                                // diagnostics: increment number of injections (of current transfer)

      //---- init values
      flagEBTimeout   = 0;                                                         // this is a 'warning flag'
      tCmdValid       = getSysTime();                                              // time when commands for DM shall become valid
      tDmTimeout      = deadline + (uint64_t)DMUNIPZ_DMTIMEOUT * (uint64_t)1000000;// absoulute time until we have to reply to DM
      tBreq           = deadline;                                                  // time, when CMD_UNI_BREQ event was received
      flagMilEvtValid = 0;                                                         // flag: valid MIL event from UNIPZ received
      tReady2Sis      = 0;                                                         // init value for timestamp of EVT_READY_TO_SIS
      flagNoCmd       = 0;                                                         // always send the commands to DM unless something goes terribly wrong

      //---- prepare commands that will be sent to Data Master later
      dmStatus = dmPrepCmdCommon(REQBEAMB, 0, 1, tCmdValid);                       // try "Schnitzeljagd" in Data Master. Here: second "flex" waiting block
      if (dmStatus == DMUNIPZ_STATUS_EBREADTIMEDOUT) {                             // in case of timeout, we probably lost a UDP packet; plan B: try a 2nd time
        flagEBTimeout = 1;
        dmStatus = dmPrepCmdCommon(REQBEAMB, 0, 1, tCmdValid);                                     
      } // if EB timeout
      if (dmStatus != DMUNIPZ_STATUS_OK) return dmStatus;                          // error: communication with DM failed even after two attempts; no plan C, give up! 
      // NB: we can't prepare the "flex" wait command  yet, as we need to timestamp the MIL event from UNIPZ first

      dmStatus = dmPrepCmdCommon(REQBEAMA, 1, 1, tCmdValid);                       // try "Schnitzeljagd" in Data Master. Here: first "slow" waiting block
      if (dmStatus == DMUNIPZ_STATUS_EBREADTIMEDOUT) {                             // in case of timeout, we probably lost a UDP packet; plan B: try a 2nd time
        flagEBTimeout = 1;
        dmStatus = dmPrepCmdCommon(REQBEAMA, 1, 1, tCmdValid);
      } // if EB timeout
      if (dmStatus != DMUNIPZ_STATUS_OK) return dmStatus;                          // error: communication with DM failed even after two attemps; no plan C, give up! 
      dmPrepCmdFlush(REQBEAMA);                                                    // prepare flush command for first "timeout" waiting block for later use

      dmStatus = dmCheckCmds(REQBEAMA, REQBEAMB);                                  // check cmds for valid addresses
      if (dmStatus != DMUNIPZ_STATUS_OK) return dmStatus;                          // error: invalid address fields; no plan B:  give up!

      //---- arm MIL Piggy 
      enableFilterEvtMil(pMILPiggy);                                               // enable filter @ MIL piggy
      clearFifoEvtMil(pMILPiggy);                                                  // get rid of junk in FIFO @ MIL piggy

      *dtBprep = getSysTime() - deadline;                                          // diagnostics: time difference between CMD_UNI_BREQ and begine to request at UNIPZ
      *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQBEAM;                   // diagnostics: update status of transfer

      //---- request beam from UNIPZ and wait for EVT_READY_TO_SIS
      if ((status = requestBeam(uniTimeout)) == DMUNIPZ_STATUS_OK) {               // request beam from UNIPZ
        *dtBreq = getSysTime() - deadline;                                         // diagnostics: time difference between CMD_UNI_BREQ and reply from UNIPZ
        if ((milStatus = wait4MILEvent(DMUNIPZ_EVT_READY2SIS, virtAcc, virtAccRec, uniTimeout)) == DMUNIPZ_STATUS_OK) {       // wait for event in MIL FIFO
          ecaInjAction = wait4ECAEvent(DMUNIPZ_QUERYTIMEOUT, &dummy1, &dummy2, &tReady2Sis, &flagIsLate);                     // wait for event from ECA (hoping this is MIL Event -> TLU)
          switch (ecaInjAction)                                                    // switch required to detect messages that are not expected at this part of the schedule
            {                                                                      
            case DMUNIPZ_ECADO_READY2SIS :                                         // no error:  received EVT_READY_TO_SIS via TLU -> ECA
              if ((getSysTime() - tReady2Sis) < DMUNIPZ_MATCHWINDOW) {             // check timestamp from TLU: only accept reasonably recent timestamp
                flagMilEvtValid = 1;                                               // everything ok: set flag for successful event reception
                status       = DMUNIPZ_STATUS_OK;
              } // if matchwindow
              else  status = DMUNIPZ_STATUS_BADTIMESTAMP;                          // error: timestamp too old
              nR2sTransfer++;                                                      // diagnostics: increment # of EVT_READY_TO_SIS events in between CMD_UNI_TKREQ and CMD_UNI_TKREL
              nR2sTotal++;                                                         // diagnostics: increment total # of EVT_READY_TO_SIS
              break;
            case DMUNIPZ_ECADO_TIMEOUT :                                           // error: timeout, no timestamp via TLU -> ECA
              status = DMUNIPZ_STATUS_NOTIMESTAMP;                                 
              break;
            default :                                                              // error: an unexpected event was received while waiting for EVT_READY_TO_SIS.
              status = DMUNIPZ_STATUS_BADSCHEDULEB;
              flagNoCmd = 1;                                                       // wrong LSA schedule or Data Master messed up: Don't increase the chaos by sending commands to DM
            } // switch (ecaInjAction)
        } // if wait4MILEvt
        else {                                                                     // error: timeout, EVT_READY_TO_SIS was not received in MIL FIFO                                          
          status = milStatus;
          if (status == DMUNIPZ_STATUS_TIMEDOUT) status = DMUNIPZ_STATUS_WAIT4UNIEVENT; 
        } // else wait4MilEvent
      } // if request beam
      else *dtBreq = getSysTime() - deadline;                                      // error: beam request at UNIPZ failed; diagnostics: time difference between CMD_UNI_BREQ and reply from UNIPZ

      //---- analyse the situation 
      if (flagMilEvtValid) {                                                                  
        tCmdFlex     = tReady2Sis   + (uint64_t)flexOffset;                        // everything is fine: add offset to obtain deadline for "flex" waiting block at Data Master
        *dtReady2Sis = tReady2Sis - deadline;                                      // diagnostics: time difference between CMD_UNI_BREQ and reply from UNIPZ
        pulseLemo2();                                                              // diagnostics: blink LED and TTL out of MIL piggy for hardware debugging with scope
      } // if MIL event was received
      else {                                                                       // error: did not receive MIL event; 
        tCmdFlex      = getSysTime() + (uint64_t)flexOffset;                       // plan B is to scacrifice the beam and continue with actual time plus offset
      } // else MIL event was received
      
      if (tCmdFlex < (getSysTime() + (uint64_t)DMUNIPZ_SAFETYMARGIN)) {            // error: not enough time is left for Data Master - risk of 'late events'
        tCmdFlex  = getSysTime()   + (uint64_t)flexOffset;                         // plan B is to scacrifice the beam and continue with actual time plus offset
        status = DMUNIPZ_STATUS_SAFETYMARGIN;
      } // if tCmdFlex

      if (getSysTime() > tDmTimeout){                                              // error: Data Master is no longer waiting on us - risk of messung up the schedule and 'late events' 
        flagNoCmd = 1;                                                             // plang B is to sacrifice the beam and continue with actual time plus offset
        status = DMUNIPZ_STATUS_DMTIMEOUT;
      }

      //---- send data to Data Master
      if (!flagNoCmd) {                                                            // after all this error checking we finally arrived at the point when we may send commands to the Data Master
        dmPrepFlexWaitCmd(REQBEAMB, tCmdFlex);                                     // prepare command for "flex" waiting block
        dmChangeBlock(REQBEAMB);                                                   // modify "flex" waiting block within DM
        dmChangeBlock(REQBEAMA);                                                   // modify "slow" waiting block within DM
      } // if getSysTime

      *dtStart     = tCmdFlex - getSysTime();                                      // diagnostics: we want to know how much of flexoffset for Data Masteris left (just to avoid the discussion), its a nice feature too

      //---- release beam and un-arm MIL piggy
      releaseBeam();                                                               // release beam request at UNIPZ
      checkClearReqNotOk(uniTimeout);                                              // check and possibly clear 'req not ok' flag at UNIPZ
      disableFilterEvtMil(pMILPiggy);                                              // disable filter @ MIL piggy to avoid accumulation of junk

      //---- conclude be setting status of transfer and status of gateway
      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELBEAM;                  // diagnostics: update status of transfer
      if (status == DMUNIPZ_STATUS_OK)
        *statusTransfer = *statusTransfer | DMUNIPZ_TRANS_REQBEAMOK;

      if ((status == DMUNIPZ_STATUS_OK) && flagEBTimeout)                          // handle warning in case we needed two attempts for our "Schnitzeljagd" within Data Master
        status = DMUNIPZ_STATUS_EBREADTIMEDOUT;                                           
          
      break;

    case DMUNIPZ_ECADO_RELTK :                                                     // received command "REL_TK" from data master

      //---- clear data, release TK, update status
      dmClearCmd(REQBEAMA);                                                        // with TK release, command data becomes invalid and must not be used any more
      dmClearCmd(REQBEAMB);                                                        // with TK release, command data becomes invalid and must not be used any more
      releaseTK();                                                                 // release TK
      *statusTransfer = *statusTransfer |  DMUNIPZ_TRANS_RELTK;                    // update status of transfer

      //---- diagnostics
      flagTkReq = 0;                                                               
      nR2sCycle     = nR2sTotal - nR2sLastTkrel;
      nR2sLastTkrel = nR2sTotal;

      break;

    case DMUNIPZ_ECADO_MBTRIGGER :                                                 // received MBTRIGGER: convenience feature triggering all kind of diagnostics

      // calculate time difference between EVT_READY_TO_SIS and EVT_MB_TRIGGER
      if (tReady2Sis == 0) *dtSync = 0xffffffffffffffff;                           // no valid timestamp for EVT_READY_TO_SIS
      else                 *dtSync = deadline - tReady2Sis;                        // we got a valid timestamp

      // calculate time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER
      if (tBreq == 0)      *dtInject = 0xfffffffffffffff;                          // no valid timestamp for EVT_READY_TO_SIS
      else                 *dtInject = deadline - tBreq;                           // we got a valid timestamp

      // calculate time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER
      if (tTkreq == 0)     *dtTransfer = 0xfffffffffffffff;                        // no valid timestamp for EVT_READY_TO_SIS
      else                 *dtTransfer = deadline - tTkreq;                        // we got a valid timestamp

      if (status == DMUNIPZ_STATUS_OK) {                                           // we don't want to overwrite an already existing bad status
        if (flagTkReq) {                                                           // only do this test, if TK is reserved (if TK is not reserved, synchronization with UNILAC is not included in the schedule)
          // check if time difference is not reasonable. It must be within a small window around the value DMUNIPZ_OFFSETINJECT.
          if ((*dtSync < (uint64_t)(DMUNIPZ_OFFSETINJECT - DMUNIPZ_MATCHWINDOW)) || (*dtSync > (uint64_t)(DMUNIPZ_OFFSETINJECT + DMUNIPZ_MATCHWINDOW))) status = DMUNIPZ_STATUS_BADSYNC;
        } // if flagTKReq
      } // if status

      if (status == DMUNIPZ_STATUS_OK) {                                           // we don't want to overwrite an already existing bad status
        // check if time difference is not reasonable. It must be larger than 10ms. A shorter difference indicates failure/missing '10s waiting block' at DM
        if (*dtInject < (uint64_t)(DMUNIPZ_OFFSETINJECT + DMUNIPZ_MATCHWINDOW)) status = DMUNIPZ_STATUS_BADSCHEDULEA;
      } // if status
      
      break;
      
    case DMUNIPZ_ECADO_READY2SIS :                                                 // diagnostics: received EVT_READY_TO_SIS via TLU 
      if (flagTkReq) nR2sTransfer++;                                               // receiving EVT_READY_TO_SIS during ongoing transfer within transfer but outside injection (injection handled by case 'DMUNIPZ_ECADO_REQBEAM')
      nR2sTotal++;                                                                 // receiving EVT_READY_TO_SIS outside ongoing transfer indicates a periodic "virtual accelerator" from UNILAC to TK

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
  uint32_t virtAccReq;                          // number of virtual accelerator requested by Data Master
  uint32_t virtAccRec;                          // number of virtual accelerator received from UNIPZ
  uint32_t noBeam;                              // no beam flag requested by Data Master
  uint64_t dtStart;                             // remaining time budget for DM after 'flex command' has been sent, minimum value is 1ms
  uint64_t dtSync;                              // time difference between EVT_READY_TO_SIS and EVT_MB_LOAD
  uint64_t dtInject;                            // time difference between CMD_UNI_BREQ and EVT_MB_TRIGGER, must be larger than 10ms
  uint64_t dtTransfer;                          // time difference between CMD_UNI_TKREQ and EVT_MB_TRIGGER
  uint64_t dtTkreq;                             // time difference between CMD_UNI_TKREQ and reply from UNIPZ
  uint64_t dtBreq;                              // time difference between CMD_UNI_BREQ and reply from UNIPZ
  uint64_t dtBprep;                            // time difference between CMD_UNI_BREQ and begin to request at UNIPZ
  uint64_t dtReady2Sis;                         // time difference between CMD_UNI_BREQ and EVT_READY_TO_SIS

  mprintf("\n");
  mprintf("dm-unipz: ***** firmware v %06d started from scratch *****\n", DMUNIPZ_FW_VERSION);
  mprintf("\n");
  
  // init local variables
  i              = 0;
  nTransfer      = 0;                           
  nInject        = 0;
  noBeam         = 0xffffffff;
  dtStart        = 0xffffffffffffffff;
  dtSync         = 0xffffffffffffffff;
  dtInject       = 0xffffffffffffffff;
  dtTransfer     = 0xffffffffffffffff;
  dtTkreq        = 0xffffffffffffffff;
  dtBreq         = 0xffffffffffffffff;
  dtBprep        = 0xffffffffffffffff;
  dtReady2Sis    = 0xffffffffffffffff;               
  nR2sTransfer   = 0;
  nR2sTotal      = 0;
  nR2sLastTkrel  = 0;
  nR2sCycle      = 0;
  virtAccReq     = 42;
  virtAccRec     = 42;
  statusTransfer = DMUNIPZ_TRANS_UNKNOWN;       
  reqState       = DMUNIPZ_STATE_S0;
  actState       = DMUNIPZ_STATE_UNKNOWN;
  status         = DMUNIPZ_STATUS_UNKNOWN;
  nBadState      = 0;
  nBadStatus     = 0;
  flagTkReq      = 0;
  tBreq          = 0;
  tReady2Sis     = 0;

  init();                                                                   // initialize stuff for lm32
  initSharedMem();                                                          // initialize shared memory
  
  while (1) {
    cmdHandler(&reqState, &statusTransfer);                                 // check for commands and possibly request state changes
    status = changeState(&actState, &reqState, status);                     // handle requested state changes
    switch(actState)                                                        // state specific do actions
      {
        case DMUNIPZ_STATE_S0 :
        status = doActionS0();                                              // important initialization that must succeed!
        if (status != DMUNIPZ_STATUS_OK) reqState = DMUNIPZ_STATE_FATAL;    // failed:  -> FATAL
        else                             reqState = DMUNIPZ_STATE_IDLE;     // success: -> IDLE
        break;
       case DMUNIPZ_STATE_OPREADY :
         status = doActionOperation(&statusTransfer, &virtAccReq, &virtAccRec, &noBeam, &dtStart, &dtSync, &dtInject, &dtTransfer, &dtTkreq, &dtBreq, &dtBprep, &dtReady2Sis, &nTransfer, &nInject, status);
        if (status == DMUNIPZ_STATUS_DEVBUSERROR)    reqState = DMUNIPZ_STATE_ERROR;
        if (status == DMUNIPZ_STATUS_ERROR)          reqState = DMUNIPZ_STATE_ERROR;
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

      // update shared memory */
    if ((*pSharedStatus == DMUNIPZ_STATUS_OK)     && (status    != DMUNIPZ_STATUS_OK))     {nBadStatus++; *pSharedNBadStatus = nBadStatus;}
    if ((*pSharedState  == DMUNIPZ_STATE_OPREADY) && (actState  != DMUNIPZ_STATE_OPREADY)) {nBadState++;  *pSharedNBadState  = nBadState;}
    *pSharedStatus       = status;
    *pSharedState        = actState;
    i++;
    *pSharedNIterMain    = i;
    *pSharedStatTrans    = statusTransfer;
    *pSharedVirtAcc      = virtAccReq;
    *pSharedVirtAccRec   = virtAccRec;
    *pSharedNoBeam       = noBeam;
    
    if (dtStart          == 0xffffffffffffffff) *pSharedDtStart      = 0xffffffff;
    else                                        *pSharedDtStart      = (uint32_t)((float)dtStart / 1000.0);
    if (dtSync           == 0xffffffffffffffff) *pSharedDtSync       = 0xffffffff;
    else                                        *pSharedDtSync       = (uint32_t)((float)dtSync / 1000.0);
    if (dtTransfer       == 0xffffffffffffffff) *pSharedDtTransfer   = 0xffffffff;
    else                                        *pSharedDtTransfer   = (uint32_t)((float)dtTransfer / 1000.0);
    if (dtInject         == 0xffffffffffffffff) *pSharedDtInject     = 0xffffffff;
    else                                        *pSharedDtInject     = (uint32_t)((float)dtInject / 1000.0);
    if (dtTkreq          == 0xffffffffffffffff) *pSharedDtTkreq      = 0xffffffff;
    else                                        *pSharedDtTkreq      = (uint32_t)((float)dtTkreq / 1000.0);
    if (dtBreq           == 0xffffffffffffffff) *pSharedDtBreq       = 0xffffffff;
    else                                        *pSharedDtBreq       = (uint32_t)((float)dtBreq / 1000.0);
    if (dtBprep          == 0xffffffffffffffff) *pSharedDtBprep      = 0xffffffff;
    else                                        *pSharedDtBprep      = (uint32_t)((float)dtBprep / 1000.0);
    if (dtReady2Sis      == 0xffffffffffffffff) *pSharedDtReady2Sis  = 0xffffffff;
    else                                        *pSharedDtReady2Sis  = (uint32_t)((float)dtReady2Sis / 1000.0);
    
    *pSharedNR2sTransfer = nR2sTransfer;
    *pSharedNR2sCycle    = nR2sCycle;
    *pSharedNTransfer    = nTransfer;
    *pSharedNInject      = nInject;

    // update OLED display
    updateOLED(statusTransfer, virtAccReq, nTransfer, nInject, status, actState);
    
  } // while
} // main

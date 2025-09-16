/********************************************************************************************
 *  dm.c
 *
 *  created : 08.03.2016
 *  author  : Mathias Kreider, GSI-Darmstadt
 *
 *  LM32 firmware, core routines of FAIR Data Master 
 *  To be used with carpeDM library
 * 
 * -------------------------------------------------------------------------------------------
 * License Agreement for this software:
 *
 * Copyright (C) 2016  Mathias Kreider
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
 * Planckstrasse 1
 * D-64291 Darmstadt
 * Germany
 *
 * Contact: m.kreider@gsi.de
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
 * For all questions and ideas contact: m.kreider@gsi.de
 * Last update: 09.01.2022
 * 
 * Currently known bugs: None 
 ********************************************************************************************/


#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
#include "ftm_common.h"
#include "dm.h"
#include "prio_regs.h"
#include "dbg.h"
#include "ftm_shared_mmap.h"

uint64_t SHARED dummy = 0; ///< dummy using the SHARED type so nothing gets optimized away

deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_]; 
nodeFuncPtr     nodeFuncs[_NODE_TYPE_END_];     
actionFuncPtr   actionFuncs[_ACT_TYPE_END_];    

// Assigning pointer shortcuts into shared memory area
uint32_t* const p         = (uint32_t*)&_startshared;                                             
uint32_t* const status    = (uint32_t*)&_startshared[SHCTL_STATUS >> 2];                          
uint64_t* const count     = (uint64_t*)&_startshared[(SHCTL_DIAG  + T_DIAG_MSG_CNT)  >> 2];       
uint64_t* const boottime  = (uint64_t*)&_startshared[(SHCTL_DIAG  + T_DIAG_BOOT_TS)  >> 2];       
#ifdef DIAGNOSTICS      
int64_t* const diffsum    = (int64_t*) &_startshared[(SHCTL_DIAG  + T_DIAG_DIF_SUM ) >> 2];       
int64_t* const diffmax    = (int64_t*) &_startshared[(SHCTL_DIAG  + T_DIAG_DIF_MAX ) >> 2];       
int64_t* const diffmin    = (int64_t*) &_startshared[(SHCTL_DIAG  + T_DIAG_DIF_MIN ) >> 2];       
int64_t* const diffwth    = (int64_t*) &_startshared[(SHCTL_DIAG  + T_DIAG_DIF_WTH ) >> 2];       
uint32_t* const diffwcnt  = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_WAR_CNT ) >> 2];       
uint32_t* const diffwhash = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_WAR_1ST_HASH ) >> 2];  
uint64_t* const diffwts   = (uint64_t*) &_startshared[(SHCTL_DIAG + T_DIAG_WAR_1ST_TS ) >> 2];    
uint32_t* const backlogmax = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_BCKLOG_STRK )  >> 2];  
uint32_t* const badwaitcnt = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_BAD_WAIT_CNT )  >> 2];
#endif
uint32_t* const start   = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_START)   >> 2];
uint32_t* const startRemote = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_CEASE) >> 2]; //FIXME Experimental fix abusing unused CEASE register as START_REMOTE
uint32_t* const running = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_RUNNING) >> 2];          
uint32_t* const abort1  = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_ABORT)   >> 2];          
uint32_t** const hp     = (uint32_t**)&_startshared[SHCTL_HEAP >> 2];                             
uint32_t* const nodeTmps[3 * _MEM_BLOCK_SIZE / _32b_SIZE_];     ///< Staging area when a node is constructed from references. BACKUP, OLD, NEW 

void prioQueueInit()
{
  #ifndef USE_SW_TX_CONTROL
    pFpqCtrl[PRIO_RESET_OWR>>2]      = 1;
    pFpqCtrl[PRIO_MODE_CLR>>2]       = 0xffffffff;
    pFpqCtrl[PRIO_ECA_ADR_RW>>2]     = ECA_GLOBAL_ADR;
    pFpqCtrl[PRIO_EBM_ADR_RW>>2]     = ((uint32_t)pEbm & ~0x80000000);
    pFpqCtrl[PRIO_TX_MAX_MSGS_RW>>2] = 40;
    pFpqCtrl[PRIO_TX_MAX_WAIT_RW>>2] = loW((uint64_t)(50000));
    pFpqCtrl[PRIO_MODE_SET>>2]       = PRIO_BIT_ENABLE     |
                                       PRIO_BIT_MSG_LIMIT  |
                                       PRIO_BIT_TIME_LIMIT;
  #endif                                    
}

void dmInit() {

  nodeFuncs[NODE_TYPE_UNKNOWN]          = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_RAW]              = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_TMSG]             = tmsg;
  nodeFuncs[NODE_TYPE_CNOOP]            = cmd;
  nodeFuncs[NODE_TYPE_CFLOW]            = cmd;
  nodeFuncs[NODE_TYPE_CSWITCH]          = cswitch;  
  nodeFuncs[NODE_TYPE_CFLUSH]           = cmd;
  nodeFuncs[NODE_TYPE_CWAIT]            = cmd;
  nodeFuncs[NODE_TYPE_BLOCK_FIXED]      = blockFixed;
  nodeFuncs[NODE_TYPE_BLOCK_ALIGN]      = blockAlign;
  nodeFuncs[NODE_TYPE_QUEUE]            = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_QBUF]             = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_SHARE]            = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_ALTDST]           = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_ORIGIN]           = origin;
  nodeFuncs[NODE_TYPE_STARTTHREAD]      = startThread;
  nodeFuncs[NODE_TYPE_NULL]             = nodeNull;

  //deadline updater. Return infinity (-1) if no or unsupported node was given
  deadlineFuncs[NODE_TYPE_UNKNOWN ]     = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_RAW]          = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_TMSG]         = dlEvt;
  deadlineFuncs[NODE_TYPE_CNOOP]        = dlEvt;
  deadlineFuncs[NODE_TYPE_CFLOW]        = dlEvt;
  deadlineFuncs[NODE_TYPE_CSWITCH]      = dlEvt; 
  deadlineFuncs[NODE_TYPE_CFLUSH]       = dlEvt;
  deadlineFuncs[NODE_TYPE_CWAIT]        = dlEvt;
  deadlineFuncs[NODE_TYPE_BLOCK_FIXED]  = dlBlock;
  deadlineFuncs[NODE_TYPE_BLOCK_ALIGN]  = dlBlock;
  deadlineFuncs[NODE_TYPE_QUEUE]        = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_QBUF]         = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_SHARE]        = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_ALTDST]       = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_ORIGIN]       = dlEvt;
  deadlineFuncs[NODE_TYPE_STARTTHREAD]  = dlEvt;
  deadlineFuncs[NODE_TYPE_NULL]         = deadlineNull;


  actionFuncs[ACT_TYPE_UNKNOWN]         = dummyActionFunc;
  actionFuncs[ACT_TYPE_NOOP]            = execNoop;
  actionFuncs[ACT_TYPE_FLOW]            = execFlow;
  actionFuncs[ACT_TYPE_FLUSH]           = execFlush;
  actionFuncs[ACT_TYPE_WAIT]            = execWait;





  uint8_t i;

  for(i=0; i < _THR_QTY_; i++) {
    //set thread times to infinity
    uint32_t* tp = (uint32_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_) >> 2));
    *(uint64_t*)&tp[T_TD_CURRTIME >> 2] = -1ULL;
    *(uint64_t*)&tp[T_TD_DEADLINE >> 2] = -1ULL;
    *(uint32_t*)&tp[T_TD_FLAGS >> 2]    = i;
    *(uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_PREPTIME  ) >> 2)) = PREPTIME_DEFAULT;
    *(uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_STARTTIME ) >> 2)) = 0ULL;
    //add thread to heap
    hp[i] = tp;
  }
  #ifdef DIAGNOSTICS
    *diffsum   = 0;
    *diffmax   = 0xffffffffffffffffLL;
    *diffmin   = 0x7fffffffffffffffLL;
    *diffwth   = 50000LL;
    *diffwcnt  = 0;
    *diffwhash = 0;
    *diffwts   = 0;
    *backlogmax = 0;
    *badwaitcnt = 0;
    *boottime  = getSysTime();
  #endif


}

uint8_t wrTimeValid() {

  const uint32_t STATE_REG       = 0x1C;
  const uint32_t PPS_VALID_MSK   = (1<<2);
  const uint32_t TS_VALID_MSK    = (1<<3);
  const uint32_t STATE_MSK       = PPS_VALID_MSK | TS_VALID_MSK;

  return  ( (pPps[STATE_REG  >> 2] & STATE_MSK) != 0);

}

uint32_t* nodeNull (uint32_t* node, uint32_t* thrData)                        { return LM32_NULL_PTR;}

uint64_t  deadlineNull (uint32_t* node, uint32_t* thrData)                    { return -1ULL; } //return infinity

uint32_t* dummyNodeFunc (uint32_t* node, uint32_t* thrData)                   { *status |= SHCTL_STATUS_BAD_NODE_TYPE_SMSK; return nodeNull(node, thrData); }

uint64_t  dummyDeadlineFunc (uint32_t* node, uint32_t* thrData)               { *status |= SHCTL_STATUS_BAD_NODE_TYPE_SMSK; return deadlineNull(node, thrData); } //return infinity

uint32_t* dummyActionFunc (uint32_t* node, uint32_t* cmd, uint32_t* thrData)  { *status |= SHCTL_STATUS_BAD_ACT_TYPE_SMSK;  return LM32_NULL_PTR;}

uint8_t getNodeType(uint32_t* node) {
  uint32_t* tmpType;
  uint32_t msk;
  uint32_t type = NODE_TYPE_UNKNOWN;

  if (node != LM32_NULL_PTR) {
    tmpType   = node + (NODE_FLAGS >> 2);
    type      = (*tmpType >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
    DBPRINT2("#%02u: Node Type b4 boundary check: %u\n", cpuId, type);
    msk       = -(type < _NODE_TYPE_END_);
    type     &= msk; //optional boundary check, if out of bounds, type will equal NODE_TYPE_UNKNOWN
  } else {
    DBPRINT2("#%02u: Null ptr detected \n", cpuId);
    return NODE_TYPE_NULL;
  }

  return type;
}

uint64_t dlEvt(uint32_t* node, uint32_t* thrData) {
  return *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] + *(uint64_t*)&node[EVT_OFFS_TIME >> 2];
}

uint64_t dlBlock(uint32_t* node, uint32_t* thrData) {
  return *(uint64_t*)&thrData[T_TD_DEADLINE >> 2];
}

uint32_t* execNoop(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* execFlow(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {
  uint32_t* ret = (uint32_t*)cmd[T_CMD_FLOW_DEST >> 2];
  DBPRINT3("#%02u: Routing Flow to 0x%08x\n", cpuId, (uint32_t)ret);
  //permanent change?
  if(cmd[T_CMD_ACT >> 2] & ACT_CHP_SMSK) node[NODE_DEF_DEST_PTR >> 2] = (uint32_t)ret;
  return ret;

}

uint32_t* execFlush(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {
  uint32_t action     = cmd[T_CMD_ACT >> 2];  // command action field
  uint8_t  flushees   =         (action >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;  // mask of flushee (target queue) priorities
  uint8_t  flusher    =  1 <<  ((action >> ACT_PRIO_POS)       & ACT_PRIO_MSK);       // mask of flusher (current command) priority
  uint32_t wrIdxs     = node[BLOCK_CMDQ_WR_IDXS >> 2] & BLOCK_CMDQ_WR_IDXS_SMSK;      // flushee buffer wr indices
  uint32_t rdIdxs     = node[BLOCK_CMDQ_RD_IDXS >> 2] & BLOCK_CMDQ_RD_IDXS_SMSK;      // flushee buffer read indices
  uint8_t  prio;                                                                      // loop variable, iterate over all  priorities

  for(prio = PRIO_LO; prio <= PRIO_IL; prio++) { // iterate priorities of flushees
    // if execution would flush the very queue containing the flush command (the flusher), we must prevent the queue from being popped in block() function afterwards.
    // Otherwise, rd idx will overtake wr idx -> queue corrupted. To minimize corner case impact, we do this by adjusting the
    // new read idx to be wr idx-1. Then the queue pop leaves us with new rd idx = wr idx as it should be after a flush.
    if(flushees & (1 << prio)) {
      uint8_t* rdIdx = ((uint8_t *)&rdIdxs + 3 - prio);
      uint8_t* wrIdx = ((uint8_t *)&wrIdxs + 3 - prio);

      uint8_t flushMyselfB4pop  = (flusher & (1 << prio)) != 0; // flag: selfflush requested (flush cmd is in flushed queue)
      *rdIdx      = ((*wrIdx - flushMyselfB4pop) & Q_IDX_MAX_OVF_MSK);  // The new rd idx. Must equal the wr idx for queue to be empty, adjust by -1 if this is a selfflush 
    } // execute flush if loop variable prio matches a flushee
  }
  //write back potentially updated read indices
  node[BLOCK_CMDQ_RD_IDXS >> 2] = rdIdxs;

  //Flush override handling. Override successor ?
  uint32_t* ret = (uint32_t*)cmd[T_CMD_FLUSH_OVR >> 2]; // Flush override: If requested, this return value changes from default successor to flush override
  if((uint32_t)ret != LM32_NULL_PTR) { // no override to idle allowed!
    //permanent change?
    if((cmd[T_CMD_ACT >> 2] & ACT_CHP_SMSK)) node[NODE_DEF_DEST_PTR >> 2] = (uint32_t)ret;
    return ret;
  }
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];


}

uint32_t* execWait(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {

  // the block period is added in blockFixed or blockAligned.
  // we must therefore subtract it here if we modify current time, as execWait is optional

  uint64_t  tWait = *(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] - *(uint64_t*)&node[BLOCK_PERIOD >> 2]; // auxiliary wait time. The block handler adds its period, so to avoid corner case, subtract period here.
  uint64_t*  tCur = (uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // current TAI in ns
  uint32_t    act = cmd[T_CMD_ACT >> 2]; // command action field
  if ( act & ACT_WAIT_ABS_SMSK) {
    // absolute wait time. Replaces current time sum
    if ( getSysTime() < tWait ) { *tCur = tWait; } //1.1 if wait time is greater than Now, proceed
    else                        { (*badwaitcnt)++; } //1.2 wait time is in the past, this is bad. Skip and increase bad wait warning counter
  } else {
    // relative wait time. replaces current block period
    if( act & ACT_CHP_SMSK) { *(uint64_t*)&node[BLOCK_PERIOD >> 2] = *(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2]; }  // 2. permanently change this block's period
    else                    { *tCur += tWait; }                                                                 // 3. temporarily change this block's period (inc. current time sum by waittime - blockperiod)
  }

  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];

}

uint32_t* cmd(uint32_t* node, uint32_t* thrData) {
        uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2]; // ptr to successor node
  const uint32_t prio = (node[CMD_ACT >> 2] >> ACT_PRIO_POS) & ACT_PRIO_MSK; // action priority, ie. which queue it is to be delivered to
  const uint32_t *tg  = (uint32_t*)node[CMD_TARGET >> 2]; // ptr to target block
  const uint32_t adrPrefix = (uint32_t)tg & ~PEER_ADR_MSK; // Address prefix. If target is on a different RAM, the B (Host) Port is used and the global adr prefix must be added 

  // if this command tries to write to priority that does not exist, this is an emergency. Halt everything, set error bit for bad cmd action.
  if(!((tg[NODE_FLAGS >> 2] >> NFLG_BLOCK_QS_POS) & (1 << prio))) {
    *abort1 = 0xffffffff; 
    *status |= SHCTL_STATUS_BAD_ACT_TYPE_SMSK;
    return node; // return this node to show where the problem was 
  }  
 
  uint32_t *bl;     // ptr to buffer list of target queue
  uint32_t *b;      // ptr to specific buffer
  uint32_t *e;      // ptr to specific element
  uint8_t  *wrIdx;  // ptr to write index of queue
  uint32_t bufOffs; // buffer's offset in buffer list
  uint32_t elOffs;  // element's offset in buffer
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited
  //mprintf("tg 0x%08x adrpre 0x%08x INTB 0x%08x\n", (uint32_t)tg, adrPrefix, INT_BASE_ADR);
  if(tg == LM32_NULL_PTR) { // check if the target is a null pointer. Used to allow removal of pattern containing target nodes
    return ret;
  }

  //check if the target queues are write locked
  const uint32_t qFlags = tg[BLOCK_CMDQ_FLAGS >> 2]; // queue flags field, contains lock bits. Only copy cmd action into buffer if Do not write (DNW) is false
  
  if(qFlags & BLOCK_CMDQ_DNW_SMSK) { return ret; }

  /*                    tg
   *       _____________/|\___________  
   *      /              |            \
   *     Bl(Lo)       Bl(Md)        Bl(Hi)
   *    /    \        /    \        /    \
   *   b0    b1      b0    b1      b0    b1
   *   / \   / \    / \   / \      / \   / \
   *  e0 e1 e2 e3  e0 e1 e2 e3    e0 e1 e2 e3
   */

  wrIdx   = ((uint8_t *)tg + BLOCK_CMDQ_WR_IDXS + 3 - prio);                           // calculate pointer (8b) to current write index
  bufOffs = (*wrIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;             // calculate Offsets
  elOffs  = (*wrIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_;
  bl      = (uint32_t*)(tg[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2]  - INT_BASE_ADR + adrPrefix); // get pointer to buf list
  b       = (uint32_t*)(bl[bufOffs >> 2]  - INT_BASE_ADR + adrPrefix);                              // get pointer to buf
  e       = (uint32_t*)&b[elOffs >> 2];                                                             // get pointer to Element to write

  DBPRINT3("#%02u: Prio: %u, pre: 0x%08x, base: 0x%08x, wrIdx: 0x%08x, target: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x\n", cpuId, prio, adrPrefix, INT_BASE_ADR, (uint32_t)wrIdx, (uint32_t)tg, (uint32_t)bl, (uint32_t)b, (uint32_t)e );

  // Important !!! So the host can verify if a command is active as seen from an LM32 by comparing with cursor timestamping,
  // we cannot place ANY tvalids into the past. All tValids must be placed sufficiently into the future so that this here function
  // terminates before tValid is reached. This is the responsibility of carpeDM!

  // !!! CAVEAT !!! when tvalid is used as relative offset in multiple commands to synchronise them, the sum of block start (current time sume) + tvalid
  // must always be greater than tMinimum, else the commands get differing tvalids assigned and will no longer synchronously become valid !!!

  uint64_t* ptValid   = (uint64_t*)&node[CMD_VALID_TIME   >> 2]; // valid time value of this command action, usually used to sync multiple commands. can absolute or relative
  uint64_t* ptCurrent = (uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // current TAI in ns
  uint64_t tValid; // final valid time, absolute or relative depending on VABS flag in action field.

  if((node[CMD_ACT  >> 2] >> ACT_VABS_POS) & ACT_VABS_MSK) tValid = *ptValid;
  else                                                     tValid = *ptCurrent + *ptValid;

  //copy cmd data to target queue
  e[(T_CMD_TIME + 0)          >> 2]  = (uint32_t)(tValid >> 32);
  e[(T_CMD_TIME + _32b_SIZE_) >> 2]  = (uint32_t)(tValid);

  e[(T_CMD_ACT + 0 * _32b_SIZE_) >> 2]  = node[(CMD_ACT + 0 * _32b_SIZE_) >> 2];
  e[(T_CMD_ACT + 1 * _32b_SIZE_) >> 2]  = node[(CMD_ACT + 1 * _32b_SIZE_) >> 2];
  e[(T_CMD_ACT + 2 * _32b_SIZE_) >> 2]  = node[(CMD_ACT + 2 * _32b_SIZE_) >> 2];

  *wrIdx = (*wrIdx + 1) & Q_IDX_MAX_OVF_MSK; //increase write index

  DBPRINT2("#%02u: Sending Cmd 0x%08x, Target: 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], (uint32_t)tg, node[NODE_DEF_DEST_PTR >> 2]);
  return ret;
}

uint32_t* cswitch(uint32_t* node, uint32_t* thrData) {
        uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
        node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited
  
        uint32_t *tg  = (uint32_t*)node[SWITCH_TARGET >> 2];
  const uint32_t adrPrefix = (uint32_t)tg & PEER_ADR_MSK; // if target is on a different RAM, all ptrs must be translated from the local to our (peer) perspective

  // check if the target is a null pointer, if so abort. Used to allow removal of pattern containing target nodes
  if(tg == LM32_NULL_PTR) { return ret; }

  //check if the target queues are write locked
  const uint32_t qFlags = tg[BLOCK_CMDQ_FLAGS >> 2];
  if(qFlags & BLOCK_CMDQ_DNW_SMSK) { return ret; }
  
  //overwrite target defdst
  tg[NODE_DEF_DEST_PTR >> 2] = (uint32_t)node[SWITCH_DEST >> 2];

  DBPRINT2("#%02u: Sending Cmd 0x%08x, Target: 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], (uint32_t)tg, node[NODE_DEF_DEST_PTR >> 2]);
  
  return ret;
}

uint32_t* tmsg(uint32_t* node, uint32_t* thrData) {
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited
  DBPRINT2("#%02u: Sending Evt 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], node[NODE_DEF_DEST_PTR >> 2]);

  uint64_t tmpPar = *(uint64_t*)&node[TMSG_PAR >> 2];

  #ifdef DIAGNOSTICS
    int64_t now = getSysTime();
    //Diagnostic Event? insert PQ Message counter. Different device, can't be placed inside atomic!
    if (*(uint64_t*)&node[TMSG_ID >> 2] == DIAG_PQ_MSG_CNT) tmpPar = *(uint64_t*)&pFpqCtrl[PRIO_CNT_OUT_ALL_GET_0>>2];
    int64_t diff  = *(uint64_t*)&thrData[T_TD_DEADLINE >> 2] - now;
    uint8_t overflow = (diff >= 0) & (*diffsum >= 0) & ((diff + *diffsum)  < 0)
                     | (diff <  0) & (*diffsum <  0) & ((diff + *diffsum) >= 0);
    *diffsum   = (overflow          ? diff    : *diffsum + diff);
    //*dbgcount  = ((diff < *diffmin) ? *count  : *dbgcount);
    *diffmin   = ((diff < *diffmin) ? diff    : *diffmin);
    *diffmax   = ((diff > *diffmax) ? diff    : *diffmax);
    *count     = (overflow          ? 0       : *count);   // necessary for calculating average: if sum resets, count must also reset
    *diffwcnt += (int64_t)(diff < *diffwth); //inc diff warning counter when diff below threshold
    *diffwhash = ((int64_t)(diff < *diffwth) && !*diffwhash) ? node[NODE_HASH >> 2] : *diffwhash; //save node hash of first warning
    *diffwts   = ((int64_t)(diff < *diffwth) && !*diffwts)   ? now : *diffwts; //save time of first warning
  #endif

  //disptach timing message to priority queue
  atomic_on();
  #ifdef USE_SW_TX_CONTROL
    #pragma message ( "HW Priority Queue deactivated, using software access to EBM!" )
    ebm_hi(ECA_GLOBAL_ADR);
    ebm_op(ECA_GLOBAL_ADR, node[TMSG_ID_HI  >> 2],          EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, node[TMSG_ID_LO  >> 2],          EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, hiW(tmpPar),                     EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, loW(tmpPar),                     EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, node[TMSG_RES    >> 2],          EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, node[TMSG_TEF    >> 2],          EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, thrData[T_TD_DEADLINE_HI >> 2],  EBM_WRITE);
    ebm_op(ECA_GLOBAL_ADR, thrData[T_TD_DEADLINE_LO >> 2],  EBM_WRITE);
    ebm_flush();
  #else
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_ID_HI  >> 2];
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_ID_LO  >> 2];
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = hiW(tmpPar);
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = loW(tmpPar);
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_RES    >> 2];
    *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_TEF    >> 2];
    *(pFpqData + (PRIO_DAT_TS_HI >> 2))  = thrData[T_TD_DEADLINE_HI >> 2];
    *(pFpqData + (PRIO_DAT_TS_LO >> 2))  = thrData[T_TD_DEADLINE_LO >> 2];
  #endif   
  atomic_off();

  ++(*((uint64_t*)&thrData[T_TD_MSG_CNT >> 2])); //increment thread message counter
  ++(*count); //increment cpu message counter

  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* block(uint32_t* node, uint32_t* thrData) {
  DBPRINT2("#%02u: Checking Block 0x%08x\n", cpuId, node[NODE_HASH >> 2]);
  uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  uint32_t *bl, *b, *cmd, *act;
  uint8_t  *rdIdx,*wrIdx;
  uint8_t skipOne = 0;

  uint32_t *ardOffs = node + (BLOCK_CMDQ_RD_IDXS >> 2), *awrOffs = node + (BLOCK_CMDQ_WR_IDXS >> 2);
  uint32_t bufOffs, elOffs, prio, actTmp, atype, qFlags = *((uint32_t*)(node + (BLOCK_CMDQ_FLAGS >> 2)));
  uint32_t qty;

  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited

  //3 ringbuffers -> 3 wr indices, 3 rd indices (one per priority).
  //////////////////////////////////////////////////////////////////////////////////////////////////////
  // Check queues for pending commands
  // If Do not Read flag is not set and the indices differ, there's work to do

  if(!(qFlags & BLOCK_CMDQ_DNR_SMSK) 
  && ((*awrOffs & BLOCK_CMDQ_WR_IDXS_SMSK) != (*ardOffs & BLOCK_CMDQ_RD_IDXS_SMSK)) ) {
    
    //Iterate over queues, highest priority first. If we find a pending command that has reached its valid time, we take it.
    int32_t i;
    for (i = PRIO_IL; i >= PRIO_LO; i--) {
      prio = (uint32_t)i;
      
      // make sure we only check queue prios this Block actually has
      if(((node[NODE_FLAGS >> 2] >> NFLG_BLOCK_QS_POS) & (1 << prio))) {
        
        //Notation "3 - prio" seems weird, but remember: It's big endian, MSB first. bit 31/byte 3 is at address offset 0.
        //Addr  0x0 0x1 0x2 0x3
        //Byte    3   2   1   0
        //Field   -  IL  HI  LO
        rdIdx   = ((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + 3 - prio); // this queues read index   (using overflow bit for empty/full)
        wrIdx   = ((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + 3 - prio); // this queues write index

        //Check if rd - wr cnt differs. 
        if (*rdIdx ^ *wrIdx) {
          
          //found pending command
          //create shortcuts to control data of corresponding queue
          bufOffs = (*rdIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;   // offset of active command buffer's pointer in this queues buffer list
          elOffs  = (*rdIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; // offset of active command data in active command buffer
          bl      = (uint32_t*)node[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];                  // pointer to this queues buffer list
          b       = (uint32_t*)bl[bufOffs >> 2];                                                  // pointer to active command buffer
          cmd     = (uint32_t*)&b[elOffs  >> 2];                                                  // pointer to active command data
  
          // if this command is already valid, leave check loop and execute it.
          if (getSysTime() >= *((uint64_t*)((void*)cmd + T_CMD_TIME))) break;
                      
          
        }
      }
    }
      
    if (getSysTime() < *((uint64_t*)((void*)cmd + T_CMD_TIME))) return ret;                 // if no command is yet valid, take default successor.

    act     = (uint32_t*)&cmd[T_CMD_ACT >> 2];          //pointer to command's action
    actTmp  = *act;                                     //create working copy of action word
    qty     = (actTmp >> ACT_QTY_POS) & ACT_QTY_MSK;    //remaining command quantity
    atype   = (actTmp >> ACT_TYPE_POS) & ACT_TYPE_MSK;  //action type enum

    DBPRINT2("#%02u: pending Cmd @ Prio: %u, awdIdx: 0x%08x, 0x%02x, ardIdx: 0x%08x, 0x%02x, buf: %u, el: %u, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x, type: %u\n", cpuId, prio, *awrOffs, *wrIdx, *ardOffs, *rdIdx, (*rdIdx & Q_IDX_MAX_OVF_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ),
      (*rdIdx & Q_IDX_MAX_OVF_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ), (uint32_t)bl, (uint32_t)b, (uint32_t)cmd, atype );
    DBPRINT3("#%02u: Act 0x%08x, Qty is at %d\n", cpuId, *act, qty);

    if(qty) {
      ret = actionFuncs[atype](node, cmd, thrData);       //carry out the type specific action
      //decrement qty bitfield in working copy and write back to action field
      actTmp &= ~ACT_QTY_SMSK; //clear qty
      actTmp |= ((--qty) & ACT_QTY_MSK) << ACT_QTY_POS;
      *act    = actTmp;
    } else {
      skipOne = 1; //qty was exhausted before decrement, action can be skipped
      DBPRINT2("#%02u: Found deactivated command\n" );
    }

    *(rdIdx) = (*rdIdx + (uint8_t)(qty == 0) ) & Q_IDX_MAX_OVF_MSK; //pop element if qty exhausted

    //If we could skip and there are more cmds pending, exit and let the scheduler come back directly to this block for the next cmd in our queue
    if( skipOne && ((*awrOffs & BLOCK_CMDQ_WR_IDXS_SMSK) != (*ardOffs & BLOCK_CMDQ_RD_IDXS_SMSK)) ) {
      DBPRINT2("#%02u: Found more pending commands, skip deactivated cmd and process next cmd\n" );
      return node;
    }

    if (qty==0) DBPRINT3("#%02u: Qty reached zero, popping\n", cpuId);

  } else {
    DBPRINT3(" nothing pending\n");

  }
  return ret;

}


uint32_t* blockFixed(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret = block(node, thrData);

  *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] += *(uint64_t*)&node[BLOCK_PERIOD >> 2];     // increment current time sum by block period
  *(uint64_t*)&thrData[T_TD_DEADLINE >> 2]  = *(uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // next Deadline unknown, set to earliest possibilty -> current time sum + 0

  return ret;
}

uint32_t* blockAlign(uint32_t* node, uint32_t* thrData) {
  uint32_t     *ret = block(node, thrData);
  uint64_t      *tx =  (uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // current time
  uint64_t       Tx = *(uint64_t*)&node[BLOCK_PERIOD >> 2];     // block duration
  const uint64_t t0 = _T_GRID_OFFS_;                            // alignment offset (fix for now, change later)
  const uint64_t T0 = _T_GRID_SIZE_;                            // alignment period

  //goal: add block duration to current time, then round result up to alignment offset + nearest multiple of alignment period
  uint64_t diff = (*tx + Tx) - t0 + (T0 - 1) ;      // 1. add block period as minimum advancement 2. subtract alignment offset for division 3. add alignment period -1 for ceil effect
  *tx = diff - (diff % T0) + t0;                    // 4. subtract remainder of modulo for rounding 5. add alignment offset again
  *(uint64_t*)&thrData[T_TD_DEADLINE >> 2]  = *tx;  // next Deadline unknown, set to earliest possibilty -> current time sum + 0

  DBPRINT2("#%02u: Rounding to nearest multiple of T\n", cpuId);

  return ret;
}

uint32_t* origin(uint32_t* node, uint32_t* thrData) {
  DBPRINT3("#%02u: Hello, Origin function check, base shared 0x%08x\n", cpuId, PEER_ADR_MSK);
  uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  uint32_t newOrigin = *(uint32_t*)&node[ORIGIN_DEST >> 2];
  uint32_t targetCpu = *(uint32_t*)&node[ORIGIN_CPU >> 2];
  uint32_t targetThr = *(uint32_t*)&node[ORIGIN_THR >> 2];




  //FIXME black magic ahead! RAM sizes are assumed to be equal, _startshared adr is assumed to be the same everywhere
  uint32_t* targetBaseP = p; //(uint8_t*)(newOrigin & PEER_ADR_MSK);// + 
  

  uint32_t* targetOrigin = (uint32_t*)&targetBaseP[( SHCTL_THR_STA + targetThr * _T_TS_SIZE_ + T_TS_NODE_PTR) >> 2]; 
  *targetOrigin = newOrigin;
  
  DBPRINT3("#%02u: Hello, Origin node, target 0x%08x, cpu %u, thr %u, new origin 0x%08x, target origin 0x%08x\n", cpuId, targetOrigin , targetCpu, targetThr, newOrigin, *targetOrigin);
  
  return ret;
}


uint32_t* startThread(uint32_t* node, uint32_t* thrData) {
  uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  uint64_t offset = *(uint64_t*)&node[STARTTHREAD_STARTOFFS >> 2];
  //uint32_t cpu = *(uint32_t*)&node[STARTTHREAD_CPU >> 2];
  uint32_t thr = *(uint32_t*)&node[STARTTHREAD_THR >> 2];

  //FIXME This must go to the selected CPUs control area, not necessarily our own!
  uint8_t i;
  //iterate all threads. All threads to be started get a copy of the spawners current time sum + desired offset as starttime.
  for(i=0;i<_THR_QTY_;i++) { 
    if (!(thr & (1<<i))) {continue;} //more probable case of not starting a thread goes to branch taken
    else { //less probable case of starting goes to not taken
      uint64_t* thrStarttime  = (uint64_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_STARTTIME) >> 2]; // thread Start time
      *thrStarttime = *((uint64_t*)&thrData[T_TD_CURRTIME >> 2]) + offset; // set time
      DBPRINT3("#%02u: Hello, StartThread function check. Thr %u, time 0x%08x%08x, ptr 0x%08x\n", cpuId, i, (uint32_t)(*thrStarttime>>32), (uint32_t)*thrStarttime, &thrData[T_TD_CURRTIME >> 2]);
    }  
  } 
  
  //*start |= (1 << thr);  // set start bit
  // Oh ffs, why?! let's cut the "I only want ONE thread" corner case and use a bitmask. Learn to bitshift, basta.

  *start |= thr;  // set start bits
  return ret;
}



void heapify() {
  //restore MinHeap property
  // Start at the second to last layer of the heap (heapsize/2-1,  last layer not have children)
  int startSrc;
  for(startSrc=(_HEAP_SIZE_>>1) - 1; startSrc >= 0; startSrc--) {
    heapReplace(startSrc); // Work your way up to the top sorting nodes
  }
}

void heapReplace(uint32_t src) {
  // HeapRrelace, aka HeapMin Extraction
  //
  // We take the min element away from top of the heap and put a new element on top.
  // Then we sort the new element downward until it rests in the right place.
  // This is the case when the parent is less or equal than both left child (l) and right child (r)

  // normally, heap sort involves a swap at each layer, but that uses more copy operations than necessary (3*n vs n+1).
  // This approach compares the children not to their parent, but the moving node, ie. the original source node.
  // Each time the moving node is greater than its children, the chosen child is copied to its parents position, thus moving it upward one layer.
  // Once the moving node is less or equal to both children (or does not have childern), we copy the moving node to the parent position and are done.
  
  uint32_t parent = src;      // index of the parent, initial value is the source node we were given  
  uint32_t choice;            // index of the child we choose in each step
  uint32_t* moving = hp[src]; // The source node is moving downward, and we compare to this moving node in each step. Must be saved because we overwrite hp[src]

  uint8_t sort;
  do { //sort loop happens at least once... 
    sort = 0;
    
    uint32_t left   = (parent << 1) + 1;  // left child idx is parent idx * 2 + 1
    uint32_t right  = left + 1;           // right child idx is left child idx +1 = parent idx * 2 + 2
             choice = parent;             // parent is the default choice. Means copy has no effect in case we dont choose a child.

    //we choose left child if...
    if( (        left < _HEAP_SIZE_   )   // left child exists...
    &&  (DL(hp[left]) < DL(moving)))      // and its deadline less than moving node's
    {  
        sort = 1;
        choice = left;
    }
    
    //we choose right child (can override left) if...
    if( (         right < _HEAP_SIZE_    )  // right child exists...
    &&  ( DL(hp[right]) < DL(moving)     )  // and its deadline is less than moving node's ...
    &&  ( DL(hp[right]) < DL(hp[left])   )) // and its deadline is less than left child's
    { 
      sort = 1;
      choice = right;
    }

    hp[parent]  = hp[choice]; // copy chosen child upward to parent location
    parent      = choice;     // parent location for the next round is the location of our chosen child.

  } while(sort); // ... and sort loop continues if nodes were rearranged last round.

  // we found the location the moving node is supposed to go. Copy it in and we are done.
  hp[choice]    = moving; 
}




uint32_t* dynamicNodeStaging(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret;
  //create our pristine copy
  memcpy(nodeTmps[NODE_TMP_BAK], node, _MEM_BLOCK_SIZE); //make a copy of the original node we can edit
      
  // first dereferencing pass, fill in nodeTmps[OLD]
  deRefNode(nodeTmps[NODE_TMP_BAK], nodeTmps[NODE_TMP_OLD]);

  //abort if we read something different 5 times
  int8_t readCnt = 0;
  while (readCnt++ < MAX_RD_RETRIES) {
    // 2nd to n-th dereferencing pass, fill in nodeTmps[NEW]
    deRefNode(nodeTmps[NODE_TMP_BAK], nodeTmps[NODE_TMP_NEW]);
    
    //check if NEW dataset matches OLD from last pass. If so, we are consistent and done. If not, increase warning counter
    if(matchNode(nodeTmps[NODE_TMP_OLD], nodeTmps[NODE_TMP_NEW])) {break;}
    
    //Copy current NEW to OLD for next comparison. Rinse and repeat
    memcpy(nodeTmps[NODE_TMP_OLD], nodeTmps[NODE_TMP_NEW], _MEM_BLOCK_SIZE);
  } 
  if(readCnt >= MAX_RD_RETRIES) {*status |= SHCTL_STATUS_DM_ERROR_SMSK | SHCTL_STATUS_DM_UNSTABLE_RD_SMSK; return LM32_NULL_PTR;}
  
  uint32_t* nodeTmp = nodeTmps[NODE_TMP_NEW];
  //call handler function
  ret = nodeFuncs[getNodeType(nodeTmp)](nodeTmp, thrData); 
  
  //copy back all changes to immediate/val fields
  uint32_t wordFormats = nodeTmp[NODE_OPT_DYN >> 2]; //reload description
  for(unsigned i = 0; i < 9; i++) {
    if ((wordFormats & DYN_MODE_MSK) < DYN_MODE_REF) { node[i] = nodeTmp[i]; } // immediate/adr ?
    wordFormats >>= 3; //shift right by 3 bits to get next wordFormat
  }

  //we must never return nodeTmp, as this is not threadsafe. if handler wants to return nodeTmp, return original node instead.
  if (ret != nodeTmp) return ret;
  else                return node;

}
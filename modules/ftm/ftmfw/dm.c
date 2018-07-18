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

uint64_t SHARED dummy = 0;

deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_];
nodeFuncPtr     nodeFuncs[_NODE_TYPE_END_];
actionFuncPtr   actionFuncs[_ACT_TYPE_END_];

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
uint32_t* const bcklogmax = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_BCKLOG_STRK )  >> 2];
uint32_t* const badwaitcnt = (uint32_t*) &_startshared[(SHCTL_DIAG + T_DIAG_BAD_WAIT_CNT )  >> 2];
#endif
uint32_t* const start   = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_START)   >> 2];
uint32_t* const running = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_RUNNING) >> 2];
uint32_t* const abort1  = (uint32_t*)&_startshared[(SHCTL_THR_CTL + T_TC_ABORT)   >> 2];
uint32_t** const hp     = (uint32_t**)&_startshared[SHCTL_HEAP >> 2]; // array of ptrs to thread data for scheduler heap

void prioQueueInit()
{
 
   pFpqCtrl[PRIO_RESET_OWR>>2]      = 1;
   pFpqCtrl[PRIO_MODE_CLR>>2]       = 0xffffffff;
   pFpqCtrl[PRIO_ECA_ADR_RW>>2]     = (uint32_t)pEca & ~0x80000000;
   pFpqCtrl[PRIO_EBM_ADR_RW>>2]     = ((uint32_t)pEbm & ~0x80000000);
   pFpqCtrl[PRIO_TX_MAX_MSGS_RW>>2] = 40;
   pFpqCtrl[PRIO_TX_MAX_WAIT_RW>>2] = loW((uint64_t)(50000));
   pFpqCtrl[PRIO_MODE_SET>>2]       = PRIO_BIT_ENABLE     | 
                                      PRIO_BIT_MSG_LIMIT  |
                                      PRIO_BIT_TIME_LIMIT;
}


void dmInit() {


  nodeFuncs[NODE_TYPE_UNKNOWN]          = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_RAW]              = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_TMSG]             = tmsg;
  nodeFuncs[NODE_TYPE_CNOOP]            = cmd;
  nodeFuncs[NODE_TYPE_CFLOW]            = cmd;
  nodeFuncs[NODE_TYPE_CFLUSH]           = cmd;
  nodeFuncs[NODE_TYPE_CWAIT]            = cmd;
  nodeFuncs[NODE_TYPE_BLOCK_FIXED]      = blockFixed;
  nodeFuncs[NODE_TYPE_BLOCK_ALIGN]      = blockAlign;
  nodeFuncs[NODE_TYPE_QUEUE]            = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_QBUF]             = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_SHARE]            = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_ALTDST]           = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_SYNC]             = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_NULL]             = nodeNull;

  //deadline updater. Return infinity (-1) if no or unsupported node was given
  deadlineFuncs[NODE_TYPE_UNKNOWN ]     = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_RAW]          = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_TMSG]         = dlEvt;
  deadlineFuncs[NODE_TYPE_CNOOP]        = dlEvt;
  deadlineFuncs[NODE_TYPE_CFLOW]        = dlEvt;
  deadlineFuncs[NODE_TYPE_CFLUSH]       = dlEvt;
  deadlineFuncs[NODE_TYPE_CWAIT]        = dlEvt;
  deadlineFuncs[NODE_TYPE_BLOCK_FIXED]  = dlBlock;
  deadlineFuncs[NODE_TYPE_BLOCK_ALIGN]  = dlBlock;
  deadlineFuncs[NODE_TYPE_QUEUE]        = dummyDeadlineFunc;
  deadlineFuncs[NODE_TYPE_QBUF]         = dummyDeadlineFunc; 
  deadlineFuncs[NODE_TYPE_SHARE]        = dummyDeadlineFunc; 
  deadlineFuncs[NODE_TYPE_ALTDST]       = dummyDeadlineFunc; 
  deadlineFuncs[NODE_TYPE_SYNC]         = dummyDeadlineFunc;
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
    *bcklogmax = 0;
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
  if(cmd[T_CMD_ACT >> 2] & ACT_CHP_SMSK) node[NODE_DEF_DEST_PTR >> 2] = (uint32_t)ret;
  return ret;

}

uint32_t* execFlush(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {
  uint8_t prios = (cmd[T_CMD_ACT >> 2] >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;
  

  if(prios & (1 << PRIO_LO)) *((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - PRIO_LO -1) = *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - PRIO_LO -1);
  if(prios & (1 << PRIO_HI)) *((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - PRIO_HI -1) = *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - PRIO_HI -1);
  // makes no sense to flush interlock priority, skipping

  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];


}

uint32_t* execWait(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {

  // the block period is added in blockFixed or blockAligned.
  // we must therefore subtract it here if we modify current time, as execWait is optional

  uint64_t  tWait = *(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] - *(uint64_t*)&node[BLOCK_PERIOD >> 2];
  uint64_t*  tCur = (uint64_t*)&thrData[T_TD_CURRTIME >> 2];
  uint32_t    act = cmd[T_CMD_ACT >> 2];

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
        uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  const uint32_t prio = (node[CMD_ACT >> 2] >> ACT_PRIO_POS) & ACT_PRIO_MSK;
  const uint32_t *tg  = (uint32_t*)node[CMD_TARGET >> 2];
  const uint32_t adrPrefix = (uint32_t)tg & 0xffff0000; // if target is on a different RAM, all ptrs must be translated from the local to our (peer) perspective

  uint32_t *bl, *b, *e;
  uint8_t  *wrIdx;
  uint32_t bufOffs, elOffs;
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited  
  
  if(tg == LM32_NULL_PTR) { // check if the target is a null pointer. Used to allow removal of pattern containing target nodes
    return ret;
  }

  wrIdx   = ((uint8_t *)tg + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - prio -1);                           // calculate pointer (8b) to current write index
  bufOffs = (*wrIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;             // calculate Offsets
  elOffs  = (*wrIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; 
  bl      = (uint32_t*)(tg[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2] - INT_BASE_ADR + adrPrefix); // get pointer to buf list
  b       = (uint32_t*)(bl[bufOffs >> 2] - INT_BASE_ADR + adrPrefix);                               // get pointer to buf
  e       = (uint32_t*)&b[elOffs >> 2];                                                             // get pointer to Element to write

  DBPRINT3("#%02u: Prio: %u, pre: 0x%08x, base: 0x%08x, wrIdx: 0x%08x, target: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x\n", cpuId, prio, adrPrefix, INT_BASE_ADR, (uint32_t)wrIdx, (uint32_t)tg, (uint32_t)bl, (uint32_t)b, (uint32_t)e );

  // Important !!! So the host can verify if a command is active as seen from an LM32 by comparing with cursor timestamping,
  // we cannot place ANY tvalids into the past. All tValids must be placed sufficiently into the future so that this here function
  // terminates before tValid is reached. This is the responsibility of carpeDM!

  // !!! CAVEAT !!! when tvalid is used as relative offset in multiple commands to synchronise them, the sum of block start (current time sume) + tvalid
  // must always be greater than tMinimum, else the commands get differing tvalids assigned and will no longer synchronously become valid !!!
  
  uint64_t* ptValid   = (uint64_t*)&node[CMD_VALID_TIME   >> 2];
  uint64_t* ptCurrent = (uint64_t*)&thrData[T_TD_CURRTIME >> 2];
  uint64_t tValid;

  if((node[CMD_ACT  >> 2] >> ACT_VABS_POS) & ACT_VABS_MSK) tValid = *ptValid;
  else                                                     tValid = *ptCurrent + *ptValid;

//TODO: find out what's wrong with this code
/*
  uint64_t vabsMsk    = (uint64_t)((node[CMD_ACT  >> 2] >> ACT_VABS_POS) & ACT_VABS_MSK) -1; // abs -> 0x0, !abs -> 0xfff..f
  uint64_t tMinimum   = getSysTime() + _T_TVALID_OFFS_;                 // minimum timestamp that we consider 'future'
  uint64_t tValidCalc = *ptValid + *ptCurrent & vabsMsk;                // if tValid is relative offset (not absolute), add current time sum
  uint64_t tMinMsk    = (uint64_t)(tMinimum < tValidCalc) -1;           // min < calc -> 0x0, min >= calc -> 0xfff..f
  uint64_t tValid     = (tMinimum & tMinMsk) | (tValidCalc & ~tMinMsk); // equiv. tValid = (tMinimum < tvalidCalc) ? tvalidCalc : tMinimum;
*/
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
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_ID_HI  >> 2];
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_ID_LO  >> 2];
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = hiW(tmpPar);
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = loW(tmpPar);
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_RES    >> 2];
  *(pFpqData + (PRIO_DAT_STD   >> 2))  = node[TMSG_TEF    >> 2];
  *(pFpqData + (PRIO_DAT_TS_HI >> 2))  = thrData[T_TD_DEADLINE_HI >> 2];
  *(pFpqData + (PRIO_DAT_TS_LO >> 2))  = thrData[T_TD_DEADLINE_LO >> 2];
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
  uint32_t bufOffs, elOffs, prio, actTmp, atype;
  uint32_t qty;
   
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK; // set paint bit to mark this node as visited

  //3 ringbuffers -> 3 wr indices, 3 rd indices (one per priority). If any differ, there's work to do
  if( (*awrOffs & 0x00ffffff) != (*ardOffs & 0x00ffffff) ) {
    //only process one command, and that of the highest priority. default is low, check up
    prio = PRIO_LO;
    //MSB first: bit 31 is at byte offset 0!
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_ - PRIO_HI - 1) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_ - PRIO_HI - 1)) { prio = PRIO_HI; }
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_ - PRIO_IL - 1) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_ - PRIO_IL - 1)) { prio = PRIO_IL; }
    //correct prio found, create shortcuts to control data of corresponding queue
    rdIdx   = ((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - prio -1);               // this queues read index   (using overflow bit for empty/full)
    wrIdx   = ((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - prio -1);               // this queues write index
    bufOffs = (*rdIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;   // offset of active command buffer's pointer in this queues buffer list
    elOffs  = (*rdIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; // offset of active command data in active command buffer
    bl      = (uint32_t*)node[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];                  // pointer to this queues buffer list
    b       = (uint32_t*)bl[bufOffs >> 2];                                                  // pointer to active command buffer
    cmd     = (uint32_t*)&b[elOffs  >> 2];                                                  // pointer to active command data 
    
    
    if (getSysTime() < *((uint64_t*)((void*)cmd + T_CMD_TIME))) return ret;                 // if chosen command is not yet valid, directly return to scheduler

    
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
      skipOne = 1; //qty was exhausted before decrement, can be skipped
      DBPRINT2("#%02u: Found deactivated command\n" );
    }

    *(rdIdx) = (*rdIdx + (uint8_t)(qty == 0) ) & Q_IDX_MAX_OVF_MSK; //pop element if qty exhausted
    
    //If we could skip and there are more cmds pending, exit and let the scheduler come back directly to this block for the next cmd in our queue
    if( skipOne && ((*awrOffs & 0x00ffffff) != (*ardOffs & 0x00ffffff)) ) {
      DBPRINT2("#%02u: Found more pending commands, skip deactivated cmd and process next cmd\n" );
      return node;
    }  

    if (qty==0) DBPRINT3("#%02u: Qty reached zero, popping\n", cpuId);

  } else {
    DBPRINT3(" nothing pending\n");
    
  }  
  return ret;
  
}


// a normal time block
uint32_t* blockFixed(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret = block(node, thrData);
   
  *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] += *(uint64_t*)&node[BLOCK_PERIOD >> 2];     // increment current time sum by block period 
  *(uint64_t*)&thrData[T_TD_DEADLINE >> 2]  = *(uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // next Deadline unknown, set to earliest possibilty -> current time sum + 0
  
  return ret;
}  

// self aligning time block. extends own length dynamically to meet given gridsize T0 originating at t0
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





void heapify() {
  int startSrc;
  //go through the heap, restore heap property
  for(startSrc=(_HEAP_SIZE_-1)>>1; startSrc >= 0; startSrc--) { heapReplace(startSrc); }
}

void heapReplace(uint32_t src) {
  uint32_t  dst = src, cl = 1, cr = 1, mask, lLEr, mGr, mGl, l, r;
  uint32_t* mov = hp[dst];
  int j;
  //for (j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
  
  DBPRINT3("#%02u: Looking at Dl Mov %u: %20s Dl LC %u: %20s, DL RC %u: %20s\n",  cpuId, dst, print64(DL(mov), 0), l, print64(DL(hp[l]), 0), r, print64(DL(hp[r]), 0) );
  
  // unrolled heap replace operation
  while (cl | cr)  {
    //for (j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
    l       = (dst<<1)+1; // left child
    r       = l + 1;      // right child
      
    lLEr    = (DL(hp[l]) <= DL(hp[r]))  | (r  > _HEAP_SIZE_ -1);  // (left child less or equal r c) or r c non existent  
    mGr     = (DL(mov)   >  DL(hp[r]))  & (r  < _HEAP_SIZE_ );    // mover greater right child and r c exists
    mGl     = (DL(mov)   >  DL(hp[l]))  & (l  < _HEAP_SIZE_ );    // mover greater left child and l c exists
      
    cl      = ( mGl &  lLEr);                    // choose left child
    cr      = ( mGr & ~lLEr);                    // choose right child 
    mask    = -(cl | cr);                        // all 0 when no chosen child, all 1 otherwise
    src     = dst + ((dst + 1) & mask) + cr;     //parent  = dst, left = dst+(dst+1), rightC  = dst+(dst+1)+1   
    
    hp[dst] = hp[src];
    dst     = src;

  }

  hp[dst] = mov;
  

}
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

uint32_t* const p       = (uint32_t*)_startshared; 
uint32_t* const status  = (uint32_t*)_startshared + ( SHCTL_STATUS >> 2);
uint32_t* const count   = (uint32_t*)_startshared + ( SHCTL_MSG_CNT >> 2);
uint32_t* const start   = (uint32_t*)_startshared + ( (SHCTL_THR_CTL + T_TC_START)    >> 2);
uint32_t* const running = (uint32_t*)_startshared + ( (SHCTL_THR_CTL + T_TC_RUNNING)  >> 2);
uint32_t* const stop    = (uint32_t*)_startshared + ( (SHCTL_THR_CTL + T_TC_STOP)     >> 2);
uint32_t** const hp     = (uint32_t**)_startshared + ( SHCTL_HEAP >> 2); // array of ptrs to thread data for scheduler heap

void prioQueueInit()
{
   //set up the pointer to the global msg count
   //pMsgCntPQ = (uint64_t*)(pFpqCtrl + (PRIO_CNT_OUT_ALL_GET_0>>2));

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


  actionFuncs[ACT_TYPE_UNKNOWN]         = dummyActionFunc;
  actionFuncs[ACT_TYPE_NOOP]            = execNoop;
  actionFuncs[ACT_TYPE_FLOW]            = execFlow;
  actionFuncs[ACT_TYPE_FLUSH]           = execFlush;
  actionFuncs[ACT_TYPE_WAIT]            = execWait;  

  uint8_t i;
  for(i=0; i < _THR_QTY_; i++) {
    //set thread times to infinity
    uint32_t* tp = (uint32_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_) >> 2));
    *(uint64_t*)&tp[T_TD_CURRTIME >> 2] = -1;
    *(uint64_t*)&tp[T_TD_DEADLINE >> 2] = -1;
    *(uint32_t*)&tp[T_TD_FLAGS >> 2]    = i;
    *(uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_PREPTIME  ) >> 2)) = 500000ULL;
    *(uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_STARTTIME  ) >> 2)) = 0ULL;
    //add thread to heap
    hp[i] = tp;
  }  


}


void threadStart() {
  //any bits in start reg set?
    //iterate bits
      //if 1, set  currenttime and deadline to starttime
  //heapify
}

uint64_t dlEvt(uint32_t* node, uint32_t* thrData) {
  return *(uint64_t*)&thrData[T_TD_DEADLINE >> 2] = *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] + *(uint64_t*)&node[EVT_OFFS_TIME >> 2];
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
  if(node[NODE_FLAGS >> 2] & ACT_CHP_SMSK) node[NODE_DEF_DEST_PTR >> 2] = (uint32_t)ret;
  return ret;

}

uint32_t* execFlush(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {
  uint8_t prios = (cmd[T_CMD_ACT >> 2] >> ACT_FLUSH_PRIO_POS) & ACT_FLUSH_PRIO_MSK;
  
  if(prios & (1 << PRIO_LO)) *((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - PRIO_LO -1) = *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - PRIO_LO -1);
  if(prios & (1 << PRIO_HI)) *((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - PRIO_HI -1) = *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - PRIO_HI -1);
  
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];


}

uint32_t* execWait(uint32_t* node, uint32_t* cmd, uint32_t* thrData) {

  // the block period is added in blockFixed or blockAligned.
  // we must therefore subtract it here if we modify current time, as execWait is optional

  if ( cmd[T_CMD_ACT >> 2] & ACT_WAIT_ABS_SMSK) {
    *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] = *(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] - *(uint64_t*)&node[BLOCK_PERIOD >> 2];                      //1. set absolute value - block period
    DBPRINT2("#%02u: Wait, Absolute to 0x%08x%08x\n", cpuId, (uint32_t)(*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] >> 32), (uint32_t)*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2]);  
  } else {
    if( cmd[T_CMD_ACT >> 2] & ACT_CHP_SMSK) {
      DBPRINT2("#%02u: Wait, Permanent relative to 0x%08x%08x\n", cpuId, (uint32_t)(*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] >> 32), (uint32_t)*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2]);  
      *(uint64_t*)&node[BLOCK_PERIOD >> 2] = *(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2];                         //2. set new block period
    } else {

      DBPRINT2("#%02u: Wait, temp relative to 0x%08x%08x\n", cpuId, (uint32_t)(*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] >> 32), (uint32_t)*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2]);  
      //mprintf("#%02u: Wait thr Old  0x%08x%08x\n", cpuId, (uint32_t)(*(uint64_t*)&thrData[T_TD_CURRTIME >> 2]>> 32), (uint32_t)*(uint64_t*)&thrData[T_TD_CURRTIME >> 2]);  
      *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] += (*(uint64_t*)&cmd[T_CMD_WAIT_TIME >> 2] - *(uint64_t*)&node[BLOCK_PERIOD]); //3. add temporary block period - block period
      //mprintf("#%02u: Wait thr new  0x%08x%08x\n", cpuId, (uint32_t)(*(uint64_t*)&thrData[T_TD_CURRTIME >> 2]>> 32), (uint32_t)*(uint64_t*)&thrData[T_TD_CURRTIME >> 2]);  
    }      
  }
   
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];

}

uint32_t* cmd(uint32_t* node, uint32_t* thrData) {

  const uint32_t prio = (node[CMD_ACT >> 2] >> ACT_PRIO_POS) & ACT_PRIO_MSK;
  const uint32_t *tg  = (uint32_t*)node[CMD_TARGET >> 2];
  const uint32_t adrPrefix = (uint32_t)tg & 0xffff0000; // if target is on a different RAM, all ptrs must be translated from the local to our (peer) perspective

  uint32_t *bl, *b, *e;
  uint8_t *wrIdx;
  uint32_t bufOffs, elOffs;
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  
  //calculate pointer (8b) to current write index
  wrIdx = ((uint8_t *)tg + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - prio -1);
  //calculate Offsets
  bufOffs = (*wrIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;
  elOffs  = (*wrIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; 

  //get pointer to buf list
  bl = (uint32_t*)(tg[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2] - INT_BASE_ADR + adrPrefix);

  //get pointer to buf
  b = (uint32_t*)(bl[bufOffs >> 2] - INT_BASE_ADR + adrPrefix);
  //get pointer to Element to write
  e = (uint32_t*)&b[elOffs >> 2];

  DBPRINT3("#%02u: Prio: %u, pre: 0x%08x, base: 0x%08x, wrIdx: 0x%08x, target: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x\n", cpuId, prio, adrPrefix, INT_BASE_ADR, (uint32_t)wrIdx, (uint32_t)tg, (uint32_t)bl, (uint32_t)b, (uint32_t)e );

  //write Cmd
  for(uint32_t offs = T_CMD_TIME; offs < T_CMD_TIME + _T_CMD_SIZE_; offs += _32b_SIZE_ ) {
    e[offs >> 2] = node[(CMD_VALID_TIME + offs) >> 2];
  }
  
  //increase write index
  *wrIdx = (*wrIdx + 1) & Q_IDX_MAX_OVF_MSK;

  DBPRINT2("#%02u: Sending Cmd 0x%08x, Target: 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], (uint32_t)tg, node[NODE_DEF_DEST_PTR >> 2]);
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* tmsg(uint32_t* node, uint32_t* thrData) {
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  DBPRINT2("#%02u: Sending Evt 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], node[NODE_DEF_DEST_PTR >> 2]);

  /*
  //Diagnostic Event? insert PQ Message counter. Different device, can't be placed inside atomic!
  if (pMsg->id == DIAG_PQ_MSG_CNT) tmpPar = *pMsgCntPQ;
  else                             tmpPar = pMsg->par;  
  */
  atomic_on();
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_ID_HI >> 2];
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_ID_LO >> 2];
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_PAR_HI >> 2];
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_PAR_LO >> 2];
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_TEF >> 2];
  *(pFpqData + (PRIO_DAT_STD>>2))   = node[TMSG_RES >> 2];
  *(pFpqData + (PRIO_DAT_TS_HI>>2)) = thrData[T_TD_DEADLINE_HI >> 2];
  *(pFpqData + (PRIO_DAT_TS_LO>>2)) = thrData[T_TD_DEADLINE_LO >> 2];
  atomic_off();
    
  ++thrData[T_TD_MSG_CNT >> 2];
   
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}



uint32_t* block(uint32_t* node, uint32_t* thrData) {
  DBPRINT2("#%02u: Checking Block 0x%08x\n", cpuId, node[NODE_HASH >> 2]);
  uint32_t *ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  uint32_t *bl, *b, *cmd, *act;
  uint8_t  *rdIdx,*wrIdx;

  uint32_t *ardOffs = node + (BLOCK_CMDQ_RD_IDXS >> 2), *awrOffs = node + (BLOCK_CMDQ_WR_IDXS >> 2);
  uint32_t bufOffs, elOffs, prio, actTmp, atype;
  uint16_t qty;
  
  
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;

  if( (*awrOffs & 0x00ffffff) != (*ardOffs & 0x00ffffff) ) {
    prio = PRIO_LO;
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_ - PRIO_HI - 1) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_ - PRIO_HI - 1)) { prio = PRIO_HI; }
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_ - PRIO_IL - 1) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_ - PRIO_IL - 1)) { prio = PRIO_IL; }
    rdIdx   = ((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + _32b_SIZE_  - prio -1);
    wrIdx   = ((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + _32b_SIZE_  - prio -1);
    bufOffs = (*rdIdx & Q_IDX_MAX_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;
    elOffs  = (*rdIdx & Q_IDX_MAX_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_;
    bl      = (uint32_t*)node[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];
    b       = (uint32_t*)bl[bufOffs >> 2];
    cmd     = (uint32_t*)&b[elOffs  >> 2];
    
    //check valid time
    if (getSysTime() < *((uint64_t*)((void*)cmd + T_CMD_TIME))) return node;

    //get action type
    act = (uint32_t*)&cmd[T_CMD_ACT >> 2];
    actTmp = *act;
    qty = (actTmp >> ACT_QTY_POS) & ACT_QTY_MSK;
    //if(qty >= 1) {
    atype = (actTmp >> ACT_TYPE_POS) & ACT_TYPE_MSK;
    DBPRINT2("#%02u: pending Cmd @ Prio: %u, awdIdx: 0x%08x, 0x%02x, ardIdx: 0x%08x, 0x%02x, buf: %u, el: %u, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x, type: %u\n", cpuId, prio, *awrOffs, *wrIdx, *ardOffs, *rdIdx, (*rdIdx & Q_IDX_MAX_OVF_MSK) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ), 
      (*rdIdx & Q_IDX_MAX_OVF_MSK) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ), (uint32_t)bl, (uint32_t)b, (uint32_t)cmd, atype );
    
    //carry out type specific action
    ret = actionFuncs[atype](node, cmd, thrData);

    //decrement qty
    
    DBPRINT3("#%02u: Act 0x%08x, Qty is at %d\n", cpuId, *act, qty);
    
    //if qty <= 1, pop cmd -> increment read offset
    if(qty <= 1) { *(rdIdx) = (*rdIdx + 1) & Q_IDX_MAX_OVF_MSK; DBPRINT3("#%02u: Qty reached zero, popping\n", cpuId);}
    //decrement qty
    actTmp &= ~ACT_QTY_SMSK; //clear qty
    actTmp |= ((--qty) & ACT_QTY_MSK) << ACT_QTY_POS; // OR in decremented and shifted qty
    *act = actTmp; //write back

    //} else {mprintf("#%02u: Error: Qty is already at %d !\n", cpuId, *act, qty); }
  } else {
    //mprintf(" nothing pending\n");
    
  }  
  return ret;
  
}



uint32_t* blockFixed(uint32_t* node, uint32_t* thrData) {
  uint32_t* ret = block(node, thrData);
    
  *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] += *(uint64_t*)&node[BLOCK_PERIOD >> 2];
  *(uint64_t*)&thrData[T_TD_DEADLINE >> 2]  = *(uint64_t*)&thrData[T_TD_CURRTIME >> 2]; // Deadline must follow block shift
  
  return ret;
}  

uint32_t* blockAlign(uint32_t* node, uint32_t* thrData) {
  uint32_t *ret = block(node, thrData);
  uint64_t      *tx =  (uint64_t*)&thrData[T_TD_CURRTIME >> 2];  // current time
  uint64_t       Tx = *(uint64_t*)&node[BLOCK_PERIOD >> 2];      // block duration
  const uint64_t t0 = 0;                                    // alignment offset
  const uint64_t T0 = 10000;                                // alignment period

  //goal: add block duration to current time, then round result up to alignment offset + nearest multiple of alignment period
  uint64_t diff = (*tx + Tx) - t0 + (T0 - 1) ; // 1. add block period as minimum advancement 2. subtract alignment offset for division 3. add alignment period -1 for ceil effect  
  *tx = diff - (diff % T0) + t0;               // 4. subtract remainder of modulo for rounding 5. add alignment offset again  
  *(uint64_t*)&thrData[T_TD_DEADLINE >> 2]  = *tx; // Deadline must follow block shift
  
  DBPRINT2("#%02u: Rounding to nearest multiple of T\n", cpuId);

  return ret;
}

uint32_t* dummyNodeFunc (uint32_t* node, uint32_t* thrData) { return NULL;}
uint64_t dummyDeadlineFunc (uint32_t* node, uint32_t* thrData) { return -1;}
uint32_t* dummyActionFunc (uint32_t* node, uint32_t* cmd, uint32_t* thrData) { return NULL;}



void heapify() {
  int startSrc;

  //go through the heap, restore heap property
  //where to start ?
  for(startSrc=(_HEAP_SIZE_-1)>>1; startSrc >= 0; startSrc--) { heapReplace(startSrc); }
  //for(startSrc=0; startSrc < _HEAP_SIZE_; startSrc++) { heapReplace(0); }  
}

void heapReplace(uint32_t src) {
  uint32_t  dst = src, cl = 1, cr = 1, mask, lLEr, mGr, mGl, l, r;
  uint32_t* mov = hp[dst];
  int j;
  //for (j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
  
  //mprintf("#%02u: Looking at Dl Mov %u: %20s Dl LC %u: %20s, DL RC %u: %20s\n",  cpuId, dst, print64(DL(mov), 0), l, print64(DL(hp[l]), 0), r, print64(DL(hp[r]), 0) );
  
  while (cl | cr)  {
    //for (j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
    l     = (dst<<1)+1; // left child
    r     = l + 1;      // right child
    
    lLEr  = (DL(hp[l]) <= DL(hp[r]))  | (r  > _HEAP_SIZE_ -1); //(left child less or equal r c) or r c non existent  
    mGr   = (DL(mov) > DL(hp[r]))     & (r  < _HEAP_SIZE_ ); //mover greater right child and r c exists
    mGl   = (DL(mov) > DL(hp[l]))     & (l  < _HEAP_SIZE_ ); //mover greater left child and l c exists
    
    cl    = ( mGl &  lLEr);                    // choose left child
    cr    = ( mGr & ~lLEr);                    // choose right child 
    mask  = -(cl | cr);                        // all 0 when no chosen child, all 1 otherwise
    src   = dst + ((dst + 1) & mask) + cr;     //parent  = dst, left = dst+(dst+1), rightC  = dst+(dst+1)+1   
    
    hp[dst] = hp[src];
    dst     = src;

  }

  hp[dst] = mov;
  

}
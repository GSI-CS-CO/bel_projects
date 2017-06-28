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
#include "../ftm_common.h"
#include "prio_regs.h"

unsigned int cpuId, cpuQty, heapCap;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;
extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


typedef uint64_t  (*deadlineFuncPtr) ( uint32_t*, uint32_t* );
typedef uint32_t* (*nodeFuncPtr)  ( uint32_t*, uint32_t* );
typedef uint32_t* (*actionFuncPtr)( uint32_t*, uint32_t*, uint32_t* );

deadlineFuncPtr deadlineFuncs[_NODE_TYPE_END_];
nodeFuncPtr         nodeFuncs[_NODE_TYPE_END_];
actionFuncPtr      actionFuncs[_ACT_TYPE_END_];

// TODO clean this up
// If there is a correct way to return literals from function pointer array, I couldn't find it. Use this workaround for undefined elements
uint32_t* dummyNodeFunc (uint32_t* node, uint32_t* thrData) { return NULL;}
uint64_t dummyDeadlineFunc (uint32_t* node, uint32_t* thrData) { return -1;}
uint32_t* dummyActionFunc (uint32_t* node, uint32_t* cmd, uint32_t* thrData) { return NULL;}

#define PRIO_BIT_ENABLE     (1<<0)
#define PRIO_BIT_MSG_LIMIT  (1<<1)
#define PRIO_BIT_TIME_LIMIT (1<<2)

#define PRIO_DAT_STD     0x00
#define PRIO_DAT_TS_HI   0x04    
#define PRIO_DAT_TS_LO   0x08 
#define PRIO_DRP_TS_HI   0x14    
#define PRIO_DRP_TS_LO   0x18  

#define DIAG_PQ_MSG_CNT  0x0FA62F9000000000

inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;}


static uint32_t getNextThreadIdx() {
  return 0;
}

void show_msi()
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

}

void isr0()
{
   mprintf("ISR0\n");   
   show_msi();
}

void isr1()
{
   mprintf("ISR1\n");   
   show_msi();
}



void ebmInit()
{
  
   int j;
   
   while (*(pEbCfg + (EBC_SRC_IP>>2)) == EBC_DEFAULT_IP) {
     for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
     mprintf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);  
   } 

   ebm_init();

   ebm_config_meta(1500, 42, 0x00000000 );
   ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff,                0xebd0); //Dst: EB broadcast 
   ebm_config_if(SOURCE,      0xd15ea5edbeef, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: bogus mac (will be replaced by WR), WR IP

}

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

void init()
{ 
   discoverPeriphery();
   uart_init_hw();
   cpuId = getCpuIdx();
   

   if (cpuId == 0) {

     ebmInit();
     prioQueueInit();
     mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId); 
   }
   
   isr_table_clr();
   irq_set_mask(0x01);
   irq_disable(); 
   
}

uint32_t __div64_32(uint64_t *n, uint32_t base)
{
        uint64_t rem = *n;
        uint64_t b = base;
        uint64_t res, d = 1;
        uint32_t high = rem >> 32;

        /* Reduce the thing a bit first */
        res = 0;
        if (high >= base) {
                high /= base;
                res = (uint64_t) high << 32;
                rem -= (uint64_t) (high*base) << 32;
        }

        while ((int64_t)b > 0 && b < rem) {
                b = b+b;
                d = d+d;
        }

        do {
                if (rem >= b) {
                        rem -= b;
                        res += d;
                }
                b >>= 1;
                d >>= 1;
        } while (d);

        *n = res;
        return rem;
}


#define PRINT64_FACTOR  1000000000LL
static char* print64(uint64_t x, int align)
{
        uint32_t h_half, l_half;
        static char buf[2*10+1];        //2x 32-bit value + \0

        if (x < PRINT64_FACTOR)
                if (align)
                        sprintf(buf, "%20u", (uint32_t)x);
                else
                        sprintf(buf, "%u", (uint32_t)x);
        else {
                l_half = __div64_32(&x, PRINT64_FACTOR);
                h_half = (uint32_t) x;
                if (align)
                        sprintf(buf, "%11u%09u", h_half, l_half);
                else
                        sprintf(buf, "%u%09u", h_half, l_half);
        }
        return buf;
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
  uint32_t* ret;

  //TODO: FLUSH
  ret = (uint32_t*)cmd[T_CMD_FLOW_DEST >> 2]; 
  return ret;

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
  bl = (uint32_t*)tg[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];

  //get pointer to buf
  b = (uint32_t*)bl[bufOffs >> 2];
  //get pointer to Element to write
  e = (uint32_t*)&b[elOffs >> 2];

  DBPRINT3("#%02u: Prio: %u, wrIdx: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x\n", cpuId, prio, (uint32_t)wrIdx, (uint32_t)bl, (uint32_t)b, (uint32_t)e );

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
  int16_t qty;
  
  
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
    actTmp &= ~ACT_QTY_SMSK;
    actTmp |= ((--qty) & ACT_QTY_MSK) << ACT_QTY_POS;
    *act = actTmp;
    //if qty <= zero, pop cmd -> increment read offset
    if(qty <= 0) { *(rdIdx) = (*rdIdx + 1) & Q_IDX_MAX_OVF_MSK; DBPRINT3("#%02u: Qty reached zero, popping\n", cpuId);}
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





void paintPath(uint32_t* node) {
  mprintf("#%02u: Received 0x%08x\n", cpuId, (uint32_t)node);
  mprintf("#%02u: @ 0x%08x FL 0x%08x # 0x%08x DD 0x%08x \n", cpuId, (uint32_t)(uint32_t*)&node[NODE_HASH >> 2], node[NODE_FLAGS >> 2], node[NODE_HASH >> 2], node[NODE_DEF_DEST_PTR >> 2]);
  if(!(node[NODE_FLAGS >> 2] & NFLG_PAINT_LM32_SMSK)) {
    node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
    mprintf("#%02u: Painted 0x%08x\n", cpuId, node[NODE_HASH >> 2]);
    if ((uint32_t*)node[NODE_DEF_DEST_PTR >> 2] != NULL) paintPath((uint32_t*)node[NODE_DEF_DEST_PTR >> 2]);
  }
}



void main(void) {
   
  int i,j;
  uint32_t   start;
  uint32_t** test;
  uint32_t *p  = (uint32_t*)_startshared; 

  nodeFuncs[NODE_TYPE_UNKNOWN]      = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_RAW]          = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_TMSG]         = tmsg;
  nodeFuncs[NODE_TYPE_CNOOP]        = cmd;
  nodeFuncs[NODE_TYPE_CFLOW]        = cmd;
  nodeFuncs[NODE_TYPE_CFLUSH]       = cmd;
  nodeFuncs[NODE_TYPE_CWAIT]        = cmd;
  nodeFuncs[NODE_TYPE_BLOCK_FIXED]  = blockFixed;
  nodeFuncs[NODE_TYPE_BLOCK_ALIGN]  = blockAlign;
  nodeFuncs[NODE_TYPE_QUEUE]        = dummyNodeFunc;
  nodeFuncs[NODE_TYPE_QBUF]         = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_SHARE]        = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_ALTDST]       = dummyNodeFunc; 
  nodeFuncs[NODE_TYPE_SYNC]         = dummyNodeFunc;

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


  actionFuncs[ACT_TYPE_UNKNOWN] = dummyActionFunc;
  actionFuncs[ACT_TYPE_NOOP]    = execNoop;
  actionFuncs[ACT_TYPE_FLOW]    = execFlow;
  actionFuncs[ACT_TYPE_FLUSH]   = execFlush;
  actionFuncs[ACT_TYPE_WAIT]    = execWait; 

  init();
   
   
   
   // wait 1s + cpuIdx * 1/10s
   for (j = 0; j < ((125000000/4)+(cpuId*300000)); ++j) { asm("nop"); }
   atomic_on();
      
   mprintf("#%02u: Rdy\n", cpuId);
   #if DEBUGLEVEL != 0
      mprintf("#%02u: Debuglevel %u. Don't expect timeley delivery with console outputs on!\n", cpuId, DEBUGLEVEL);
   #endif   
   #if DEBUGTIME == 1
      mprintf("#%02u: Debugtime mode ON. Par Field of Msgs will be overwritten be dispatch time at lm32\n", cpuId);
   #endif
   #if DEBUGPRIOQ == 1
      mprintf("#%02u: Priority Queue Debugmode ON, timestamps will be written to 0x%08x on receivers", cpuId, DEBUGPRIOQDST);
   #endif
   //mprintf("Found MsgBox at 0x%08x. MSI Path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);
   mprintf("#%02u: This Graph Test v 0.1 \n", cpuId);

   atomic_off();
   if (getMsiBoxCpuSlot(cpuId, 0) == -1) {mprintf("#%02u: Mail box slot acquisition failed\n", cpuId);}
  

   
   while (1) {


    uint32_t* start = p + (( SHCTL_THR_CTL + T_TC_START ) >> 2);
    uint32_t* running = p + (( SHCTL_THR_CTL + T_TC_RUNNING ) >> 2);
    uint32_t* stop = p + (( SHCTL_THR_CTL + T_TC_STOP  ) >> 2);

    for(i=0;i<8;i++) {
      
      DBPRINT2("#%02u: b4 Start 0x%08x, Stop 0x%08x, Running 0x%08x\n",  cpuId, *start, *stop, *running);


      if (*start & 1<<i) {
        *running |= (1<<i);
        *start   &= ~(1<<i);
        //mprintf("#%02u: Start Thr #%u\n", cpuId, i);
        uint32_t* tp;
        uint32_t** np;
        uint32_t type;
        uint64_t now;

        tp = (uint32_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_) >> 2));  
        np = (uint32_t**)&tp[T_TD_NODE_PTR >> 2];
        // init to requested start node  
        *np = (uint32_t*)*(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_NODE_PTR ) >> 2));
        DBPRINT1("#%02u: ThrPtr: 0x%08x, Initial Node Ptr @ 0x%08x is 0x%08x\n",  cpuId, tp, np, *np);
         

        //now = getSysTime();
        

        *(uint64_t*)&tp[T_TD_CURRTIME >> 2] = getSysTime() + 5000000ULL;
        *(uint64_t*)&tp[T_TD_DEADLINE >> 2] = *(uint64_t*)&tp[T_TD_CURRTIME >> 2];

        while(*np != NULL && (*running & 1<<i) ) {
          *running &= ~(*stop & 1<<i);
          *stop  &= ~(1<<i);  
          //while (!(*start & 1<<i)) {
            //for (j = 0; j < ((12500000/8)); ++j) { asm volatile ("nop"); } 
            //mprintf("#%02u: Waiting at 0x%08x\n", cpuId, *np);
          //};
          //*start &= ~(1<<i);


          if ((*(uint64_t*)&tp[T_TD_DEADLINE >> 2] - 500000ULL) > getSysTime()) continue;

          //process node and update node ptr in threadData

          //*np is checked to be not null, so
          uint32_t* tmpType = (uint32_t*)&(*np)[NODE_FLAGS >> 2];
          type = (*(uint32_t*)&(*np)[NODE_FLAGS >> 2] >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          
          uint32_t msk = -(type < _NODE_TYPE_END_);
          //mprintf("#%02u: type is %u, end is %u, msk is %u\n", cpuId, type, _NODE_TYPE_END_, msk);
          type &= msk; //optional boundary check, if out of bounds, type will equal NODE_TYPE_UNKNOWN  
          DBPRINT1("#%02u: Node Ptr is 0x%08x, Dl: %s, type @ 0x%08x is %u\n", cpuId, *np, print64(*(uint64_t*)&tp[T_TD_DEADLINE >> 2], 0),  tmpType, type);

          *np = nodeFuncs[type](*np, tp);
          
          //now *np could be NULL, tread carefully!
          if (*np != NULL) {
            type = (*(uint32_t*)&(*np)[NODE_FLAGS >> 2] >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
            type &= -(type < _NODE_TYPE_END_); //optional boundary check, if out of bounds, type will equal NODE_TYPE_UNKNOWN  
          } else {
            type = NODE_TYPE_UNKNOWN;
          }
          //update thread deadline for next node
          deadlineFuncs[type](*np, tp);
          
        }

      }
    DBPRINT2("#%02u: Flow of Thr %u completed, going to idle\n",  cpuId, i);  
    } 
    //for (j = 0; j < ((125000000/2)); ++j) { asm("nop"); }
   }




}

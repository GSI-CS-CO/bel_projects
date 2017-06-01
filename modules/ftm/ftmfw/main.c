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
 
unsigned int cpuId, cpuQty, heapCap;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;
extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


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

void init()
{ 
   discoverPeriphery();
   uart_init_hw();
   cpuId = getCpuIdx();
   //ftmInit();

   if (cpuId == 0) {

     ebmInit();
     //prioQueueInit();
     mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId); 
   }
   
   isr_table_clr();
   irq_set_mask(0x01);
   irq_disable(); 
   
}


uint32_t* cmd(uint32_t* node) {

  const uint32_t prio = (node[CMD_ACT >> 2] >> ACT_PRIO_POS) & ACT_PRIO_MSK;
  const uint32_t *tg  = (uint32_t*)node[CMD_TARGET >> 2];
  uint32_t *bl, *b, *e;
  uint8_t *wrIdx;
  uint32_t bufOffs, elOffs;
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  
  //TODO check valid time


  //calculate pointer (8b) to current write index
  wrIdx = ((uint8_t *)tg + BLOCK_CMDQ_WR_IDXS + prio);
  //calculate Offsets
  bufOffs = (*wrIdx & Q_IDX_MAX) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;
  elOffs  = (*wrIdx & Q_IDX_MAX) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; 

  //get pointer to buf list
  bl = (uint32_t*)tg[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];

  //get pointer to buf
  b = (uint32_t*)bl[bufOffs >> 2];
  //get pointer to Element to write
  e = (uint32_t*)&b[elOffs >> 2];

  mprintf("#%02u: Prio: %u, wrIdx: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x\n", cpuId, prio, (uint32_t)wrIdx, (uint32_t)bl, (uint32_t)b, (uint32_t)e );

  //write Cmd
  for(uint32_t offs = T_CMD_TIME; offs < T_CMD_TIME + _T_CMD_SIZE_; offs += _32b_SIZE_ ) {
    e[offs >> 2] = node[(CMD_VALID_TIME + offs) >> 2];
  }
  
  //increase write index
  *wrIdx = (*wrIdx + 1) & Q_IDX_MAX_OVF;

  mprintf("#%02u: Sending Cmd 0x%08x, Target: 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], (uint32_t)tg, node[NODE_DEF_DEST_PTR >> 2]);
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* tmsg(uint32_t* node) {
  //TODO check valid time

  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  mprintf("#%02u: Sending Evt 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], node[NODE_DEF_DEST_PTR >> 2]);

  //TODO Send Evt

  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* block(uint32_t* node) {
  uint32_t* ret = (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  
  mprintf("#%02u: Checking Block 0x%08x \n", cpuId, node[NODE_HASH >> 2]);
  uint32_t ardOffs = node[BLOCK_CMDQ_RD_IDXS >> 2], awrOffs = node[BLOCK_CMDQ_WR_IDXS >> 2], bufOffs, elOffs, prio, atype;
  uint32_t *bl, *b, *e, *act;
  int16_t* qty;
  uint8_t *rdIdx;
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;

  if( awrOffs != ardOffs ) {
    prio = PRIO_LO;
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + PRIO_HI) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + PRIO_HI)) { prio = PRIO_HI; }
    if (*((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + PRIO_IL) ^ *((uint8_t *)node + BLOCK_CMDQ_WR_IDXS + PRIO_IL)) { prio = PRIO_IL; }

    rdIdx = ((uint8_t *)node + BLOCK_CMDQ_RD_IDXS + prio);
    bufOffs = (*rdIdx & Q_IDX_MAX) / (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _PTR_SIZE_;
    elOffs  = (*rdIdx & Q_IDX_MAX) % (_MEM_BLOCK_SIZE / _T_CMD_SIZE_  ) * _T_CMD_SIZE_; 
    bl = (uint32_t*)node[(BLOCK_CMDQ_PTRS + prio * _PTR_SIZE_) >> 2];
    b  = (uint32_t*)bl[bufOffs >> 2];
    e = (uint32_t*)&b[elOffs >> 2];
    //get action type
    uint32_t* act = (uint32_t*)&e[T_CMD_ACT >> 2];
    atype = (*act >> ACT_TYPE_POS) & ACT_TYPE_MSK;
    mprintf("#%02u: pending Cmd @ Prio: %u, rdIdx: 0x%08x, BufList: 0x%08x, Buf: 0x%08x, Element: 0x%08x, type: %u\n", cpuId, prio, (uint32_t)rdIdx, (uint32_t)bl, (uint32_t)b, (uint32_t)e, atype );
    
    
    //cmdExec[*atype](node);
    if (atype == ACT_TYPE_FLOW) { mprintf("#%02u: Redirecting to 0x%08x\n", cpuId, (uint32_t)ret); ret = (uint32_t*)e[T_CMD_FLOW_DEST >> 2]; node[NODE_DEF_DEST_PTR >> 2] = (uint32_t)ret;}
    
    //decrement qty
    int16_t qty = (*act >> ACT_QTY_POS) & ACT_QTY_MSK;
    mprintf("#%02u: Act 0x%08x, Qty is at 0x%04x\n", cpuId, *act, qty);
    --qty;
    *act = (qty & ACT_QTY_MSK) << ACT_QTY_POS;
    //if qty <= zero, pop -> increment read offset
    if(qty <= 0) { *(rdIdx) = (*rdIdx + 1) & Q_IDX_MAX_OVF_MSK; mprintf("#%02u: Qty reached zero, popping\n", cpuId);}
    
  } else {
    mprintf(" nothing pending\n");
    
  }  
  return ret;
  
}

typedef uint32_t* (*nodeFuncPtr)( uint32_t* );
nodeFuncPtr nodeFuncs[8];




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

   uint32_t   *lbtIdx, *lbtPtr, *tcGet;
   uint32_t   start;
    
   uint32_t** test;

  nodeFuncs[NODE_TYPE_UNKNOWN]  = ( NULL );
  nodeFuncs[NODE_TYPE_RAW]      = ( NULL );
  nodeFuncs[NODE_TYPE_TMSG]     = tmsg;
  nodeFuncs[NODE_TYPE_CNOOP]    = cmd;
  nodeFuncs[NODE_TYPE_CFLOW]    = cmd;
  nodeFuncs[NODE_TYPE_CFLUSH]   = cmd;
  nodeFuncs[NODE_TYPE_CWAIT]    = cmd;
  nodeFuncs[NODE_TYPE_BLOCK]    = block; 

   init();
   //uint32_t test = &pFtmIf->tPrep;
   
   
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
  
   uint32_t *p  = (uint32_t*)_startshared; 
   
   while (1) {


    uint32_t* start = p + (( SHCTL_THR_CTL + T_TC_START )>> 2);
    for(i=0;i<8;i++) {
      if (*start & 1<<i) {
        //mprintf("#%02u: Start Thr #%u\n", cpuId, i);
        //mprintf("#%02u: Looking at Node Ptr @ 0x%08x\n", cpuId, (uint32_t)(uint32_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TD_NODE_PTR )>> 2] );
        uint32_t* np = (uint32_t*)*(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TD_NODE_PTR )>> 2));
        
        
        
        while(np != NULL) {
          while (!(*start & 1<<i)) {for (j = 0; j < ((125000000/8)); ++j) { asm volatile ("nop"); } mprintf("#%02u: Waiting at 0x%08x\n", cpuId, np);};
          *start &= ~(1<<i);
          //mprintf("#%02u: Node Ptr is 0x%08x\n", cpuId, np);
          uint32_t flags = np[NODE_FLAGS >> 2];
          uint8_t  type = (flags >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          //mprintf("#%02u: flags @ 0x%08x are 0x%08x, Type is 0x%08x\n", cpuId, (uint32_t)&np[NODE_FLAGS >> 2], flags, type);
          np = nodeFuncs[type](np);


        }

      }
      
    } 
    for (j = 0; j < ((125000000/2)); ++j) { asm("nop"); }
   }




}

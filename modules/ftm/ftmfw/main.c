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



uint32_t* event(uint32_t* node) {
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  mprintf("#%02u: Sending Evt 0x%08x, next: 0x%08x\n", cpuId, node[NODE_HASH >> 2], node[NODE_DEF_DEST_PTR >> 2]);
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
}

uint32_t* block(uint32_t* node) {
  node[NODE_FLAGS >> 2] |= NFLG_PAINT_LM32_SMSK;
  mprintf("#%02u: Checking Block 0x%08x ...", cpuId, node[NODE_HASH >> 2]);
  uint32_t ardOffs = node[BLOCK_CMDQ_RD_IDXS >> 2], awrOffs = node[BLOCK_CMDQ_WR_IDXS >> 2], rdOffs, bufOffs, cmdOffs, idxPrio, ptrPrio;
  uint32_t *qbl, *qb, *prd;

  if( awrOffs != ardOffs ) {
    mprintf(" pending Cmd @");
    if (((awrOffs ^ ardOffs) >> (PRIO_LO * 8)) & 0xff) { idxPrio = PRIO_LO; ptrPrio = BLOCK_CMDQ_LO_PTR; }
    if (((awrOffs ^ ardOffs) >> (PRIO_HI * 8)) & 0xff) { idxPrio = PRIO_HI; ptrPrio = BLOCK_CMDQ_HI_PTR; }
    if (((awrOffs ^ ardOffs) >> (PRIO_IL * 8)) & 0xff) { idxPrio = PRIO_IL; ptrPrio = BLOCK_CMDQ_IL_PTR; }
    mprintf(" Prio %u\n", idxPrio);

    rdOffs  = (ardOffs >> (idxPrio * 8)) & 0xff;
    bufOffs =  rdOffs / (_MEM_BLOCK_SIZE / _T_CMD_SIZE  ) * _PTR_SIZE_;
    cmdOffs =  rdOffs % (_MEM_BLOCK_SIZE / _T_CMD_SIZE  ) * _T_CMD_SIZE;
    qbl = (uint32_t*)node[BLOCK_CMDQ_IL_PTR >> 2];
    qb  = (uint32_t*)qbl[bufOffs >> 2];
    prd = (uint32_t*)&qb[cmdOffs >> 2];
    
    mprintf(" Reading from 0x%08x -> 0x%08x\n", prd, *prd);

    node[BLOCK_CMDQ_RD_IDXS >> 2] = 
  } else {
    mprintf(" nothing pending\n");
    
  }  
  return (uint32_t*)node[NODE_DEF_DEST_PTR >> 2];
  
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
  nodeFuncs[NODE_TYPE_TMSG]     = event;
  nodeFuncs[NODE_TYPE_CNOOP]    = event;
  nodeFuncs[NODE_TYPE_CFLOW]    = event;
  nodeFuncs[NODE_TYPE_CFLUSH]   = event;
  nodeFuncs[NODE_TYPE_CWAIT]    = event;
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
        
        *start &= ~(1<<i);
        
        while(np != NULL) {
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

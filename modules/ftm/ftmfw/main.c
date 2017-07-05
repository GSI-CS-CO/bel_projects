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

uint8_t cpuId, cpuQty;



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
  *status = 0;
  *count  = 0;


  discoverPeriphery();
  uart_init_hw();     *status |= SHCTL_STATUS_UART_INIT_SMSK;

  cpuId = getCpuIdx();

  if (cpuId == 0) {
    ebmInit();        *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    prioQueueInit();  *status |= SHCTL_STATUS_PQ_INIT_SMSK;
    mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId); 
  }

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  dmInit(); *status |= SHCTL_STATUS_DM_INIT_SMSK;
   
}







void main(void) {
   
  int i,j;

  uint32_t* tp;
  uint32_t** np;
  uint32_t type;
  uint64_t now;
  uint64_t *currTime, *deadline;     
  uint32_t* tmpType;
  uint32_t msk;



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

    if (DL(*hp) - 500000ULL > getSysTime()) {
      //no rush. did the host request any threads to be started?
      //mprintf("#%02u: Got Time\n",  cpuId);
      for(i=0;i<8;i++) {
        for (j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
        mprintf("#%02u: b4 Start 0x%08x, Stop 0x%08x, Running 0x%08x, i %u\n",  cpuId, *start, *stop, *running, i);  
        if (*start & (1<<i)) {
          mprintf("#%02u: %u is starting\n",  cpuId, i);  
          

          uint64_t* startTime = (uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_STARTTIME ) >> 2));
          uint64_t* prepTime  = (uint64_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TD_PREPTIME  ) >> 2));
          uint64_t* currTime  = (uint64_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_CURRTIME  ) >> 2));
          uint64_t* deadline  = (uint64_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_DEADLINE  ) >> 2));
          
          uint32_t* origin    = (uint32_t*)(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_NODE_PTR) >> 2));
          uint32_t* cursor    = (uint32_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_NODE_PTR) >> 2));
          
          if (!(*startTime)) {*currTime = getSysTime() + 5000000ULL; }
          else *currTime = *startTime;
          *deadline = *currTime;

          *cursor = *origin;
        }
        *running |= *start & (1<<i);
        *start   &= ~(1 << i);  
      }

      
      continue;
    }



    for (j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
    //process node and update node ptr in threadData

    if (pN(hp) != NULL) {
    tmpType = pN(hp) + (NODE_FLAGS >> 2);
    type = (*tmpType >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
    
    
    msk = -(type < _NODE_TYPE_END_);
    type &= msk; //optional boundary check, if out of bounds, type will equal NODE_TYPE_UNKNOWN  
    }
    uint8_t thrIdx = *(uint32_t*)(pT(hp) + (T_TD_FLAGS >> 2)) & 0x7;  
    DBPRINT1("#%02u: ThrIdx %u, Node Ptr is 0x%08x, Dl: %s, type @ 0x%08x is %u\n", cpuId, thrIdx, pN(hp), print64(DL(pT(hp)), 0),  tmpType, type);

    *pncN(hp) = nodeFuncs[type](pN(hp), pT(hp));
    
    //now *np could be NULL, tread carefully!
    if (pN(hp) != NULL) {
      tmpType = pN(*hp) + (NODE_FLAGS >> 2);
      type = (*tmpType >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
      type &= -(type < _NODE_TYPE_END_); //optional boundary check, if out of bounds, type will equal NODE_TYPE_UNKNOWN  
    } else {
      type = NODE_TYPE_UNKNOWN;
    }
    //update thread deadline for next node
    deadlineFuncs[type](pN(hp), pT(hp));




    //*running   &= ~(*stop & (1<<thrIdx));
    DL(pT(hp)) |= (((uint64_t)*running >> thrIdx) & 1) -1;; // if not running, or with infinity
    //*stop      &= ~(1 << thrIdx);
    //replaceSort();
        


    //check if there is still time til heap top is due
      //check if a new thread was requested to start
        //while there are ...
          //prepare start time and thread ptr and add to heap
        //heapify
    //else
      //run heap top
      //update successor
      //check if not idle
        //update deadline
        //heap replace
      //else
        //heap remove
        //mark thread as stopped



    /*
    for(i=0;i<8;i++) {
      
      DBPRINT2("#%02u: b4 Start 0x%08x, Stop 0x%08x, Running 0x%08x\n",  cpuId, *start, *stop, *running);


      if (*start & 1<<i) {
        *running |= (1<<i);
        *start   &= ~(1<<i);


        tp        = (uint32_t*)(p + (( SHCTL_THR_DAT + i * _T_TD_SIZE_) >> 2));
        currTime  = (uint64_t*)&tp[T_TD_CURRTIME >> 2];
        deadline  = (uint64_t*)&tp[T_TD_DEADLINE >> 2];
        np        = (uint32_t**)&tp[T_TD_NODE_PTR >> 2];
        *np       = (uint32_t*)*(p + (( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_NODE_PTR ) >> 2));
        
        DBPRINT1("#%02u: ThrPtr: 0x%08x, Initial Node Ptr @ 0x%08x is 0x%08x\n",  cpuId, tp, np, *np);
        
        *currTime = getSysTime() + 5000000ULL;
        *deadline = *currTime;



        while(*np != NULL && (*running & 1<<i) ) {
          *running &= ~(*stop & 1<<i);
          *stop  &= ~(1<<i);  

          if ((*(uint64_t*)&tp[T_TD_DEADLINE >> 2] - 500000ULL) > getSysTime()) continue;

          //process node and update node ptr in threadData

          //*np is checked to be not null, so
          uint32_t* tmpType = (uint32_t*)&(*np)[NODE_FLAGS >> 2];
          type = (*(uint32_t*)&(*np)[NODE_FLAGS >> 2] >> NFLG_TYPE_POS) & NFLG_TYPE_MSK;
          
          uint32_t msk = -(type < _NODE_TYPE_END_);
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
          replaceSort();
        }


      }
    DBPRINT2("#%02u: Flow of Thr %u completed, going to idle\n",  cpuId, i);  
    } 
    //for (j = 0; j < ((125000000/2)); ++j) { asm("nop"); }
   */
  }




}

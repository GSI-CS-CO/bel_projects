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
   ebm_config_meta(1500, 42, EBM_NOREPLY );                                         //MTU, max EB msgs, flags
   ebm_config_if(DESTINATION, 0xffffffffffff, 0xffffffff,                0xebd0);   //Dst: EB broadcast
   ebm_config_if(SOURCE,      0xd15ea5edbeef, *(pEbCfg + (EBC_SRC_IP>>2)), 0xebd0); //Src: bogus mac (will be replaced by WR), WR IP

}


void init()
{
  *status = 0;
  *count  = 0;


  discoverPeriphery();
  cpuId = getCpuIdx();



  if (cpuId == 0) {
    //TODO replace bogus system status flags by real ones
    uart_init_hw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;
    ebmInit();        *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    prioQueueInit();  *status |= SHCTL_STATUS_PQ_INIT_SMSK;
    //mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId);
  } else {
    *status |= SHCTL_STATUS_UART_INIT_SMSK;
    *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    *status |= SHCTL_STATUS_PQ_INIT_SMSK;
  }

  int j;


  while(!wrTimeValid()) {
    for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
    if (cpuId == 0) mprintf("#%02u: DM cores Waiting for WRC synchronisation...\n", cpuId);
  }
  if (cpuId == 0) mprintf("#%02u: WR time now in sync\n", cpuId);

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  dmInit();
  *status  |= SHCTL_STATUS_DM_INIT_SMSK;
  *boottime = getSysTime();

}




void main(void) {


  int i,j;

  uint32_t* tp;
  uint32_t** np;
  uint32_t backlog = 0;


  init();

  //FIXME why is uart_hw_init here twice ???
  // wait 1s + cpuIdx * 1/10s
  for (j = 0; j < ((125000000/4)+(cpuId*2500000)); ++j) { asm("nop"); }
  if (cpuId != 0) uart_init_hw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;

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
  mprintf("#%02u: This is Doomsday DM FW %s \n", cpuId, DM_VERSION);

  atomic_off();

  if (getMsiBoxCpuSlot(cpuId, 0) == -1) {mprintf("#%02u: Mail box slot acquisition failed\n", cpuId);}

   while (1) {


    // Hard abort is an emergency and gets priority over everything else
    if (*abort1) {
      *running &= ~(*abort1);   // clear all aborted running bits
      for(i=0; i<_THR_QTY_; i++) {
        uint64_t* deadline  = (uint64_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_DEADLINE ) >> 2];
        *deadline |= (~((uint64_t)*abort1 >> i) & 1) -1;  // if abort bit was set, move deadline to infinity
      }
      heapify(); // re-sort all threads in schedulder (necessary because multiple threads may have been aborted
      *abort1 = 0; // clear abort bits
    }

    uint8_t thrIdx = *(uint32_t*)(pT(hp) + (T_TD_FLAGS >> 2)) & 0x7;
    if (DL(pT(hp))  <= getSysTime() + *(uint64_t*)(p + (( SHCTL_THR_STA + thrIdx * _T_TS_SIZE_ + T_TS_PREPTIME   ) >> 2) )) {
      backlog++;
      *pncN(hp)   = (uint32_t)nodeFuncs[getNodeType(pN(hp))](pN(hp), pT(hp));       //process node and return thread's next node
      DL(pT(hp))  = (uint64_t)deadlineFuncs[getNodeType(pN(hp))](pN(hp), pT(hp));   // return thread's next deadline (returns infinity on upcoming NULL ptr)
      *running   &= ~((DL(pT(hp)) == -1ULL) << thrIdx);                             // clear running bit if deadline is at infinity
      heapReplace(0);                                                               // call scheduler, re-sort only current thread

    } else {
      //nothing due right now. did the host request any new threads to be started?
      *bcklogmax   = ((backlog > *bcklogmax) ? backlog : *bcklogmax);
      backlog = 0;

      if(*start) {
        for(i=0;i<_THR_QTY_;i++) {
          if (*start & (1<<i)) {
            uint64_t* startTime = (uint64_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_STARTTIME) >> 2];
            uint64_t* prepTime  = (uint64_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_PREPTIME ) >> 2];
            uint64_t* currTime  = (uint64_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_CURRTIME ) >> 2];
            uint64_t* deadline  = (uint64_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_DEADLINE ) >> 2];
            uint32_t* origin    = (uint32_t*)&p[( SHCTL_THR_STA + i * _T_TS_SIZE_ + T_TS_NODE_PTR ) >> 2];
            uint32_t* cursor    = (uint32_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_NODE_PTR ) >> 2];
            uint32_t* msgcnt    = (uint32_t*)&p[( SHCTL_THR_DAT + i * _T_TD_SIZE_ + T_TD_MSG_CNT  ) >> 2];

            DBPRINT1("#%02u: ThrIdx %u, Preptime: %s\n", cpuId, i, print64(*prepTime, 0));

            if (!(*startTime)) {*currTime = getSysTime() + (*prepTime << 1); } // if 0, set to now + 2 * preptime
            else                *currTime = *startTime;
            *deadline = *currTime;

            *cursor   = *origin;          // Set cursor to origin node
            *running |= *start & (1<<i);  // copy this start bit to running bits
            *start   &= ~(1 << i);        // clear this start bit
            *msgcnt   = 0;                // clear msg counter
          }
        }

        heapify(); // re-sort all threads in schedulder (necessary because multiple threads may have been started)
      }
    }
  }
}

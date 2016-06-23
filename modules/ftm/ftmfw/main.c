#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "ftm.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
 
unsigned int cpuId, cpuQty, heapCap;
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0;


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
     for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
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
   cmdCnt = 0;
   cpuId = getCpuIdx();
   ftmInit();
   //mprintf("#%02u: Configured EBM and PQ\n", cpuId); 
   if (cpuId == 0) {

     ebmInit();
     prioQueueInit();
     mprintf("#%02u: Configured EBM and PQ\n", cpuId); 
   }
   
   isr_table_clr();
   irq_set_mask(0x00);
   irq_disable(); 
   
}




int insertFpqEntry()
{
   //test function for prio queue
   
   static unsigned int run = 0;
   int ret = 0;
   unsigned int diff;
   unsigned long long stime;
   
   const unsigned long long ct_trn = 200000 /8; // 200 us in 8ns steps
   const unsigned long long ct_sec = 1000000000 /8; // 1s

   const unsigned int c_period = 375000000/1;
      
   stime = getSysTime() + ct_sec + ct_trn - ((run++)); //+ (1 + ((run>>5)*5))*ct_sec ;
   
      atomic_on();
      *pFpqData = (unsigned int)(stime>>32);
      *pFpqData = (unsigned int)(stime);
       *pFpqData = 0xDEADBEEF;
       *pFpqData = 0xCAFEBABE;
       *pFpqData = 0x11111111;
       *pFpqData = 0x22222222;
       *pFpqData = 0x33333333;
       *pFpqData = run;
      atomic_off(); 
  
    
    return ret;
 }

void main(void) {
   
   int j;

   init();
   uint32_t test = &pFtmIf->tPrep;
   
   
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


   atomic_off();

  
   //mprintf("#%02u: Tprep @ 0x%08x\n", cpuId, test);
   //hexDump ("Plan 0 Chain 0 : \n", (void*)pFtmIf->pAct->plans[0], 128);
   /*
   t_time now, later;
   int64_t diff, diffSum;
   int64_t diffMin, diffMax, diffAvg;
   int64_t div = 1000000;
   diffMin =  0x7fffffff;
   diffMax =  0xffffffff;
   diffAvg =  0;
   */
   
   while (1) {
      cmdEval();
      processFtm();
      
      //mprintf("pAct 0x%08x, Qty 0x%08x, pStart 0x%08x, lans[0] 0x%08x,\n", pFtmIf->pAct, pFtmIf->pAct->planQty, pFtmIf->pAct->pStart, pFtmIf->pAct->plans[0]);
      
      //for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
      /*
      ebm_hi(0x0);
      atomic_on();
      ebm_op(0x0, 0x0BEEBABE, EBM_WRITE);
      ebm_op(0x0, 0x1BEEBABE, EBM_WRITE);
      atomic_off();
      ebm_flush();
      
      for (j = 0; j < div; ++j) {
         now   = getSysTime();
         later = getSysTime();
         diff  = (int64_t) later - (int64_t) now;
         //mprintf("Min: %d Max: %d\n", (int32_t)diffMin, (int32_t)diffMax);
         if(diffMin > diff) diffMin = diff;
         if(diffMax < diff) diffMax = diff;
         diffSum += diff;
      }
      mprintf("Min: %d Max: %d Avg: %d\n", (int32_t)diffMin, (int32_t)diffMax, ((int32_t)(diffSum / div)));
      diffSum = 0;
      */
   }

}

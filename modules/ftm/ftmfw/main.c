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
   cmdCnt = 0;
   cpuId = getCpuIdx();
   ftmInit();

   if (cpuId == 0) {

     ebmInit();
     prioQueueInit();
     mprintf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId); 
   }
   
   isr_table_clr();
   irq_set_mask(0x01);
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
   uint32_t*  lbtIdx;
   uint32_t   thrIdx, lbtBmp;



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
   

   atomic_off();
   if (getMsiBoxCpuSlot(cpuId, 0) == -1) {mprintf("#%02u: Mail box slot acquisition failed\n");}
  
   
   while (1) {
      cmdEval();
      
      thrIdx    = getNextThreadIdx();
      idx       = -1;
      tcGet     = (uint32_t*)&p[(SHCTL_THR_CTL + TC_GET) >>2];
      lbtBmp    = (uint32_t)&p[(SHCTL_LBTAB + LBT_BMP) >> 2];
      lbtIdx    = (uint32_t*)&p[(SHCTL_THR_DAT + thrIdx * _TDS_SIZE_ + TD_LBT_IDX) >>2];
      pCurrent  = (uint32_t**)&p[(SHCTL_THR_DAT + thrIdx * _TDS_SIZE_ + TD_LB_PTR) >>2];
      
     
     
      //run
      if ( (*tcGet >> thrIdx) & 1) {
          if (*pCurrent != NULL) idx = processChain(pCurrent);
          else                   idx = *lbtIdx; 

          if (idx != -1) {
            //successor block ?
            if((lbtBmp >> idx) & 1) {
              //if not -1, this calls a new block. Assign idx return value to lbtIdx of this thread
              *lbtIdx = idx;
              //Assign value of LB_PTR of table entry at *lbtIdx to referenced ptr  
              *pCurrent = (uint32_t*)p[(SHCTL_LBTAB + LBT_TAB + *lbtIdx * _LB_SIZE_ + LB_PTR)>>2];
            } else {
              //No ptr at this entry.   
              *pCurrent = NULL;
              //deactivate thread
              *tcGet = *tcGet & ~(1 << thrIdx);
            }
          }


      }

      
     
   }




}

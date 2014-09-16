#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "mini_sdb.h"
#include "irq.h"
#include "ftm.h"
#include "timer.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
 
unsigned int cpuId, cpuQty, heapCap;


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
   ebm_config_if(LOCAL,   "hw/08:00:30:e3:b0:5a/udp/192.168.191.254/port/60368");
   //ebm_config_if(REMOTE,  "hw/00:14:d1:fa:01:aa/udp/192.168.191.131/port/60368");
   //ebm_config_if(REMOTE,  "hw/00:26:7b:00:04:08/udp/192.168.191.72/port/60368");
   ebm_config_if(REMOTE,  "hw/ff:ff:ff:ff:ff:ff/udp/192.168.191.255/port/60368");
   ebm_config_meta(1500, 0x11, 255, 0x00000000 );
}

void init()
{ 
   char buffer[] = "\nFTM Core #00 rdy ________"; 
   char itoaBuffer[3];
   
   discoverPeriphery();
   uart_init_hw();
   ebmInit();
   ftmInit();
 
   cmdCnt = 0;
   cpuId = getCpuIdx();
   itoa(cpuId, itoaBuffer, 10);
   itoa((uint32_t)&pFtmIf->cmd, &buffer[18], 8);
   if(itoaBuffer[1]) {buffer[12] = itoaBuffer[1]; buffer[11] = itoaBuffer[0];}
   if(itoaBuffer[0]) {buffer[12] = itoaBuffer[0];}

   isr_table_clr();
   isr_ptr_table[0] = isr0; //timer
   isr_ptr_table[1] = isr1;   
   irq_set_mask(0x03);
   irq_enable();
   
   
   atomic_on(); 
   uart_write_string(buffer);
   atomic_off(); 
}



void showFpqStatus()
{
   mprintf("Fpq: Cfg %x HeapCnt %d MsgO %d MsgI %d\n", *(pFpqCtrl + r_FPQ.cfgGet), *(pFpqCtrl + r_FPQ.heapCnt), *(pFpqCtrl + r_FPQ.msgCntO), *(pFpqCtrl + r_FPQ.msgCntI));
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
      
   stime = getSysTime() + ct_sec + ct_trn - ((run++)<<3); //+ (1 + ((run>>5)*5))*ct_sec ;
   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
    {  
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
    } else {
       ret = -1;
       mprintf("Queue full, waiting\n");
    }   
 
    
    return ret;
 }

void main(void) {
   
   int j;

   init();
   for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
   mprintf("\n");

   while (1) {
      cmdEval();
      processFtm();
   }

}

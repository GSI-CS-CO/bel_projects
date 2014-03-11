#include <stdio.h>
#include <string.h>
#include "display.h"
#include "irq.h"
#include "ftm.h"
#include "mini_sdb.h"
#include "timer.h"
#include "ebm.h"
#include "aux.h"

#define WORLD_CON 0x80000000

volatile unsigned int* pSDB_base    = (unsigned int*)0x7FFFFA00;
volatile unsigned int* pEca         = (unsigned int*)(WORLD_CON + 0x7FFFFFF0);
volatile unsigned int* pCpuID ;
volatile unsigned int* pTimeSys;
volatile unsigned int* pIrqCtrl;
volatile unsigned int* pTimer;
volatile unsigned int* pTest;     
volatile unsigned int* pDisplay;
volatile unsigned int* pEbm;     
volatile unsigned int* pClusterInfo;
volatile unsigned int* pAtomic;        
volatile unsigned int* pFpqCtrl;
volatile unsigned int* pFpqData;

volatile char color; 
unsigned int cpuID, cpuMAX;
   char buffer[12];
volatile unsigned long long timestamp, timestamp_old;

const unsigned int c_period = 375000000/1;

void show_msi()
{
  char buffer[12];

  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.adr);
  disp_put_str("A ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, (unsigned long)global_msi.sel);
  disp_put_str("S ");
  disp_put_str(buffer);
  disp_put_c('\n');
}

void pause_and_show_msi() 
{
  

}




void isr0()
{
      
   
   char buffer[12];
   unsigned char tm_idx = global_msi.adr>>2 & 0xf;
   unsigned long long deadline = irq_tm_deadl_get(tm_idx);
   static unsigned int calls = 0;  
   static unsigned int msg_old;   
   static unsigned int deltamax;
   static unsigned int deltamin = -1;
   unsigned int delta;

   atomic_on();
   ebm_op(0x100000E0, global_msi.adr, WRITE);
   ebm_op(0x100000E4, (unsigned int)(*cpu_ID & 0xf)+1, WRITE); 
   ebm_op(0x100000E8, (unsigned int)(timestamp>>32), WRITE);
   ebm_op(0x100000EC, (unsigned int)timestamp, WRITE);
   ebm_op(0x100000F0, (((unsigned int)tm_idx)<<16) + (unsigned int)calls, WRITE);    
   ebm_flush(); 
   atomic_off();

   
}


void isr2()
{
 disp_put_str("ILCK\n");
  show_msi();

      
}

void isr3()
{
  unsigned int j;
  
  disp_put_str("OTH\n");
  show_msi();

   for (j = 0; j < 125000000; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }
  
}


const char mytext[] = "Hallo Welt!...\n\n";


void discovery()
{
   pCpuID         = (unsigned int*)find_device(CPU_INFO_ROM);
   pClusterInfo   = (unsigned int*)find_device(LM32_CLUSTER_INFO_ROM);
   pDisplay       = (unsigned int*)find_device(SCU_OLED_DISPLAY);  
   pEbm           = (unsigned int*)find_device(ETHERBONE_MASTER);
   pFpqCtrl       = (unsigned int*)find_device(FTM_PRIOQ_CTRL); 
   pFpqData       = (unsigned int*)find_device(FTM_PRIOQ_DATA);  
   pIrqCtrl       = (unsigned int*)find_device(IRQ_MSI_CTRL_IF);   
   pTimeSys       = (unsigned int*)find_device(SYSTEM_TIME);
   pTimer         = (unsigned int*)find_device(IRQ_TIMER_CTRL_IF);
   pAtomic        = (unsigned int*)find_device(ATOMIC_BUS_ACCESS);
}

void ebmInit()
{
   ebm_config_if(LOCAL,   "hw/08:00:30:e3:b0:5a/udp/192.168.191.254/port/60368");
   ebm_config_if(REMOTE,  "hw/00:14:d1:fa:01:aa/udp/192.168.191.131/port/60368");
   ebm_config_meta(80, 0x11, 16, 0x00000000 );
}

void prioQueueInit()
{
   *(pFpqCtrl + r_FPQ.clear)  = 1;
   *(pFpqCtrl + r_FPQ.dstAdr) = (unsigned int)pEca;
   *(pFpqCtrl + r_FPQ.ebmAdr) = (unsigned int)pEbm;
   *(pFpqCtrl + r_FPQ.msgMax) = 5;
   *(pFpqCtrl + r_FPQ.tTrnHi) = 0;
   *(pFpqCtrl + r_FPQ.tTrnLo) = 0;
   *(pFpqCtrl + r_FPQ.tDueHi) = 0;
   *(pFpqCtrl + r_FPQ.tDueLo) = 0;
   *(pFpqCtrl + r_FPQ.cfgSet) = //r_FPQ.cfg_AUTOFLUSH_TIME | 
                                  r_FPQ.cfg_AUTOFLUSH_MSGS |
                                  //r_FPQ.cfg_AUTOPOP | 
                                  r_FPQ.cfg_FIFO | 
                                  r_FPQ.cfg_ENA;
}


void init()
{

   
   discovery();
   ebmInit(); 
   prioQueueInit();
   
   isr_table_clr();
   isr_ptr_table[0]= ISR_timer; //timer
   isr_ptr_table[1]= 0; //lm32
   isr_ptr_table[2]= isr2; //ilck
   isr_ptr_table[3]= isr3; //other    
   irq_set_mask(0x0f);
   irq_enable();
   cpuID = irq_get_mask();
   disp_reset();	
   disp_put_c('\f'); 
}

void main(void) {


   int j;

   init();
  
 


disp_put_c('\f');

  disp_put_str("FTM ready\n");
  
  

   for (j = 0; j < (125000000/160)*(cpuID<<3); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }

  while (1) {
      
  }

}

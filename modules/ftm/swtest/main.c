#include <stdio.h>
#include <string.h>
#include "mini_sdb.h"
#include "display.h"
#include "irq.h"
#include "ftm.h"
#include "timer.h"
#include "ebm.h"
#include "aux.h"

char buffer[12];
volatile char color; 
unsigned int cpuID, cpuMAX, heapCap;
   char buffer[12];
volatile unsigned long long timestamp, timestamp_old;

const unsigned long long ct_trn = 200000 /8; // 200 us in 8ns steps
const unsigned long long ct_sec = 1000000000 /8; // 1s

const unsigned int c_period = 375000000/1;

void show_msi()
{


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
      
   
   
   unsigned char tm_idx = global_msi.adr>>2 & 0xf;
   unsigned long long deadline = irq_tm_deadl_get(tm_idx);
   static unsigned int calls = 0;  
   static unsigned int msg_old;   
   static unsigned int deltamax;
   static unsigned int deltamin = -1;
   unsigned int delta;

   atomic_on();
   ebm_op(0x100000E0, global_msi.adr, WRITE);
   ebm_op(0x100000E4, (unsigned int)(*pCpuId & 0xf)+1, WRITE); 
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

void ebmInit()
{
   ebm_config_if(LOCAL,   "hw/08:00:30:e3:b0:5a/udp/192.168.191.254/port/60368");
   ebm_config_if(REMOTE,  "hw/00:14:d1:fa:01:aa/udp/192.168.191.131/port/60368");
   ebm_config_meta(80, 0x11, 16, 0x00000000 );
}








void init()
{

   
   
   discoverPeriphery();
   
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
   mprintf("Init EBM...");
   ebmInit(); 
   mprintf("done.\n");
   mprintf("Init FTM...");
   ftmInit();
   mprintf("done.\n");
   //prioQueueInit(5000, 10000);
   
/*   isr_table_clr();
   isr_ptr_table[0]= ISR_timer; //timer
   isr_ptr_table[1]= 0; //lm32
   isr_ptr_table[2]= isr2; //ilck
   isr_ptr_table[3]= isr3; //other    
   irq_set_mask(0x0f);
   irq_enable();
*/
   disp_reset();	
   disp_put_c('\f'); 
}

int insertFpqEntry()
{
   static unsigned int run = 0;
   int ret = 0;
   unsigned int diff;
   unsigned long long stime;
   
   stime = getSysTime() + ct_sec + ct_trn - ((run++)<<3); //+ (1 + ((run>>5)*5))*ct_sec ;
   //mprintf("etime: %8x%8x\n", (unsigned int)(stime>>32), (unsigned int)(stime)); 
   diff = ( heapCap - *(pFpqCtrl + r_FPQ.heapCnt));
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

void showFpqStatus()
{
   mprintf("Fpq: Cfg %x HeapCnt %d MsgO %d MsgI %d\n", *(pFpqCtrl + r_FPQ.cfgGet), *(pFpqCtrl + r_FPQ.heapCnt), *(pFpqCtrl + r_FPQ.msgCntO), *(pFpqCtrl + r_FPQ.msgCntI));
}

void main(void) {


   int j;

   init();
  

disp_put_c('\f');

  disp_put_str("FTM ready\n");
  
  

   for (j = 0; j < (125000000/160)*(cpuID<<3); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }
   
   
   
   disp_put_str(mat_sprinthex(buffer, (unsigned int)pCpuId)); disp_put_c('\n');   
   disp_put_str(mat_sprinthex(buffer, (unsigned int)pCluInfo)); disp_put_c('\n');
   disp_put_str(mat_sprinthex(buffer, (unsigned int)pUart)); disp_put_c('\n');
   mprintf("Hello World!\n");
   mprintf("PrioQC 0x%8x\n", pFpqCtrl);
   mprintf("PrioQD 0x%8x\n", pFpqData);
   mprintf("EBM 0x%8x\n", pEbm);
   mprintf("ECA 0x%8x\n", pEca);
   mprintf("Time: 0x%8x%8x\n", *pCpuSysTime, *(pCpuSysTime+1));
   mprintf("%8x\n%8x\n", (unsigned int)(ct_sec>>32), (unsigned int)(ct_sec));
    showFpqStatus();
   for (j = 0; j < 64; ++j) insertFpqEntry();
   mprintf("Fpq: capacity %d heapcnt %d\n", *(pFpqCtrl + r_FPQ.capacity), *(pFpqCtrl + r_FPQ.heapCnt) );
   for (j = 0; j < 63; ++j) insertFpqEntry();
   mprintf("Fpq: capacity %d heapcnt %d\n", *(pFpqCtrl + r_FPQ.capacity), *(pFpqCtrl + r_FPQ.heapCnt) );
   showFpqStatus();
   
   for (j = 0; j < 3500000; ++j) {asm("# noop");}
  
   
   while (1) {
      /*
      *(pFpqCtrl + r_FPQ.force) = r_FPQ.force_POP;
      *(pFpqCtrl + r_FPQ.force) = r_FPQ.force_POP;
      for (j = 0; j < 31500000; ++j) {asm("# noop");}
      showFpqStatus();
      */
  }

}

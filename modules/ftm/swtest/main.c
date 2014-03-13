#include <stdio.h>
#include <string.h>
#include "mini_sdb.h"
#include "display.h"
#include "irq.h"
//#include "ftm.h"
#include "timer.h"
#include "ebm.h"
#include "aux.h"

char buffer[12];
volatile char color; 
unsigned int cpuID, cpuMAX;
   char buffer[12];
volatile unsigned long long timestamp, timestamp_old;

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




// Priority Queue RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int force;
   unsigned int dbgSet;
   unsigned int dbgGet;
   unsigned int clear;
   unsigned int cfgGet;
   unsigned int cfgSet;
   unsigned int cfgClr;
   unsigned int dstAdr;
   unsigned int heapCnt;
   unsigned int msgCntO;
   unsigned int msgCntI;
   unsigned int tTrnHi;
   unsigned int tTrnLo;
   unsigned int tDueHi;
   unsigned int tDueLo;
   unsigned int msgMin;
   unsigned int msgMax;
   unsigned int ebmAdr;
   unsigned int cfg_ENA;
   unsigned int cfg_FIFO;    
   unsigned int cfg_IRQ;
   unsigned int cfg_AUTOPOP;
   unsigned int cfg_AUTOFLUSH_TIME;
   unsigned int cfg_AUTOFLUSH_MSGS;
   unsigned int force_POP;
   unsigned int force_FLUSH;
} r_FPQ = {    .rst     =  0x00 >> 2,
               .force   =  0x04 >> 2,
               .dbgSet  =  0x08 >> 2,
               .dbgGet  =  0x0c >> 2,
               .clear   =  0x10 >> 2,
               .cfgGet  =  0x14 >> 2,
               .cfgSet  =  0x18 >> 2,
               .cfgClr  =  0x1C >> 2,
               .dstAdr  =  0x20 >> 2,
               .heapCnt =  0x24 >> 2,
               .msgCntO =  0x28 >> 2,
               .msgCntI =  0x2C >> 2,
               .tTrnHi  =  0x30 >> 2,
               .tTrnLo  =  0x34 >> 2,
               .tDueHi  =  0x38 >> 2,
               .tDueLo  =  0x3C >> 2,
               .msgMin  =  0x40 >> 2,
               .msgMax  =  0x44 >> 2,
               .ebmAdr  =  0x48 >> 2,
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1
};

void prioQueueInit()
{
   *(pFpqCtrl + r_FPQ.clear)  = 1;
   *(pFpqCtrl + r_FPQ.dstAdr) = (unsigned int)pEca;
   *(pFpqCtrl + r_FPQ.ebmAdr) = (unsigned int)pEbm;
   *(pFpqCtrl + r_FPQ.msgMax) = 2;
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

   
   discoverPeriphery();
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
   ebmInit(); 
   prioQueueInit();
   
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

void insertFpqEntry()
{
   static unsigned int run = 0;
  
   atomic_on();   
   *pFpqData = 0;
   *pFpqData = run++;
   *pFpqData = 0xDEADBEEF;
   *pFpqData = 0xCAFEBABE;
   *pFpqData = 0x11111111;
   *pFpqData = 0x11111111;
   *pFpqData = 0x11111111;
   *pFpqData = 20 - run;
   atomic_off();  

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
    showFpqStatus();
   /*
   while(1){ 
   insertFpqEntry();
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   */
   /*
   insertFpqEntry();
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   insertFpqEntry();
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   
   *(pFpqCtrl + r_FPQ.force) = r_FPQ.force_POP;
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   
   *(pFpqCtrl + r_FPQ.force) = r_FPQ.force_POP;
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   
   *(pFpqCtrl + r_FPQ.force) = r_FPQ.force_POP;
   for (j = 0; j < 31500000; ++j) {asm("# noop");}
   showFpqStatus();
   */
   }
   
  while (1) {
      
  }

}

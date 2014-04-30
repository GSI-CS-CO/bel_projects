#include "ftm.h"
#include "irq.h"
#include "aux.h"
#include "timer.h"
#include "display.h"
#include "ebm.h"

const t_time tProc = { .v64 = 125000};
volatile t_fesaFtmIf* pFesaFtmIf = (t_fesaFtmIf*)_startshared;

char buffer[10];

t_ftmMsg* addFtmMsg(unsigned int eca_adr, t_ftmMsg* pMsg)
{
   atomic_on();   
   pFPQ = pMsg->id.v32.hi;
   pFPQ = pMsg->id.v32.lo;
   pFPQ = pMsg->par.v32.hi;
   pFPQ = pMsg->par.v32.lo;
   pFPQ = pMsg->res;
   pFPQ = pMsg->tef;
   pFPQ = pMsg->ts.v32.hi;
   pFPQ = pMsg->ts.v32.lo;
   atomic_off();   
   return pMsg;
}



void fesaInit()
{ 
   unsigned int ps, cs, ms;
   cs = 1;
   ps = 0;
   ms = 0;
   
   pFesaFtmIf->pageSel = ps;
   pFesaFtmIf->page[ps].cycleSel = cs;
   pFesaFtmIf->page[ps].cycles[cs].tTrn.v64         = 5000;
   pFesaFtmIf->page[ps].cycles[cs].tMargin.v64      = 1000;
   pFesaFtmIf->page[ps].cycles[cs].tStart.v64       = getSysTime() + 125000000; 
   pFesaFtmIf->page[ps].cycles[cs].tPeriod.v64      = 5*5000000;
   pFesaFtmIf->page[ps].cycles[cs].rep              = 250;
   pFesaFtmIf->page[ps].cycles[cs].repCnt           = 0;
   pFesaFtmIf->page[ps].cycles[cs].qtyMsgs          = 1;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].id.v64   = 0x100000000000ABCD;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].par.v64  = 0x2000000000000123;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].res      = 0;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].tef      = 0xDEADBEEF;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].ts.v64   = 0;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].offs.v64 = 5000000;
     
}


void ISR_timer()
{
  
   disp_put_c(progressWheel());
   disp_put_c('A');
   updateAllExecTimes();
   disp_put_c('B');
   if(1) {
      //updateTimers();
   }
   
   if(global_msi.msg & ((TIMER_CYC_START) | (TIMER_CYC_PREP)))
   {
      if(pPageAct->cycles[pPageAct->cycleSel].repCnt < pPageAct->cycles[pPageAct->cycleSel].rep) 
      {
      msgProcPending = true;
      pPageAct->cycles[pPageAct->cycleSel].repCnt++;
      }
  /* 
      if ((pPageAct->cycles[pPageAct->cycleSel].status & CYC_ACTIVE)
      && (pPageAct->cycles[pPageAct->cycleSel].repCnt <= pPageAct->cycles[pPageAct->cycleSel].rep))
        {
     
        
   } else {
   */
   }
        
}

void ISR_Cmd()
{

}

void processDueMsgs()
{
   t_ftmCycle* cyc = pPageAct->cycles + pPageAct->cycleSel;
   unsigned char dueIdx, nextIdx;
   unsigned char  dispatch = 0;
   t_time tDue;
   
   dueIdx = cyc->procMsg;
   
   if( ((cyc->rep == -1) || (cyc->rep > cyc->repCnt)) && (cyc->status & CYC_ACTIVE) && msgProcPending )
   {
      tDue = cyc->msgs[cyc->procMsg].ts;
      if(getSysTime() >= tDue.v64 - (cyc->tMargin.v64 + cyc->tTrn.v64) ) // 
      disp_put_c('X');
      for(dueIdx = cyc->procMsg; dueIdx <  cyc->qtyMsgs; dueIdx++)
      {
         disp_put_c('A' + dueIdx);
         if ( (tDue.v64 + tProc.v64 > cyc->msgs[dueIdx].ts.v64) // diff between msgs less than time to process...
         ||   (cyc->msgs[dueIdx].ts.v64 + tProc.v64 >= cyc->tExec.v64 + cyc->tPeriod.v64) ) // or diff to cycle end less than time to process? 
         {
            disp_put_c('0' + dueIdx);
            //dispatch msg
            addFtmMsg( 0x0, cyc->msgs+dueIdx); 
            dispatch = true;
            tDue = cyc->msgs[dueIdx].ts;
         } else {
         
            if(dispatch) {sendFtmMsgPacket(); msgProcPending = false; disp_put_str("Sent!\n");}
            
            nextIdx = (dueIdx == cyc->qtyMsgs-1) ? 0 : dueIdx+1;
           
            cyc->procMsg = nextIdx;
         } 
      }
   } 
}


inline void updateCycExecTime(t_ftmCycle* c)
{
   disp_put_c('Z');
   unsigned int i;
   if((c->rep == -1) || (c->rep > c->repCnt)) c->tExec.v64 = c->tStart.v64 + c->repCnt * c->tPeriod.v64;
   for(i=0; i < c->qtyMsgs; i++) c->msgs[i].ts.v64 = c->tExec.v64 + c->msgs[i].offs.v64;
}


inline void updatePageExecTimes(t_fesaPage* pPage)
{
   disp_put_c('Y');
   updateCycExecTime(pPage->cycles + 0);
   updateCycExecTime(pPage->cycles + 1);
}

inline void updateAllExecTimes()
{
   disp_put_c('X');
   updatePageExecTimes(pPageInAct);
   updatePageExecTimes(pPageAct);
}

unsigned int setCycleTimer(t_ftmCycle* cyc, unsigned int mode)
{
   t_time   tPrep, tExec;
   s_timer  tm0, tm1;
   unsigned long long factor;
      
   //timer0
   //calculate due time for start
   tExec = cyc->tStart;
   tPrep.v64 = cyc->tMargin.v64 + cyc->tTrn.v64;
   
   if (!mode && (getSysTime() + tPrep.v64 > tExec.v64))
   {
    return TIMER_CFG_ERROR_0;
   }
   tm0.mode      = TIMER_1TIME;
   tm0.src       = (mode) ? TIMER_ABS_TIME : TIMER_REL_TIME; //absolute or relative value
   tm0.cascade   = TIMER_NO_CASCADE;
   tm0.deadline  = (mode == NOW) ? 10 : tExec.v64 - tPrep.v64;
   tm0.msi_dst   = 0;
   tm0.msi_msg   = TIMER_CYC_PREP;
   /*
   // if the cycle duration is shorter than processing time, pack multiple cycles in one packet
   if(cyc->tPeriod.v64 > tProc.v64) { tPeriod.v64 = cyc->tPeriod.v64; factor = 1;}
   else  {  factor = tProc.v64/cyc->tPeriod.v64 + ((tProc.v64 % cyc->tPeriod.v64) ? 1 : 0);
            tPeriod.v64 = factor * cyc->tPeriod.v64;     
   }
   */                        
   //timer1
   //cascade to Timer1, periodic with cycle period
   tm1.mode      = TIMER_PERIODIC;
   tm1.src       = TIMER_REL_TIME;
   tm1.cascade   = TIMER_0;
   tm1.deadline  = cyc->tPeriod.v64;
   tm1.msi_dst   = 0;
   tm1.msi_msg   = TIMER_CYC_START;
 
   atomic_on();
   irq_tm_write_config(0, &tm0);
   irq_tm_write_config(1, &tm1);
   irq_tm_set_arm(1<<TIMER_1 | 1<<TIMER_0);
   atomic_off();
   return TIMER_CFG_SUCCESS;
}


.rst     =  0x00 >> 2,
               .dbgSet  =  0x04 >> 2,
               .dbgGet  =  0x08 >> 2,
               .clear   =  0x0C >> 2,
               .cfgGet  =  0x10 >> 2,
               .cfgSet  =  0x14 >> 2,
               .cfgClr  =  0x18 >> 2,
               .dstAdr  =  0x1C >> 2,
               .heapCnt =  0x20 >> 2,
               .msgCntO =  0x24 >> 2,
               .msgCntI =  0x28 >> 2,
               .tTrnHi  =  0x2C >> 2,
               .tTrnLo  =  0x30 >> 2,
               .tDueHi  =  0x34 >> 2,
               .tDueLo  =  0x38 >> 2,
               .msgMin  =  0x3C >> 2,
               .msgMax  =  0x40 >> 2,
               .ebmAdr  =  0x44 >> 2,
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1

void prioQueueInit()
{
   *(pFPQctrl + r_FPQ.clear)       = 1;
   *(pFPQctrl + r_FPQ.dstAdr)      = pEcaAdr;
   *(pFPQctrl + r_FPQ.ebmAdr)      = pEbmAdr;
   *(pFPQctrl + r_FPQ.msgMax)      = 32;
   *(pFPQctrl + r_FPQ.tTrnHi)      = pFesaFtmIf->tTrn.v32.hi;
   *(pFPQctrl + r_FPQ.tTrnLo)      = pFesaFtmIf->tTrn.v32.lo;
   *(pFPQctrl + r_FPQ.tMarginHi)   = pFesaFtmIf->tMargin.v32.hi;
   *(pFPQctrl + r_FPQ.tMarginLo)   = pFesaFtmIf->tMargin.v32.lo;
   *(pFPQctrl + r_FPQ.cfgSet)      = r_FPQ.cfg_AUTOFLUSH_TIME | 
                                     r_FPQ.cfg_AUTOFLUSH_MSGS |
                                     r_FPQ.cfg_AUTOPOP | 
                                     r_FPQ.cfg_FIFO;
}

*(pFPQctrl + r_FPQ.cfgSet)      = r_FPQ.cfg_EN
*(pFPQctrl + r_FPQ.cfgClr)      = r_FPQ.cfg_ENA;


void ftmInit()
{
  irq_tm_clr_arm(0xffffffff);
  fesaInit();
  prioQueueInit();
  pPageAct    = pFesaFtmIf->page + (pFesaFtmIf->pageSel & 1);
  pPageInAct  = pFesaFtmIf->page + (~pFesaFtmIf->pageSel & 1);   
}

void fesaCmdEval()
{
   unsigned int cmd, ftmRunning;
   cmd         = pFesaFtmIf->cmd;
   
   
   if(cmd)
   {
      if(cmd & CMD_RST) ftmInit();
      
      if(cmd & (CMD_PAGESWAP | CMD_PAGESWAP_I))
      {
         ftmRunning = (pFesaFtmIf->status & FTM_RUNNING);
         swap |= ( (pPageAct->cycles[pPageAct->cycleSel].status   & CYC_ACTIVE) && !(cmd & CMD_PAGESWAP_I)   && ftmRunning) <<0;
         swap |= ( (pPageInAct->cycles[pPageAct->cycleSel].status & CYC_ACTIVE)                              && ftmRunning) <<1;
         
         
         //update page pointers
         pPageAct    = pFesaFtmIf->page + (pFesaFtmIf->pageSel & 1);
         pPageInAct  = pFesaFtmIf->page + (~pFesaFtmIf->pageSel & 1);   
      }//if pageswap
      
      if(cmd & CMD_RUN) setCycleTimer(pPageAct->cycles + pPageAct->cycleSel, NORMAL) ? disp_put_str("\n+CfgErr") : disp_put_str("\n+CfgOK");
      
      if(cmd & CMD_RUN_NOW) 
      {
         msgProcPending = false; 
         pPageAct->cycles[pPageAct->cycleSel].status |= CYC_ACTIVE;
         setCycleTimer(pPageAct->cycles + pPageAct->cycleSel, NOW) ? disp_put_str("\n!CfgErr") : disp_put_str("\n!CfgOK");
         
         
      }
      
     
      
      if(cmd & CMD_DBG_0) 
      {
         disp_put_c(0x0c);
         disp_put_str("State: \n");
         disp_put_str(sprinthex(buffer, pFesaFtmIf->status, 8));
         disp_put_str("\nPage: ");
         disp_put_str(sprinthex(buffer, pFesaFtmIf->pageSel, 2));   
         disp_put_str("\nCyc:  ");
         disp_put_str(sprinthex(buffer, pPageAct->cycleSel, 2));
       
         disp_put_str("\nPer: ");
         disp_put_str(sprinthex(buffer, (unsigned int)irq_tm_deadl_get(TIMER_1), 8));
         disp_put_str("\nStat: ");
         disp_put_str(sprinthex(buffer, irq_tm_get_arm(), 4));
         
      }
      
      if(cmd & CMD_DBG_1)
      {
         irq_tm_clr_arm(0x2);
         irq_tm_timer_sel(1);
         irq_tm_deadl_set(125000000);
         irq_tm_set_arm(0x2);
      }     
      pFesaFtmIf->cmd = 0;      
   }
}





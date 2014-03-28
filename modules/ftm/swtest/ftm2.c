#include <stdio.h>
#include <string.h>

#include "ftm.h"
#include "irq.h"
#include "aux.h"
#include "timer.h"


unsigned short getIdFID(unsigned long long id)     return ((id >> (64 - ID_FID_POS))   & (ID_MSK_B16 >> (16 - ID_FID_LEN)));
unsigned short getIdGID(unsigned long longd id)    return ((id >> (64 - ID_GID_POS))   & (ID_MSK_B16 >> (16 - ID_GID_LEN)));
unsigned short getIdEVTNO(unsigned long long id)   return ((id >> (64 - ID_EVTNO_POS)) & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN)));
unsigned short getIdSID(unsigned long long id)     return ((id >> (64 - ID_SID_POS))   & (ID_MSK_B16 >> (16 - ID_SID_LEN)));
unsigned short getIdBPID(unsigned long long id)    return ((id >> (64 - ID_BPID_POS))  & (ID_MSK_B16 >> (16 - ID_BPID_LEN)));
unsigned short getIdSCTR(unsigned long long id)    return ((id >> (64 - ID_SCTR_POS))  & (ID_MSK_B16 >> (16 - ID_SCTR_LEN)));


char buffer[10];

void fesaInit()
{ 
   unsigned int ps, cs, ms;
   cs = 1;
   ps = 0;
   ms = 0;
   
   pFesaFtmIf->pageSel = ps;
   pFesaFtmIf->page[ps].cycleSel = cs;
   pFesaFtmIf->page[ps].cycles[cs].tTrn.v64           = 5000;
   pFesaFtmIf->page[ps].cycles[cs].tMargin.v64        = 1000;
   pFesaFtmIf->page[ps].cycles[cs].tStart.v64         = getSysTime() + 125000000; 
   pFesaFtmIf->page[ps].cycles[cs].tPeriod.v64        = 5*5000000;
   pFesaFtmIf->page[ps].cycles[cs].rep                = 250;
   pFesaFtmIf->page[ps].cycles[cs].repCnt             = 0;
   pFesaFtmIf->page[ps].cycles[cs].qtyMsgs            = 1;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].id.v64    = 0x100000000000ABCD;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].par.v64   = 0x2000000000000123;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].res       = 0;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].tef       = 0xDEADBEEF;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].ts.v64    = 0;
   pFesaFtmIf->page[ps].cycles[cs].msgs[ms].offs.v64  = 5000000;
     
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
   }
}


void prioQueueInit()
{
   *(pFPQctrl + r_FPQ.clear)       = 1;
   *(pFPQctrl + r_FPQ.dstAdr)      = pEcaAdr;
   *(pFPQctrl + r_FPQ.ebmAdr)      = pEbmAdr;
   *(pFPQctrl + r_FPQ.msgQty)      = 32;
   *(pFPQctrl + r_FPQ.tTrnHi)      = pFesaFtmIf->tTrn.v32.hi;
   *(pFPQctrl + r_FPQ.tTrnLo)      = pFesaFtmIf->tTrn.v32.lo;
   *(pFPQctrl + r_FPQ.tMarginHi)   = pFesaFtmIf->tMargin.v32.hi;
   *(pFPQctrl + r_FPQ.tMarginLo)   = pFesaFtmIf->tMargin.v32.lo;
   *(pFPQctrl + r_FPQ.cfgSet)      = r_FPQ.cfg_AUTOFLUSH_TIME | 
                                     r_FPQ.cfg_AUTOFLUSH_MSGS |
                                     r_FPQ.cfg_AUTOPOP | 
                                     r_FPQ.cfg_FIFO |
                                     r_FPQ.cfg_ENA;
}

void ftmInit()
{
   volatile  pFtmIf = (t_FtmIf*)_startshared;
   ftmRunning = 0;
   ftmStopReq = 0;
   pAct->pAlt = &Idle;  
   
   fesaInit();
   prioQueueInit();
}

void cmdEval()
{
   unsigned int cmd, ftmRunning;
   
   cmd = pFesaFtmIf->cmd;
   pFesaFtmIf->cmd = 0; 
   
   if(cmd)
   {
      if(cmd & CMD_RST)          { ftmInit();}
      if(cmd & CMD_START)        { pAct->pAlt = pAct->pStart; ftmRunning = 1; }
      if(cmd & CMD_IDLE)         { pAct->pAlt = &Idle;}
      if(cmd & CMD_STOP_REQ)     { if(ftmRunning) ftmStopReq = 1;}
      if(cmd & CMD_STOP_NOW)     { ftmRunning = 0;}
      
      if(cmd & CMD_COMMIT_PAGE)  {pTmp = pIna; pIna = pAct; pAct = pTmp; pAct->pAlt = pAct->pStart;}
      if(cmd & CMD_COMMIT_BP)    {pAct->pAlt = *pBP;}
      
      if(cmd & CMD_DBG_0)        {mprintf("DBG0\n");}
      if(cmd & CMD_DBG_1)        {mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  if(!ftmRunning) showPage(pAct);}
      if(cmd & CMD_SHOW_INA)     {  if(!ftmRunning) showPage(pIna);}
      
      //if(cmd & CMD_SHOW_PLANS)   {  if(!ftmRunning) showPage(pIna);}
   }
}

pPlans

void showPage(t_ftmPage* pPage)
{
   pageIdx = (pPage - pBase);
   mprintf("Active Page:   %x, %u\n", pPage, pageIdx);
   mprintf("Plans:         %x\n", pPage->planQty);
   pageAltIdx  = (t_ftmPage)(pPage->pAlt) - pBase;
   cycAltIdx   = pPage->pAlt - (t_ftmCycle)pageAltIdx;
   
   mprintf("Alt Plan:      %x Page %u \n", pPage->pAlt, pageAltIdx, cycAltIdx);
   mprintf("Cur Plan, Idx: %x %u\n", pCur, (pCur - pPage->pPlans) );
}


int dispatch(t_ftmMsg* pMsg)
{
   int ret = 0;
   unsigned int diff;
   
   diff = ( heapCap - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      atomic_on();
      pFpqData = pMsg->id.v32.hi;
      pFpqData = pMsg->id.v32.lo;
      pFpqData = pMsg->par.v32.hi;
      pFpqData = pMsg->par.v32.lo;
      pFpqData = pMsg->tag
      pFpqData = pMsg->tef;
      pFpqData = pMsg->ts.v32.hi;
      pFpqData = pMsg->ts.v32.lo;
      atomic_off(); 
   } else {
      ret = -1;
      mprintf("Queue full, waiting\n");
   }   
   
   return ret;
}


t_Cyc* processCycle(t_Cyc* this)
{
   t_Cyc* pCur = this; 
   
   //  or not conditional and cycle repetitions != 0 -> execute
   
   if( (!(this->flags & FLAGS_IS_COND) && (this->repQty != 0)) //not conditional and cycle repetitions != 0 ? or ...
   || (this->flags & FLAGS_IS_COND) && ((*pCondInput & condMsk) == (*pCondInput & condMsk)) ) //conditional and condition met?
         pCur = processCycleAux(this); //execute
   else if((this->flags & FLAGS_IS_BP) && pAlt != NULL)  pCur = pAct->pAlt; // else check for breakpoint so we get around inf wait
      
   return pCur;    
}

t_Cyc* processCycleAux(t_Cyc* this)
{
   t_Cyc* pCur = this; 
   unsigned long long tMsgExec;
   
   //get execution time for due msg
   tMsgExec    = this->tStart + (this->pMsg + this->msgIdx)->tOffset); 
    
   //time to hand it over to prio queue ?
   if( getSysTime() + tPrep >= tMsgExec)  
   {
      //update 
      (this->pMsg + this->msgIdx)->ts = tMsgExec;
      if( dispatch( (t_ftmMsg*)(this->pMsg + this->msgIdx) ) //enough space in prio queue
      {  
         if(this->msgIdx < this->msgQty) this->msgIdx++;     //msgs left to process?
         else
         {
            this->msgIdx = 0;
            //is this a breakpoint? if so and break is desired, continue at the address in the page alternate plan
            if((this->flags & FLAGS_IS_BP) && pAct->pAlt != NULL)       pCur = pAct->pAlt;  //BP? go to alt cycle
            else
            { 
               if( this->repCnt < this->repQty )   {this->repCnt++;     pCur = this;}        //repetions left? stay with this cycle
               else                                {this->repCnt = 0;   pCur = this->pNext;} //done, go to next cycle
            }   
            
            //propagate updateted start time to next cycle to be executed
            pCur->Start = this->tStart + this->tPeriod;
         }
      }
   }
   
   return pCur;    
}











































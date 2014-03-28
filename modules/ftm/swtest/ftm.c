#include <stdio.h>
#include <string.h>

#include "mini_sdb.h"
#include "ftm.h"
#include "irq.h"
#include "aux.h"
#include "timer.h"


unsigned short getIdFID(unsigned long long id)     {return ((unsigned short)(id >> ID_FID_POS)) & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
unsigned short getIdGID(unsigned long long id)     {return ((unsigned short)(id >> ID_GID_POS))    & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
unsigned short getIdEVTNO(unsigned long long id)   {return ((unsigned short)(id >> ID_EVTNO_POS))  & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
unsigned short getIdSID(unsigned long long id)     {return ((unsigned short)(id >> ID_SID_POS))    & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
unsigned short getIdBPID(unsigned long long id)    {return ((unsigned short)(id >> ID_BPID_POS))   & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
unsigned short getIdSCTR(unsigned long long id)    {return ((unsigned short)(id >> ID_SCTR_POS))   & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}



const t_ftmCycle Idle = { .tTrn           = 0,
                          .tPrep          = 0,
                          .tStart         = 0,
                          .tPeriod        = 500,
                          .tExec          = 0,
                          .flags          = (FLAGS_IS_BP | FLAGS_IS_COND),
                          .pCondInput     = NULL,
                          .pCondPattern   = NULL,
                          .condMsk        = 0,
                          .repQty         = -1,
                          .repCnt         = 0,
                          .msgQty         = 0,
                          .msgIdx         = 0,
                          .pMsg           = NULL,
                          .pNext          = NULL,
                          .planID         = 0x12340000,
                          .planID         = 0x10000000
                          };
/*
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
*/



void prioQueueInit(unsigned long long trn, unsigned long long due)
{
   *(pFpqCtrl + r_FPQ.clear)  = 1;
   *(pFpqCtrl + r_FPQ.dstAdr) = 0x7FFFFFF0;
   *(pFpqCtrl + r_FPQ.ebmAdr) = (unsigned int)pEbm;
   *(pFpqCtrl + r_FPQ.msgMax) = 32;
   *(pFpqCtrl + r_FPQ.tTrnHi) = (unsigned int)(trn>>32);
   *(pFpqCtrl + r_FPQ.tTrnLo) = (unsigned int)trn;
   *(pFpqCtrl + r_FPQ.tDueHi) = (unsigned int)(due>>32);
   *(pFpqCtrl + r_FPQ.tDueLo) = (unsigned int)due;
   *(pFpqCtrl + r_FPQ.cfgSet) =   r_FPQ.cfg_AUTOFLUSH_TIME | 
                                  r_FPQ.cfg_AUTOFLUSH_MSGS |
                                  r_FPQ.cfg_AUTOPOP | 
                                  r_FPQ.cfg_FIFO | 
                                  r_FPQ.cfg_ENA;
}

void ftmInit()
{
   pFtmIf = (t_FtmIf*)_startshared;
   pFtmIf->cmd          = 0;
   pFtmIf->status       = 0;
   pFtmIf->pAct->pAlt   = (t_ftmCycle*)(&Idle);  
   
   //fesaInit();
   prioQueueInit(5000, 10000);
}

void cmdEval()
{
   unsigned int cmd;
   t_ftmPage* pTmp;
   
   
   cmd = pFtmIf->cmd;
   pFtmIf->cmd = 0; 
   
   if(cmd)
   {
      if(cmd & CMD_RST)          { ftmInit();}
      if(cmd & CMD_START)        { pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart; pFtmIf->status |= FTM_IS_RUNNING; }
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pAlt = (t_ftmCycle*)(&Idle);}
      if(cmd & CMD_STOP_REQ)     { if(pFtmIf->status & FTM_IS_RUNNING) pFtmIf->status |= FTM_IS_STOP_REQ;}
      if(cmd & CMD_STOP_NOW)     { pFtmIf->status &= ~FTM_IS_RUNNING;}
      
      if(cmd & CMD_COMMIT_PAGE)  {pTmp = (t_ftmPage*)pFtmIf->pIna; pFtmIf->pIna = pFtmIf->pAct; pFtmIf->pAct = pTmp; pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart;}
      if(cmd & CMD_COMMIT_ALT)    {pFtmIf->pAct->pAlt = pFtmIf->pBP;}
      
      if(cmd & CMD_DBG_0)        {mprintf("DBG0\n");}
      if(cmd & CMD_DBG_1)        {mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  if(!(pFtmIf->status & FTM_IS_RUNNING)) showPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  if(!(pFtmIf->status & FTM_IS_RUNNING)) showPage(pFtmIf->pIna);}
      
//      if(cmd & CMD_SHOW_PLANS)   {  if(!pFtmIf->status & FTM_IS_RUNNING) showPage(pFtmIf->pIna);}
   }
}


void showPage(t_ftmPage* pPage)
{
   /*
   pageIdx = (pPage - pBase);
   mprintf("Active Page:   %x, %u\n", pPage, pageIdx);
   mprintf("Plans:         %x\n", pPage->planQty);
   pageAltIdx  = (t_ftmPage)(pPage->pAlt) - pBase;
   cycAltIdx   = pPage->pAlt - (t_ftmCycle)pageAltIdx;
   
   mprintf("Alt Plan:      %x Page %u \n", pPage->pAlt, pageAltIdx, cycAltIdx);
   mprintf("Cur Plan, Idx: %x %u\n", pCur, (pCur - pPage->pPlans) );
   */
}



int dispatch(t_ftmMsg* pMsg)
{
   int ret = 0;
   unsigned int diff;
   
   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      atomic_on();
      *pFpqData = pMsg->id.v32.hi;
      *pFpqData = pMsg->id.v32.lo;
      *pFpqData = pMsg->par.v32.hi;
      *pFpqData = pMsg->par.v32.lo;
      *pFpqData = pMsg->tef;
      *pFpqData = pMsg->res;
      *pFpqData = pMsg->ts.v32.hi;
      *pFpqData = pMsg->ts.v32.lo;
      atomic_off(); 
   } else {
      ret = -1;
      mprintf("Queue full, waiting\n");
   }   
   
   return ret;
}


t_ftmCycle* processCycle(t_ftmCycle* this)
{
   t_ftmCycle* pCur = this; 
   
   //  or not conditional and cycle repetitions != 0 -> execute
   
   if( (!(this->flags & FLAGS_IS_COND) && (this->repQty != 0)) //not conditional and cycle repetitions != 0 ? or ...
   || (this->flags & FLAGS_IS_COND) && ((*this->pCondInput & this->condMsk) == (*this->pCondInput & this->condMsk)) ) //conditional and condition met?
         pCur = processCycleAux(this); //execute
   else
   {
      if((this->flags & FLAGS_IS_BP) && pFtmIf->pAct->pAlt != NULL)  
      {
         pCur = pFtmIf->pAct->pAlt; // else check for breakpoint so we get around inf wait
         pCur->tStart.v64 = getSysTime() + this->tPeriod.v64;
      }
   }
      
   return pCur;    
}

t_ftmCycle* processCycleAux(t_ftmCycle* this)
{
   t_ftmCycle* pCur = this; 
   unsigned long long tMsgExec;
   t_ftmMsg* pCurMsg;
   
   //get execution time for due msg
   pCurMsg     = (t_ftmMsg*)(this->pMsg + this->msgIdx);
   tMsgExec    = this->tStart.v64 + pCurMsg->offs.v64; 
    
   //time to hand it over to prio queue ?
   if( getSysTime() + pFtmIf->tPrep >= tMsgExec)  
   {
      //update 
      pCurMsg->ts.v64 = tMsgExec;
      if( dispatch(pCurMsg) ) //enough space in prio queue
      {  
         if(this->msgIdx < this->msgQty) this->msgIdx++;     //msgs left to process?
         else
         {
            this->msgIdx = 0;
            //is this a breakpoint? if so and break is desired, continue at the address in the page alternate plan
            if((this->flags & FLAGS_IS_BP) && pFtmIf->pAct->pAlt != NULL)       pCur = pFtmIf->pAct->pAlt;  //BP? go to alt cycle
            else
            { 
               if( this->repCnt < this->repQty )   {this->repCnt++;     pCur = this;}        //repetions left? stay with this cycle
               else                                {this->repCnt = 0;   pCur = (t_ftmCycle*)this->pNext;} //done, go to next cycle
            }   
            
            //propagate updateted start time to next cycle to be executed
            if(this->flags & FLAGS_IS_COND)  pCur->tStart.v64 = getSysTime() + this->tPeriod.v64;  
            else                             pCur->tStart.v64 = this->tStart.v64 + this->tPeriod.v64; 
         }
      }
   }
   
   return pCur;    
}











































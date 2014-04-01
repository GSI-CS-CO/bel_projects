#include <stdio.h>
#include <string.h>


#include "mini_sdb.h"
#include "ftm.h"
#include "irq.h"
#include "aux.h"
#include "timer.h"


uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}



const t_ftmCycle Idle = { .tTrn           = 0,
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
                          .pNext          = NULL
                          };


uint8_t* serCycle(uint8_t*    buf, 
                  uint64_t    tTrn,
                  uint64_t    tStart,
                  uint64_t    tPeriod,
                  uint64_t    tExec,       
                  uint32_t    flags,
                  uint32_t*   pCondInput,    
                  uint32_t*   pCondPattern,  
                  uint32_t    condMsk,       
                  uint32_t    repQty,        
                  uint32_t    repCnt,
                  uint32_t    msgQty,
                  uint32_t    msgIdx,
                  t_ftmMsg*   pMsg,
                  t_ftmCycle* pNext)
{
   uint8_t i;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TTRN         + i]  = (tTrn           >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TSTART       + i]  = (tStart         >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TPERIOD      + i]  = (tPeriod        >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_CYC_TEXEC        + i]  = (tExec          >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_FLAGS        + i]  = (flags          >> (8*i)) & 0xff;
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PCONDINDPUT  + i]  = ((uint32_t)pCondInput     >> (8*i)) & 0xff;
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PCONDPATTERN + i]  = ((uint32_t)pCondPattern   >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_CONDMSK      + i]  = (condMsk        >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_REPQTY       + i]  = (repQty         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_REPCNT       + i]  = (repCnt         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_MSGQTY       + i]  = (msgQty         >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_CYC_MSGIDX       + i]  = (msgIdx         >> (8*i)) & 0xff;
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PMSG         + i]  = ((uint32_t)pMsg           >> (8*i)) & 0xff;
   for(i=0;i<FTM_PTR_SIZE;    i++) buf[FTM_CYC_PNEXT        + i]  = ((uint32_t)pNext          >> (8*i)) & 0xff;
   
   return (buf + FTM_CYC_END_);   

}

t_ftmCycle* deserCycle(  uint8_t*    buf, t_ftmCycle* cyc)
{
   uint8_t i;
   uint32_t tmp;
   
   cyc->tTrn.v64        = 0;         
   cyc->tStart.v64      = 0;          
   cyc->tPeriod.v64     = 0;        
   cyc->tExec.v64       = 0;         
   cyc->flags           = 0;  
   cyc->condMsk         = 0;       
   cyc->repQty          = 0;      
   cyc->repCnt          = 0;     
   cyc->msgQty          = 0;      
   cyc->msgIdx          = 0;  
   
   for(i=FTM_TIME_SIZE-1; i>=0;  i--) cyc->tTrn.v64      |= buf[FTM_CYC_TTRN           + i] << (8*i); 
   for(i=FTM_TIME_SIZE-1; i>=0;  i--) cyc->tStart.v64    |= buf[FTM_CYC_TSTART         + i] << (8*i);  
   for(i=FTM_TIME_SIZE-1; i>=0;  i--) cyc->tPeriod.v64   |= buf[FTM_CYC_TPERIOD        + i] << (8*i); 
   for(i=FTM_TIME_SIZE-1; i>=0;  i--) cyc->tExec.v64     |= buf[FTM_CYC_TEXEC          + i] << (8*i);
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->flags         |= buf[FTM_CYC_FLAGS          + i] << (8*i);
   tmp = 0; for(i=FTM_PTR_SIZE-1;  i>=0;  i--) tmp       |= buf[FTM_CYC_PCONDINDPUT    + i] << (8*i); cyc->pCondInput   = (uint32_t*)tmp;
   tmp = 0; for(i=FTM_PTR_SIZE-1;  i>=0;  i--) tmp       |= buf[FTM_CYC_PCONDPATTERN   + i] << (8*i); cyc->pCondPattern = (uint32_t*)tmp;
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->condMsk       |= buf[FTM_CYC_CONDMSK        + i] << (8*i);
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->repQty        |= buf[FTM_CYC_REPQTY         + i] << (8*i);
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->repCnt        |= buf[FTM_CYC_REPCNT         + i] << (8*i);
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->msgQty        |= buf[FTM_CYC_MSGQTY         + i] << (8*i);
   for(i=FTM_WORD_SIZE-1; i>=0;  i--) cyc->msgIdx        |= buf[FTM_CYC_MSGIDX         + i] << (8*i);
   tmp = 0; for(i=FTM_PTR_SIZE-1;  i>=0;  i--) tmp       |= buf[FTM_CYC_PMSG           + i] << (8*i); cyc->pMsg  = (t_ftmMsg*)tmp; 
   tmp = 0; for(i=FTM_PTR_SIZE-1;  i>=0;  i--)tmp        |= buf[FTM_CYC_PNEXT          + i] << (8*i); cyc->pNext = (struct t_ftmCycle*)tmp;
 
   return cyc;   

}

uint8_t* serMsg(  uint8_t* buf, 
                  uint64_t id,
                  uint64_t par,
                  uint32_t tef,
                  uint32_t res,
                  uint64_t ts,
                  uint64_t offs
                  )
{
   uint8_t i;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_MSG_ID     + i]  = (id    >> (8*i)) & 0xff;
   for(i=0;i<FTM_DWORD_SIZE;  i++) buf[FTM_MSG_PAR    + i]  = (par   >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_MSG_TEF    + i]  = (tef   >> (8*i)) & 0xff;
   for(i=0;i<FTM_WORD_SIZE;   i++) buf[FTM_MSG_RES    + i]  = (res   >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_MSG_TS     + i]  = (ts    >> (8*i)) & 0xff;
   for(i=0;i<FTM_TIME_SIZE;   i++) buf[FTM_MSG_OFFS   + i]  = (offs  >> (8*i)) & 0xff;
   
   return buf + FTM_MSG_END_;
}

t_ftmMsg* deserMsg(  uint8_t*  buf, 
                     t_ftmMsg* msg)
{
   uint8_t i;
   msg->id.v64    = 0;
   msg->par.v64   = 0;
   msg->tef       = 0;
   msg->res       = 0;
   msg->ts.v64    = 0;
   msg->offs.v64  = 0;
   
   for(i=FTM_DWORD_SIZE-1; i>=0; i--)  msg->id.v64    |= buf[FTM_MSG_ID    + i] << (8*i);
   for(i=FTM_DWORD_SIZE-1; i>=0; i--)  msg->par.v64   |= buf[FTM_MSG_PAR   + i] << (8*i);
   for(i=FTM_WORD_SIZE-1;  i>=0; i--)  msg->tef       |= buf[FTM_MSG_TEF   + i] << (8*i);
   for(i=FTM_WORD_SIZE-1;  i>=0; i--)  msg->res       |= buf[FTM_MSG_RES   + i] << (8*i);
   for(i=FTM_TIME_SIZE-1;  i>=0; i--)  msg->ts.v64    |= buf[FTM_MSG_TS    + i] << (8*i);
   for(i=FTM_TIME_SIZE-1;  i>=0; i--)  msg->offs.v64  |= buf[FTM_MSG_OFFS  + i] << (8*i);
   
   return msg;
}


void demoInit()
{ 
   unsigned int ps, cs, ms;
   cs = 1;
   ps = 0;
   ms = 0;
   /*
   pFtmIf->pIna->
  
   
   
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
*/     
}

void prioQueueInit(uint64_t trn, uint64_t due)
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
   pFtmIf->pAct         = (t_ftmPage*)&pFtmIf->pPages[0];
   pFtmIf->pIna         = (t_ftmPage*)&pFtmIf->pPages[1];
   pFtmIf->pAct->pAlt   = (t_ftmCycle*)(&Idle);
   pFtmIf->pIna->pAlt   = (t_ftmCycle*)(&Idle);    
   pFtmIf->tPrep        = 10000;
   prioQueueInit(5000, 10000);
}

void cmdEval()
{
   unsigned int   cmd;
   t_ftmPage*     pTmp;
   
   cmd = pFtmIf->cmd;
   pFtmIf->cmd = 0; 
   
   if(cmd)
   {
      if(cmd & CMD_RST)          { ftmInit();}
      if(cmd & CMD_START)        { pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart; pFtmIf->status |= FTM_IS_RUNNING; }
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pAlt = (t_ftmCycle*)(&Idle); }
      if(cmd & CMD_STOP_REQ)     { if(pFtmIf->status & FTM_IS_RUNNING) pFtmIf->status |= FTM_IS_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { pFtmIf->status &= ~FTM_IS_RUNNING; }
      
      if(cmd & CMD_COMMIT_PAGE)  { pTmp = (t_ftmPage*)pFtmIf->pIna; pFtmIf->pIna = pFtmIf->pAct; pFtmIf->pAct = pTmp; pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart; }
      if(cmd & CMD_COMMIT_ALT)   { pFtmIf->pAct->pAlt = pFtmIf->pBP; }
      
      if(cmd & CMD_DBG_0)        { mprintf("DBG0\n"); }
      if(cmd & CMD_DBG_1)        { mprintf("DBG1\n"); }
      
      if(cmd & CMD_SHOW_ACT)     { if(!(pFtmIf->status & FTM_IS_RUNNING)) showPage(pFtmIf->pAct); }
      if(cmd & CMD_SHOW_INA)     { if(!(pFtmIf->status & FTM_IS_RUNNING)) showPage(pFtmIf->pIna); }
      
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
   uint64_t tMsgExec;
   t_ftmMsg* pCurMsg;
   
   //get execution time for due msg
   pCurMsg     = (t_ftmMsg*)(this->pMsg + this->msgIdx);
   tMsgExec    = this->tStart.v64 + pCurMsg->offs.v64; 
    
   //time to hand it over to prio queue ?
   if( getSysTime() + pFtmIf->tPrep >= tMsgExec)  
   {
      //update 
      pCurMsg->ts.v64 = tMsgExec;
      if( dispatch(pCurMsg) ) //enough space in prio queue?
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











































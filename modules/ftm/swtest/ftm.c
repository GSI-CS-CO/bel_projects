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
   pFtmIf = (t_FtmIf*)_startshared; 
   pFtmIf->status = STAT_STOPPED;
   pFtmIf->pAct = &pPages[0];
   pFtmIf->pIna = &pPages[1];
   pFtmIf->Idle = { .tStart.v64     = 0,
                    .tPeriod.v64    = 5000,
                    .tExec.v64      = 0,
                    .flags          = (FLAGS_IS_BP),
                    .condVal        = 0,
                    .condMsk        = 0,
                    .sigDst         = 0,
                    .sigVal         = 0,
                    .repQty         = -1,
                    .repCnt         = 0,
                    .msgQty         = 0,
                    .msgIdx         = 0,
                    .pMsg           = NULL,
                    .pNext          = NULL
                    };
   
   prioQueueInit();
   
}

void cmdEval()
{
   unsigned int cmd, ftmRunning;
   
   cmd = pFtmIf->cmd;
   stat = pFtmIf->status;
   pFtmIf->cmd = 0; 
   
   if(cmd)
   {
      if(cmd & CMD_RST)          { ftmInit();}
      if(cmd & CMD_START)        { pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart; stat = (stat & STAT_ERROR) | STAT_RUNNING;}
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pAlt = &pFtmIf->Idle;}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ;
      if(cmd & CMD_STOP_NOW)     { stat = (stat & STAT_ERROR) | STAT_STOPPED;}
      
      if(cmd & CMD_COMMIT_PAGE)  {pTmp = pFtmIf->pIna; pFtmIf->pIna = pFtmIf->pAct; pFtmIf->pAct = pTmp; pFtmIf->pAct->pAlt = pFtmIf->pAct->pStart;}
      if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pAlt = pFtmIf->pBP;}
      
      if(cmd & CMD_DBG_0)        {mprintf("DBG0\n");}
      if(cmd & CMD_DBG_1)        {mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  if(!ftmRunning) showPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  if(!ftmRunning) showPage(pFtmIf->pIna);}
      
                            
   }
   
   if(pCur == &pFtmIf->idle)  {stat |=  STAT_IDLE;}
   else                       {stat &= ~STAT_IDLE;}
   if(pCur == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) | STAT_STOPPED; }
   
   pFtmIf->status = stat; 
}

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

uint8_t condValid(t_Cyc* cyc)
{
   uint8_t ret = 0;
   
   
   if(this->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED) )
   {
      if(this->flags & FLAGS_IS_COND_MSI)
      {
         uint32_t  ip, msg;
         irq_disable();
         asm ("rcsr %0, ip": "=r"(ip)); //get pending irq flags
         if(ip & 1<<MSISIG)
         {
            irq_pop_msi(MSISIG);      //pop msg from msi queue into global_msi variable
            msg = global_msi.msg;
            irq_clear(1<<MSISIG);     //clear pending bit
         }      
         irq_enable();
         if(cyc->condVal & cyc->condMsk) == (msg & cyc->condMsk & 0xFFFFFFFF) ret = 1;   
      }
      else
      {if((cyc->condVal & cyc->condMsk) == *(pFtmIf->pAct->pSharedMem)) {ret = 1;} }
   }
   else ret = 1;
   
   return ret; 
} 

t_Cyc* processCycle(t_Cyc* this)
{
   t_Cyc* pCur = this; 
   
   if(condValid(this)) pCur = processCycleAux(this);
   else 
   {
      if((this->flags & FLAGS_IS_BP) && pFtmIf->pAlt != NULL)
      { pCur = pFtmIf->pAct->pAlt; pFtmIf->pAct->pAlt = NULL;} // else check for breakpoint so we get around inf wait
   }   
   return pCur;    
}

t_Cyc* processCycleAux(t_Cyc* this)
{
   t_Cyc* pCur = this; 
   unsigned long long tMsgExec;
   
   //get execution time for due msg
   tMsgExec    = this->tStart + (this->pMsg + this->msgIdx)->tOffset); 
    
   if(this->msgIdx < this->msgQty) //msgs left to process?
   {
      //time to hand it over to prio queue ?
      if( getSysTime() + tPrep >= tMsgExec)  
      {
         (this->pMsg + this->msgIdx)->ts = tMsgExec; //calc execution timestamp
         if( dispatch( (t_ftmMsg*)(this->pMsg + this->msgIdx) ) this->msgIdx++;  //enough space in prio queue? dispatch and inc msgidx
      }
   } 
   else
   {
      this->msgIdx = 0;
      //is this a breakpoint? if so and break is desired, continue at the address in the page alternate plan
      if((this->flags & FLAGS_IS_BP) && pAct->pAlt != NULL)       
      { 
         pCur = pAct->pAlt;  //BP? go to alt cycle
         pAct->pAlt = NULL;
      }
      else
      { 
         if( this->repCnt < this->repQty )   {this->repCnt++;     pCur = this;}        //repetions left? stay with this cycle
         else                                {this->repCnt = 0;   pCur = this->pNext;} //done, go to next cycle
      }   
      
      //propagate updated start time to next cycle to be executed
      pCur->Start = this->tStart + this->tPeriod;
   }
     
   return pCur;    
}

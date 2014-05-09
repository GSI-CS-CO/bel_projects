#include "ftm.h"

uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}

uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

void prioQueueInit()
{
   
   *(pFpqCtrl + r_FPQ.clear)     = 1;
   *(pFpqCtrl + r_FPQ.dstAdr)    = (uint32_t)pEca;
   *(pFpqCtrl + r_FPQ.ebmAdr)    = (uint32_t)pEbm;
   *(pFpqCtrl + r_FPQ.msgMax)    = 32;
   *(pFpqCtrl + r_FPQ.tTrnHi)    = hiW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tTrnLo)    = loW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tDueHi)    = hiW(pFtmIf->tDue);
   *(pFpqCtrl + r_FPQ.tDueLo)    = loW(pFtmIf->tDue);
   *(pFpqCtrl + r_FPQ.cfgSet)    = r_FPQ.cfg_AUTOFLUSH_TIME | 
                                   r_FPQ.cfg_AUTOFLUSH_MSGS |
                                   r_FPQ.cfg_AUTOPOP | 
                                   r_FPQ.cfg_FIFO |
                                   r_FPQ.cfg_ENA;
}

void ftmInit()
{
   pFtmIf = (t_ftmIf*)_startshared; 
   pFtmIf->status = 0x0;
   pFtmIf->pAct = (t_ftmPage*)&(pFtmIf->pPages[0]);
   pFtmIf->pIna = (t_ftmPage*)&(pFtmIf->pPages[1]);
   pFtmIf->idle = (t_ftmChain){ .tStart     = 0,
                                .tPeriod    = 5000,
                                .tExec      = 0,
                                .flags      = (FLAGS_IS_BP),
                                .condVal    = 0,
                                .condMsk    = 0,
                                .sigDst     = 0,
                                .sigVal     = 0,
                                .repQty     = -1,
                                .repCnt     = 0,
                                .msgQty     = 0,
                                .msgIdx     = 0,
                                .pMsg       = NULL,
                                .pNext      = NULL
                                };
   pFtmIf->sema.sig  = 1;
   pFtmIf->sema.cond = 1;
   pCurrentChain = (t_ftmChain*)&pFtmIf->idle;;
   
   prioQueueInit();
   
}

void cmdEval()
{
   uint32_t cmd, stat;
   t_ftmPage* pTmp;
   
   cmd = pFtmIf->cmd;
   stat = pFtmIf->status;
   pFtmIf->cmd = 0; 
   
   if(cmd)
   {
      if(cmd & CMD_RST)          { ftmInit();}
      if(cmd & CMD_START)        { pFtmIf->pAct->pBp = pFtmIf->pAct->pStart; stat = (stat & STAT_ERROR) | STAT_RUNNING;}
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pBp = (t_ftmChain*)&pFtmIf->idle;}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { stat = (stat & STAT_ERROR) & ~STAT_RUNNING;}
      
      if(cmd & CMD_COMMIT_PAGE)  {pTmp = pFtmIf->pIna; pFtmIf->pIna = pFtmIf->pAct; pFtmIf->pAct = pTmp; pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;}
      if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pBp = pFtmIf->pNewBp;}
      
      if(cmd & CMD_DBG_0)        {mprintf("DBG0\n");}
      if(cmd & CMD_DBG_1)        {mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  DBPRINT("SHOW ACT\n"); showFtmPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  DBPRINT("SHOW INA\n"); showFtmPage(pFtmIf->pIna);}
      
                            
   }
   
   if(pCurrentChain == &pFtmIf->idle)  {stat |=  STAT_IDLE;}
   else                       {stat &= ~STAT_IDLE;}
   if(pCurrentChain == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) & ~STAT_RUNNING;}
   
   pFtmIf->status = stat; 
}

void processFtm()
{
   if (pFtmIf->status & STAT_RUNNING) pCurrentChain = processChain(pCurrentChain); 
}

void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   mprintf("---PAGE %08x\n", pPage);
   mprintf("StartPlan:\t");
   
   if(pPage->pStart == &(pFtmIf->idle) ) mprintf("idle\n");
   else { 
          if(pPage->pStart == NULL) mprintf("NULL\n");
          else mprintf("%08x\n", pPage->pStart);
        } 
   
   mprintf("AltPlan:\t");
   if(pPage->pBp == &(pFtmIf->idle) ) mprintf("idle\n");
   else { 
          if(pPage->pBp == NULL) mprintf("NULL\n");
          else mprintf("%08x\n", pPage->pBp);
        }  
   mprintf("PlanQty:\t%u\t%08x\n", pPage->planQty, &(pPage->planQty));
    
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      mprintf("\t---PLAN %c\n", planIdx+'A');
      chainIdx = 0;
      pChain = pPage->plans[planIdx];
      while(pChain != NULL)
      {
         mprintf("\t\t---CHAIN %c%u\n", planIdx+'A', chainIdx-1);
         mprintf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\n\t\tmsg:\t\t\t%08x\n", 
         (uint32_t)(pChain->tStart>>32), (uint32_t)pChain->tStart, 
         (uint32_t)(pChain->tPeriod>>32), (uint32_t)pChain->tPeriod,
         pChain->repQty,
         pChain->msgQty);
         
         mprintf("\t\tFlags:\t");
         if(pChain->flags & FLAGS_IS_BP) mprintf("-IS_BP\t");
         if(pChain->flags & FLAGS_IS_COND_MSI) mprintf("-IS_CMSI\t");
         if(pChain->flags & FLAGS_IS_COND_SHARED) mprintf("-IS_CSHA\t");
         if(pChain->flags & FLAGS_IS_SIG_SHARED) mprintf("-IS_SIG_SHARED");
         if(pChain->flags & FLAGS_IS_SIG_MSI)    mprintf("-IS_SIG_MSI");
         if(pChain->flags & FLAGS_IS_END) mprintf("-IS_END");
         mprintf("\n");
         
         mprintf("\t\tCondVal:\t%08x\n\t\tCondMsk:\t%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            mprintf("\t\t\t---MSG %c%u%c\n", planIdx+'A', chainIdx-1, msgIdx+'A');
            mprintf("\t\t\tid:\t%08x%08x\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id, 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         if(pChain->flags & FLAGS_IS_END) pChain = NULL;
         else pChain = (t_ftmChain*)pChain->pNext;
      }
           
   }
   uint64_t j;
  for (j = 0; j < (250000000); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }    
   
}

void showStatus()
{
   uint32_t stat = pFtmIf->status;
   mprintf("\f%08x\tStatus:\t", (uint32_t)(&(pFtmIf->cmd)) );
   if(stat & STAT_RUNNING) mprintf("\t\t-RUNNING"); else mprintf("\t\t-\t");
   if(stat & STAT_IDLE) mprintf("\t\t-IDLE"); else mprintf("\t\t-\n");
   if(stat & STAT_STOP_REQ) mprintf("\t\t-STOP_REQ"); else mprintf("\t\t-\t");
   if(stat & STAT_ERROR) mprintf("\t\t-ERROR"); else mprintf("\t\t-\t");
   mprintf("\n");
   
}

int dispatch(t_ftmMsg* pMsg)
{
   int ret = 0;
   unsigned int diff;
   
   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      atomic_on();
      *pFpqData = hiW(pMsg->id);
      *pFpqData = loW(pMsg->id);
      *pFpqData = hiW(pMsg->par);
      *pFpqData = loW(pMsg->par);
      *pFpqData = pMsg->tef;
      *pFpqData = pMsg->res;
      *pFpqData = hiW(pMsg->ts);
      *pFpqData = loW(pMsg->ts);
      atomic_off(); 
   } else {
      ret = -1;
      mprintf("Queue full, waiting\n");
   }   
   
   return ret;
}

uint8_t condValid(t_ftmChain* c)
{
   uint8_t ret = 0;
   
   
   if(c->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED) )
   {
      if(c->flags & FLAGS_IS_COND_MSI)
      {
         uint32_t  ip, msg;
         irq_disable();
         asm ("rcsr %0, ip": "=r"(ip)); //get pending irq flags
         if(ip & 1<<MSI_SIG)
         {
            irq_pop_msi(MSI_SIG);      //pop msg from msi queue into global_msi variable
            msg = global_msi.msg;
            irq_clear(1<<MSI_SIG);     //clear pending bit
         }      
         irq_enable();
         if((c->condVal & c->condMsk) == (msg & c->condMsk)) ret = 1;   
      }
      else
      {
         if((c->condVal & c->condMsk) == ((*pFtmIf->pSharedMem).value & c->condMsk) ) 
         {
            if(c->flags & FLAGS_IS_SHARED_TIME) c->tStart = (*pFtmIf->pSharedMem).time;
            else                                c->tStart = getSysTime();         
            ret = 1;
            (*pFtmIf->pSharedMem).value = 0x0;
         }
         
      }
   }
   else ret = 1;
   
   if(ret) pFtmIf->sema.cond = 0;
   
   return ret; 
} 

void sigSend(t_ftmChain* c)
{
   t_time time;
   
   time = c->tStart + c->tPeriod;
   
   *(c->sigDst) = c->sigVal;
   if(c->flags & FLAGS_IS_SIG_SHARED)
   {
      *(c->sigDst+1) = hiW(time);
      *(c->sigDst+2) = loW(time);
   }
   pFtmIf->sema.sig = 0; 
}


t_ftmChain* processChain(t_ftmChain* c)
{
   t_ftmChain* pCur = c; 
   if( getSysTime() + pFtmIf->tPrep >= c->tStart)
   {
      if(condValid(c) || !pFtmIf->sema.cond) { pCur = processChainAux(c); }   
      else 
      {
         if((c->flags & FLAGS_IS_BP) && pFtmIf->pAct->pBp != NULL)
         { pCur = pFtmIf->pAct->pBp; pFtmIf->pAct->pBp = NULL;} // else check for breakpoint so we get around inf wait
      }
   }   
   return pCur;    
}

t_ftmChain* processChainAux(t_ftmChain* c)
{
   t_ftmChain* pCur = c; 
   unsigned long long tMsgExec;
   
   //signal to send ?
   if(pFtmIf->sema.sig)
   if(   (c->flags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED)) 
     && (    !(c->flags & FLAGS_IS_SIG_LAST)
         ||  ((c->flags & FLAGS_IS_SIG_LAST) && (c->repCnt == c->repQty-1)))) 
   {
      if( getSysTime() + pFtmIf->tPrep >= c->tStart)  sigSend(c);
   }
   
   //get execution time for due msg
   tMsgExec    = c->tStart + (c->pMsg + c->msgIdx)->offs; 
    
   if(c->msgIdx < c->msgQty) //msgs left to process?
   {
      //time to hand it over to prio queue ?
      if( getSysTime() + pFtmIf->tPrep >= tMsgExec)  
      {
         (c->pMsg + c->msgIdx)->ts = tMsgExec; //calc execution timestamp
         if( dispatch( (t_ftmMsg*)(c->pMsg + c->msgIdx) )) c->msgIdx++;  //enough space in prio queue? dispatch and inc msgidx
      }
   } 
   else
   {
      c->msgIdx = 0;
      //is c a breakpoint? if so and break is desired, continue at the address in the page alternate plan
      if((c->flags & FLAGS_IS_BP) && (pFtmIf->pAct->pBp != NULL))       
      { 
         pCur = pFtmIf->pAct->pBp;  //BP? go to alt chain
         pFtmIf->pAct->pBp = NULL;
         pFtmIf->sema.sig = 1;
         pFtmIf->sema.cond = 1;
      }
      else
      { 
         if( c->repCnt < c->repQty )   
         {
            //repetions left? stay with this chain
            c->repCnt++;     
            pCur = c; 
            if(c->flags & FLAGS_IS_SIG_ALL) pFtmIf->sema.sig = 1;
         } 
         else
         {
            //done, go to next chain
            c->repCnt = 0;
            pCur = (t_ftmChain*)c->pNext;
            pFtmIf->sema.sig = 1;
            pFtmIf->sema.cond = 1;
         } 
         
      }   
      
      //propagate updated start time to next chain to be executed
      pCur->tStart = c->tStart + c->tPeriod;
   }
     
   return pCur;    
}

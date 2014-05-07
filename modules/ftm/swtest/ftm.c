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
      
      if(cmd & CMD_SHOW_ACT)     {  if (!(pFtmIf->status & STAT_RUNNING)) showPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  if (!(pFtmIf->status & STAT_RUNNING)) showPage(pFtmIf->pIna);}
      
                            
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

void showPage(t_ftmPage* pPage)
{
   /*
   pageIdx = (pPage - pBase);
   mprintf("Active Page:   %x, %u\n", pPage, pageIdx);
   mprintf("Plans:         %x\n", pPage->planQty);
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

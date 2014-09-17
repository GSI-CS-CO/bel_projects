#include "ftm.h"

uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}

void incIdSCTR(uint64_t* id, volatile uint16_t* sctr)   {*id = ( *id & 0xfffffffffffffc00) | *sctr; *sctr = (*sctr+1) & ~0xfc00; DBPRINT3("id: %x sctr: %x\n", *id, *sctr);}


uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

uint64_t execCnt; 

void prioQueueInit()
{
   
   *(pFpqCtrl + r_FPQ.rst)       = 1;
   *(pFpqCtrl + r_FPQ.cfgClr)    = 0xffffffff;
   *(pFpqCtrl + r_FPQ.clear)     = 1;
   *(pFpqCtrl + r_FPQ.dstAdr)    = (uint32_t)pEca & ~0x80000000;
   *(pFpqCtrl + r_FPQ.tsAdr)     = ((uint32_t)pTlu & ~0x80000000);
   *(pFpqCtrl + r_FPQ.tsCh)      = 0;
   *(pFpqCtrl + r_FPQ.ebmAdr)    = ((uint32_t)pEbm & ~0x80000000);
   *(pFpqCtrl + r_FPQ.msgMax)    = 5;
   *(pFpqCtrl + r_FPQ.tTrnHi)    = hiW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tTrnLo)    = loW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tDueHi)    = hiW(pFtmIf->tDue);
   *(pFpqCtrl + r_FPQ.tDueLo)    = loW(pFtmIf->tDue);
  
   *(pFpqCtrl + r_FPQ.cfgSet)    = //r_FPQ.cfg_MSG_ARR_TS |
                                   r_FPQ.cfg_AUTOFLUSH_TIME | 
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
   
   pFtmIf->pSharedMem   = (t_shared*)pSharedRam;
   pFtmIf->sema.sig     = 1;
   pFtmIf->sema.cond    = 1;
   pCurrentChain        = (t_ftmChain*)&pFtmIf->idle;
   pFtmIf->tPrep        = 0x0000000000017A12;
   pFtmIf->tTrn         = 0x0000000000007A12;
   pFtmIf->tDue         = 0x0000000000003A12;
   prioQueueInit();
   
}

static void showId()
{
   mprintf("#%02u: ", getCpuIdx()); 
}

void cmdEval()
{
   uint32_t cmd, stat;
   t_ftmPage* pTmp;
   
   cmd = pFtmIf->cmd;
   stat = pFtmIf->status;
   
   
   if(cmd)
   {
      
      
      if(cmd & CMD_RST)          { showId(); mprintf("Ftm Init done\n"); stat = 0; ftmInit(); }
      if(cmd & CMD_START)        { showId(); mprintf("Starting, IP: 0x%08x, SP: 0x%08x\n", &pFtmIf->idle, pFtmIf->pAct->pStart); 
                                   pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                   stat = (stat & STAT_ERROR) | STAT_RUNNING;
                                 }
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pBp = (t_ftmChain*)&pFtmIf->idle; showId(); mprintf("Going to Idle\n");}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stopped (forced)\n");} 
      
      if(cmd & CMD_COMMIT_PAGE)  {//showId(); mprintf("Page Commit\n");
                                  pTmp = pFtmIf->pIna;
                                  pFtmIf->pIna = pFtmIf->pAct;
                                  pFtmIf->pAct = pTmp;
                                  pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                 }
      //if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pBp = pFtmIf->pNewBp;}
      
      if(cmd & CMD_DBG_0)        {showStatus();}
      if(cmd & CMD_DBG_1)        {showId(); mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  showId(); mprintf("Showing Active\n"); showFtmPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  showId(); mprintf("Showing Inactive\n"); showFtmPage(pFtmIf->pIna);}
      
      //only zero the command reg if you found a command. otherwise this becomes race-condition-hell!
      pFtmIf->cmd = 0;                       
   }
   
   if(pCurrentChain == &pFtmIf->idle)  {stat |=  STAT_IDLE;}
   else                       {stat &= ~STAT_IDLE;}
   if(pCurrentChain == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; mprintf("Stopped\n");}
   
   pFtmIf->status = stat;
   
}

void processFtm()
{
   if (pFtmIf->status & STAT_RUNNING) { pCurrentChain = processChain(pCurrentChain); execCnt++;} 
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
         mprintf("\t\t---CHAIN %c%u\n", planIdx+'A', chainIdx);
         mprintf("\t\tpNext: 0x%08x\n", pChain->pNext);
         mprintf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\nrepcnt:\t\t%08x\n\t\tmsg:\t\t\t%08x\nmsgIdx:\t\t%08x\n", 
         (uint32_t)(pChain->tStart>>32), (uint32_t)pChain->tStart, 
         (uint32_t)(pChain->tPeriod>>32), (uint32_t)pChain->tPeriod,
         pChain->repQty,
         pChain->repCnt,
         pChain->msgQty,
         pChain->msgIdx);
         
         mprintf("\t\tFlags:\t");
         if(pChain->flags & FLAGS_IS_BP) mprintf("-IS_BP\t");
         if(pChain->flags & FLAGS_IS_COND_MSI) mprintf("-IS_CMSI\t");
         if(pChain->flags & FLAGS_IS_COND_SHARED) mprintf("-IS_CSHA\t");
         if(pChain->flags & FLAGS_IS_SIG_SHARED) mprintf("-IS_SIG_SHARED");
         if(pChain->flags & FLAGS_IS_SIG_MSI)    mprintf("-IS_SIG_MSI");
         if(pChain->flags & FLAGS_IS_END) mprintf("-IS_END");
         if(pChain->flags & FLAGS_IS_ENDLOOP) mprintf("-IS_ENDLOOP");
         mprintf("\n");
         
         mprintf("\t\tCondSrc:\t%08x\n\t\tCondVal:\t%08x\n\t\tCondMsk:\t%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)pChain->condSrc,
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            mprintf("\t\t\t---MSG %u\n", msgIdx);
            mprintf("\t\t\tid:\t%08x%08x\n\t\t\tFID:\t%u\n\t\t\tGID:\t%u\n\t\t\tEVTNO:\t%u\n\t\t\tSID:\t%u\n\t\t\tBPID:\t%u\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id,
            getIdFID(pMsg[msgIdx].id),
            getIdGID(pMsg[msgIdx].id),
            getIdEVTNO(pMsg[msgIdx].id),
            getIdSID(pMsg[msgIdx].id),
            getIdBPID(pMsg[msgIdx].id), 
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
   mprintf("\t\tE:\t%x%08x", (uint32_t)(execCnt), (uint32_t)(execCnt>>32) );
   mprintf("\n");
   
}

int dispatch(t_ftmMsg* pMsg)
{
   int ret = 0;
   unsigned int diff;
   uint64_t now =  getSysTime();
   
   ///////////////////////////
   execCnt = 0;
   ///////////////////////////
   uint32_t msgCnt, stat; 
   stat = pFtmIf->status;
   DBPRINT3("#ST: %08x %08x \n TS: %08x %08x\n ID: %08x %08x \n", now, pMsg->ts, pMsg->id);
   DBPRINT3("Msg by #%u ...\n", getCpuIdx());
   DBPRINT3("\t\t\tid:\t%08x%08x\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg->id>>32), (uint32_t)pMsg->id, 
            (uint32_t)(pMsg->par>>32), (uint32_t)pMsg->par,
            pMsg->tef,
            (uint32_t)(pMsg->offs>>32), (uint32_t)pMsg->offs);
   
   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      //incIdSCTR(&pMsg->id, &pFtmIf->sctr); //copy sequence counter (sctr) into msg id and inc sctr
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
      msgCnt = (stat >> 16); 
      pFtmIf->status = (stat & 0x0000ffff) | ((msgCnt+1) << 16);
   } else {
      ret = -1;
      DBPRINT1("Queue full, waiting\n");
   }   
   DBPRINT3("...done\n");
   return ret;
}

uint8_t condValid(t_ftmChain* c)
{
   uint8_t ret = 0;
   t_time time;
   t_ftmChain* tmp; 
   
   if(c->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED | FLAGS_IS_COND_ADR) )
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
         tmp = (t_ftmChain*)c->pNext;
            
         DBPRINT3("CPU %u Val: %08x Con: %08x Adr: %08x\n", getCpuID() , (c->condVal & c->condMsk),  (*c->condSrc), c->condSrc );
         if((c->condVal & c->condMsk) == ((*c->condSrc) & c->condMsk) ) 
         {
            if(c->flags & FLAGS_IS_COND_TIME)
            {
               time  =  (uint64_t)(*c->condSrc+1);
               time |= ((uint64_t)(*c->condSrc+2))<<32;
               c->tStart = time;
            }   
            else c->tStart = getSysTime()+ pFtmIf->tPrep;
            tmp = (t_ftmChain*)c->pNext;
                 
            ret = 1;
            DBPRINT1("Val: %08x Src: %08x Adr: %08x\n", (c->condVal & c->condMsk),  (*c->condSrc), c->condSrc );
            (*c->condSrc) = 0x0;
         }
      }
   }
   else ret = 1;
   
   if(ret) {pFtmIf->status &= ~STAT_WAIT; pFtmIf->sema.cond = 0;}    
   else    {pFtmIf->status |=  STAT_WAIT; }
   
   return ret; 
} 

void sigSend(t_ftmChain* c)
{
   t_time time;
   
   //DBPRINT1("Sig Val: %08x Dst: %08x\n", c->sigVal,  c->sigDst );
   time = c->tStart + c->tPeriod;
   
   *(c->sigDst) = c->sigVal;
   if(c->flags & FLAGS_IS_SIG_TIME)
   {
      *(c->sigDst+1) = hiW(time);
      *(c->sigDst+2) = loW(time);
   }
   pFtmIf->sema.sig = 0; 
}


t_ftmChain* processChain(t_ftmChain* c)
{
   t_ftmChain* pCur = c;
   t_time now = getSysTime();
   
   //if starttime is 0 or in the past, set to earliest possible time
   //   || c->tStart < now
   if ( !c->tStart ) {c->tStart = now + pFtmIf->tPrep; DBPRINT2("Adjust time\n#ST: %08x %08x \n TS: %08x %08x\n", now, c->tStart);}
   
   DBPRINT3("Time to process Chain reached\n");
   if(pFtmIf->sema.cond) condValid(c);
   
   if(!pFtmIf->sema.cond) pCur = processChainAux(c); 
   else 
   {
      if((c->flags & FLAGS_IS_BP) && pFtmIf->pAct->pBp != NULL)
      { 
         pCur = pFtmIf->pAct->pBp; 
         
         pFtmIf->sctr      = 0;
         pFtmIf->pAct->pBp = NULL;
      } 
   }
      
   return pCur;    
}

t_ftmChain* processChainAux(t_ftmChain* c)
{
   t_ftmChain* pCur;
   uint64_t tMsgExec, now;
   
   pCur  = c; 
   now   = getSysTime();
   
   if( now + pFtmIf->tPrep >= c->tStart) {
   
   //signal to send ?
   if(pFtmIf->sema.sig &&  (c->flags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED))) {
       
      sigSend(c);
   }
   
   
   if( c->repCnt < c->repQty || c->repQty == -1) //reps left ?  
   {
      if(c->msgIdx < c->msgQty) //msgs left to process?
      {
         c->pMsg[c->msgIdx].ts = c->tStart + c->pMsg[c->msgIdx].offs; //set execution time for msg 
         if( now + pFtmIf->tPrep >= (c->pMsg + c->msgIdx)->ts)  //### time to hand it over to prio queue ? ###
         {
            if ( !dispatch((t_ftmMsg*)(c->pMsg + c->msgIdx)) ) 
            c->msgIdx++;  //enough space in prio queue? dispatch and inc msgidx
    
            DBPRINT3("Msg %u sent ", c->msgIdx);
         } 
      } 
      else
      {
        
   
         c->msgIdx = 0; c->repCnt++; //repetions left, stay with this chain
              
         if( (c->flags & FLAGS_IS_END) && (c->flags & FLAGS_IS_ENDLOOP)) 
         { 
            pCur = (t_ftmChain*)c->pNext;
            pFtmIf->sema.sig  = 1;
            pFtmIf->sema.cond = 1;
            DBPRINT1("Chain Loop to 0x%08x\n", pCur); 
         }
         else pCur = c;
         
         pCur->tStart = c->tStart + c->tPeriod; 
         if(c->flags & FLAGS_IS_SIG_ALL)  pFtmIf->sema.sig  = 1;
         if(c->flags & FLAGS_IS_COND_ALL) pFtmIf->sema.cond = 1;

      }
   } 
   else
   {
      //done, go to next chain
      DBPRINT3("RepCnt: %u RepQty: %u\n", c->repCnt, c->repQty);
      c->msgIdx = 0; c->repCnt = 0;
      if( ((c->flags & FLAGS_IS_END) && (c->flags & FLAGS_IS_ENDLOOP)) || (c->pNext == NULL) ) pCur = (t_ftmChain*)&pFtmIf->idle;
      else pCur = (t_ftmChain*)c->pNext;
      
      pCur->tStart = c->tStart + c->tPeriod;
      pFtmIf->sema.sig  = 1;
      pFtmIf->sema.cond = 1;
      DBPRINT3("NC SemaCond: %u\n", pFtmIf->sema.cond);
     
   } 
   }
   
   //is c a branchpoint? if so, jump, reset sequence counter (sctr), semaphores and BP
   if((c->flags & FLAGS_IS_BP) && (pFtmIf->pAct->pBp != NULL))       
   { 
      pCur = pFtmIf->pAct->pBp;  //BP? go to alt chain
      
      pFtmIf->sctr      = 0;
      pFtmIf->pAct->pBp = NULL;
      pFtmIf->sema.sig  = 1;
      pFtmIf->sema.cond = 1;
      
      DBPRINT3("BP SemaCond: %u\n", pFtmIf->sema.cond);
   }
    
   return pCur;    
}


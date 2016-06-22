#include "ftm.h"
#include "dbg.h"




uint64_t dbg_sum = 0;

uint64_t execCnt; 

void prioQueueInit()
{
   //set up the pointer to the global msg count
   pMsgCntPQ = (uint64_t*)(pFpqCtrl + (PRIO_CNT_OUT_ALL_GET_0>>2));

   pFpqCtrl[PRIO_RESET_OWR>>2]      = 1;
   pFpqCtrl[PRIO_MODE_CLR>>2]       = 0xffffffff;
   pFpqCtrl[PRIO_ECA_ADR_RW>>2]     = (uint32_t)pEca & ~0x80000000;
   pFpqCtrl[PRIO_EBM_ADR_RW>>2]     = ((uint32_t)pEbm & ~0x80000000);
   pFpqCtrl[PRIO_TX_MAX_MSGS_RW>>2] = 5;
   pFpqCtrl[PRIO_TX_MAX_WAIT_RW>>2] = loW(pFtmIf->tDue);
   pFpqCtrl[PRIO_MODE_SET>>2]       = PRIO_BIT_ENABLE     | 
                                          PRIO_BIT_MSG_LIMIT  |
                                          PRIO_BIT_TIME_LIMIT;
}

void ftmInit()
{
   pFtmIf = (t_ftmIf*)_startshared;
   //mprintf("Shared Area @ 0x%08x\n", (uint32_t)pFtmIf); 
   pFtmIf->cmd = 0;
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
   
   //pFtmIf->pSharedMem   = (t_shared*)pSharedRam;
   pFtmIf->sema.sig     = 1;
   pFtmIf->sema.cond    = 1;
   pCurrentChain        = (t_ftmChain*)&pFtmIf->idle;
   pFtmIf->tPrep        = 150000;
//   pFtmIf->tTrn         = 15000/8;
   pFtmIf->tDue         = 5000;
   prioQueueInit();

   // MODELSIM FIRMWARE
   //pFtmIf->cmd = CMD_START;    
 
   pFtmIf->debug[DBG_DISP_DUR_MIN] = 0xffffffff;
   pFtmIf->debug[DBG_DISP_DUR_MAX] = 0x0;
   pFtmIf->debug[DBG_DISP_DUR_AVG] = 0x0;
}

inline void showId()
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
      if(cmd & CMD_START)        { showId(); mprintf("Run\n"); 
                                   pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                   stat = (stat & STAT_ERROR) | STAT_RUNNING;
                                 }
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pBp = (t_ftmChain*)&pFtmIf->idle; showId(); mprintf("Going to Idle\n");}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stop (forced)\n");} 
      
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
   if(pCurrentChain == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stop\n");}
   
   pFtmIf->status = stat;
   
}



void showFtmPage(t_ftmPage* pPage)
{
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

inline int dispatch(t_ftmMsg* pMsg)
{
  
  unsigned int diff;
  int ret = 1;
  uint32_t msgCnt, stat;
  uint64_t tmpPar; 
  stat = pFtmIf->status;
   
  //incIdSCTR(&pMsg->id, &pFtmIf->sctr); //copy sequence counter (sctr) into msg id and inc sctr
  
  //Diagnostic Event? insert PQ Message counter. Different device, can't be placed inside atomic!
  if (pMsg->id == DIAG_PQ_MSG_CNT) tmpPar = *pMsgCntPQ;
  else                             tmpPar = pMsg->par;  

  atomic_on();
  *(pFpqData + (PRIO_DAT_STD>>2))   = hiW(pMsg->id);
  *(pFpqData + (PRIO_DAT_STD>>2))   = loW(pMsg->id);
  *(pFpqData + (PRIO_DAT_STD>>2))   = hiW(tmpPar);
  *(pFpqData + (PRIO_DAT_STD>>2))   = loW(tmpPar);
  *(pFpqData + (PRIO_DAT_STD>>2))   = pMsg->tef;
  *(pFpqData + (PRIO_DAT_STD>>2))   = pMsg->res;
  *(pFpqData + (PRIO_DAT_TS_HI>>2)) = hiW(pMsg->ts);
  *(pFpqData + (PRIO_DAT_TS_LO>>2)) = loW(pMsg->ts);
  atomic_off();
  msgCnt = (stat >> 16); 
  pFtmIf->status = (stat & 0x0000ffff) | ((msgCnt+1) << 16);
   
   
  return ret;

}

inline uint8_t condValid(t_ftmChain* c)
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

inline void sigSend(t_ftmChain* c)
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

inline t_ftmChain* processChainAux(t_ftmChain* c)
{
   t_ftmChain* pCur;
   t_ftmMsg*   pCurMsg;
   uint64_t tMsgExec, now;

   uint64_t dbg_now, dbg_then; 
   uint32_t dbg_dur;

   DBPRINT2("Time to process Chain %08x reached\n", c);
   pCur  = c; 
   now   = getSysTime();
   
   if( now + pFtmIf->tPrep >= c->tStart) {
      //DBPRINT3("now+tp\t: x%08x%08x\ncstart\t: x%08x%08x\n", hiW(now + pFtmIf->tPrep), loW(now + pFtmIf->tPrep), hiW(c->tStart), loW(c->tStart)    ); 
      //signal to send ?
      
      if(pFtmIf->sema.sig &&  (c->flags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED))) {
          
         sigSend(c);
      }
      
      DBPRINT3("repcnt %04x repqty %04x\n", c->repCnt, c->repQty);
      if( c->repCnt < c->repQty || c->repQty == -1) //reps left ?  
      {
         DBPRINT3("repcnt %u repqty %u", c->repCnt, c->repQty);
         while(c->msgIdx < c->msgQty) //msgs left to process?
         {
            pCurMsg = c->pMsg + c->msgIdx;
            pCurMsg->ts = c->tStart + pCurMsg->offs; //set execution time for msg 
            if( now + pFtmIf->tPrep >= pCurMsg->ts)  //### time to hand it over to prio queue ? ###
            {
               uint32_t msgCnt = (pFtmIf->status >> 16); 
               
               //dbg_then = getSysTime();
               if(dispatch(pCurMsg)) c->msgIdx++;
               //dbg_now = getSysTime();
      
                  //Debug Stuff
                  
               /*dbg_dur = (uint32_t)(dbg_now-dbg_then);
               if(msgCnt == 0) dbg_sum = 0;
               else dbg_sum += (uint64_t)dbg_dur;
               if(pFtmIf->debug[DBG_DISP_DUR_MIN] > dbg_dur) pFtmIf->debug[DBG_DISP_DUR_MIN] = dbg_dur; //min
               if(pFtmIf->debug[DBG_DISP_DUR_MAX] < dbg_dur) pFtmIf->debug[DBG_DISP_DUR_MAX] = dbg_dur; //max
               pFtmIf->debug[DBG_DISP_DUR_AVG] = dbg_sum/(msgCnt+1);
               */
            } else {break; DBPRINT3("Too early for Msg %u", c->msgIdx);}
         } 
         if(c->msgIdx == c->msgQty)
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


inline t_ftmChain* processChain(t_ftmChain* c)
{
   t_ftmChain* pCur = c;
   t_time now = getSysTime();
   
   //if starttime is 0 or in the past, set to earliest possible time
   //   || c->tStart < now
   if ( !c->tStart ) {c->tStart = now + pFtmIf->tPrep; DBPRINT2("Adjust time\n#ST: %08x %08x \n TS: %08x %08x\n", now, c->tStart);}
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


void processFtm()
{
   DBPRINT3("c = %08x\n", pCurrentChain);
   if (pFtmIf->status & STAT_RUNNING) { pCurrentChain = processChain(pCurrentChain); execCnt++;} 
}


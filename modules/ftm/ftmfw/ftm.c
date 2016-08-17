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
   pFpqCtrl[PRIO_TX_MAX_WAIT_RW>>2] = loW(*(uint64_t*)(pV + SHCTL_TGATHER));
   pFpqCtrl[PRIO_MODE_SET>>2]       = PRIO_BIT_ENABLE     | 
                                      PRIO_BIT_MSG_LIMIT  |
                                      PRIO_BIT_TIME_LIMIT;
}

void ftmInit()
{
   p  = (uint32_t*)_startshared;
   pV = (void*)_startshared;
   //mprintf("Shared Area @ 0x%08x\n", (uint32_t)p); 

   p[SHCTL_STATUS]  = 0;
   p[SHCTL_MSG_CNT] = 0;
   p[SHCTL_CMD]     = 0; 

   //FIXME this belongs to thread ctrl now
   /* 
   p[pAct = (t_ftmPage*)&(p[pPages[0]);
   p[pIna = (t_ftmPage*)&(p[pPages[1]);
   */
  
   //FIXME obsolete
   /* 
   p[idle = (t_ftmChain){ .tStart     = 0,
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
   */

   //FIXME if at all, this belongs to thread data now
   /* 
   p[sema.sig     = 1;
   p[sema.cond    = 1;
   */
   pCurrent  = NULL;
   *(uint64_t*)(pV + SHCTL_TPREP) = 150000;
   *(uint64_t*)(pV + SHCTL_TGATHER)  = 5000;

   prioQueueInit();

}

inline void showId()
{
   mprintf("#%02u: ", getCpuIdx()); 
}

void cmdEval()
{
   uint32_t cmd, stat;

   
   cmd  = p[SHCTL_CMD >>2];
   stat = p[SHCTL_STATUS >>2];
   
   
   if(cmd)
   {
      
      
      if(cmd & CMD_RST)          { showId(); mprintf("Ftm Init done\n"); stat = 0; ftmInit(); }
      if(cmd & CMD_START)        { showId(); mprintf("Run\n"); 
                                   //FIXME No more direct editing in future, and add ctrl for all individual threads
                                   p[(SHCTL_THR_CTL + TC_GET) >>2] = -1;
                                      
                                   stat = (stat & STAT_ERROR) | STAT_RUNNING;
                                 }
      if(cmd & CMD_IDLE)         { }//FIXME No such thing as IDLE anymore
                                    //pFtmIf->pAct->pBp = (t_ftmChain*)&pFtmIf->idle; showId(); mprintf("Going to Idle\n");}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { //FIXME No more direct editing in future, and add ctrl for all individual threads
                                   p[(SHCTL_THR_CTL + TC_GET) >>2] = -0;
                                   stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stop (forced)\n");} 
      
      if(cmd & CMD_COMMIT_PAGE)  { //FIXME No such thing as commit anymore, any mounted block can be activated
                                  /*
                                  pTmp = pFtmIf->pIna;
                                  pFtmIf->pIna = pFtmIf->pAct;
                                  pFtmIf->pAct = pTmp;
                                  pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                  */    
                                 }
      //if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pBp = pFtmIf->pNewBp;}
      
      if(cmd & CMD_DBG_0)        {showStatus();}
      if(cmd & CMD_DBG_1)        {showId(); mprintf("DBG1\n");}
 
      //only zero the command reg if you found a command. otherwise this becomes race-condition-hell!
      p[SHCTL_CMD >>2] = 0;                       
   }
   
   //FIXME No such thing as IDLE anymore
   /* 
   if(pCurrent == &pFtmIf->idle)  {stat |=  STAT_IDLE;}
   else                       {stat &= ~STAT_IDLE;}
    */
    /*
   if(pCurrent == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stop\n");}
   */
   p[SHCTL_STATUS >>2] = stat;
   
}


/*
void showFtmPage(t_ftmPage* pPage)
{
}
*/
void showStatus()
{
   //FIXME obsolete bullshit
   /*

   uint32_t stat = p[SHCTL_STATUS >>2];
   mprintf("\f%08x\tStatus:\t", (uint32_t)(&(pFtmIf->cmd)) );
   if(stat & STAT_RUNNING) mprintf("\t\t-RUNNING"); else mprintf("\t\t-\t");
   //FIXME No such thing as IDLE anymore 
   //if(stat & STAT_IDLE) mprintf("\t\t-IDLE"); else mprintf("\t\t-\n");
   if(stat & STAT_STOP_REQ) mprintf("\t\t-STOP_REQ"); else mprintf("\t\t-\t");
   if(stat & STAT_ERROR) mprintf("\t\t-ERROR"); else mprintf("\t\t-\t");
   mprintf("\t\tE:\t%x%08x", (uint32_t)(execCnt), (uint32_t)(execCnt>>32) );
   mprintf("\n");
   */
}

inline int dispatch(t_ftmMsg* pMsg)
{
  
  unsigned int diff;
  int ret = 1;
  uint32_t msgCnt, stat;
  uint64_t tmpPar; 

   
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
    
  //increase Msg count for this CPU
  p[SHCTL_MSG_CNT >>2]++;
   
   
  return ret;

}

inline uint8_t condValid(t_ftmChain* c)
{
   //FIXME obsolete, this is per sig event now  
  
   uint8_t ret = 0;
   /* 
   t_time time;
   t_ftmChain* tmp; 
   
   if(c->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED | FLAGS_IS_COND_ADR) )
   {
      if(c->flags & FLAGS_IS_COND_MSI)
      {
         uint32_t  ip, msg;

         asm ("rcsr %0, ip": "=r"(ip)); //get pending irq flags
         if(ip & 1<<MSI_SIG)
         {
            irq_pop_msi(MSI_SIG);      //pop msg from msi queue into global_msi variable
            msg = global_msi.msg;
            irq_clear(1<<MSI_SIG);     //clear pending bit
         }      

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
            else c->tStart = getSysTime()+ *(uint64_t*)(pV + SHCTL_TPREP);
            tmp = (t_ftmChain*)c->pNext;
                 
            ret = 1;
            DBPRINT1("Val: %08x Src: %08x Adr: %08x\n", (c->condVal & c->condMsk),  (*c->condSrc), c->condSrc );
            (*c->condSrc) = 0x0;
         }
      }
   }
   else ret = 1;
   
   if(ret) {p[SHCTL_STATUS >>2] &= ~STAT_WAIT; pFtmIf->sema.cond = 0;}    
   else    {p[SHCTL_STATUS >>2] |=  STAT_WAIT; }
   */
   return ret; 
} 


inline void sigSend(t_ftmChain* c)
{
   //FIXME obsolete, this is per sig event now
         /* 
   t_time time;
   uint32_t slot;

   //DBPRINT1("Sig Val: %08x Dst: %08x\n", c->sigVal,  c->sigDst );
   time = c->tStart + c->tPeriod;

   //want to signal via MSI? use the Msg Box
    
   if (c->flags & FLAGS_IS_SIG_MSI) {
     
     slot = ((uint32_t)c->sigDst & 0x1f);   
     *(pCpuMsiBox + (slot <<1)) = c->sigVal;
    } else { 

     *(c->sigDst) = c->sigVal;
     if(c->flags & FLAGS_IS_SIG_TIME)
     {
        *(c->sigDst+1) = hiW(time);
        *(c->sigDst+2) = loW(time);
   }
   }
   pFtmIf->sema.sig = 0; p[SHCTL_STATUS >>2]
  */
}

inline uint32_t processChainAux(t_ftmChain** pCur)
{
   t_ftmChain* c; = pCur;
   t_ftmMsg*   pCurMsg;
   uint64_t tMsgExec, now;

   uint64_t dbg_now, dbg_then; 
   uint32_t dbg_dur;

   DBPRINT2("Time to process Chain %08x reached\n", c);


   c = pCur; 
   now   = getSysTime();
   tNewStart = c->tStart + c->tPeriod; 

   
   if( now + *(uint64_t*)(pV + SHCTL_TPREP) >= c->tStart) {
      
      DBPRINT3("repcnt %04x repqty %04x\n", c->repCnt, c->repQty);
      if( c->repCnt < c->repQty || c->repQty == -1) //reps left ?  
      {
         DBPRINT3("repcnt %u repqty %u", c->repCnt, c->repQty);
         while(c->msgIdx < c->msgQty) //msgs left to process?
         {
            pCurMsg = (t_ftmMsg*)((uint32_t)c + (c->msgOffset + (c->msgIdx * FTM_MSG_END_)));
            pCurMsg->ts = c->tStart + pCurMsg->offs; //set execution time for msg 
            if( now + *(uint64_t*)(pV + SHCTL_TPREP) >= pCurMsg->ts)  //### time to hand it over to prio queue ? ###
            {
               uint32_t msgCnt = (p[SHCTL_STATUS >>2] >> 16); 
               
               //dbg_then = getSysTime();
               if(dispatch(pCurMsg)) c->msgIdx++;
              
            } else {break; DBPRINT3("Too early for Msg %u", c->msgIdx);}
         } 
         if(c->msgIdx == c->msgQty)
         {
            c->msgIdx = 0; c->repCnt++; //repetions left, stay with this chain
            c->tStart = tNewStart; 
         }
      } 
      else
      {
         
         DBPRINT3("RepCnt: %u RepQty: %u\n", c->repCnt, c->repQty);
         c->msgIdx = 0; c->repCnt = 0;
         c->tStart = tNewStart;

         if(c->nextOffset != NULL) {
           //move to next chain 
          *pCur += c->nextOffset;
         } else {         
          //move to successor block
          return c->nextIdx; 

         }
        
      }
      
   }

  return -1;    
}


inline uint32_t processChain(t_ftmChain** c)
{
   uint32_t idx;
   t_time now = getSysTime();
   
   //if starttime is 0 or in the past, set to earliest possible time
   //   || c->tStart < now
   if ( !c->tStart ) {c->tStart = now + *(uint64_t*)(pV + SHCTL_TPREP)  ; DBPRINT2("Adjust time\n#ST: %08x %08x \n TS: %08x %08x\n", now, c->tStart);}
   //FIXME obsolete, if at all, this belongs to thread data now 
   //if(pFtmIf->sema.cond) condValid(c);
   
   //FIXME obsolete, if at all, this belongs to thread data now
   //if(!pFtmIf->sema.cond) {
      idx = processChainAux(c); 

   //} else {
    //FIXME this belongs to block Qs now (not yet implemented)
         /*
      if((c->flags & FLAGS_IS_BP) && pFtmIf->pAct->pBp != NULL)
      { 
         
         pCur = pFtmIf->pAct->pBp; 
         pFtmIf->sctr      = 0;
         pFtmIf->pAct->pBp = NULL;
         
      } 

   }
    */  
   return idx;    
}
/*

void processFtm()
{
   DBPRINT3("c = %08x\n", pCurrent);
   if (p[SHCTL_STATUS >>2] & STAT_RUNNING) { pCurrent = processChain(pCurrent); execCnt++;} 


}
*/

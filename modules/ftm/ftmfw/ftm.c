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
   
   
  if(cmd) {
      
      
    if(cmd & CMD_RST)          { showId(); mprintf("Ftm Init done\n"); stat = 0; ftmInit(); }
    if(cmd & CMD_START)        { showId(); mprintf("Run\n"); 
                                 //FIXME No more direct editing in future, and add ctrl for all individual threads
                                 p[(SHCTL_THR_CTL + TC_GET) >>2] = -1;
                                    
                                 stat = (stat & STAT_ERROR) | STAT_RUNNING;
                               }
    if(cmd & CMD_IDLE)         { }//FIXME No such thing as IDLE anymore

    if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
    if(cmd & CMD_STOP_NOW)     { //FIXME No more direct editing in future, and add ctrl for all individual threads
                                 p[(SHCTL_THR_CTL + TC_GET) >>2] = -0;
                                 stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stop (forced)\n");} 

    if(cmd & CMD_COMMIT_PAGE)  {} //FIXME No such thing as commit anymore, any mounted block can be activated

    //TODO this belong sto block Qs now                                 }
    //if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pBp = pFtmIf->pNewBp;}

    if(cmd & CMD_DBG_0)        {showStatus();}
    if(cmd & CMD_DBG_1)        {showId(); mprintf("DBG1\n");}

    //only zero the command reg if you found a command. otherwise this becomes race-condition-hell!
    p[SHCTL_CMD >>2] = 0;                       


    //FIXME No such thing as IDLE anymore
    p[SHCTL_STATUS >>2] = stat;
  }  
}



void showStatus()
{
   //FIXME obsolete bullshit

}

inline int dispatch(t_ftmMsg* pMsg)
{
  
  unsigned int diff;
  int ret = 1;
  uint32_t msgCnt, stat;
  uint64_t tmpPar; 

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



inline uint32_t processChainAux(t_ftmChain** pCur)
{
  t_ftmChain* c;
  t_ftmMsg*   pCurMsg;
  uint64_t    now, tNewStart;


  c         = *pCur; 
  now       = getSysTime();
  tNewStart = c->tStart + c->tPeriod; 


  //reps left ? 
  DBPRINT3("repcnt %04x repqty %04x\n", c->repCnt, c->repQty);
  if( c->repCnt < c->repQty || c->repQty == -1) {
     DBPRINT3("repcnt %u repqty %u", c->repCnt, c->repQty);
     //msgs left ?
     while(c->msgIdx < c->msgQty) {
        pCurMsg = (t_ftmMsg*)((uint32_t)c + (c->msgOffset + (c->msgIdx * FTM_MSG_END_)));
        pCurMsg->ts = c->tStart + pCurMsg->offs; //set execution time for msg 

        //msg due ?
        if( now + *(uint64_t*)(pV + SHCTL_TPREP) >= pCurMsg->ts) {
           uint32_t msgCnt = (p[SHCTL_STATUS >>2] >> 16); 
           
           //dbg_then = getSysTime();
           if(dispatch(pCurMsg)) c->msgIdx++;
          
        } else {break; DBPRINT3("Too early for Msg %u", c->msgIdx);}
     } 
     c->msgIdx = 0;
     c->repCnt++; //repetions left, stay with this chain
     c->tStart = tNewStart; 
    
  } else {
     DBPRINT3("RepCnt: %u RepQty: %u\n", c->repCnt, c->repQty);
     c->msgIdx = 0;
     c->repCnt = 0;
     c->tStart = tNewStart;

     if(c->nextOffset != 0) {
       //move to next chain 
      *pCur = (t_ftmChain*)((void*)(*pCur) + c->nextOffset);
     } else {         
      //move to successor block
      return c->nextIdx; 
     }
    
  }


  return -1;    
}


inline uint32_t processChain(t_ftmChain** pCur)
{
  uint32_t idx;
  t_time now = getSysTime();
  t_ftmChain* c;
  c  = *pCur; 

  //if starttime is 0 or in the past, set to earliest possible time

  if ( !c->tStart ) {c->tStart = now + *(uint64_t*)(pV + SHCTL_TPREP)  ; DBPRINT2("Adjust time\n#ST: %08x %08x \n TS: %08x %08x\n", now, c->tStart);}
  idx = processChainAux(pCur); 
  //TODO block Qs now (not yet implemented)
  return idx;    
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


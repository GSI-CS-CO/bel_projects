#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ftmx86.h"
#include <etherbone.h>
#include "access.h"

extern uint8_t cpuIdx;
extern t_ftmAccess* p;
uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}

static uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart);
static uint8_t* deserMsg(uint8_t* pBuf, t_ftmMsg* pMsg);

static uint8_t* serChain(t_ftmChain* pChain, uint32_t pPlanStart, uint8_t* pBufStart, uint8_t* pBuf, uint8_t cpuId);
static uint8_t* serMsg(  t_ftmMsg* pMsg, uint8_t* pBuf);

uint64_t getId(uint16_t fid, uint16_t gid, uint16_t evtno, uint16_t sid, uint16_t bpid, uint16_t sctr)
{
   uint64_t ret;
   
   //printf("ID: %u %u %u %u %u %u\n", fid, gid, evtno, sid, bpid, sctr);
   ret =    ((uint64_t)fid    << ID_FID_POS)    |
            ((uint64_t)gid    << ID_GID_POS)    |
            ((uint64_t)evtno  << ID_EVTNO_POS)  |
            ((uint64_t)sid    << ID_SID_POS)    |
            ((uint64_t)bpid   << ID_BPID_POS)   |
            ((uint64_t)sctr   << ID_SCTR_POS);

   return ret;         
}

const t_ftmChain Idle = { .tStart         = 0,
                          .tPeriod        = 500,
                          .tExec          = 0,
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


                          
static uint8_t* writeLeNumberToBeBytes(uint8_t* pBuf, uint32_t val)
{
   uint8_t i;
   for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*i) & 0xff;
   
   return pBuf+4;
}

static uint8_t* writeLeNumberToBeBytes(uint8_t* pBuf, uint64_t val)
{
  writeLeNumberToBeBytes(pBuf+0, (uint32_t)(val>>32));
  writeLeNumberToBeBytes(pBuf+4, (uint32_t)val);

   return pBuf+8;
}

static uint32_t bytesToUint32(uint8_t* pBuf)
{
   uint8_t i;
   uint32_t val=0;
   
   for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*i);
   return val;
}

static uint64_t bytesToUint64(uint8_t* pBuf)
{
   uint64_t val=0;
   
   val |= (uint64_t)bytesToUint32(pBuf+0)<<32;
   val |= (uint64_t)bytesToUint32(pBuf+4);
   return val;
}

uint8_t* serPage (t_ftmPage*  pPage, uint8_t* pBufStart, uint8_t cpuId) 
{
   //printf("Writing ptrPage %p to ptrWrite %p for cpu %u\n", pPage, pBufStart, cpuId);
   uint8_t j,  planIdx, chainIdx;
   uint8_t*    pBuf;
   t_ftmChain* pChain;
   uint32_t    pBufPlans[FTM_PLAN_MAX];
   pBuf = pBufStart + FTM_PAGEMETA;
   
   if(pPage == NULL) {fprintf(stderr, "error: Got no DOM to serialize from XML Parser. Something went wrong.\n"); return NULL;};
   //leave space for page meta info, write all plans to pBuffer

   //FIXME ONLY ONE PLAN FOR NOW!!!
// for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   //printf("Serializing page\n");
   for(planIdx = 0; planIdx < 1; planIdx++)
   {
          

      pBufPlans[planIdx] = ((uint32_t)((uintptr_t)pBuf - (uintptr_t)pBufStart));
      pChain   = pPage->plans[planIdx].pStart;
      chainIdx = 0;
      

      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         //printf("Ser Flags: 0x%08x, ChainQty; %u \n", pChain->flags, pPage->plans[planIdx].chainQty);
         pBuf     = serChain(pChain, pBufPlans[planIdx], pBufStart, pBuf, cpuId);
         pChain   = (t_ftmChain*)pChain->pNext;
      }   
   }
   
   // now write page meta
   writeLeNumberToBeBytes(&pBufStart[FTM_PAGE_QTY], pPage->planQty);
   for(j=0;j<pPage->planQty;    j++)
      writeLeNumberToBeBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], pBufPlans[j]);
   for(j=pPage->planQty;j<FTM_PLAN_MAX;    j++)
      writeLeNumberToBeBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], FTM_NULL);
   
   if( pPage->idxBp == 0xdeadbeef)    pPage->pBp = ftm_shared_offs + FTM_IDLE_OFFSET;
   else pPage->pBp     = pBufPlans[pPage->idxBp];
   
   if( pPage->idxStart == 0xdeadbeef) pPage->pStart = ftm_shared_offs + FTM_IDLE_OFFSET;
   else pPage->pStart  = pBufPlans[pPage->idxStart];
   
   //printf("BP: %08x, Start: %08x\n", pPage->pBp, pPage->pStart);
   
   writeLeNumberToBeBytes(&pBufStart[FTM_PAGE_PTR_BP],         pPage->pBp);
   writeLeNumberToBeBytes(&pBufStart[FTM_PAGE_PTR_START],      pPage->pStart);
   
   return pBuf;   
}

static uint8_t* serChain(t_ftmChain* pChain, uint32_t pPlanStart, uint8_t* pBufStart, uint8_t* pBuf, uint8_t cpuId)
{
   uint8_t msgIdx;
   
   uint32_t pBufNext, pBufMsg, sigDst, condSrc;
   t_ftmMsg* pMsg = pChain->pMsg; 
   
   sigDst   = 0;
   condSrc  = 0;
   
   if (pChain->flags & FLAGS_IS_SIG_MSI)           sigDst = pChain->sigCpu;
   else if (pChain->flags & FLAGS_IS_SIG_ADR)      sigDst = pChain->sigDst;

   if (pChain->flags & FLAGS_IS_COND_MSI)          condSrc = 0x0;
   else if (pChain->flags & FLAGS_IS_COND_ADR)     condSrc = pChain->condSrc;
   
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_TSTART],    pChain->tStart);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_TPERIOD],   pChain->tPeriod);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_TEXEC],     0);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_FLAGS],     pChain->flags);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_CONDSRC],   condSrc);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_CONDVAL],   pChain->condVal);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_CONDMSK],   pChain->condMsk);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_SIGDST],    sigDst);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_SIGVAL],    pChain->sigVal);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_REPQTY],    pChain->repQty);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_REPCNT],    0);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_MSGQTY],    pChain->msgQty);
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_MSGIDX],    0);


   pBufMsg  = FTM_CHAIN_END_ + ( (uint32_t)( (uintptr_t)pBuf - (uintptr_t)pBufStart ) );
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_PMSG],    pBufMsg);
   
   if(pChain->pNext != NULL) {
      if(((pChain->flags & FLAGS_IS_END) && (pChain->flags & FLAGS_IS_ENDLOOP)))   pBufNext = (uint32_t)pPlanStart;
      else                                                                         pBufNext = pBufMsg + pChain->msgQty * FTM_MSG_END_;
   } 
   else pBufNext = FTM_NULL;
   writeLeNumberToBeBytes(&pBuf[FTM_CHAIN_PNEXT],    pBufNext);
 
   pBuf += FTM_CHAIN_END_;
   for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++) pBuf = serMsg(&pMsg[msgIdx], pBuf);   
   //printf("Chain placed @ 0x%08x\n", pBuf-pBufStart);
   return pBuf;   

}

static uint8_t* serMsg(  t_ftmMsg* pMsg, uint8_t* pBuf)
{
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_ID],     pMsg->id);
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_PAR],    pMsg->par);
   
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_TEF],    pMsg->tef); 
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_RES],    pMsg->res); 
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_TS],     0);
   writeLeNumberToBeBytes(&pBuf[FTM_MSG_OFFS],   pMsg->offs);
   return pBuf + FTM_MSG_END_;
}

t_ftmPage* deserPage(t_ftmPage* pPage, uint8_t* pBufStart)
{
   uint32_t j, chainQty, planStart;
   uint8_t*    pBufPlans[FTM_PLAN_MAX];
   uint8_t*    pBuf     = pBufStart;
   t_ftmChain* pChain   = NULL;
   t_ftmChain* pNext    = NULL;
  
   //get plan qty
   if(pPage == NULL) printf("page not allocated\n");
   pPage->planQty = bytesToUint32(&pBufStart[FTM_PAGE_QTY]);
   
   for(j=0; j<pPage->planQty; j++)
   {
      //read the offset to start of first chain
      planStart      = bytesToUint32(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE]);
       
      pBufPlans[j]   = pBufStart + (uintptr_t)planStart;
      
      //allocate first chain and a Next Chain
      chainQty = 1;
      pChain   = calloc(1, sizeof(t_ftmChain));
      pNext    = calloc(1, sizeof(t_ftmChain));
      //set plan start to first chain
      pPage->plans[j].pStart = pChain;
      //deserialise (pBufStart +  offset) to pChain and fix pChains next ptr
      pBuf = deserChain(pChain, pNext, pBufPlans[j], pBufStart);

      //deserialise chains until we reached the end or we reached max
      //printf("Flags: 0x%08x, ChainQty; %u \n", pChain->flags, chainQty);
      while(!(pChain->flags & FLAGS_IS_END) && chainQty < 10)
      {
         pChain   = pNext;
         pNext    = calloc(1, sizeof(t_ftmChain));
         pBuf     = deserChain(pChain, pNext, pBuf, pBufStart);
         chainQty++;
         //printf("Plan %u Chain %u @ %08x\n", j, chainQty, pBufStart-pBuf);
         
      } 
      //no next after the end, free this one
      //free(pNext);
      pChain->pNext = NULL;
      //save to plan how many chains it contains
      pPage->plans[j].chainQty = chainQty;
   }
   
   pPage->pBp        = bytesToUint32(&pBufStart[FTM_PAGE_PTR_BP]); 
   pPage->pStart     = bytesToUint32(&pBufStart[FTM_PAGE_PTR_START]);
   
   if       (pPage->pBp    == FTM_NULL)         {pPage->idxBp  = 0xcafebabe;}
   else if  (pPage->pBp    == ftm_shared_offs + FTM_IDLE_OFFSET)  {pPage->idxBp  = 0xdeadbeef;}
   else for (j=0; j<pPage->planQty; j++)      
   {
      if(pBufPlans[j]-pBufStart == (uintptr_t)pPage->pBp) {pPage->idxBp = j;}
   }  
      
   if       (pPage->pStart == FTM_NULL)           {pPage->idxStart = 0xcafebabe;}
   else if  (pPage->pStart == ftm_shared_offs + FTM_IDLE_OFFSET)    {pPage->idxStart = 0xdeadbeef;}
   else for(j=0; j<pPage->planQty; j++) 
   {
      if(pBufPlans[j]-pBufStart == (uintptr_t)pPage->pStart) {pPage->idxStart = j;}
   }    
   //pPage->pp->sharedAdr = pBufStart[FTM_PAGE_PTR_p->sharedAdr];    
      
   return pPage;     
}

static uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart)
{
   uint8_t* pBuf = pChainStart;
   uint32_t msgIdx;

      //printf("deserchain\n");
      //printf("pChain %p pNext %p pBufPLansJ %p pBuf: %p\n", pChain, pNext, pChainStart, pBufStart); 
   pChain->tStart   = bytesToUint64(&pChainStart[FTM_CHAIN_TSTART]);
   pChain->tPeriod  = bytesToUint64(&pChainStart[FTM_CHAIN_TPERIOD]);
   pChain->flags    = bytesToUint32(&pChainStart[FTM_CHAIN_FLAGS]);
   pChain->condVal  = bytesToUint32(&pChainStart[FTM_CHAIN_CONDVAL]);
   pChain->condMsk  = bytesToUint32(&pChainStart[FTM_CHAIN_CONDMSK]);
   pChain->sigDst   = bytesToUint32(&pChainStart[FTM_CHAIN_SIGDST]);
   pChain->sigVal   = bytesToUint32(&pChainStart[FTM_CHAIN_SIGVAL]);
   pChain->repQty   = bytesToUint32(&pChainStart[FTM_CHAIN_REPQTY]);
   pChain->repCnt   = bytesToUint32(&pChainStart[FTM_CHAIN_REPCNT]);
   pChain->msgQty   = bytesToUint32(&pChainStart[FTM_CHAIN_MSGQTY]);
   pChain->msgIdx   = bytesToUint32(&pChainStart[FTM_CHAIN_MSGIDX]);
   //allocate space for this chains messages
   pChain->pMsg     = calloc(pChain->msgQty, sizeof(t_ftmMsg));
   pChain->pNext    = (struct t_ftmChain*)pNext;
   //deserialise messages until msgQty is reached
   pBuf = &pBuf[FTM_CHAIN_END_];
   for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++) pBuf = deserMsg(pBuf, &pChain->pMsg[msgIdx]);   
   //set pBuf ptr to 
   
   return pBufStart + (uintptr_t)bytesToUint32(&pChainStart[FTM_CHAIN_PNEXT]);
   
}

static uint8_t* deserMsg(uint8_t* pBuf, t_ftmMsg* pMsg)
{
   //printf("desermsg\n");   
   pMsg->id    = bytesToUint64(&pBuf[FTM_MSG_ID]);
   pMsg->par   = bytesToUint64(&pBuf[FTM_MSG_PAR]); 
   pMsg->tef   = bytesToUint32(&pBuf[FTM_MSG_TEF]);
   pMsg->res   = bytesToUint32(&pBuf[FTM_MSG_RES]);
   pMsg->ts    = bytesToUint64(&pBuf[FTM_MSG_TS]); 
   pMsg->offs  = bytesToUint64(&pBuf[FTM_MSG_OFFS]); 
   
   return pBuf + (uintptr_t)FTM_MSG_END_; 
}

t_ftmPage* freePage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx;
   t_ftmChain* pChain   = NULL;
   t_ftmChain* pNext    = NULL;
   t_ftmMsg*   pMsg     = NULL;
                    
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      chainIdx = 0;
      pChain = pPage->plans[planIdx].pStart;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         pMsg = pChain->pMsg;
         free(pMsg);
         pNext = (t_ftmChain*)pChain->pNext;
         free(pChain);
         pChain = pNext;
      }
   }
   free(pPage);
   return NULL; 
}



//DEBUG functions, mainly for FESA bullshit
t_ftmChain* getChain(t_ftmPage* pPage, uint32_t planIdx, uint32_t chainIdx)
{
   uint32_t cIdx = 0;
   t_ftmChain* pChain   = NULL;

   if(chainIdx < pPage->plans[planIdx].chainQty) {
   
      pChain = pPage->plans[planIdx].pStart;
      while(cIdx++ < chainIdx && pChain != NULL) {pChain = (t_ftmChain*)pChain->pNext;}
   }  
   
   return pChain;
}

t_ftmMsg* getMsg(t_ftmChain* pChain, uint32_t msgIdx)
{
   t_ftmMsg*   pMsg     = NULL;
   
   if(pChain != NULL) {
      pMsg = pChain->pMsg;
      if(msgIdx < pChain->msgQty) pMsg += (uintptr_t)msgIdx;
   }
   
   return pMsg;
}



int showFtmPage(t_ftmPage* pPage, char* buff)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain   = NULL;
   t_ftmMsg*   pMsg     = NULL;
   char noFlag[]        = "        -        ";
   char* sB = buff; //current string buffer pointer                
   
   SNTPRINTF(sB , "---PAGE \nStartPlan:\t");
   if(pPage->idxStart == 0xdeadbeef) SNTPRINTF(sB , "idle\n");
   else { 
          if(pPage->idxStart == 0xcafebabe)  SNTPRINTF(sB , "NULL\n");
          else                               SNTPRINTF(sB , "%c\n", pPage->idxStart + 'A');
        } 
   
   SNTPRINTF(sB , "AltPlan:\t");
   if(pPage->idxBp == 0xdeadbeef) SNTPRINTF(sB , "idle\n");
   else { 
          if(pPage->idxBp == 0xcafebabe)  SNTPRINTF(sB , "NULL\n");
          else                            SNTPRINTF(sB , "%c\n", pPage->idxBp + 'A');
        }  
   
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      SNTPRINTF(sB , "\t*******************************************************************************************************\n");
      SNTPRINTF(sB , "\t---   PLAN %c\n", planIdx+'A');
      SNTPRINTF(sB , "\t*******************************************************************************************************\n");
      chainIdx = 0;
      pChain = pPage->plans[planIdx].pStart;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         SNTPRINTF(sB , "\t\t-----------------------------------------------------------------------------------------------\n");
         SNTPRINTF(sB , "\t\t---   CHAIN %c%u\n", planIdx+'A', chainIdx-1);
         SNTPRINTF(sB , "\t\t-----------------------------------------------------------------------------------------------\n");
         SNTPRINTF(sB , "\t\tFlags: 0x%08x\t", pChain->flags );                            
         if(pChain->flags & FLAGS_IS_BP)                 SNTPRINTF(sB , "      IS_BP      "); else SNTPRINTF(sB , "%s", noFlag);
         if(pChain->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED))
         {
              if(pChain->flags & FLAGS_IS_COND_SHARED)   SNTPRINTF(sB , " IS_COND_SHA");
              if(pChain->flags & FLAGS_IS_COND_MSI)      SNTPRINTF(sB , " IS_COND_MSI");
              if(pChain->flags & FLAGS_IS_COND_ALL)      SNTPRINTF(sB , "_ALL ");
              else                                       SNTPRINTF(sB , "     ");
         } else SNTPRINTF(sB , "%s", noFlag);      
         if(pChain->flags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED))
         {
              if(pChain->flags & FLAGS_IS_SIG_SHARED)    SNTPRINTF(sB , " IS_SIG_SHA");
              if(pChain->flags & FLAGS_IS_SIG_MSI)       SNTPRINTF(sB , " IS_SIG_MSI");
              if(pChain->flags & FLAGS_IS_SIG_ALL)       SNTPRINTF(sB , "_ALL  ");
              else                                       SNTPRINTF(sB , "      ");
         } else SNTPRINTF(sB , "%s", noFlag); 
         
         if(pChain->flags & FLAGS_IS_END)       SNTPRINTF(sB , "      IS_END     "); else SNTPRINTF(sB , "%s", noFlag);
         if(pChain->flags & FLAGS_IS_ENDLOOP)   SNTPRINTF(sB , "      IS_LOOP    "); else SNTPRINTF(sB , "%s", noFlag);
         SNTPRINTF(sB , "\n");
         
         SNTPRINTF(sB , "\t\tStart:\t\t%18llu\n\t\tperiod:\t\t%18llu\n\t\trep:\t\t\t%10u\n\t\tmsg:\t\t\t%10u\n", 
         (long long unsigned int)(pChain->tStart), (long long unsigned int)(pChain->tPeriod), pChain->repQty, pChain->msgQty);
       
         SNTPRINTF(sB , "\t\tCondVal:\t\t0x%08x\n\t\tCondMsk:\t\t0x%08x\n\t\tSigDst:\t\t\t0x%08x\n\t\tSigVal:\t\t\t0x%08x\n", 
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            SNTPRINTF(sB , "\t\t\t---MSG %c%u%c\n", planIdx+'A', chainIdx-1, msgIdx+'A');
            SNTPRINTF(sB , "\t\t\tid:\t0x%08x%08x", (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id);
            SNTPRINTF(sB , "\t%u\t%u\t%u\t%u\t%u\t%u\n", getIdFID(pMsg[msgIdx].id), getIdGID(pMsg[msgIdx].id),
            getIdEVTNO(pMsg[msgIdx].id), getIdSID(pMsg[msgIdx].id), getIdBPID(pMsg[msgIdx].id), getIdSCTR(pMsg[msgIdx].id));
            SNTPRINTF(sB , "\t\t\tpar:\t0x%08x%08x\n\t\t\ttef:\t\t0x%08x\n", 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par, pMsg[msgIdx].tef);
            SNTPRINTF(sB , "\t\t\toffs:\t%18llu \n", (long long unsigned int)((pMsg[msgIdx].offs) + (uint64_t)(pMsg[msgIdx].tef >> 29)));  
         }
         pChain = (t_ftmChain*)pChain->pNext;
      }
   }   
   SNTPRINTF(sB , "\n\n");
   
   return (int)(sB-buff);
}

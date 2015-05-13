#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ftmx86.h"
#include <etherbone.h>
#include "access.h"

extern uint8_t cpuIdx;
extern bool bigEndian = true;
extern t_ftmAccess* pAccess;
uint16_t getIdFID(uint64_t id)     {return ((uint16_t)(id >> ID_FID_POS))     & (ID_MSK_B16 >> (16 - ID_FID_LEN));}
uint16_t getIdGID(uint64_t id)     {return ((uint16_t)(id >> ID_GID_POS))     & (ID_MSK_B16 >> (16 - ID_GID_LEN));}
uint16_t getIdEVTNO(uint64_t id)   {return ((uint16_t)(id >> ID_EVTNO_POS))   & (ID_MSK_B16 >> (16 - ID_EVTNO_LEN));}
uint16_t getIdSID(uint64_t id)     {return ((uint16_t)(id >> ID_SID_POS))     & (ID_MSK_B16 >> (16 - ID_SID_LEN));}
uint16_t getIdBPID(uint64_t id)    {return ((uint16_t)(id >> ID_BPID_POS))    & (ID_MSK_B16 >> (16 - ID_BPID_LEN));}
uint16_t getIdSCTR(uint64_t id)    {return ((uint16_t)(id >> ID_SCTR_POS))    & (ID_MSK_B16 >> (16 - ID_SCTR_LEN));}

static uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart, uint32_t embeddedOffs);
static uint8_t* deserMsg(uint8_t* pBuf, t_ftmMsg* pMsg);

static uint8_t* serChain(t_ftmChain* pChain, uint32_t pPlanStart, uint8_t* pBufStart, uint8_t* pBuf, uint32_t embeddedOffs, uint8_t cpuId);
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


                          
static uint8_t* uint32ToBytes(uint8_t* pBuf, uint32_t val)
{
   uint8_t i;
   if(bigEndian)  
   for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*i) & 0xff;
   else           for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*(FTM_WORD_SIZE - 1 -i)) & 0xff;
   
   return pBuf+4;
}

static uint8_t* uint64ToBytes(uint8_t* pBuf, uint64_t val)
{
  uint32ToBytes(pBuf+0, (uint32_t)(val>>32));
  uint32ToBytes(pBuf+4, (uint32_t)val);

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

uint8_t* serPage (t_ftmPage*  pPage, uint8_t* pBufStart, uint32_t embeddedOffs, uint8_t cpuId) 
{
   printf("Writing ptrPage %p to ptrWrite %p with offset %08x for cpu %u\n", pPage, pBufStart, embeddedOffs, cpuId);
   uint8_t j,  planIdx, chainIdx;
   uint8_t*    pBuf;
   t_ftmChain* pChain;
   uint32_t    pBufPlans[FTM_PLAN_MAX];
   if(bigEndian) printf("BIGENDIAN FOUND!\n");
   pBuf = pBufStart + FTM_PAGEMETA;
   //leave space for page meta info, write all plans to pBuffer
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      pBufPlans[planIdx] = embeddedOffs + ((uint32_t)((uintptr_t)pBuf - (uintptr_t)pBufStart));
      pChain   = pPage->plans[planIdx].pStart;
      chainIdx = 0;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         pBuf     = serChain(pChain, pBufPlans[planIdx], pBufStart, pBuf, embeddedOffs, cpuId);
         pChain   = (t_ftmChain*)pChain->pNext;
      }   
   }
   
   // now write page meta
   uint32ToBytes(&pBufStart[FTM_PAGE_QTY], pPage->planQty);
   for(j=0;j<pPage->planQty;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], pBufPlans[j]);
   for(j=pPage->planQty;j<FTM_PLAN_MAX;    j++)
      uint32ToBytes(&pBufStart[FTM_PAGE_PLANPTRS + j*FTM_WORD_SIZE], FTM_NULL);
   
   if( pPage->idxBp == 0xdeadbeef)    pPage->pBp = FTM_SHARED_OFFSET + FTM_IDLE_OFFSET;
   else pPage->pBp     = pBufPlans[pPage->idxBp];
   
   if( pPage->idxStart == 0xdeadbeef) pPage->pStart = FTM_SHARED_OFFSET + FTM_IDLE_OFFSET;
   else pPage->pStart  = pBufPlans[pPage->idxStart];
   
   printf("BP: %08x, Start: %08x\n", pPage->pBp, pPage->pStart);
   
   uint32ToBytes(&pBufStart[FTM_PAGE_PTR_BP],         pPage->pBp);
   uint32ToBytes(&pBufStart[FTM_PAGE_PTR_START],      pPage->pStart);
   //printf ("Shared mem will be at %08x", pPage->ppAccess->sharedAdr);
   //uint32ToBytes(&pBufStart[FTM_PAGE_PTR_pAccess->sharedAdr],  pPage->ppAccess->sharedAdr);
   
   return pBuf;   
}

static uint8_t* serChain(t_ftmChain* pChain, uint32_t pPlanStart, uint8_t* pBufStart, uint8_t* pBuf, uint32_t embeddedOffs, uint8_t cpuId)
{
   uint8_t msgIdx;
   
   uint32_t pBufNext, pBufMsg, sigDst, condSrc;
   t_ftmMsg* pMsg = pChain->pMsg; 
   
   sigDst   = 0;
   condSrc  = 0;
   
   //FIXME: change to proper sdb find
   if (pChain->flags & FLAGS_IS_SIG_MSI)           sigDst = 0x40000800 + pChain->sigCpu * 0x100; //FIXME !!!!
   else if (pChain->flags & FLAGS_IS_SIG_SHARED)   sigDst = pAccess->sharedAdr  + pChain->sigCpu * 0xC; 
   else if (pChain->flags & FLAGS_IS_SIG_ADR)      sigDst = pChain->sigDst;
   
   //FIXME: change to proper sdb find
   if (pChain->flags & FLAGS_IS_COND_MSI)          condSrc = 0x0;
   else if (pChain->flags & FLAGS_IS_COND_SHARED)  { condSrc = pAccess->sharedAdr + cpuId * 0xC; printf("SharedCondLoc: 0x%08x\n", condSrc);}
   else if (pChain->flags & FLAGS_IS_COND_ADR)     condSrc = pChain->condSrc;
   
   uint64ToBytes(&pBuf[FTM_CHAIN_TSTART],    pChain->tStart);
   uint64ToBytes(&pBuf[FTM_CHAIN_TPERIOD],   pChain->tPeriod);
   uint64ToBytes(&pBuf[FTM_CHAIN_TEXEC],     0);
   uint32ToBytes(&pBuf[FTM_CHAIN_FLAGS],     pChain->flags);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDSRC],   condSrc);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDVAL],   pChain->condVal);
   uint32ToBytes(&pBuf[FTM_CHAIN_CONDMSK],   pChain->condMsk);
   uint32ToBytes(&pBuf[FTM_CHAIN_SIGDST],    sigDst);
   uint32ToBytes(&pBuf[FTM_CHAIN_SIGVAL],    pChain->sigVal);
   uint32ToBytes(&pBuf[FTM_CHAIN_REPQTY],    pChain->repQty);
   uint32ToBytes(&pBuf[FTM_CHAIN_REPCNT],    0);
   uint32ToBytes(&pBuf[FTM_CHAIN_MSGQTY],    pChain->msgQty);
   uint32ToBytes(&pBuf[FTM_CHAIN_MSGIDX],    0);
   
   pBufMsg  = embeddedOffs + FTM_CHAIN_END_ + ( (uint32_t)( (uintptr_t)pBuf - (uintptr_t)pBufStart ) );
   uint32ToBytes(&pBuf[FTM_CHAIN_PMSG],    pBufMsg);
   
   if(pChain->pNext != NULL) {
      if(((pChain->flags & FLAGS_IS_END) && (pChain->flags & FLAGS_IS_ENDLOOP)))   pBufNext = (uint32_t)pPlanStart;
      else                                                                         pBufNext = pBufMsg + pChain->msgQty * FTM_MSG_END_;
   } 
   else pBufNext = FTM_NULL;
   uint32ToBytes(&pBuf[FTM_CHAIN_PNEXT],    pBufNext);
 
   pBuf += FTM_CHAIN_END_;
   for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++) pBuf = serMsg(&pMsg[msgIdx], pBuf);   
   printf("Chain placed @ 0x%08x\n", pBuf-pBufStart);
   return pBuf;   

}

static uint8_t* serMsg(  t_ftmMsg* pMsg, uint8_t* pBuf)
{
   uint64ToBytes(&pBuf[FTM_MSG_ID],     pMsg->id);
   uint64ToBytes(&pBuf[FTM_MSG_PAR],    pMsg->par);
   uint32ToBytes(&pBuf[FTM_MSG_TEF],    pMsg->tef); 
   uint32ToBytes(&pBuf[FTM_MSG_RES],    pMsg->res); 
   uint64ToBytes(&pBuf[FTM_MSG_TS],     0);
   uint64ToBytes(&pBuf[FTM_MSG_OFFS],   pMsg->offs);
   
   return pBuf + FTM_MSG_END_;
}

t_ftmPage* deserPage(t_ftmPage* pPage, uint8_t* pBufStart, uint32_t embeddedOffs)
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
       
      pBufPlans[j]   = pBufStart + (uintptr_t)planStart - (uintptr_t)embeddedOffs;
      
      //allocate first chain and a Next Chain
      chainQty = 1;
      pChain   = calloc(1, sizeof(t_ftmChain));
      pNext    = calloc(1, sizeof(t_ftmChain));
      //set plan start to first chain
      pPage->plans[j].pStart = pChain;
      //deserialise (pBufStart +  offset) to pChain and fix pChains next ptr
      pBuf = deserChain(pChain, pNext, pBufPlans[j], pBufStart, embeddedOffs);
      printf("PChain %p Start %p offs %08x\n", pChain, pBufStart, embeddedOffs);
      //printf("Plan %u Chain 0 @ %08x\n", j, pBufStart-pBuf);
      //deserialise chains until we reached the end or we reached max
      
      while(!(pChain->flags & FLAGS_IS_END) && chainQty < 10)
      {
         pChain   = pNext;
         pNext    = calloc(1, sizeof(t_ftmChain));
         pBuf     = deserChain(pChain, pNext, pBuf, pBufStart, embeddedOffs);
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
   else if  (pPage->pBp    == FTM_SHARED_OFFSET + FTM_IDLE_OFFSET)  {pPage->idxBp  = 0xdeadbeef;}
   else for (j=0; j<pPage->planQty; j++)      
   {
      if(pBufPlans[j]-pBufStart == ((uintptr_t)pPage->pBp - (uintptr_t)embeddedOffs)) {pPage->idxBp = j;}
   }  
      
   if       (pPage->pStart == FTM_NULL)           {pPage->idxStart = 0xcafebabe;}
   else if  (pPage->pStart == FTM_SHARED_OFFSET + FTM_IDLE_OFFSET)    {pPage->idxStart = 0xdeadbeef;}
   else for(j=0; j<pPage->planQty; j++) 
   {
      if(pBufPlans[j]-pBufStart == ((uintptr_t)pPage->pStart - (uintptr_t)embeddedOffs)) {pPage->idxStart = j;}
   }    
   //pPage->ppAccess->sharedAdr = pBufStart[FTM_PAGE_PTR_pAccess->sharedAdr];    
      
   return pPage;     
}

static uint8_t* deserChain(t_ftmChain* pChain, t_ftmChain* pNext, uint8_t* pChainStart, uint8_t* pBufStart, uint32_t embeddedOffs)
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
   
   return pBufStart + (uintptr_t)bytesToUint32(&pChainStart[FTM_CHAIN_PNEXT]) - (uintptr_t)embeddedOffs;
   
}

static uint8_t* deserMsg(uint8_t* pBuf, t_ftmMsg* pMsg)
{
   //printf("desermsg\n");   
   pMsg->id    = bytesToUint64(&pBuf[FTM_MSG_ID]);
   pMsg->par   = bytesToUint64(&pBuf[FTM_MSG_PAR]); 
   pMsg->tef   = bytesToUint64(&pBuf[FTM_MSG_TEF]);
   pMsg->res   = bytesToUint64(&pBuf[FTM_MSG_RES]);
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



void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain   = NULL;
   t_ftmMsg*   pMsg     = NULL;
   char noFlag[]        = "        -        ";
                    
   printf("---PAGE \n");
   printf("StartPlan:\t");
   
   if(pPage->idxStart == 0xdeadbeef) printf("idle\n");
   else { 
          if(pPage->idxStart == 0xcafebabe)  printf("NULL\n");
          else                               printf("%c\n", pPage->idxStart + 'A');
        } 
   
   printf("AltPlan:\t");
   if(pPage->idxBp == 0xdeadbeef) printf("idle\n");
   else { 
          if(pPage->idxBp == 0xcafebabe)  printf("NULL\n");
          else                            printf("%c\n", pPage->idxBp + 'A');
        }  
   
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      printf("\t*******************************************************************************************************\n");
      printf("\t---   PLAN %c\n", planIdx+'A');
      printf("\t*******************************************************************************************************\n");
      chainIdx = 0;
      pChain = pPage->plans[planIdx].pStart;
      while(chainIdx++ < pPage->plans[planIdx].chainQty && pChain != NULL)
      {
         printf("\t\t-----------------------------------------------------------------------------------------------\n");
         printf("\t\t---   CHAIN %c%u\n", planIdx+'A', chainIdx-1);
         printf("\t\t-----------------------------------------------------------------------------------------------\n");
         printf("\t\tFlags: 0x%08x\t", pChain->flags );                            
         if(pChain->flags & FLAGS_IS_BP)                 printf("      IS_BP      "); else printf("%s", noFlag);
         if(pChain->flags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED))
         {
              if(pChain->flags & FLAGS_IS_COND_SHARED)   printf(" IS_COND_SHA");
              if(pChain->flags & FLAGS_IS_COND_MSI)      printf(" IS_COND_MSI");
              if(pChain->flags & FLAGS_IS_COND_ALL)      printf("_ALL ");
              else                                       printf("     ");
         } else printf("%s", noFlag);      
         if(pChain->flags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED))
         {
              if(pChain->flags & FLAGS_IS_SIG_SHARED)    printf(" IS_SIG_SHA");
              if(pChain->flags & FLAGS_IS_SIG_MSI)       printf(" IS_SIG_MSI");
              if(pChain->flags & FLAGS_IS_SIG_ALL)       printf("_ALL  ");
              else                                       printf("      ");
         } else printf("%s", noFlag); 
         
         if(pChain->flags & FLAGS_IS_END)       printf("      IS_END     "); else printf("%s", noFlag);
         if(pChain->flags & FLAGS_IS_ENDLOOP)   printf("      IS_LOOP    "); else printf("%s", noFlag);
         printf("\n");
         printf("\t\tStart:\t\t0x%08x%08x\n\t\tperiod:\t\t0x%08x%08x\n\t\trep:\t\t\t%10u\n\t\tmsg:\t\t\t%10u\n", 
         (uint32_t)(pChain->tStart>>32), (uint32_t)pChain->tStart, 
         (uint32_t)(pChain->tPeriod>>32), (uint32_t)pChain->tPeriod,
         pChain->repQty,
         pChain->msgQty);
       
         printf("\t\tCondVal:\t\t0x%08x\n\t\tCondMsk:\t\t0x%08x\n\t\tSigDst:\t\t\t0x%08x\n\t\tSigVal:\t\t\t0x%08x\n", 
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            printf("\t\t\t---MSG %c%u%c\n", planIdx+'A', chainIdx-1, msgIdx+'A');
            printf("\t\t\tid:\t0x%08x%08x\n\t\t\tpar:\t0x%08x%08x\n\t\t\ttef:\t\t0x%08x\n\t\t\toffs:\t0x%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id, 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         pChain = (t_ftmChain*)pChain->pNext;
      }
   }   
   printf("\n\n");
}

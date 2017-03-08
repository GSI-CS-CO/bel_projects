#include <stdlib.h>
#include <stdio.h>
#include "block.h"

const char sUnknown[]  = "Unknown  ";
const char sTmsg[]     = "TimingMsg";
const char sCmd[]      = "Command  ";

const char *sEvtType[4] = {sUnknown, sTmsg, sCmd, sUnknown};


static uint8_t* uint32ToBytes(uint8_t* pBuf, uint32_t val)
{
   uint8_t i;
   for(i=0;i<FTM_WORD_SIZE;   i++) pBuf[i]  = val >> (8*i) & 0xff;
   
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


Block::Block(uint8_t *buf, uint32_t len) {
  uint8_t *pC, *pEnd;
  

  
  uint64_t tOffs, id, par, tValid;
  uint32_t tef, act;
  uint16_t type, flags;
  Event*   pE;

  pC = buf;

  tOffs = bytesToUint64(pC + EVT_OFFS_TIME);  // get time offset
  type  = bytesToUint16(pC + EVT_TYPE);       // get type
  flags = bytesToUint16(pC + EVT_FLAGS);      // get flags

  switch(type) {
    case EVT_TYPE_TMSG: { id  = bytesToUint64(pC + EVT_TM_ID);
                          par = bytesToUint64(pC + EVT_TM_PAR);  
                          tef = bytesToUint64(pC + EVT_TM_PAR);
                          pE = new TimingMsg::TimingMsg(tOffs, flags, id, par, tef); break;}          
    case EVT_TYPE_CMD:  { tValid = bytesToUint64(pC + EVT_CM_TIME);
                          act    = bytesToUint32(pC + EVT_CM_AC);  
                          pE = new Command::Command(tOffs, flags, tValid, act); break;}  
    case default:       { pE = NULL; break;}
  }

  return pE;

}

static int cntEvt(t_event *el) {
  int res = 0;
  t_event *e = el;
  uint8_t detectRing = 0;
  if (e == NULL) return 0;

  while (e != NULL) {
    if (e == el) {
      if (detectRing) {break;}
      detectRing = 1;
    };
    e = (t_event*)e->next;
    res++;
  }
  return res;
}

int showEvtList(t_event *el, const char* prefix) {
  char* p;
  if (prefix == NULL) p = "";
  else p = (char*)prefix;
  int cnt     = 0;
  int cntTmsg = 0;
  int cntCmd  = 0;
  t_event *e = el;
  uint8_t detectRing = 0;

  printf("\n%s### Event List:\n", p);
  while (e != NULL) {
    if ((e == el) & detectRing) {printf("%s!!! WARNING: Eventlist forms a ring! Exiting Show function\n", p); break;}
    if (e->type == EVT_TYPE_CMD)  cntCmd++;
    if (e->type == EVT_TYPE_TMSG) cntTmsg++;
    if (e == el) detectRing = 1;
    e = (t_event*)showEvt(e, cnt, prefix)->next;
    cnt++;
    printf("\n");
  }
  printf("%s### %u Events, %u Tmsgs, %u Cmds\n\n", p, cnt, cntTmsg, cntCmd);
  return cnt;
}   

t_block* createBlock(uint64_t period, uint64_t tStart, uint16_t flags, t_block* prev, t_event* el) {

  t_block* b   = calloc(1, sizeof(t_block));
  int cnt      = cntEvt(el);
  if (b != NULL) { 
    b->period  = period;
    b->tStart  = tStart;
    b->flags   = flags;

    // get border of events - is there a Q?
    b->qOffs   = _BLOCK_HDR_SIZE + cnt * _EVT_SIZE;
    b->next    = NULL;
    if (prev != NULL) prev->next = (struct t_block*)b;
    b->evt     = el;

    b->eQty    = cnt;
    b->eCnt    = 0;
    b->tCurr   = 0;
  }

  return b;
}


t_block* showBlock(t_block *b, uint32_t cnt) {
  if (b != NULL) {
    printf("***------- b%3u -------\n", cnt);
    printf("*** Period %llu @ ", (long long unsigned int)b->period);
    switch((b->flags >> BLOCK_FLAGS_START_POS) & BLOCK_FLAGS_START_MSK) {
	    case BLOCK_START_AT:  printf("Starts @ %llu\n", (long long unsigned int)b->tStart); break;
	    case BLOCK_START_NOW: printf("Starts @ next execution\n"); break;
      case BLOCK_START_PPS: printf("Starts @ next PPS > 1s\n"); break;
	    default: printf("!!! Invalid Blockstart type %u\n", (b->flags >> BLOCK_FLAGS_START_POS) & BLOCK_FLAGS_START_MSK); break;
    }
    //Flags
    if (b->flags & BLOCK_FLAGS_HAS_Q) printf("CMDQ ");
    else                              printf("  -  ");
    printf("\n");

    if (b->flags & BLOCK_FLAGS_HAS_Q) printf("Queue %2u deep, Offs %u\n", CMDQ_DEPTH, b->qOffs);  
    printf("Current Time %llu\n", (long long unsigned int)b->tCurr);    
    printf("EvtQty %u, EvtCnt %u\n", b->eQty, b->eCnt);
    printf("Idx %u, Successor %u\n", b->thisIdx, b->nextIdx);
    showEvtList(b->evt, "  ");
   
  } else {
    printf("*** You gave me a NULL Block to show ...\n");
  }
  return b;
}


static int cntBlock(t_block *bl) {
  int res = 0;
  t_block *b = bl;
  uint8_t detectRing = 0;
  if (b == NULL) return 0;

  while (b != NULL) {
    if (b == bl) {
      if (detectRing) {break;}
      detectRing = 1;
    };
    b = (t_block*)b->next;
    res++;
  }
  return res;
}

int showBlockList(t_block *bl) {
  int cnt    = 0;
  t_block *b = bl;
  uint8_t detectRing = 0;

  printf("\n### Block List:\n");
  while (b != NULL) {
    if ((b == bl) & detectRing) {printf("!!! WARNING: Eventlist forms a ring! Exiting Show function\n"); break;}
    if (b == bl) detectRing = 1;
    b = (t_block*)showBlock(b, cnt)->next;
    printf("\n");
    cnt++;
  }
  printf("### %u Blocks\n\n", cnt);
  return cnt;
}    


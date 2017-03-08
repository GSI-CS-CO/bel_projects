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


t_event* createEvt(uint64_t tOffs, uint16_t type, uint16_t flags, t_event *prev) {
  t_event *e    = calloc(1, sizeof(t_event));
  if (e != NULL) {
    e->tOffs    = tOffs;
    e->type     = type;
    e->flags    = flags;
    e->next     = NULL;
    if (prev != NULL) prev->next = (struct t_event*)e;
  } else {printf("failed to allocate Event!\n");}
  return e;
}

t_event* createCmd(uint64_t tOffs, uint16_t flags, t_event *prev, uint64_t tValid, uint32_t act) {
  t_event *e = createEvt(tOffs, EVT_TYPE_CMD, flags, prev);
  if (e != NULL) {   
    e->c.tValid = tValid;
    e->c.act    = act;
  } else {printf("failed to allocate Command!\n");}
  return e;
}

t_event* createTmsg(uint64_t tOffs, uint16_t flags, t_event *prev, uint64_t id, uint64_t par, uint32_t tef) {
  t_event *e = createEvt(tOffs, EVT_TYPE_TMSG, flags, prev);
  if (e != NULL) {    
    e->t.id  = id;
    e->t.par = par;
    e->t.tef = tef;
  }
  return e;
}

t_event* showEvt(t_event *e, uint32_t cnt, const char* prefix) {
  char* p;
  if (prefix == NULL) p = "";
  else p = (char*)prefix;

  if (e != NULL) {
    printf("%s***------- %3u -------\n", p, cnt);
    printf("%s*** %s @ %llu\n", p, sEvtType[e->type & 0x3], (long long unsigned int)e->tOffs);
    switch(e->type & 0x3) {
	    case EVT_TYPE_CMD:  printf("%sValid @ %llu, Action 0x%08x\n", p, (long long unsigned int)e->c.tValid, e->c.act); break;
	    case EVT_TYPE_TMSG: printf("%sID 0x%08x%08x, Par 0x%08x%08x, Tef 0x%08x\n", p, (uint32_t)(e->t.id >> 32), (uint32_t)e->t.id, (uint32_t)(e->t.par >> 32), (uint32_t)e->t.par, e->t.tef); break;
	    default: printf("%s!!! Invalid Event type %u\n", p, e->type); break;
    }
  } else {
    printf("%s*** You gave me a NULL Evt to show ...\n", p);
  }
  return e;
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


#ifndef _BLOCK_H_
#define _BLOCK_H_
#include <stdint.h>
#include "../ftm_common.h"

extern const char *sEvtType[4];

typedef struct {
  uint64_t ts;
  uint32_t act;
} t_cmd;

typedef struct {
  uint8_t rdIdx;
  uint8_t wrIdx;
  t_cmd cmd[CMDQ_DEPTH]; 
} t_cmdq;

typedef struct {
  t_cmdq il;
  t_cmdq hi;
  t_cmdq lo;
} t_cmdqs;

typedef struct {
  uint64_t tValid;
  uint32_t act;
  uint32_t dummy[2];
} t_event_cmd;

typedef struct {
  uint64_t id;
  uint64_t par;
  uint32_t tef;
} t_event_tmsg;


typedef struct {
  uint64_t tOffs;  
  uint16_t type;
  uint16_t flags;
  union {
    t_event_tmsg t;
    t_event_cmd  c;
  };
  
  struct t_event *next; 
} t_event;

typedef struct {
  uint64_t period;
  uint64_t tStart;
  uint16_t flags;
  uint16_t qOffs;  
  uint32_t thisIdx;  
  uint32_t nextIdx;
  t_event  *evt; 
  struct t_block *next;
  t_cmdqs  q;

  uint32_t eQty;
  uint32_t eCnt;
  uint64_t tCurr;

} t_block;


t_event* createEvt(uint64_t tOffs, uint16_t type, uint16_t flags, t_event *prev);
t_event* createCmd(uint64_t tOffs, uint16_t flags, t_event *prev, uint64_t tValid, uint32_t act);
t_event* createTmsg(uint64_t tOffs, uint16_t flags, t_event *prev, uint64_t id, uint64_t par, uint32_t tef);
t_event* showEvt(t_event *e, uint32_t cnt, const char* prefix);
int showEvtList(t_event *el, const char* prefix);


t_block* createBlock(uint64_t period, uint64_t tStart, uint16_t flags, t_block* prev, t_event* el);
t_block* showBlock(t_block *b, uint32_t cnt);
int showBlockList(t_block *bl);


#endif

////////////////////////////////////////////////////////////////
//Triple Command Queue Structure
////////////////////////////////////////////////////////////////
#ifndef _CMDQ_H_
#define _CMDQ_H_

#include <stdint.h>
#include "../ftm_common.h"

extern const uint8_t cOff[4];

uint32_t createFlow(uint32_t nextIdx, uint32_t qty);
uint32_t createNop(uint32_t qty);
uint32_t createFlush(uint8_t flushHi, uint8_t flushIdxHi, uint8_t flushLo, uint8_t flushIdxLo);
uint32_t* pushQ(uint32_t* pQs, uint32_t priority, uint32_t action, uint64_t time);
int32_t checkQs(uint32_t* pQs, int32_t idxnext, uint64_t now);
  
#endif

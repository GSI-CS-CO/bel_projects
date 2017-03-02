////////////////////////////////////////////////////////////////
//Triple Command Queue Structure
////////////////////////////////////////////////////////////////
#ifndef CMDQ_H
#define CMDQ_H

#include <stdint.h>


#define PRIO_LO 0
#define PRIO_HI 1
#define PRIO_IL 2

//CMD structure
#define CMD_TS_OFF            0x0
#define CMD_ACT_OFF           (CMD_TS_OFF  + 8)
#define _CMD_SIZE             (CMD_ACT_OFF + 4)

//Action substructure
#define CMD_TYPE_MSK          0xf0000000
#define ACT_TYPE_NOP          0x00000000
#define ACT_TYPE_FLOW         0x10000000
#define ACT_TYPE_FLUSH        0x20000000 // save the bitshift
#define ACT_TYPE_POS          28

//Action of Nop type
#define ACT_FNF_QTY_MSK       0x0000ffff
#define ACT_FNF_QTY_POS       0
//Action of Flow type
#define ACT_FLOW_NEXT_MSK     0x0fff0000
#define ACT_FLOW_NEXT_POS     16
//Action of Flush type
#define ACT_FLUSH_LO_MSK      0x00010000
#define ACT_FLUSH_LO_POS      16 
#define ACT_FLUSH_HI_MSK      0x00020000
#define ACT_FLUSH_HI_POS      17   
#define ACT_FLUSH_LO_RNG_MSK  0x000000ff
#define ACT_FLUSH_LO_RNG_POS  0
#define ACT_FLUSH_HI_RNG_MSK  0x0000ff00
#define ACT_FLUSH_HI_RNG_POS  8

//CMDQ structure
#define CMDQ_QTY              4
#define _CMDQ_BUF_SIZE             (CMDQ_QTY * _CMD_SIZE)
#define CMDQ_IDX_OF_MSK       (2 * CMDQ_QTY -1) // x2 for overflow bit
#define CMDQ_IDX_MSK          (CMDQ_QTY -1) // 
#define _CMDQ_IDX_SIZE        4
#define CMDQ_OFF              0
#define CMDQ_RD_OFF           (CMDQ_OFF)
#define CMDQ_WR_OFF           (CMDQ_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_BUF_OFF          (CMDQ_WR_OFF  + _CMDQ_IDX_SIZE)
#define _CMDQ_SIZE            (CMDQ_BUF_OFF + _CMDQ_BUF_SIZE)

////////////////////////////////////////////////////////////////
//CMDQS structure
////////////////////////////////////////////////////////////////
//Interlock Queue
#define CMDQS_OFF             0
#define CMDQ_LO_RD_OFF        (CMDQS_OFF       + 0)
#define CMDQ_LO_OFF           (CMDQ_LO_RD_OFF)
#define CMDQ_LO_WR_OFF        (CMDQ_LO_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_LO_BUF_OFF       (CMDQ_LO_WR_OFF  + _CMDQ_IDX_SIZE)
//High Priority Queue
#define CMDQ_HI_RD_OFF        (CMDQ_LO_BUF_OFF + _CMDQ_BUF_SIZE)
#define CMDQ_HI_OFF           (CMDQ_HI_RD_OFF)
#define CMDQ_HI_WR_OFF        (CMDQ_HI_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_HI_BUF_OFF       (CMDQ_HI_WR_OFF  + _CMDQ_IDX_SIZE)
//Low Priority Queue
#define CMDQ_IL_RD_OFF        (CMDQ_HI_BUF_OFF + _CMDQ_BUF_SIZE)
#define CMDQ_IL_OFF           (CMDQ_IL_RD_OFF)
#define CMDQ_IL_WR_OFF        (CMDQ_IL_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_IL_BUF_OFF       (CMDQ_IL_WR_OFF  + _CMDQ_IDX_SIZE)
#define CMDQS_SIZE            (CMDQ_IL_BUF_OFF + _CMDQ_BUF_SIZE)

extern const uint8_t cOff[4];

uint32_t createFlow(uint32_t nextIdx, uint32_t qty);
uint32_t createNop(uint32_t qty);
uint32_t createFlush(uint8_t flushHi, uint8_t flushIdxHi, uint8_t flushLo, uint8_t flushIdxLo);
uint32_t* pushQ(uint32_t* pQs, uint32_t priority, uint32_t action, uint64_t time);
int32_t checkQs(uint32_t* pQs, int32_t idxnext, uint64_t now);
  
#endif

#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>
#include "ftm_common_legacy.h"

//////////////////////////////////////////////////////////////////////
//  DM FPGA Control                                                 //
//////////////////////////////////////////////////////////////////////
#define FTM_RST_FPGA        0x0                                     //              
#define FTM_RST_FPGA_CMD    0xDEADBEEF                              //
#define FTM_RST_GET         (FTM_RST_FPGA + 4)                      //
#define FTM_RST_SET         (FTM_RST_GET  + 4)                      //
#define FTM_RST_CLR         (FTM_RST_SET  + 4)                      //
                                                                    //
#define FTM_SHARED_OFFSET_NEW 0x0500                                //
#define FTM_SHARED_OFFSET     FTM_SHARED_OFFSET_NEW                 //
                                                                    //
//////////////////////////////////////////////////////////////////////
//  DM Configuration                                                //
//////////////////////////////////////////////////////////////////////
#define QTY_CORES         9                                         //
#define QTY_THREADS       8                                         //
#define QTY_THREAD_VARS   8                                         //
#define QTY_BLOCKS        32                                        //
#define QTY_HPQ_ELEMENTS  4                                         //
#define QTY_LPQ_ELEMENTS  8                                         //
#define QTY_STATUS_FIELDS 16                                        //
//////////////////////////////////////////////////////////////////////

//basic definitions
#define _32b_SIZE_    4
#define _64b_SIZE_    8
#define _PTR_SIZE_    _32b_SIZE_
#define _OFFS_SIZE_   _32b_SIZE_
#define _TS_SIZE_     _64b_SIZE_

//Sizes that can be calculated from config only

#define _TD_VARS_SIZE_  (QTY_THREAD_VARS * _32b_SIZE_)
#define _STAT_SIZE_     (QTY_STATUS_FIELDS * _32b_SIZE_)
#define _IBS_SIZE_      (QTY_CORES * QTY_THREADS * _32b_SIZE_)

//////////////////////////////////////////////////////////////////////
//  Dynamic Memory Allocation System                                //
//////////////////////////////////////////////////////////////////////
//Live Block
#define _LB_START_    0
#define LB_PTR        (_LB_START_)
#define LB_SIZE       (LB_PTR   + _PTR_SIZE_)
#define _LB_SIZE_     (LB_SIZE  + _OFFS_SIZE_)

//Live Block Table
//Live Block Table Bitmap
#define _LBT_BMP_SIZE_  ((QTY_BLOCKS + QTY_BLOCKS-1)/32 * _32b_SIZE_)
//Live Block Table
#define _LBT_TAB_SIZE_  (QTY_BLOCKS * _LB_SIZE_)

#define _LBT_START_   0
#define LBT_BMP       (_LBT_START_)   
#define LBT_TAB       (LBT_BMP + _LBT_BMP_SIZE_)
#define _LBT_SIZE_    (LBT_TAB + _LBT_TAB_SIZE_)

//Update Request
#define _UR_START_    0
#define UR_LBT_IDX    (_UR_START_)  
#define UR_NEW_LB_PTR (UR_LBT_IDX     + _OFFS_SIZE_)
#define _UR_SIZE_     (UR_NEW_LB_PTR  + _PTR_SIZE_)

//Thread Control
#define _TC_START_    0
#define TC_START      (_TC_START_)
#define TC_STOP       (TC_START + _32b_SIZE_)
#define TC_GET        (TC_STOP  + _32b_SIZE_)
#define _TC_SIZE_     (TC_GET   + _32b_SIZE_)

//Thread Data

//Thread 
#define _TD_VARS_SIZE_  (QTY_THREAD_VARS * _32b_SIZE_)

#define _TD_START_    0 
#define TD_LBT_IDX    (_TD_START_ )
#define TD_LB_PTR     (TD_LBT_IDX   + _OFFS_SIZE_)
#define TD_OFFS       (TD_LB_PTR    + _PTR_SIZE_)
#define TD_DEADLINE   (TD_OFFS      + _OFFS_SIZE_)
#define TD_VARS       (TD_DEADLINE  + _TS_SIZE_)
#define _TD_SIZE_     (TD_VARS      + _TD_VARS_SIZE_)
//All Thread Data
#define _TDS_SIZE_    (QTY_THREADS  * _TD_SIZE_)

//Host Queue Entry
#define _QE_START_    0
#define QE_GEN_CNT    (_QE_START_)
#define QE_SRC_DST    (QE_GEN_CNT + _32b_SIZE_)
#define QE_TVALID     (QE_SRC_DST + _32b_SIZE_)
#define _QE_SIZE_     (QE_TVALID  + _TS_SIZE_)

//Host Queue
#define _HQ_START_    0
#define HQ_HP_R_PTR   (_HQ_START_)
#define HQ_HP_W_PTR   (HQ_HP_R_PTR  + _PTR_SIZE_)
#define HQ_HPQ        (HQ_HP_W_PTR  + _PTR_SIZE_)
#define HQ_LP_R_PTR   (HQ_HPQ       + (QTY_HPQ_ELEMENTS * _QE_SIZE_))
#define HQ_LP_W_PTR   (HQ_LP_R_PTR  + _PTR_SIZE_)
#define HQ_LPQ        (HQ_LP_W_PTR  + _PTR_SIZE_)
#define _HQ_SIZE_     (HQ_LPQ       + (QTY_LPQ_ELEMENTS * _QE_SIZE_))
//Host Queues
#define _HQS_SIZE_    (QTY_THREADS  * _HQ_SIZE_)

//Top shared control memory layout
#define _SHCTL_START_    0
#define SHCTL_STATUS     (_SHCTL_START_)                     //Status Registers
#define SHCTL_MSG_CNT    (SHCTL_STATUS       + _32b_SIZE_ ) //Command Register
#define SHCTL_CMD        (SHCTL_MSG_CNT      + _32b_SIZE_ ) //Command Register
#define SHCTL_TPREP      (SHCTL_CMD          + _32b_SIZE_ ) //Preparation Time Register
#define SHCTL_TGATHER    (SHCTL_TPREP        + _TS_SIZE_  ) //Gather Time Register 
#define SHCTL_LBTAB      (SHCTL_TGATHER      + _TS_SIZE_  ) //Live Block Table 
#define SHCTL_U_REQ      (SHCTL_LBTAB        + _LBT_SIZE_ ) //Block Update Request Registers     
#define SHCTL_THR_CTL    (SHCTL_U_REQ        + _UR_SIZE_  ) //Thread Control Registers (Start Stop Status)
#define SHCTL_THR_DAT    (SHCTL_THR_CTL      + _TC_SIZE_  ) //Thread Data (1 per Thread )
#define SHCTL_INBOXES    (SHCTL_THR_DAT      + _TDS_SIZE_ ) //Inboxes for MSI (1 per Core in System )
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Payload Data (Blocks and Events                                  //
//////////////////////////////////////////////////////////////////////
// Block Hdr

#define BLOCK_FLAGS_HAS_Q     ( 1 << 0)
#define BLOCK_FLAGS_START_POS 4
#define BLOCK_FLAGS_START_MSK 0x3
#define BLOCK_START_AT  0
#define BLOCK_START_NOW 1
#define BLOCK_START_PPS 2

#define BLOCK_PERIOD        0
#define BLOCK_START_TIME    (BLOCK_PERIOD       + 8)
#define BLOCK_FLAGS         (BLOCK_START_TIME   + 8)
#define BLOCK_Q_OFF         (BLOCK_FLAGS        + 2)
#define BLOCK_NEXT_IDX      (BLOCK_Q_OFF        + 2)
// this will be removed > v4.0 /////////////////////////
#define BLOCK_EVT_QTY       (BLOCK_NEXT_IDX     + 4)  //
#define BLOCK_EVT_CNT       (BLOCK_EVT_QTY      + 4)  //
#define BLOCK_CURRENT_TIME  (BLOCK_EVT_CNT      + 4)  //
#define _BLOCK_HDR_SIZE     (BLOCK_CURRENT_TIME + 8)  //
////////////////////////////////////////////////////////
//#define _BLOCK_HDR_SIZE     (BLOCK_NEXT_IDX     + 4)

// Event Hdr
#define EVT_TYPE_TMSG       1
#define EVT_TYPE_CMD        2

#define EVT_OFFS_TIME       0
#define EVT_TYPE            (EVT_OFFS_TIME  + 8)
#define EVT_FLAGS           (EVT_TYPE       + 2)
#define _EVT_HDR_SIZE       (EVT_FLAGS      + 2)    

// Timing Message Event
#define EVT_TM_ID           (_EVT_HDR_SIZE)
#define EVT_TM_PAR          (EVT_TM_ID      + 8)
#define EVT_TM_TEF          (EVT_TM_PAR     + 8)

// Command Message Event
#define EVT_CM_TIME         (_EVT_HDR_SIZE)
#define EVT_CM_ACT          (EVT_CM_TIME    + 8)
#define EVT_CMD_RESERVED    (EVT_CM_ACT     + 4)

#define _EVT_SIZE           (_EVT_HDR_SIZE  + 20)

//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
//Command Queue System structures                                   //
//////////////////////////////////////////////////////////////////////
// 3 priorities, Low, High, Interlock
#define PRIO_LO 0
#define PRIO_HI 1
#define PRIO_IL 2

//Command structure, Action & Timestamp
#define CMD_TS_OFF            0x0
#define CMD_ACT_OFF           (CMD_TS_OFF  + 8)
#define _CMD_SIZE             (CMD_ACT_OFF + 4)

//Action substructure
//FIXME this is very odd... add unshifted values?
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
#define ACT_FLUSH_RANGE_ALL   0xff
#define ACT_FLUSH_LO_MSK      0x00010000
#define ACT_FLUSH_LO_POS      16 
#define ACT_FLUSH_HI_MSK      0x00020000
#define ACT_FLUSH_HI_POS      17
#define ACT_FLUSH_IL_MSK      0x00040000
#define ACT_FLUSH_IL_POS      18 
#define ACT_FLUSH_LO_RNG_MSK  0x000000ff
#define ACT_FLUSH_LO_RNG_POS  0
#define ACT_FLUSH_HI_RNG_MSK  0x0000ff00
#define ACT_FLUSH_HI_RNG_POS  8
#define _ACT_SIZE             4

//Command Queue structure
#define CMDQ_DEPTH              4 ///
#define _CMDQ_BUF_SIZE        (CMDQ_DEPTH * _CMD_SIZE)
#define CMDQ_IDX_OF_MSK       (2 * CMDQ_DEPTH -1) // x2 for overflow bit
#define CMDQ_IDX_MSK          (CMDQ_DEPTH -1) // 
#define _CMDQ_IDX_SIZE        4
#define CMDQ_OFF              0
#define CMDQ_RD_OFF           (CMDQ_OFF)
#define CMDQ_WR_OFF           (CMDQ_RD_OFF  + _CMDQ_IDX_SIZE)
#define CMDQ_BUF_OFF          (CMDQ_WR_OFF  + _CMDQ_IDX_SIZE)
#define _CMDQ_SIZE            (CMDQ_BUF_OFF + _CMDQ_BUF_SIZE)

//Command Queue System structure (Triple CMDQ)
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
//////////////////////////////////////////////////////////////////////

#endif




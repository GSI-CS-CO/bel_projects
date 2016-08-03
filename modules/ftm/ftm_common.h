#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>

#define FTM_RST_FPGA        0x0
#define FTM_RST_FPGA_CMD    0xDEADBEEF
#define FTM_RST_GET         (FTM_RST_FPGA + 4)
#define FTM_RST_SET         (FTM_RST_GET  + 4)
#define FTM_RST_CLR         (FTM_RST_SET  + 4)

#define FTM_SHARED_OFFSET_NEW 0x0500


//////////////////////////////////////////////////////////////////////
//  DM Configuration                                                //
//////////////////////////////////////////////////////////////////////
#define QTY_CORES         9                                         //
#define QTY_THREADS       8                                         //
#define QTY_THREAD_VARS   8                                         //
#define QTY_BLOCKS        64                                        //
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
#define TC_START_STOP (_TC_START_)
#define TC_GET        (TC_START_STOP + _32b_SIZE_)
#define _TC_SIZE_     (TC_GET        + _32b_SIZE_)

//Thread Data

//Thread 
#define _TD_VARS_SIZE_  (QTY_THREAD_VARS * _32b_SIZE_)

#define _TD_START_
#define TD_LBT_IDX    (_TD_START_)
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
#define SHCTL_CMD        (SHCTL_STATUS       + _STAT_SIZE_ ) //Command Register
#define SHCTL_TPREP      (SHCTL_CMD          + _32b_SIZE_  ) //Preparation Time Register
#define SHCTL_LBTAB      (SHCTL_TPREP        + _TS_SIZE_   ) //Live Block Table 
#define SHCTL_U_REQ      (SHCTL_BLOCK_TABLE  + _LBT_SIZE_  ) //Block Update Request Registers     
#define SHCTL_THR_CTL    (SHCTL_UPDATE_REQ   + _UR_SIZE_   ) //Thread Control Registers (Start Stop Status)
#define SHCTL_THR_DAT    (SHCTL_THREAD_CTL   + _TC_SIZE_   ) //Thread Data (1 per Thread )
#define SHCTL_HOST_QS    (SHCTL_THREAD_DATA  + _TDS_SIZE_  ) //Host Queues (1 per Thread )
#define SHCTL_INBOXES    (SHCTL_HOST_QS      + _HQS_SIZE_  ) //Inboxes for MSI (1 per Core in System )
#define _SHCTL_SIZE_     (SHCTL_INBOXES      + _IBS_SIZE_  )




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
#define FTM_SHARED_OFFSET     FTM_SHARED_OFFSET_NEW

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

/// LEGACY SHIT
#define FTM_IDLE_OFFSET  (SHCTL_INBOXES      + _IBS_SIZE_ ) 
#define FTM_DEBUG_DATA   (FTM_IDLE_OFFSET    + _32b_SIZE_ )     //c8
#define _SHCTL_SIZE_     (FTM_DEBUG_DATA     + _32b_SIZE_ )

/////////////////////////////////////////////////////////////////////////////////
// Backward compatible layouts
/////////////////////////////////////////////////////////////////////////////////

#define THR_SHARED_SIZE     16
#define CPU_SHARED_SIZE     (8*THR_SHARED_SIZE + 40) 

#define FTM_PAGE_QTY        0
#define FTM_PLAN_MAX        16
#define FTM_PAGEDATA        0x0600
#define FTM_PAGEMETA        (4 + 4 * FTM_PLAN_MAX + 4 + 4)   
#define FTM_PAGESIZE        (FTM_PAGEDATA + FTM_PAGEMETA)

#define BUF_SIZE            FTM_PAGESIZE





   


#define DBG_DISP_DUR_MIN      0
#define DBG_DISP_DUR_MAX      DBG_DISP_DUR_MIN+1
#define DBG_DISP_DUR_AVG      DBG_DISP_DUR_MAX+1


//masks & constants
#define CMD_RST           		0x0001	//Reset FTM status and counters
#define CMD_START      		    0x0002	//Start FTM
#define CMD_IDLE   		        0x0004	//Jump into IDLE at next BP
#define CMD_STOP_REQ          0x0008	//Stop FTM if when it reaches IDLE state
#define CMD_STOP_NOW          0x0010	//Stop FTM immediately

#define CMD_COMMIT_PAGE       0x0100  //Commmit new data and validate
#define CMD_COMMIT_BP         0x0200  //Commit alt Plan pointer. Will be selected at next BP if not NULL
#define CMD_PAGE_SWAP         0x0400  //swap Page at next BP
//#define CMD_SHOW_ACT          0x0800
//#define CMD_SHOW_INA          0x1000
#define CMD_DBG_0             0x2000  //DBG case 0
#define CMD_DBG_1             0x4000  //DBG case 1

#define STAT_RUNNING          (1<<0)   //the FTM is running
#define STAT_IDLE             (1<<1)   //the FTM is idling  
#define STAT_STOP_REQ         (1<<2)   //alt ptr has been set to IDLE 
#define STAT_ERROR            (1<<3)   //FTM encountered an error, check error register
#define STAT_WAIT             (1<<4)   //FTM waiting on condition
#define STAT_STOPPED          (1<<5)   //FTM is Stopped - will not execute anything 


#define ID_MSK_B16            0xffff
#define ID_FID_LEN            4
#define ID_GID_LEN            12
#define ID_EVTNO_LEN          12
#define ID_SID_LEN            12
#define ID_BPID_LEN           14
#define ID_SCTR_LEN           10
#define ID_FID_POS            (ID_GID_LEN + ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_GID_POS            (ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_EVTNO_POS          (ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_SID_POS            (ID_BPID_LEN + ID_SCTR_LEN)
#define ID_BPID_POS           (ID_SCTR_LEN)
#define ID_SCTR_POS           0

#define FLAGS_IS_BP           (1<<0)

#define FLAGS_IS_COND_MSI     (1<<4)
#define FLAGS_IS_COND_SHARED  (1<<5)
#define FLAGS_IS_COND_ADR     (1<<6)
#define FLAGS_IS_COND_TIME    (1<<7)
#define FLAGS_IS_COND_ALL     (1<<8)

#define FLAGS_IS_SIG_MSI      (1<<12)
#define FLAGS_IS_SIG_SHARED   (1<<13)
#define FLAGS_IS_SIG_ADR      (1<<14)
#define FLAGS_IS_SIG_TIME     (1<<15)

#define FLAGS_IS_SIG_FIRST    (1<<16)
#define FLAGS_IS_SIG_LAST     (1<<17)
#define FLAGS_IS_SIG_ALL      (1<<18)

#define FLAGS_IS_START        (1<<20) 
#define FLAGS_IS_END          (1<<21) 
#define FLAGS_IS_ENDLOOP      (1<<22)
#define FLAGS_IS_PERS_REP_CNT (1<<23)


#define FTM_TIME_SIZE         8
#define FTM_DWORD_SIZE        8
#define FTM_WORD_SIZE         4
#define FTM_PTR_SIZE          4
#define FTM_NULL              0x0

#define FTM_MSG_ID            0
#define FTM_MSG_PAR           (FTM_MSG_ID    + 8)
#define FTM_MSG_TEF           (FTM_MSG_PAR   + 8)
#define FTM_MSG_RES           (FTM_MSG_TEF   + 4)
#define FTM_MSG_TS            (FTM_MSG_RES   + 4)
#define FTM_MSG_OFFS          (FTM_MSG_TS    + 8)
#define FTM_MSG_END_          (FTM_MSG_OFFS  + 8)

#define FTM_CHAIN_TTRN          0
#define FTM_CHAIN_TSTART        0
#define FTM_CHAIN_TPERIOD       (FTM_CHAIN_TSTART         + 8)
#define FTM_CHAIN_TEXEC         (FTM_CHAIN_TPERIOD        + 8)
#define FTM_CHAIN_FLAGS         (FTM_CHAIN_TEXEC          + 8)
#define FTM_CHAIN_CONDSRC       (FTM_CHAIN_FLAGS          + 4)
#define FTM_CHAIN_CONDVAL       (FTM_CHAIN_CONDSRC        + 4)
#define FTM_CHAIN_CONDMSK       (FTM_CHAIN_CONDVAL        + 4)
#define FTM_CHAIN_SIGDST        (FTM_CHAIN_CONDMSK        + 4)
#define FTM_CHAIN_SIGVAL        (FTM_CHAIN_SIGDST         + 4)
#define FTM_CHAIN_REPQTY        (FTM_CHAIN_SIGVAL         + 4)
#define FTM_CHAIN_REPCNT        (FTM_CHAIN_REPQTY         + 4)
#define FTM_CHAIN_MSGQTY        (FTM_CHAIN_REPCNT         + 4)
#define FTM_CHAIN_MSGIDX        (FTM_CHAIN_MSGQTY         + 4)
#define FTM_CHAIN_PMSG          (FTM_CHAIN_MSGIDX         + 4)
#define FTM_CHAIN_PNEXT         (FTM_CHAIN_PMSG           + 4)
#define FTM_CHAIN_END_          (FTM_CHAIN_PNEXT          + 4)

#endif




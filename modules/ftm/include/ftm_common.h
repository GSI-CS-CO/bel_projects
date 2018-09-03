#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>

/// Debugging Macros
#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
///
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define PREPTIME_DEFAULT 1000000ULL // standard preptime offset, sets lead to 1 ms

//basic definitions
#define _8b_SIZE_               1
#define _16b_SIZE_              2
#define _32b_SIZE_              4
#define _64b_SIZE_              8
#define _PTR_SIZE_              _32b_SIZE_
#define _OFFS_SIZE_             _32b_SIZE_
#define _TS_SIZE_               _64b_SIZE_

#define PRIO_IL 2
#define PRIO_HI 1
#define PRIO_LO 0

#define WR_PPS_GEN_STATE_MSK
#define WR_PPS_GEN_STATE_MSK
#define WR_PPS_GEN_STATE_MSK



//////////////////////////////////////////////////////////////////////
//  Defines for dynamic Memory Allocation System                    //
//////////////////////////////////////////////////////////////////////
// Memory Block Size

// allow unaligned blocks ... I'd say yes, but not right now
#define ALLLOW_UNALIGNED        0

//Memory Block (Node) Size in Bytes
#define _MEM_BLOCK_SIZE         52
#define WORLD_BASE_ADR          0x80000000


#define LM32_NULL_PTR           0x0

#define _THR_QTY_               8
#define _HEAP_SIZE_             (_THR_QTY_)

#define _T_GRID_OFFS_           0ULL      //Origin of time grid for align blocks in ns
#define _T_GRID_SIZE_           10000ULL  //grid size for align blocks ns
#define _T_TVALID_OFFS_         50000ULL  //Min offset from 'now' for validTime when generating a command


//////////////////////////////////////////////////////////////////////
//"struct" types                                                    //
//////////////////////////////////////////////////////////////////////

//Thread Control bits
#define T_TC_START              (0)                     //WR Host, RW LM32
#define T_TC_CEASE              (T_TC_START     + _32b_SIZE_) //WR Host, RW LM32
#define T_TC_ABORT              (T_TC_CEASE     + _32b_SIZE_) //WR Host, RW LM32
#define T_TC_RUNNING            (T_TC_ABORT     + _32b_SIZE_) //RD Host, WR LM32
#define _T_TC_SIZE_             (T_TC_RUNNING   + _32b_SIZE_)

//Thread Data
#define T_TD_FLAGS              (0)                           //RD Host, RW LM32
#define T_TD_MSG_CNT            (T_TD_FLAGS     + _32b_SIZE_) //RD Host, RW LM32
#define T_TD_NODE_PTR           (T_TD_MSG_CNT   + _64b_SIZE_) //RD Host, RW LM32
#define T_TD_DEADLINE           (T_TD_NODE_PTR  + _PTR_SIZE_) //RD Host, RW LM32
#define T_TD_DEADLINE_HI        (T_TD_DEADLINE  + 0) //RD Host, RW LM32
#define T_TD_DEADLINE_LO        (T_TD_DEADLINE_HI + _32b_SIZE_) //RD Host, RW LM32
#define T_TD_CURRTIME           (T_TD_DEADLINE  + _TS_SIZE_)  //RD Host, RW LM32
#define _T_TD_SIZE_             (T_TD_CURRTIME  + _TS_SIZE_)

//Thread Starter Stage
#define T_TS_FLAGS              (0)                           //RW Host, RW LM32
#define T_TS_NODE_PTR           (T_TS_FLAGS     + _32b_SIZE_) //WR Host, RD LM32
#define T_TS_STARTTIME          (T_TS_NODE_PTR  + _PTR_SIZE_) //WR Host, RD LM32
#define T_TS_PREPTIME           (T_TS_STARTTIME + _TS_SIZE_)  //WR Host, RD LM32
#define _T_TS_SIZE_             (T_TS_PREPTIME  + _TS_SIZE_)



//Sync node entry
#define T_SYNC_SRC              (0)
#define T_SYNC_DST              (T_SYNC_SRC   + _PTR_SIZE_)
#define T_SYNC_TIME             (T_SYNC_DST   + _PTR_SIZE_)
#define _T_SYNC_SIZE_           (T_SYNC_TIME  + _TS_SIZE_)

//Command (the received format)
#define T_CMD_TIME              (0)                     // time this command becomes valid
#define T_CMD_ACT               (T_CMD_TIME + _TS_SIZE_)  // action describing the command
//either ...
#define T_CMD_WAIT_TIME         (T_CMD_ACT  + _32b_SIZE_)  // if it's a wait command, this is the new block tPeriod to add/new absolute thread time to use
// ... or
#define T_CMD_FLOW_DEST         (T_CMD_ACT  + _32b_SIZE_) // if it's a flow command, this is the alternative destination to use
#define T_CMD_RES               (T_CMD_FLOW_DEST + _32b_SIZE_)
// ... or
#define T_CMD_FLUSH_RNG_HILO    (T_CMD_ACT  + _32b_SIZE_) // if it's a flush command and mode bits are set, this defines the q from/to idx to flush per qbuf
#define T_CMD_FLUSH_RNG_IL      (T_CMD_FLUSH_RNG_HILO + _32b_SIZE_)
//
#define _T_CMD_SIZE_             (_TS_SIZE_ + _32b_SIZE_ + _64b_SIZE_)


#define T_MOD_INFO_TS      (0)                             //Timestamp of last modification
#define T_MOD_INFO_IID     (T_MOD_INFO_TS   + _TS_SIZE_  ) //Issuer ID of last modification
#define T_MOD_INFO_MID     (T_MOD_INFO_IID  + _64b_SIZE_ ) //Machine ID of last modification
#define T_MOD_INFO_TYPE    (T_MOD_INFO_MID  + _64b_SIZE_ ) //Type of last modification
#define T_MOD_INFO_CNT     (T_MOD_INFO_TYPE + _32b_SIZE_ ) //Modification counter
#define _T_MOD_INFO_SIZE   (T_MOD_INFO_CNT  + _32b_SIZE_ ) //Struct size

#define T_DIAG_MSG_CNT      (0)                              //CPU wide timing message counter
#define T_DIAG_BOOT_TS      (T_DIAG_MSG_CNT       + _64b_SIZE_ ) //Timestamp of Uptime beginning
#define T_DIAG_SCH_MOD      (T_DIAG_BOOT_TS       + _TS_SIZE_  ) //Schedule modification info
#define T_DIAG_CMD_MOD      (T_DIAG_SCH_MOD       + _T_MOD_INFO_SIZE ) //Cmd modification info
#define T_DIAG_DIF_MIN      (T_DIAG_CMD_MOD       + _T_MOD_INFO_SIZE ) //All time min diff between dispatch time and deadline   (signed!)
#define T_DIAG_DIF_MAX      (T_DIAG_DIF_MIN       + _TS_SIZE_  ) //All time max diff between dispatch time and deadline   (signed!)
#define T_DIAG_DIF_SUM      (T_DIAG_DIF_MAX       + _TS_SIZE_  ) //Running sum of diff between dispatch time and deadline (signed!)
#define T_DIAG_DIF_WTH      (T_DIAG_DIF_SUM       + _64b_SIZE_ ) //Diff Threshold between dispatch time and deadline which will trigger a warning (signed!)
#define T_DIAG_WAR_CNT      (T_DIAG_DIF_WTH       + _TS_SIZE_  ) //Diff warning counter
#define T_DIAG_WAR_1ST_HASH (T_DIAG_WAR_CNT       + _32b_SIZE_ ) //Hash of node at first diff warning
#define T_DIAG_WAR_1ST_TS   (T_DIAG_WAR_1ST_HASH  + _32b_SIZE_ ) //TS at first diff warning
#define T_DIAG_BCKLOG_STRK  (T_DIAG_WAR_1ST_TS    + _TS_SIZE_  ) //Maximum Backlog streak size
#define T_DIAG_BAD_WAIT_CNT (T_DIAG_BCKLOG_STRK   + _32b_SIZE_ ) //Maximum Backlog streak size
#define _T_DIAG_SIZE_       (256)

#if _T_DIAG_SIZE_ < (T_DIAG_BAD_WAIT_CNT + _32b_SIZE_)
  #error Actual diagnostics area size exceeds fixed _T_DIAG_SIZE_
#endif



#define T_META_START_PTR    (0)                               //same for all cpus, can be on any CPU. External view, read/write for host only. Must lie within bitmap range
#define T_META_CON_SIZE     (T_META_START_PTR   + _PTR_SIZE_) //container size in byte ( is groupsTable + covenantTable size)
#define T_META_GRPTAB_SIZE  (T_META_CON_SIZE    + _32b_SIZE_) //groupsTable size in byte
#define T_META_COVTAB_SIZE  (T_META_GRPTAB_SIZE + _32b_SIZE_) //covenantTable size in byte
#define T_META_FLAGS        (T_META_COVTAB_SIZE + _32b_SIZE_) //
#define _T_META_SIZE_       (T_META_FLAGS       + _32b_SIZE_)


//////////////////////////////////////////////////////////////////////
// Control Interface                                                //
//////////////////////////////////////////////////////////////////////

#define _SHCTL_START_    0
#define SHCTL_HEAP       (_SHCTL_START_)                              //Scheduler Heap
#define SHCTL_STATUS     (SHCTL_HEAP    + _THR_QTY_ * _PTR_SIZE_)     //Status Registers
#define SHCTL_META       (SHCTL_STATUS  + _32b_SIZE_ )                //Group/Node Name Meta Information
#define SHCTL_DIAG       (SHCTL_META    + _T_META_SIZE_ )             //Diagnostic Registers
#define SHCTL_CMD        (SHCTL_DIAG    + _T_DIAG_SIZE_ )             //Command Register
#define SHCTL_TGATHER    (SHCTL_CMD     + _32b_SIZE_ )                //Gather Time (HW Priority Queue Config) Register
#define SHCTL_THR_CTL    (SHCTL_TGATHER + _TS_SIZE_  )                //Thread Control Registers (Start Stop Status)
#define SHCTL_THR_STA    (SHCTL_THR_CTL + _T_TC_SIZE_  )              //Thread Start Staging Area (1 per Thread )
#define SHCTL_THR_DAT    (SHCTL_THR_STA + _THR_QTY_ * _T_TS_SIZE_  )  //Thread Runtime Data (1 per Thread )
#define SHCTL_INBOXES    (SHCTL_THR_DAT + _THR_QTY_ * _T_TD_SIZE_  )  //Inboxes for MSI (1 per Core in System )
#define _SHCTL_END_      (SHCTL_INBOXES + _THR_QTY_ * _32b_SIZE_)
/*
#pragma message(VAR_NAME_VALUE(_SHCTL_START_))
#pragma message(VAR_NAME_VALUE(SHCTL_HEAP   ))
#pragma message(VAR_NAME_VALUE(SHCTL_STATUS ))
#pragma message(VAR_NAME_VALUE(SHCTL_META   ))
#pragma message(VAR_NAME_VALUE(SHCTL_DIAG   ))
#pragma message(VAR_NAME_VALUE(SHCTL_CMD    ))
#pragma message(VAR_NAME_VALUE(SHCTL_TGATHER))
#pragma message(VAR_NAME_VALUE(SHCTL_THR_CTL))
#pragma message(VAR_NAME_VALUE(SHCTL_THR_STA))
#pragma message(VAR_NAME_VALUE(SHCTL_THR_DAT))
#pragma message(VAR_NAME_VALUE(SHCTL_INBOXES))
#pragma message(VAR_NAME_VALUE(_SHCTL_END_  ))
*/

//////////////////////////////////////////////////////////////////////

// Global Status field bits
#define SHCTL_STATUS_UART_INIT_MSK      0x1
#define SHCTL_STATUS_UART_INIT_POS      0
#define SHCTL_STATUS_UART_INIT_SMSK     (SHCTL_STATUS_UART_INIT_MSK << SHCTL_STATUS_UART_INIT_POS)

#define SHCTL_STATUS_EBM_INIT_MSK       0x1
#define SHCTL_STATUS_EBM_INIT_POS       1
#define SHCTL_STATUS_EBM_INIT_SMSK      (SHCTL_STATUS_EBM_INIT_MSK << SHCTL_STATUS_EBM_INIT_POS)

#define SHCTL_STATUS_PQ_INIT_MSK        0x1
#define SHCTL_STATUS_PQ_INIT_POS        2
#define SHCTL_STATUS_PQ_INIT_SMSK       (SHCTL_STATUS_PQ_INIT_MSK << SHCTL_STATUS_PQ_INIT_POS)

#define SHCTL_STATUS_DM_INIT_MSK        0x1
#define SHCTL_STATUS_DM_INIT_POS        3
#define SHCTL_STATUS_DM_INIT_SMSK       (SHCTL_STATUS_DM_INIT_MSK << SHCTL_STATUS_DM_INIT_POS)

#define SHCTL_STATUS_DM_ERROR_MSK       0x1
#define SHCTL_STATUS_DM_ERROR_POS       8
#define SHCTL_STATUS_DM_ERROR_SMSK      (SHCTL_STATUS_DM_ERROR_MSK << SHCTL_STATUS_DM_ERROR_POS)

#define SHCTL_STATUS_BAD_NODE_TYPE_MSK  0x1
#define SHCTL_STATUS_BAD_NODE_TYPE_POS  12
#define SHCTL_STATUS_BAD_NODE_TYPE_SMSK (SHCTL_STATUS_BAD_ACT_TYPE_MSK << SHCTL_STATUS_BAD_ACT_TYPE_POS)

#define SHCTL_STATUS_BAD_ACT_TYPE_MSK   0x1
#define SHCTL_STATUS_BAD_ACT_TYPE_POS   13
#define SHCTL_STATUS_BAD_ACT_TYPE_SMSK  (SHCTL_STATUS_BAD_ACT_TYPE_MSK << SHCTL_STATUS_BAD_ACT_TYPE_POS)


//####################################################################
// #########################  Nodes  #################################
//####################################################################


//////////////////////////////////////////////////////////////////////
// Generic Node Attributes ///////////////////////////////////////////
#define NODE_BEGIN              (0)

// This pack all generic data to the end! Makes ptr arithmetic easier for array types
#define NODE_HASH               (0x28)
#define NODE_FLAGS              (NODE_HASH  + _32b_SIZE_)
#define NODE_DEF_DEST_PTR       (NODE_FLAGS + _32b_SIZE_)


//////////////////////////////////////////////////////////////////////
//// Meta Nodes - Specific Attributes ////////////////////////////////

////// Block
//
#define BLOCK_BEGIN             (NODE_BEGIN)      // a bit unusual layout, see above
#define BLOCK_PERIOD            (BLOCK_BEGIN)
#define BLOCK_PERIOD_HI         (BLOCK_BEGIN + 0)
#define BLOCK_PERIOD_LO         (BLOCK_PERIOD_HI    + _32b_SIZE_)
#define BLOCK_ALT_DEST_PTR      (BLOCK_PERIOD       + _TS_SIZE_)
#define BLOCK_CMDQ_LO_PTR       (BLOCK_ALT_DEST_PTR + _PTR_SIZE_)
#define BLOCK_CMDQ_HI_PTR       (BLOCK_CMDQ_LO_PTR  + _PTR_SIZE_)
#define BLOCK_CMDQ_IL_PTR       (BLOCK_CMDQ_HI_PTR  + _PTR_SIZE_)
#define BLOCK_CMDQ_WR_IDXS      (BLOCK_CMDQ_IL_PTR  + _PTR_SIZE_)
#define BLOCK_CMDQ_RD_IDXS      (BLOCK_CMDQ_WR_IDXS + _32b_SIZE_)
#define BLOCK_CMDQ_PTRS          BLOCK_CMDQ_LO_PTR

#define BLOCK_CMDQ_IDX_IL    (3 - PRIO_IL)
#define BLOCK_CMDQ_IDX_HI    (3 - PRIO_HI)
#define BLOCK_CMDQ_IDX_LO    (3 - PRIO_LO)


#define Q_IDX_MAX_OVF          3
#define Q_IDX_MAX              2
#define Q_IDX_MAX_OVF_MSK      ~(-(1 << Q_IDX_MAX_OVF))
#define Q_IDX_MAX_MSK          ~(-(1 << Q_IDX_MAX))

///// CMDQ Meta Node
//
// Array of pointers to cmd buffer nodes
#define CMDQ_BUF_ARRAY          (NODE_BEGIN)
#define CMDQ_BUF_ARRAY_END      (CMDQ_BUF_ARRAY + 10 * _PTR_SIZE_)

///// CMD Buffer Meta Node
//
// Elements of size _CMD_SIZE
#define CMDB_CMD_ARRAY          (NODE_BEGIN)


///// Alternative Destinations Meta Node
//
// Host only, array of pointers to nodes
#define DST_ARRAY               (NODE_BEGIN)
#define DST_ARRAY_END           (DST_ARRAY + 10 * _PTR_SIZE_)

///// Sync Meta Node
//
// Host only, Element format: Src Ptr 32b, Dst Ptr 32b, Time Offset 64b
#define SYNC_ARRAY              (NODE_BEGIN)

//////////////////////////////////////////////////////////////////////
//// Generic Event Attributes ////////////////////////////////////////
#define EVT_BEGIN               (NODE_BEGIN)
#define EVT_OFFS_TIME           (EVT_BEGIN)
#define EVT_HDR_END             (EVT_OFFS_TIME + _TS_SIZE_)

//// Timing Msg
//
#define TMSG_BEGIN              (EVT_HDR_END)
#define TMSG_ID                 (TMSG_BEGIN)
#define TMSG_ID_HI              (TMSG_ID      + 0)
#define TMSG_ID_LO              (TMSG_ID_HI   + _32b_SIZE_)
#define TMSG_PAR                (TMSG_ID      + _64b_SIZE_)
#define TMSG_PAR_HI             (TMSG_PAR     + 0)
#define TMSG_PAR_LO             (TMSG_PAR_HI  + _32b_SIZE_)
#define TMSG_RES                (TMSG_PAR     + _64b_SIZE_)
#define TMSG_TEF                (TMSG_RES     + _32b_SIZE_)


//////////////////////////////////////////////////////////////////////
//// Generic Command Attributes //////////////////////////////////////
#define CMD_BEGIN               (EVT_HDR_END)
#define CMD_TARGET              (CMD_BEGIN)
#define CMD_VALID_TIME          (CMD_TARGET         + _PTR_SIZE_)
#define CMD_VALID_TIME_HI       (CMD_VALID_TIME     + 0)
#define CMD_VALID_TIME_LO       (CMD_VALID_TIME_HI  + _32b_SIZE_)
#define CMD_ACT                 (CMD_VALID_TIME     + _TS_SIZE_)
#define CMD_HDR_END             (CMD_ACT            + _32b_SIZE_)


//// Cmd Flow ////////////////////////////////////////////////////////
//

#define CMD_FLOW_DEST           (CMD_HDR_END)

//// Cmd Flush - Specific Attributes /////////////////////////////////
//
#define CMD_FLUSHRNG_IL         (CMD_HDR_END)

#define CMD_FLUSHRNG_IL_FRM     (CMD_FLUSHRNG_IL)
#define CMD_FLUSHRNG_IL_TO      (CMD_FLUSHRNG_IL_FRM  + _8b_SIZE_)

#define CMD_FLUSHRNG_HILO       (CMD_FLUSHRNG_IL      + _32b_SIZE_)

#define CMD_FLUSHRNG_HI_FRM     (CMD_FLUSHRNG_HILO    + _8b_SIZE_)
#define CMD_FLUSHRNG_HI_TO      (CMD_FLUSHRNG_HI_FRM  + _8b_SIZE_)
#define CMD_FLUSHRNG_LO_FRM     (CMD_FLUSHRNG_HI_TO   + _8b_SIZE_)
#define CMD_FLUSHRNG_LO_TO      (CMD_FLUSHRNG_LO_FRM  + _8b_SIZE_)




//// Cmd Nop - Specific Attributes ///////////////////////////////////
//
#define CMD_NOOP_RES            (CMD_HDR_END)

//// Cmd Wait - Specific Attributes //////////////////////////////////
//
#define CMD_WAIT_TIME           (CMD_HDR_END)
#define CMD_WAIT_TIME_HI        (CMD_WAIT_TIME     + 0)
#define CMD_WAIT_TIME_LO        (CMD_WAIT_TIME_HI  + _32b_SIZE_)

//////////////////////////////////////////////////////////////////////

// Address Vector Order
//
// Destination
#define ADR_DEF_DST            0
#define ADR_ALT_DST_ARRAY      1 // only if multiple destinations

//
// Block
#define ADR_BLOCK_DST_LST  1 // only if multiple destinations


#define ADR_BLOCK_Q_LO     (ADR_BLOCK_DST_LST + 1 + PRIO_LO) // only if multiple destinations
#define ADR_BLOCK_Q_HI     (ADR_BLOCK_DST_LST + 1 + PRIO_HI)  // only if multiple destinations
#define ADR_BLOCK_Q_IL     (ADR_BLOCK_DST_LST + 1 + PRIO_IL)  // only if multiple destinations
//
// Timing Message
#define ADR_DYN_ID         1
#define ADR_DYN_PAR1       2
#define ADR_DYN_PAR0       3
#define ADR_DYN_TEF        4
#define ADR_DYN_RES        5

//
// Command
#define ADR_CMD_TARGET     1
#define ADR_CMD_FLOW_DEST  2 // only if command is Flow change

// Command Queue
#define ADR_CMDQ_BUF_ARRAY 1




//////////////////////////////////////////////////////////////////////
// Action Flags Bitfield /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Action Flags - Type field Enums
#define ACT_TYPE_UNKNOWN        0 // tUndefined
#define ACT_TYPE_NOOP           (ACT_TYPE_UNKNOWN +1)  // noop command
#define ACT_TYPE_FLOW           (ACT_TYPE_NOOP +1)  // flow change command
#define ACT_TYPE_FLUSH          (ACT_TYPE_FLOW +1)  // flush command
#define ACT_TYPE_WAIT           (ACT_TYPE_FLUSH +1)  // wait command
#define _ACT_TYPE_END_          (ACT_TYPE_WAIT +1)  // Number of Action types

//Action Quantity
#define ACT_QTY_MSK             0xfffff
#define ACT_QTY_POS             0
#define ACT_QTY_SMSK            (ACT_QTY_MSK << ACT_QTY_POS)

//Action Type
#define ACT_TYPE_MSK            0xf
#define ACT_TYPE_POS            20
#define ACT_TYPE_SMSK           (ACT_TYPE_MSK << ACT_TYPE_POS)

//Action Prirority (which Q at target it is going to)
#define ACT_PRIO_MSK            0x3
#define ACT_PRIO_POS            24
#define ACT_PRIO_SMSK           (ACT_PRIO_MSK << ACT_PRIO_POS)

//Valid time is absolute (0) or relative (1)
#define ACT_VABS_MSK             0x1
#define ACT_VABS_POS             26
#define ACT_VABS_SMSK            (ACT_VABS_MSK << ACT_VABS_POS)

//Action changes are permanent (1) or temporary (0) (Flow -> DEF_DEST_PTR, Wait -> BLOCK_PERIOD (only use with relative wait!))
#define ACT_CHP_MSK             0x1
#define ACT_CHP_POS             27
#define ACT_CHP_SMSK            (ACT_CHP_MSK << ACT_CHP_POS)

//Cmd Action Flags - type specific bit definitions

#define ACT_BITS_SPECIFIC_POS   28

//Command Flush Buffer Priority
#define ACT_FLUSH_PRIO_MSK      0x7
#define ACT_FLUSH_PRIO_POS      (ACT_BITS_SPECIFIC_POS + 0)
#define ACT_FLUSH_PRIO_SMSK     (ACT_FLUSH_PRIO_MSK << ACT_FLUSH_PRIO_POS)

//Command Flush Mode
#define ACT_FLUSH_MODE_MSK      0x7
#define ACT_FLUSH_MODE_POS      (ACT_BITS_SPECIFIC_POS + 3)
#define ACT_FLUSH_MODE_SMSK     (ACT_FLUSH_MODE_MSK << ACT_FLUSH_MODE_POS)

//Command Wait Time absolute/relative (tPeriod)
#define ACT_WAIT_ABS_MSK        0x1
#define ACT_WAIT_ABS_POS        (ACT_BITS_SPECIFIC_POS + 0)
#define ACT_WAIT_ABS_SMSK       (ACT_WAIT_ABS_MSK << ACT_WAIT_ABS_POS)


//////////////////////////////////////////////////////////////////////
// Node Flags Bitfield ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//Node Flags - bit definitions

// Node Flags - Type field Enums. Sparsity allows using array of handler function in LM32
//Unknown/Undef Node Enum
#define NODE_TYPE_UNKNOWN       0  // unknown content, ERROR
// Defined but unspecified data
#define NODE_TYPE_RAW           (NODE_TYPE_UNKNOWN      +1)  // raw data, do not interprete DEV ONLY!
//Timing Message Enums
#define NODE_TYPE_TMSG          (NODE_TYPE_RAW          +1)  // dispatches a timing message
//Command Type Enums
#define NODE_TYPE_CNOOP         (NODE_TYPE_TMSG         +1)   // sends a noop command to designated block
#define NODE_TYPE_CFLOW         (NODE_TYPE_CNOOP        +1)   // sends a flow change command to designated block
#define NODE_TYPE_CFLUSH        (NODE_TYPE_CFLOW        +1)   // sends a flush command to designated block
#define NODE_TYPE_CWAIT         (NODE_TYPE_CFLUSH       +1)   // sends a wait command to designated block
//Shared Meta Type Enums
#define NODE_TYPE_BLOCK_FIXED   (NODE_TYPE_CWAIT        +1)   // shows time block tPeriod and if necessary links to Q Meta and altdest nodes
#define NODE_TYPE_BLOCK_ALIGN   (NODE_TYPE_BLOCK_FIXED  +1)   // shows time block tPeriod and if necessary links to Q Meta and altdest nodes
#define NODE_TYPE_QUEUE         (NODE_TYPE_BLOCK_ALIGN  +1)   // a command queue meta node (array of pointers to buffer nodes)
#define NODE_TYPE_QBUF          (NODE_TYPE_QUEUE        +1)   // a buffer for a command queue
#define NODE_TYPE_SHARE         (NODE_TYPE_QBUF         +1)   // share a value via MSI to multiple memory destinations
//Host only Meta Type Enums
#define NODE_TYPE_ALTDST        (NODE_TYPE_SHARE        +1)   // lists all alternative destinations of a decision block
#define NODE_TYPE_SYNC          (NODE_TYPE_ALTDST       +1)   // used to denote the time offset for pattern rows
#define NODE_TYPE_MGMT          (NODE_TYPE_SYNC         +1)   // contain the part of the groups and node name table in compressed form
#define NODE_TYPE_COVENANT      (NODE_TYPE_MGMT         +1)   // contain the addresses of commands (in queues) which the user agrees not to preempt if optimised safe2remove is to work
#define NODE_TYPE_NULL          (NODE_TYPE_COVENANT     +1)   // type returned by getNodeType if the node ptr was NULL. Intentionally not 0x000...
#define _NODE_TYPE_END_         (NODE_TYPE_NULL         +1)   // Node type Quantity
//Node type
#define NFLG_TYPE_MSK           0xff
#define NFLG_TYPE_POS           0
#define NFLG_TYPE_SMSK          (NFLG_TYPE_MSK << NFLG_TYPE_POS)

// paint bit - the lm32 has visited this node
#define NFLG_PAINT_LM32_MSK     0x1
#define NFLG_PAINT_LM32_POS     8
#define NFLG_PAINT_LM32_SMSK    (NFLG_PAINT_LM32_MSK << NFLG_PAINT_LM32_POS)

//paint bit - the host has visited this node
#define NFLG_PAINT_HOST_MSK     0x1
#define NFLG_PAINT_HOST_POS     9
#define NFLG_PAINT_HOST_SMSK    (NFLG_PAINT_HOST_MSK << NFLG_PAINT_HOST_POS)

// sync bit - this node should only be started synchronous to another
#define NFLG_SYNC_MSK           0x1
#define NFLG_SYNC_POS           10
#define NFLG_SYNC_SMSK          (NFLG_SYNC_MSK << NFLG_SYNC_POS)

// Entry point bits - entry into beam processes and patterns
#define NFLG_BP_ENTRY_LM32_MSK    0x1
#define NFLG_BP_ENTRY_LM32_POS    12
#define NFLG_BP_ENTRY_LM32_SMSK   (NFLG_BP_ENTRY_LM32_MSK << NFLG_BP_ENTRY_LM32_POS)

#define NFLG_PAT_ENTRY_LM32_MSK   0x1
#define NFLG_PAT_ENTRY_LM32_POS   13
#define NFLG_PAT_ENTRY_LM32_SMSK  (NFLG_PAT_ENTRY_LM32_MSK << NFLG_PAT_ENTRY_LM32_POS)

// Exit point bits - exit from beam processes and patterns
#define NFLG_BP_EXIT_LM32_MSK     0x1
#define NFLG_BP_EXIT_LM32_POS     14
#define NFLG_BP_EXIT_LM32_SMSK    (NFLG_BP_EXIT_LM32_MSK << NFLG_BP_EXIT_LM32_POS)

#define NFLG_PAT_EXIT_LM32_MSK    0x1
#define NFLG_PAT_EXIT_LM32_POS    15
#define NFLG_PAT_EXIT_LM32_SMSK   (NFLG_PAT_EXIT_LM32_MSK << NFLG_PAT_EXIT_LM32_POS)

// Debug 0 Flag - for arbitrary use during debugging
#define NFLG_DEBUG0_MSK     0x1
#define NFLG_DEBUG0_POS     16
#define NFLG_DEBUG0_SMSK    (NFLG_DEBUG0_MSK << NFLG_DEBUG0_POS)

// Debug 10 Flag - for arbitrary use during debugging
#define NFLG_DEBUG1_MSK     0x1
#define NFLG_DEBUG1_POS     17
#define NFLG_DEBUG1_SMSK    (NFLG_DEBUG1_MSK << NFLG_DEBUG1_POS)

// Type dependent Flags ////////////////////////////////////////////////////////////
#define NFLG_BITS_SPECIFIC_POS  20

// Timing Message //////////////////////////////////////////////////////////////////

//FIXME selling my soul here by allowing dynamic changes to timing msg content.
// Prime BS caused by Jutta's "we must know which pattern it belongs to and dont want to use a proper DB lookup".
// Evil stuff and likely to explode in our faces at some point

//The command is targeting a peer, i.e., target and dest address are inside a peers memory, not inside own
#define NFLG_CMD_PEER_MSK    0x1
#define NFLG_CMD_PEER_POS    (NFLG_BITS_SPECIFIC_POS + 0)
#define NFLG_CMD_PEER_SMSK   (NFLG_CMD_PEER_MSK << NFLG_CMD_PEER_POS)


//interprete ID low word as 64b ptr
#define NFLG_TMSG_DYN_ID_MSK    0x1
#define NFLG_TMSG_DYN_ID_POS    (NFLG_BITS_SPECIFIC_POS + 0)
#define NFLG_TMSG_DYN_ID_SMSK   (NFLG_TMSG_DYN_ID_MSK << NFLG_TMSG_DYN_ID_POS)

//interprete PAR low word as 64b ptr
#define NFLG_TMSG_DYN_PAR_MSK   0x1
#define NFLG_TMSG_DYN_PAR_POS   (NFLG_BITS_SPECIFIC_POS + 1)
#define NFLG_TMSG_DYN_PAR_SMSK  (NFLG_TMSG_DYN_PAR_MSK << NFLG_TMSG_DYN_PAR_POS)

//interprete PAR low word as 32b ptr
#define NFLG_TMSG_DYN_PAR0_MSK  0x1
#define NFLG_TMSG_DYN_PAR0_POS  (NFLG_BITS_SPECIFIC_POS + 2)
#define NFLG_TMSG_DYN_PAR0_SMSK (NFLG_TMSG_DYN_PAR0_MSK << NFLG_TMSG_DYN_PAR0_POS)

//interprete PAR high word as 32b ptr
#define NFLG_TMSG_DYN_PAR1_MSK  0x1
#define NFLG_TMSG_DYN_PAR1_POS  (NFLG_BITS_SPECIFIC_POS + 3)
#define NFLG_TMSG_DYN_PAR1_SMSK (NFLG_TMSG_DYN_PAR1_MSK << NFLG_TMSG_DYN_PAR1_POS)

//interprete TEF low word as 32b ptr
#define NFLG_TMSG_DYN_TEF_MSK   0x1
#define NFLG_TMSG_DYN_TEF_POS   (NFLG_BITS_SPECIFIC_POS + 4)
#define NFLG_TMSG_DYN_TEF_SMSK  (NFLG_TMSG_DYN_TEF_MSK << NFLG_TMSG_DYN_TEF_POS)

//interprete RES low word as 32b ptr
#define NFLG_TMSG_DYN_RES_MSK   0x1
#define NFLG_TMSG_DYN_RES_POS   (NFLG_BITS_SPECIFIC_POS + 5)
#define NFLG_TMSG_DYN_RES_SMSK  (NFLG_TMSG_DYN_RES_MSK << NFLG_TMSG_DYN_RES_POS)

//resolve all ptr fields when executing
#define NFLG_TMSG_RES_PTR_MSK   0x1
#define NFLG_TMSG_RES_PTR_POS   (NFLG_BITS_SPECIFIC_POS + 6)
#define NFLG_TMSG_RES_PTR_SMSK  (NFLG_TMSG_RES_PTR_MSK << NFLG_TMSG_RES_PTR_POS)


//Block has the following Queues
#define NFLG_BLOCK_QS_MSK   0x7
#define NFLG_BLOCK_QS_POS   (NFLG_BITS_SPECIFIC_POS + 0)
#define NFLG_BLOCK_QS_SMSK  (NFLG_BLOCK_QS_MSK << NFLG_BLOCK_QS_POS)


#define OP_TYPE_SCH_BASE            0x10
#define OP_TYPE_SCH_CLEAR           0x11
#define OP_TYPE_SCH_ADD             0x12
#define OP_TYPE_SCH_OVERWRITE       0x13
#define OP_TYPE_SCH_REMOVE          0x14
#define OP_TYPE_SCH_KEEP            0x15

#define OP_TYPE_CMD_BASE            0x20
#define OP_TYPE_CMD_FLOW            (OP_TYPE_CMD_BASE + ACT_TYPE_FLOW)
#define OP_TYPE_CMD_NOP             (OP_TYPE_CMD_BASE + ACT_TYPE_NOOP)
#define OP_TYPE_CMD_WAIT            (OP_TYPE_CMD_BASE + ACT_TYPE_WAIT)
#define OP_TYPE_CMD_FLUSH           (OP_TYPE_CMD_BASE + ACT_TYPE_FLUSH)
#define OP_TYPE_CMD_START           (OP_TYPE_CMD_BASE + 0x10)
#define OP_TYPE_CMD_STOP            (OP_TYPE_CMD_BASE + 0x11)
#define OP_TYPE_CMD_CEASE           (OP_TYPE_CMD_BASE + 0x12)
#define OP_TYPE_CMD_ABORT           (OP_TYPE_CMD_BASE + 0x13)
#define OP_TYPE_CMD_HALT            (OP_TYPE_CMD_BASE + 0x14)

#endif




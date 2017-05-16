#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>

//basic definitions
#define _8b_SIZE_               1
#define _16b_SIZE_              2
#define _32b_SIZE_              4
#define _64b_SIZE_              8
#define _PTR_SIZE_              _32b_SIZE_
#define _OFFS_SIZE_             _32b_SIZE_
#define _TS_SIZE_               _64b_SIZE_


//////////////////////////////////////////////////////////////////////
//  Defines for dynamic Memory Allocation System                    //
//////////////////////////////////////////////////////////////////////
// Memory Block Size

// allow unaligned blocks ... I'd say yes, but not right now
#define ALLLOW_UNALIGNED        0

//Memory Block (Node) Size in Bytes
#define _MEM_BLOCK_SIZE         52

#define LM32_NULL_PTR           0x0

//////////////////////////////////////////////////////////////////////
//"struct" types                                                    //
//////////////////////////////////////////////////////////////////////

//Thread Control bits
#define T_TC_START              (0)                     //WR Host, RW LM32
#define T_TC_STOP               (TC_START + _32b_SIZE_) //WR Host, RW LM32
#define T_TC_GET                (TC_STOP  + _32b_SIZE_) //RD Host, WR LM32
#define _T_TC_SIZE_             (TC_GET   + _32b_SIZE_) 

//Thread Data
#define T_TD_FLAGS              (0)                           //RD Host, RW LM32
#define T_TD_NODE_PTR           (T_TD_FLAGS     + _32b_SIZE_) //RD Host, RW LM32
#define T_TD_DEADLINE           (T_TD_NODE_PTR  + _PTR_SIZE_) //RD Host, RW LM32
#define T_TD_CURRTIME           (T_TD_DEADLINE  + _TS_SIZE_)  //RD Host, RW LM32
#define T_TD_PREPTIME           (TD_DEADLINE    + _TS_SIZE_)  //RD Host, RW LM32
#define _T_TD_SIZE_             (T_TD_PREPTIME  + _TS_SIZE_)

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
#define _T_SYNC_SIZE            (T_SYNC_TIME  + _TS_SIZE_)

//Command (the received format)
#define T_CMD_TIME              (0)                     // time this command becomes valid
#define T_CMD_ACT               (T_CMD_TIME + _TS_SIZE_)  // action describing the command
//either ...
#define T_CMD_WAIT_TIME         (T_CMD_ACT  + _32b_SIZE_)  // if it's a wait command, this is the new block tPeriod to add/new absolute thread time to use
// ... or
#define T_CMD_FLOW_DEST         (T_CMD_ACT  + _32b_SIZE_) // if it's a flow command, this is the alternative destination to use
#define T_CMD_RES               (T_CMD_DEST + _32b_SIZE_)
// ... or
#define T_CMD_FLUSH_RNG_HILO    (T_CMD_ACT  + _32b_SIZE_) // if it's a flush command and mode bits are set, this defines the q from/to idx to flush per qbuf
#define T_CMD_FLUSH_RNG_IL      (T_CMD_FLUSH_RNG_HILO + _32b_SIZE_)
//
#define _T_CMD_SIZE             (T_CMD_TIME + _TS_SIZE_ + _64b_SIZE_)


//////////////////////////////////////////////////////////////////////
// Control Interface                                                //
//////////////////////////////////////////////////////////////////////

#define _SHCTL_START_    0
#define SHCTL_STATUS     (_SHCTL_START_)                    //Status Registers
#define SHCTL_MSG_CNT    (SHCTL_STATUS       + _32b_SIZE_ ) //CPU wide timing message counter
#define SHCTL_CMD        (SHCTL_MSG_CNT      + _64b_SIZE_ ) //Command Register
#define SHCTL_TGATHER    (SHCTL_CMD          + _32b_SIZE_ ) //Gather Time (HW Priority Queue Config) Register 
#define SHCTL_THR_CTL    (SHCTL_TGATHER      + _T_TS_SIZE_  ) //Thread Control Registers (Start Stop Status) 
#define SHCTL_THR_STA    (SHCTL_THR_CTL      + _T_TC_SIZE_  ) //Thread Start Staging Area (1 per Thread )
#define SHCTL_THR_DAT    (SHCTL_THR_STA      + _THR_QTY_ * _T_TS_SIZE_  ) //Thread Runtime Data (1 per Thread )
#define SHCTL_INBOXES    (SHCTL_THR_DAT      + _THR_QTY_ * _T_TD_SIZE_  ) //Inboxes for MSI (1 per Core in System )
//////////////////////////////////////////////////////////////////////






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
#define BLOCK_ALT_DEST_PTR      (BLOCK_PERIOD       + _TS_SIZE_)   
#define BLOCK_CMDQ_IL_PTR       (BLOCK_ALT_DEST_PTR + _PTR_SIZE_)   
#define BLOCK_CMDQ_HI_PTR       (BLOCK_CMDQ_IL_PTR  + _PTR_SIZE_)   
#define BLOCK_CMDQ_LO_PTR       (BLOCK_CMDQ_HI_PTR  + _PTR_SIZE_)   
#define BLOCK_CMDQ_WR_IDXS      (BLOCK_CMDQ_LO_PTR  + _PTR_SIZE_)   
#define BLOCK_CMDQ_RD_IDXS      (BLOCK_CMDQ_WR_IDXS + _32b_SIZE_)   
                                                                
///// CMDQ Meta Node 
//
// Array of pointers to cmd buffer nodes
#define CMDQ_BUF_ARRAY          (NODE_BEGIN)


///// CMD Buffer Meta Node
//
// Elements of size _CMD_SIZE
#define CMDB_CMD_ARRAY          (NODE_BEGIN)


///// Alternative Destinations Meta Node
//
// Host only, array of pointers to nodes
#define DST_ARRAY            (NODE_BEGIN)

///// Sync Meta Node
//
// Host only, Element format: Src Ptr 32b, Dst Ptr 32b, Time Offset 64b
#define SYNC_ARRAY              (NODE_BEGIN)

//////////////////////////////////////////////////////////////////////
//// Generic Event Attributes ////////////////////////////////////////
#define EVT_BEGIN        (NODE_BEGIN) 
#define EVT_OFFS_TIME           (EVT_BEGIN)                     
#define EVT_HDR_END             (EVT_OFFS_TIME + _TS_SIZE_)       

//// Timing Msg
//
#define TMSG_BEGIN        (EVT_HDR_END)
#define TMSG_ID                 (TMSG_BEGIN)                        
#define TMSG_PAR                (TMSG_ID      + _64b_SIZE_)          
#define TMSG_TEF                (TMSG_PAR     + _64b_SIZE_)          
#define TMSG_RES                (TMSG_TEF     + _32b_SIZE_)          
                                                               
   
//////////////////////////////////////////////////////////////////////
//// Generic Command Attributes //////////////////////////////////////
#define CMD_BEGIN               (EVT_HDR_END)
#define CMD_TARGET              (CMD_BEGIN) 
#define CMD_VALID_TIME          (CMD_TARGET      + _PTR_SIZE_)                          
#define CMD_ACT                 (CMD_VALID_TIME    + _TS_SIZE_)           
#define CMD_HDR_END             (CMD_ACT        + _32b_SIZE_)           
                                                             

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
                                                              
//////////////////////////////////////////////////////////////////////

// Address Vector Order
//
// Destination
#define ADR_DEF_DST            0  
#define ADR_ALT_DST_ARRAY      1 // only if multiple destinations    
//
// Block
#define ADR_BLOCK_DST_LST  1 // only if multiple destinations
#define ADR_BLOCK_Q_IL     2 // only if multiple destinations
#define ADR_BLOCK_Q_HI     3 // only if multiple destinations
#define ADR_BLOCK_Q_LO     4 // only if multiple destinations

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
#define ACT_TYPE_NOOP           0x0  // noop command
#define ACT_TYPE_FLOW           0x1  // flow change command
#define ACT_TYPE_FLUSH          0x2  // flush command 
#define ACT_TYPE_WAIT           0x3  // wait command

//Action Quantity
#define ACT_QTY_MSK             0xffff
#define ACT_QTY_POS             0
#define ACT_QTY_SMSK            ACT_QTY_MSK << ACT_QTY_POS

//Action Type
#define ACT_TYPE_MSK            0xf
#define ACT_TYPE_POS            16
#define ACT_TYPE_SMSK           ACT_TYPE_MSK << ACT_TYPE_POS

//Cmd Action Flags - type specific bit definitions

#define ACT_BITS_SPECIFIC_POS   20
  
//Command Flush Buffer Priority
#define ACT_FLUSH_PRIO_MSK      0x7
#define ACT_FLUSH_PRIO_POS      (ACT_BITS_SPECIFIC_POS + 0)
#define ACT_FLUSH_PRIO_SMSK     ACT_FLUSH_PRIO_MSK << ACT_FLUSH_PRIO_POS

#define PRIO_IL 2
#define PRIO_HI 1
#define PRIO_LO 0

//Command Flush Mode
#define ACT_FLUSH_MODE_MSK      0x7
#define ACT_FLUSH_MODE_POS      (ACT_BITS_SPECIFIC_POS + 3)
#define ACT_FLUSH_MODE_SMSK     ACT_FLUSH_MODE_MSK << ACT_FLUSH_MODE_POS

//Command Wait Time absolute/relative (tPeriod)
#define ACT_WAIT_ABS_MSK        0x1
#define ACT_WAIT_ABS_POS        (ACT_BITS_SPECIFIC_POS + 0)
#define ACT_WAIT_ABS_SMSK       ACT_WAIT_ABS_MSK << ACT_WAIT_ABS_POS


//////////////////////////////////////////////////////////////////////
// Node Flags Bitfield ///////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//Node Flags - bit definitions

// Node Flags - Type field Enums
//Unknown/Undef Node Enum
#define NODE_TYPE_UNKNOWN       0x00  // unknown content, ERROR
// Defined but unspecified data
#define NODE_TYPE_RAW           0xFF  // raw data, do not interprete DEV ONLY!
//Timing Message Enums
#define NODE_TYPE_TMSG          0x10  // dispatches a timing message
//Command Type Enums
#define NODE_TYPE_CNOOP         0x20  // sends a noop command to designated block
#define NODE_TYPE_CFLOW         0x21  // sends a flow change command to designated block
#define NODE_TYPE_CFLUSH        0x22  // sends a flush command to designated block
#define NODE_TYPE_CWAIT         0x23  // sends a wait command to designated block 
//Shared Meta Type Enums
#define NODE_TYPE_BLOCK         0x40  // shows time block tPeriod and if necessary links to Q Meta and altdest nodes
#define NODE_TYPE_QUEUE         0x41  // a command queue meta node (array of pointers to buffer nodes)
#define NODE_TYPE_QBUF          0x42  // a buffer for a command queue
#define NODE_TYPE_SHARE         0x43  // share a value via MSI to multiple memory destinations
//Host only Meta Type Enums
#define NODE_TYPE_ALTDST        0x80  // lists all alternative destinations of a decision block
#define NODE_TYPE_SYNC          0x81  // used to denote the time offset for pattern rows

//Node type
#define NFLG_TYPE_MSK           0xff
#define NFLG_TYPE_POS           0
#define NFLG_TYPE_SMSK          NFLG_TYPE_MSK << NFLG_TYPE_POS

// paint bit - the lm32 has visited this node
#define NFLG_PAINT_LM32_MSK     0x1
#define NFLG_PAINT_LM32_POS     8
#define NFLG_PAINT_LM32_SMSK    NFLG_PAINT_LM32_MSK << NFLG_PAINT_LM32_POS

//paint bit - the host has visited this node
#define NFLG_PAINT_HOST_MSK     0x1
#define NFLG_PAINT_HOST_POS     9
#define NFLG_PAINT_HOST_SMSK    NFLG_PAINT_HOST_MSK << NFLG_PAINT_HOST_POS

// sync bit - this node should only be started synchronous to another
#define NFLG_SYNC_MSK           0x1
#define NFLG_SYNC_POS           10
#define NFLG_SYNC_SMSK          NFLG_SYNC_MSK << NFLG_SYNC_POS


// Type dependent Flags ////////////////////////////////////////////////////////////
#define NFLG_BITS_SPECIFIC_POS  16

// Timing Message //////////////////////////////////////////////////////////////////

//FIXME selling my soul here by allowing dynamic changes to timing msg content. 
// Prime BS caused by Jutta's "we must know which pattern it belongs to and dont want to use a proper DB lookup".
// Evil stuff and likely to explode in our faces at some point

//interprete ID low word as 64b ptr
#define NFLG_TMSG_DYN_ID_MSK    0x1
#define NFLG_TMSG_DYN_ID_POS    (NFLG_BITS_SPECIFIC_POS + 0)
#define NFLG_TMSG_DYN_ID_SMSK   NFLG_TMSG_DYN_ID_MSK << NFLG_TMSG_DYN_ID_POS

//interprete PAR low word as 64b ptr
#define NFLG_TMSG_DYN_PAR_MSK   0x1
#define NFLG_TMSG_DYN_PAR_POS   (NFLG_BITS_SPECIFIC_POS + 1)
#define NFLG_TMSG_DYN_PAR_SMSK  NFLG_TMSG_DYN_PAR_MSK << NFLG_TMSG_DYN_PAR_POS

//interprete TEF low word as 32b ptr
#define NFLG_TMSG_DYN_TEF_MSK   0x1
#define NFLG_TMSG_DYN_TEF_POS   (NFLG_BITS_SPECIFIC_POS + 2)
#define NFLG_TMSG_DYN_TEF_SMSK  NFLG_TMSG_DYN_TEF_MSK << NFLG_TMSG_DYN_TEF_POS

//interprete RES low word as 32b ptr
#define NFLG_TMSG_DYN_RES_MSK   0x1
#define NFLG_TMSG_DYN_RES_POS   (NFLG_BITS_SPECIFIC_POS + 3)
#define NFLG_TMSG_DYN_RES_SMSK  NFLG_TMSG_DYN_RES_MSK << NFLG_TMSG_DYN_RES_POS

#endif




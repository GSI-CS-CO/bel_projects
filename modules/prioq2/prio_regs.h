/** @file        prio_regs.h
  * DesignUnit   prio
  * @author      M. Kreider <m.kreider@gsi.de>
  * @date        23/06/2016
  * @version     0.0.1
  * @copyright   2016 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <prio_auto>
  */

#ifndef _PRIO_H_
#define _PRIO_H_

  #define PRIO_SDB_VENDOR_ID 0x00000651
  #define PRIO_SDB_DEVICE_ID 0x10040200

  #define PRIO_RESET_OWR          0x00  //wo,          1 b, Resets the Priority Queue
  #define PRIO_MODE_GET           0x04  //ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  #define PRIO_MODE_CLR           0x08  //wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  #define PRIO_MODE_SET           0x0c  //wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
  #define PRIO_CLEAR_OWR          0x10  //wo,          1 b, Clears counters and status
  #define PRIO_ST_FULL_GET        0x14  //ro, g_channels b, Channel Full flag (n..0) 
  #define PRIO_ST_LATE_GET        0x18  //ro,          1 b, Late message detected
  #define PRIO_EBM_ADR_RW         0x1c  //rw,         32 b, Etherbone Master address
  #define PRIO_ECA_ADR_RW         0x20  //rw,         32 b, Event Condition Action Unit address
  #define PRIO_TX_MAX_MSGS_RW     0x24  //rw,          8 b, Max msgs per packet
  #define PRIO_TX_MAX_WAIT_RW     0x28  //rw,         32 b, Max wait time for non empty packet
  #define PRIO_TX_RATE_LIMIT_RW   0x2c  //rw,         32 b, Max msgs per milliseconds
  #define PRIO_OFFS_LATE_RW_0     0x30  //rw,         32 b, Time offset before message is late
  #define PRIO_OFFS_LATE_RW_1     0x34  //rw,         32 b, Time offset before message is late
  #define PRIO_CNT_LATE_GET       0x38  //ro,         32 b, Sum of all late messages
  #define PRIO_TS_LATE_GET_0      0x3c  //ro,         32 b, First late Timestamp
  #define PRIO_TS_LATE_GET_1      0x40  //ro,         32 b, First late Timestamp
  #define PRIO_CNT_OUT_ALL_GET_0  0x44  //ro,         32 b, Sum of all outgoing messages
  #define PRIO_CNT_OUT_ALL_GET_1  0x48  //ro,         32 b, Sum of all outgoing messages

#endif

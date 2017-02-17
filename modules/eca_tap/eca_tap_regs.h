/** @file        eca_tap_regs.h
  * DesignUnit   eca_tap
  * @author      M. Kreider <m.kreider@gsi.de>
  * @date        17/02/2017
  * @version     0.0.1
  * @copyright   2017 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <eca_tap_auto>
  */

#ifndef _ECA_TAP_H_
#define _ECA_TAP_H_

  #define ECA_TAP_SDB_VENDOR_ID 0x00000651
  #define ECA_TAP_SDB_DEVICE_ID 0x0eca07a2

  #define ECA_TAP_RESET_OWR      0x00  //wo,  1 b, Resets ECA-Tap
  #define ECA_TAP_CLEAR_OWR      0x04  //wo,  3 b, b2: clear count/accu, b1: clear max, b0: clear min
  #define ECA_TAP_CNT_MSG_GET_0  0x08  //ro, 32 b, Message Count
  #define ECA_TAP_CNT_MSG_GET_1  0x0c  //ro, 32 b, Message Count
  #define ECA_TAP_DIFF_ACC_GET_0 0x10  //ro, 32 b, Accumulated differences (dl - ts)
  #define ECA_TAP_DIFF_ACC_GET_1 0x14  //ro, 32 b, Accumulated differences (dl - ts)
  #define ECA_TAP_DIFF_MIN_GET_0 0x18  //ro, 32 b, Minimum difference
  #define ECA_TAP_DIFF_MIN_GET_1 0x1c  //ro, 32 b, Minimum difference
  #define ECA_TAP_DIFF_MAX_GET_0 0x20  //ro, 32 b, Maximum difference
  #define ECA_TAP_DIFF_MAX_GET_1 0x24  //ro, 32 b, Maximum difference

#endif

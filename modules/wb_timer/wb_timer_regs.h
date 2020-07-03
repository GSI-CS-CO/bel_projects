/** @file        wb_timer_regs.h
  * DesignUnit   wb_timer
  * @author      S. Rauch <s.rauch@gsi.de>
  * @date        15/05/2020
  * @version     0.2.0
  * @copyright   2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <wb_timer_auto>
  */

#ifndef _WB_TIMER_H_
#define _WB_TIMER_H_

  #define WB_TIMER_SDB_VENDOR_ID 0x00000651
  #define WB_TIMER_SDB_DEVICE_ID 0xd8baaa13

  #define WB_TIMER_CONFIG         0x0   //rw,  1 b, bit 0: enable counter
  #define WB_TIMER_PRESET         0x4   //rw, 32 b, counter preset value
  #define WB_TIMER_COUNTER        0x8   //ro, 32 b, actual counter value (when counting down from PRESET)
  #define WB_TIMER_TICKLEN        0xc   //ro, 32 b, period of one timer tick [ns]
  #define WB_TIMER_TIMESTAMP_LO   0x10  //ro, 32 b, timestamp [ticks] low word; the timestamp is latched when reading the low word
  #define WB_TIMER_TIMESTAMP_HI   0x14  //ro, 32 b, timestamp [ticks] high word; read the low word first, high word second

#endif

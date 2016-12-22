/** @file        oled_regs.h
  * DesignUnit   oled
  * @author      M. Kreider <m.kreider@gsi.de>
  * @date        22/12/2016
  * @version     0.2.0
  * @copyright   2016 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <oled_auto>
  */

#ifndef _OLED_H_
#define _OLED_H_

  #define OLED_SDB_VENDOR_ID 0x00000651
  #define OLED_SDB_DEVICE_ID 0x93a6f3c4

  #define OLED_RESET_OWR    0x0   //wo,  1 b, Resets the OLED display
  #define OLED_COL_OFFS_RW  0x4   //rw,  8 b, first visible pixel column. 0x23 for old, 0x30 for new controllers
  #define OLED_UART_OWR     0x8   //wo,  8 b, UART input FIFO. Ascii on b7..0
  #define OLED_CHAR_OWR     0xc   //wo, 19 b, Char input FIFO. Row b18..16, Col b11..8, Ascii b7..0
  #define OLED_RAW_OWR      0x10  //wo, 19 b, Raw  input FIFO. Disp RAM Adr b18..8, Pixel (Col) b7..0

#endif

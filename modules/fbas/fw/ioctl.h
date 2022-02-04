#ifndef _IOCTL_H_
#define _IOCTL_H_

#include <stddef.h>     // size_t
#include <stdint.h>

#include "dbg.h"        // DBPRINT
#include "fbas_common.h"

// LEMO signals (signed int)
enum {
  MPS_SIGNAL_LOW = 0,   // logical '0'
  MPS_SIGNAL_HIGH,      // logical '1'
  MPS_SIGNAL_INVALID    // invalid
};

// For IO control operations look in ip_cores/saftlib/drivers/InoutImpl.cpp
// IO-CTRL register map (ip_cores/saftlib/drivers/io_control_regs.h)
#define IO_CFG_CHANNEL_GPIO          0
#define IO_CFG_CHANNEL_LVDS          1
#define IO_CFG_CHANNEL_FIXED         2

#define IO_GPIO_OE_LEGACYLOW  0x0000
#define IO_GPIO_OE_LEGACYHIGH 0x0008
#define IO_CONFIG             0x0010
#define IO_VERSION            0x0100
#define IO_GPIO_INFO          0x0104
#define IO_GPIO_OE_SETLOW     0x0200
#define IO_GPIO_OE_SETHIGH    0x0204
#define IO_GPIO_OE_RESETLOW   0x0208
#define IO_GPIO_OE_RESETHIGH  0x020c
#define IO_LVDS_OE_SETLOW     0x0300
#define IO_LVDS_OE_SETHIGH    0x0304
#define IO_LVDS_OE_RESETLOW   0x0308
#define IO_LVDS_OE_RESETHIGH  0x030c
#define IO_GPIO_SET_OUTBEGIN  0xa000
#define IO_LVDS_SET_OUTBEGIN  0xb000
#define IO_GPIO_GET_INBEGIN   0xc000
#define IO_LVDS_GET_INBEGIN   0xd000

extern volatile uint32_t *pIOCtrl;             // WB address of IO Control

uint32_t setIoOe(uint32_t channel, uint32_t idx);
uint32_t getIoOe(uint32_t channel);
void driveIo(uint32_t channel, uint32_t idx, uint8_t value);
void driveEffLogOut(uint32_t channel, mpsTimParam_t* buf);
void qualifyInput(size_t len, mpsTimParam_t* buf);
void testOutput(size_t len, mpsTimParam_t* buf);

#endif

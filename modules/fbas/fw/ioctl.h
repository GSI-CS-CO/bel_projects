#ifndef _IOCTL_H_
#define _IOCTL_H_

#include <stddef.h>     // size_t
#include <stdint.h>
#include <stdbool.h>

#include "dbg.h"        // DBPRINT
#include "common-defs.h"
#include "fbas_common.h"

// LEMO signals (signed int)
enum {
  MPS_SIGNAL_LOW = 0,   // logical '0'
  MPS_SIGNAL_HIGH,      // logical '1'
  MPS_SIGNAL_INVALID    // invalid
};

// LEMO output ports
enum {
  N_LEMO_OUT_SCU = 2,     // number of LEMO outputs in SCU
  N_LEMO_OUT_PEXARIA = 3  // Pexaria
};

typedef struct io_port_struct io_port_t;
struct io_port_struct {
  uint32_t type;          // port channel type
  uint8_t  idx;           // port channel index
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

status_t setIoOe(uint32_t channel, uint32_t idx, bool val);
uint32_t getIoOe(uint32_t channel);
void setupEffLogOut(uint8_t buf_idx, uint32_t ch_type, uint8_t ch_idx);
status_t getEffLogOut(uint8_t buf_idx, io_port_t* port);
void driveEffLogOut(mpsMsg_t* buf, uint8_t offset);
void driveOutPort(uint32_t channel, uint8_t idx, uint8_t value);
void qualifyInput(size_t len, mpsMsg_t* buf);
void testOutput(size_t len, mpsMsg_t* buf);

void diagPrintMapIoMsgIdx(void);
#endif

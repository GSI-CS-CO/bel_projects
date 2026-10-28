#ifndef __HKBASE_H
#define __HKBASE_H

#include <stdint.h>

/* board.h (wrpc-import) maps BASE_* to fixed WR core addresses.
 * The housekeeping LM32 discovers its peripherals via SDB at runtime,
 * so use the variables from memlayout.h instead. Include after board.h. */
#undef BASE_MINIC
#undef BASE_EP
#undef BASE_SOFTPLL
#undef BASE_PPS_GEN
#undef BASE_SYSCON
#undef BASE_UART
#undef BASE_ONEWIRE
#undef BASE_ETHERBONE_CFG

#include "memlayout.h"

#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200ULL
#endif

/* no longer provided by hw/wb_uart.h, same layout as in wrpc-import/uart.c */
struct UART_WB {
  uint32_t SR;
  uint32_t BCR;
  uint32_t TDR;
  uint32_t RDR;
} __attribute__ ((packed));

/* busy-wait, implemented in main.c */
int usleep(unsigned usec);

#endif

#ifndef __BOARD_H
#define __BOARD_H

#define CPU_CLOCK 62500000ULL

#define BASE_PIO	    0x100400
#define BASE_OW_WR    0x80060600
#define BASE_OW_EXT   0x80000200
#define BASE_SCU_REG  0x100800

#define UART_BAUDRATE 115200ULL /* not a real UART */

static inline void delay(int x)
{
  while(x--) asm volatile("nop");
}

extern void usleep(int x);
extern volatile unsigned int* BASE_ONEWIRE;
extern volatile unsigned int* BASE_UART;
#endif

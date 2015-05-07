#ifndef __BOARD_H
#define __BOARD_H

#ifndef CPU_CLOCK
#define CPU_CLOCK 62500000ULL
#endif

#define BASE_PIO	    0x100400
#define BASE_OW_WR    0x80060600
#define BASE_OW_EXT   0x80000300
#define BASE_SCU_REG  0x100800

#define UART_BAUDRATE 115200ULL /* not a real UART */

static inline void delay(int x)
{
  while(x--) asm volatile("nop");
}

//extern void usleep(uint32_t x);
//extern  unsigned char* BASE_ONEWIRE;
//extern  unsigned char* BASE_UART;
#endif

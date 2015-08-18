#ifndef __BOARD_H
#define __BOARD_H

#ifndef CPU_CLOCK
#define CPU_CLOCK 125000000ULL
#endif

#define BASE_PIO	    0x100400
#define BASE_OW_WR    0x80060600
#define BASE_OW_EXT   0x80000300
#define BASE_SCU_REG  0x100800
#define BASE_ARU      0x100900

#define CONFIG_SRC    0x0
#define WD_TIMEOUT    0x2
#define WD_EN         0x3
#define PAGE_SEL      0x4
#define CONFIG_MODE   0x5
#define CONFIG        0x8
#define POF_ERROR     0x9

#define CRCERROR      0x1
#define NSTATUS       0x2
#define LOGIC         0x4
#define NCONFIG       0x8
#define WDTIMER       0x10

#define UART_BAUDRATE 115200ULL /* not a real UART */

static inline void delay(int x)
{
  while(x--) asm volatile("nop");
}

extern void usleep(int x);
extern volatile unsigned int* BASE_ONEWIRE;
extern volatile uint32_t* BASE_UART;
#endif

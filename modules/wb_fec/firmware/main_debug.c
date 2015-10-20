#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "pp-printf.h"
#include "mini_sdb.h"
#include "sdb_arg.h"
#include "uart.h"
#include "irq.h"
#include "ethernet.h"
#include <unistd.h>

#define WORDS_MEMORY    0
#define FRAMES_MEMORY   4
#define WORDS_FRAME     8
#define OVERFLOW_RAM    12
#define WORDS_FRAME_WB  16
#define OTHERS_WB       20

#define WR_PPS_GEN_CNTR_UTCLO   0x8

volatile uint8_t *BASE_FRAME_RAM = (uint8_t *) 0x100000;
volatile uint8_t *BASE_FR2WB     = (uint8_t *) 0x200000;

int frame = 0;

void init()
{
   enable_irq();
   discoverPeriphery();
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
}

int main(void) {

  uint32_t words_frame;

  init();

  pp_printf("FEC Unit starting!\n");
  pp_printf("SDB Record %x \n", r_sdb_add());

  //words_frame = *(volatile uint32_t *)(BASE_FR2WB+OTHERS_WB);
  pp_printf("End of FEC reg %x \n",*(volatile uint32_t *)(BASE_FR2WB+OTHERS_WB));
  //pp_printf("Frames Memo    %x \n",*(volatile uint32_t *)(BASE_FR2WB+FRAMES_MEMORY));
  //pp_printf("Words Frames reg %x \n",*(volatile uint32_t *)(BASE_FR2WB+WORDS_FRAME));
  //pp_printf("Overflow reg %x \n",*(volatile uint32_t *)(BASE_FR2WB+OVERFLOW_RAM));
  //pp_printf("Words Frames WB reg  %x \n",*(volatile uint32_t *)(BASE_FR2WB+WORDS_FRAME_WB));

  while(1)
  {
  }

  return 0;

}

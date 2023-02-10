// Build and test
// make lm32-simple-access-clean && make lm32-simple-access && lm32-elf-objdump -D modules/lm32-example/simpleAccess.elf  > dis && eb-fwload -v dev/ttyUSB0 u 0x0 modules/lm32-example/simpleAccess.bin

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mini_sdb.h"
#include "pp-printf.h"
#include "uart.h"

#define LVDS_OE_ON_OFFSET  (0x300>>2)
#define LVDS_OE_OFF_OFFSET (0x308>>2)
#define ALL_BITS           0xffffffff
#define LOOP_CNT           1000000

void loop(void);

void loop(void)
{
  volatile uint32_t cnt = 0;
  for (cnt = 0; cnt < LOOP_CNT; ++cnt) { asm("nop"); }
}

int main(void)
{
  volatile uint32_t *pGPIO_on = NULL;
  volatile uint32_t *pGPIO_off = NULL;

  discoverPeriphery();
  uart_init_hw();

  pGPIO_on = pIOC+LVDS_OE_ON_OFFSET;
  pGPIO_off = pIOC+LVDS_OE_OFF_OFFSET;

  pp_printf("GPIO Base: 0x%x 0x%x 0x%x\n", pIOC, pGPIO_on, pGPIO_off);

  while(1)
  {
    *pGPIO_on = ALL_BITS;
    loop();
    *pGPIO_off = ALL_BITS;
    loop();
  }

  return 0;
}

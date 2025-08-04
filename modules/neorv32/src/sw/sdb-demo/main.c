#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>

#define BAUD_RATE 115200
//#define BAUD_RATE 19200

#define IO_CTL_BASE   0x4010000
#define IO_GPIO_SET   0x0200
#define IO_GPIO_RESET 0x0208

int main(void)
{

  int * p_set = (int*) (IO_CTL_BASE+IO_GPIO_SET);
  int * p_reset = (int*) (IO_CTL_BASE+IO_GPIO_RESET);

  /* Test Wishbone access */
  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  while (true)
  {
    neorv32_aux_print_logo();
    //*p_set = 0xffffffff;
    neorv32_uart0_puts("NEORV32: Hello world!\n");
  }
  //*p_reset = 0xffffffff;

  /* Test return to start.s */
  return 0;
}

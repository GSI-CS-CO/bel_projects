#include <stddef.h>
#include <stdlib.h>

#include <neorv32.h>
#include <neorv32_uart.h>

#include <mini_sdb.h>

#define BAUD_RATE 115200

int main(void)
{
  /* Test SDB */
  discoverPeriphery();

  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  while (true)
  {
    neorv32_aux_print_logo();
    neorv32_uart0_puts("NEORV32: Hello world!\n");
    neorv32_uart0_printf("NEORV32: Found ECA at %p\n", (void*)&pEca);
  }

  /* Test return to start.s */
  return 0;
}

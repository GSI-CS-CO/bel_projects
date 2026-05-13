#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
#include <sdb_add_neorv32.h>

#include <mini_sdb.h>

#define BAUD_RATE 115200
#define CLOCK_HZ 62500000

int main(void)
{
  uint32_t ui_SDBroot = 0;
  uint32_t ui_IMemSize = 0;
  uint32_t ui_DMemSize = 0;
  char keyin;

  /* Set up UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);

  /* Get SDB root */
  ui_SDBroot = (uint32_t) sdb_add();

  /* Start endless loop */
  neorv32_aux_print_logo();
  neorv32_aux_print_hw_config();
  neorv32_uart0_puts("NEORV32: Hello world!\n");
  neorv32_uart0_printf("NEORV32: Found SDB root at 0x%x\n", ui_SDBroot);
  
  while (true)
  {
    neorv32_uart0_puts("\nNEORV32: Press any key to continue\n");
    keyin = neorv32_uart0_getc(); 
    neorv32_uart0_printf("User keyin: %c\n", keyin);
  }

  /* Test return to start.s */
  return 0;
}
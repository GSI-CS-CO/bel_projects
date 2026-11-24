#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <neorv32.h>
#include <neorv32_uart.h>
#include <sdb_add_neorv32.h>

#include <mini_sdb.h>

#define BAUD_RATE 19200   //sim:921600
#define CLOCK_HZ 62500000
#define IDLE_MS 3000

__attribute__ ((__noinline__))
void * get_pc () { return __builtin_return_address(0); }

int main(void)
{ 
  neorv32_uart0_printf("%p\n", get_pc());
  neorv32_uart0_puts("test---test---");
  uint32_t ui_SDBroot = 0;

  //uint32_t ui_IMemSize = 0;
  //uint32_t ui_DMemSize = 0;

  /* Set up UART */
  //neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);

  /* Get SDB root */
  //ui_SDBroot = (uint32_t) sdb_add();
  
  /* Start endless loop */
  neorv32_uart0_printf("%p\n", get_pc());
  neorv32_uart0_puts("test---test---");

  while (true)
  {
    neorv32_uart0_printf("%p\n", get_pc());

    neorv32_aux_print_logo();
    neorv32_aux_print_hw_config();
    neorv32_uart0_puts("NEORV32: Hello world from Yulien!\n");
    neorv32_uart0_printf("NEORV32: Found SDB root at 0x%x\n", ui_SDBroot);
    neorv32_aux_delay_ms(CLOCK_HZ, IDLE_MS);

    neorv32_uart0_printf("%p\n", get_pc());
  }
  
  /* Test return to start.s */
  return 0;
}

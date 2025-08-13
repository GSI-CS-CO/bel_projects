#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>

#define BAUD_RATE 921600

int add_test(int a, int b);

int main(void)
{
  int * cfs_register_0 = (int*) 0xffeb0000;
  int enable = 0x00000001;
  int disable = 0x00000000;
  int * RAM_base_address = (int*) 0x00000000;

  /* Test Wishbone access */
  *cfs_register_0 = enable;
  *(RAM_base_address)   = 0x00000001;
  *(RAM_base_address + 0x4)   = 0x00000002;
  *(RAM_base_address + 0x8)   = 0x00000003;
  *(RAM_base_address + 0xc)   = 0x00000004;
  *(RAM_base_address + 0x10)  = 0x00000005;
  *(RAM_base_address + 0x14)  = 0x00000006;

  *cfs_register_0 = disable;

  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("Hello world!\n");
  *cfs_register_0 = enable;
  neorv32_uart0_printf("Got 0x%x\n", *(RAM_base_address + 0xc));
  *cfs_register_0 = disable;

  /* Test return to start.s */
  return 0;
}

/* Simple test function with arguments */
int add_test(int a, int b)
{
  return a+b;
}

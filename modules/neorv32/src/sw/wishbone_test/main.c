#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>

#define BAUD_RATE 115200

int add_test(int a, int b);

int main(void)
{
  int foo = 0;
  int * p_foo = (int*) 0x4000700;
  int foo_read_1 = 0;
  int foo_read_2 = 0;
  int test = 0x12345678;

  /* Test Wishbone access */
  foo_read_1 = *p_foo;
  *p_foo = 0x00000001;
  *p_foo = 0x00000000;
  foo_read_2 = *p_foo;

  /* Test function call */
  foo = add_test(5,4);

  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("Hello world!\n");
  neorv32_uart0_printf("Got 0x%x\n", foo_read_1);
  neorv32_uart0_printf("Then got 0x%x\n", foo_read_2);

  /* Test return to start.s */
  return foo_read_1;
}

/* Simple test function with arguments */
int add_test(int a, int b)
{
  return a+b;
}

#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <gsi_test_neorv32.h>

#define BAUD_RATE 921600
#define TEST_VALUE 0xffffabcd

int add_test(int a, int b);

int main(void)
{
  int foo = 0;
  int * p_foo = (int*) 0x40000008;
  int foo_read = 0;
  int test = TEST_VALUE;

  /* Test Wishbone access */
  *p_foo = TEST_VALUE;
  foo_read = *p_foo;

  /* Test function call */
  foo = add_test(5,4);

  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("Hello world!\n");
  neorv32_uart0_printf("Got 0x%x\n", foo_read);

  /* End test */
  if (foo_read == TEST_VALUE) { gsi_test_passed(); }
  else                        { gsi_test_failed(); }

  /* Test return to start.s */
  return foo_read;
}

/* Simple test function with arguments */
int add_test(int a, int b)
{
  return a+b;
}

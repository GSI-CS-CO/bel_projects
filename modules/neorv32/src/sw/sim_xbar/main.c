#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <neorv32_gpio.h>

#define BAUD_RATE 115200

#define deadlineHigh  0x00000101
#define deadlineLow   0xc9a08560;
#define evtIDHigh     0x00000000;
#define evtIDLow      0x00236ba9;
#define paramHigh     0x00000000;
#define paramLow      0x133358a4;

int add_test(int a, int b);

int main(void)
{
  int * eca_queue = (int*) 0x7ffffff0;
  int enable = 0x00000001;
  int disable = 0x00000000;
  int * RAM_base_address = (int*) 0x71001000;

  /* Test Wishbone access */
  // *cfs_register_0 = enable;
  neorv32_gpio_pin_set(0, 1);

  *(eca_queue)   = evtIDHigh;
  *(eca_queue)   = evtIDLow;
  *(eca_queue)   = paramHigh;
  *(eca_queue)   = paramLow;
  *(eca_queue)   = 0x0;
  *(eca_queue)   = 0x0;
  *(eca_queue)   = deadlineHigh;
  *(eca_queue)   = deadlineLow;

  neorv32_gpio_pin_set(0, 0);
  // *cfs_register_0 = disable;

  /* Test UART */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("Hello world!\n");
  // *cfs_register_0 = enable;
  // neorv32_gpio_pin_set(0, 1);
  // neorv32_uart0_printf("Got 0x%x\n", *(RAM_base_address + 0xc));
  // neorv32_gpio_pin_set(0, 0);
  // *cfs_register_0 = disable;

  /* Test return to start.s */
  return 0;
}

/* Simple test function with arguments */
int add_test(int a, int b)
{
  return a+b;
}

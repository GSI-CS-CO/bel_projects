#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>

// 1152000 for real hardware, 921600 for simulation
#define BAUD_RATE 921600

int main(void)
{
  int * RAM_base_address = (int*) 0x00000000;

  /* Test block write access */
  neorv32_gpio_pin_set(0, 1);
  *RAM_base_address = 0x00000001;
  *(RAM_base_address + 0x4)   = 0x00000002;
  *(RAM_base_address + 0x8)   = 0x00000003;
  *(RAM_base_address + 0xc)   = 0x00000004;
  *(RAM_base_address + 0x10)  = 0x00000005;
  *(RAM_base_address + 0x14)  = 0x00000006;
  neorv32_gpio_pin_set(0, 0);

  /* Test if the data was written correctly */
  neorv32_gpio_pin_set(0, 1);
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_puts("Hello world!\n");
  neorv32_uart0_printf("0x00: %x\n", *RAM_base_address);
  neorv32_uart0_printf("0x04%x\n", *(RAM_base_address + 0x4));
  neorv32_uart0_printf("0x08: %x\n", *(RAM_base_address + 0x8));
  neorv32_uart0_printf("0x0c: %x\n", *(RAM_base_address + 0xc));
  neorv32_uart0_printf("0x10: %x\n", *(RAM_base_address + 0x10));
  neorv32_uart0_printf("0x14: %x\n", *(RAM_base_address + 0x14));
  neorv32_gpio_pin_set(0, 0);

  return 0;
}

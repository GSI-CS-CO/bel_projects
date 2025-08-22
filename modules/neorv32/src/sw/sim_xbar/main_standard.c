#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <neorv32_gpio.h>

// 1152000 for real hardware, 921600 for simulation
#define BAUD_RATE 921600 

#define deadlineHigh  0x00000101
#define deadlineLow   0xc9a08560;
#define evtIDHigh     0x00000000;
#define evtIDLow      0x00236ba9;
#define paramHigh     0x00000000;
#define paramLow      0x133358a4;

int main(void)
{
  int * eca_queue = (int*) 0x7ffffff0;

  /* Test ECA queue injection */
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

  return 0;
}

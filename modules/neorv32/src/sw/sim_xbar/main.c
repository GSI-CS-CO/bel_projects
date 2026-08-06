#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <stdbool.h>

// 1152000 for real hardware, 921600 for simulation
#define BAUD_RATE 921600

#define N 6

int main(void)
{
  int * RAM_base_address = (int*) 0x04060000;
  //int * RAM_base_address = (int*) 0x00200000; // for simulation


  int nums[N] = {0};
  int nums_test[N] = {0};
  bool failed;

  /* Test block write access */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_printf("Starting the loop.\n");

  while(true) {
    failed = false;
    for(int i = 0; i < N; i++) {
      nums[i] = i; // neorv32_aux_xorshift32();
    }

    neorv32_uart0_printf("#1.\n");

    //neorv32_gpio_pin_set(0, 1);
    for(int i = 0; i < N; i++) {
      *(RAM_base_address + i)   = nums[i];
    }
    //neorv32_gpio_pin_set(0, 0);

    neorv32_uart0_printf("#2.\n");

    //neorv32_gpio_pin_set(0, 1);
    for(int i = 0; i < N; i++) {
      nums_test[i] = *(RAM_base_address + i);
    }
    //neorv32_gpio_pin_set(0, 0);

    neorv32_uart0_printf("#3.\n");

    for(int i = 0; i < N; i++) {
      if(nums[i] != nums_test[i]) {
        neorv32_uart0_printf("Data at address 0x%x is not correct, expected 0x%x, got 0x%x\n", (RAM_base_address + i), nums[i], nums_test[i]);
        failed = true;
      }
    }

    if(!failed) {
       neorv32_uart0_printf("Data transfers successfull, no data lost.\n");
       for(int i = 0; i < N; i++) {
         if(nums[i] == nums_test[i]) {
           neorv32_uart0_printf("Expected 0x%x, got 0x%x\n", nums[i], nums_test[i]);
         }
       }
    }
  }

  return 0;
}

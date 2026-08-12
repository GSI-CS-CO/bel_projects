#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <stdbool.h>

// 1152000 for real hardware, 921600 for simulation
#define BAUD_RATE 19200

#define N 6

int main(void)
{
  int * RAM_base_address = (int*) 0x71000000;   //origin:0x4060000

  int nums[N];
  int nums_test[N];
  bool failed;

  /* Test block write access */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);
  neorv32_uart0_printf("Starting the loop.\n");
    
  failed = false;
    for(int i = 0; i < N; i++) {
      nums[i] = neorv32_aux_xorshift32();
      neorv32_uart0_printf("1st for: 0x%x\n", nums[i]);
    }

    neorv32_gpio_pin_set(0, 1);
    for(int i = 0; i < N; i++) {
      neorv32_uart0_printf("2nd before: 0x%x\n",*(RAM_base_address + i));
      *(RAM_base_address + i)   = nums[i];
      neorv32_uart0_printf("2nd after: 0x%x\n",*(RAM_base_address + i));
    }
    neorv32_gpio_pin_set(0, 0);

    neorv32_gpio_pin_set(0, 1);
    for(int i = 0; i < N; i++) {
      nums_test[i] = *(RAM_base_address + i);
      neorv32_uart0_printf("3rd for: 0x%x\n", nums_test[i]);
    }
    neorv32_gpio_pin_set(0, 0);

    for(int i = 0; i < N; i++) {
      neorv32_uart0_printf("nums[i]=0x%x\n", nums[i]);
      neorv32_uart0_printf("nums_test[i]=0x%x\n", nums_test[i]);
      if(nums[i] != nums_test[i]) {
        neorv32_uart0_printf("Data at address 0x%x is not correct, expected 0x%x, got 0x%x\n", (RAM_base_address + i), nums[i], nums_test[i]);
        failed = true;
      }
    }

    // if(!failed) {
    //   neorv32_uart0_printf("Data transfers successfull, no data lost.\n");
    // }
  

  return 0;
}

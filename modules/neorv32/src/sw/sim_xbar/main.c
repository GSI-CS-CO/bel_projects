#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <gsi_test_neorv32.h>
#include <gsi_wishbone_neorv32.h>
#include <stdbool.h>

#define BAUD_RATE 921600 /* 1152000 for real hardware, 921600 for simulation */
#define SEED_0 0x12345678
#define SEED_1 0x90abcdef
#define N 6

void run_test(bool atomic);

int main(void)
{
  /* Test block write access */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);

  run_test(false);
  run_test(true);
  gsi_test_passed();

  return 0;
}

void run_test(bool atomic)
{
  int * RAM_base_address = (int*) 0x04060000;
  int nums[N] = {0};
  int nums_test[N] = {0};
  bool failed = false;
  bool stop = false;

  neorv32_uart0_printf("Starting the loop.\n");

  while(!stop) {
    failed = false;

    for(int i = 0; i < N; i++) {
      if (!atomic) nums[i] = SEED_0 << i;
      else         nums[i] = SEED_1 << i;
    }

    if (atomic) gsi_wishbone_start_atomic_access();
    for(int i = 0; i < N; i++) {
      *(RAM_base_address + i)   = nums[i];
    }
    if (atomic) gsi_wishbone_stop_atomic_access();

    if (atomic) gsi_wishbone_start_atomic_access();
    for(int i = 0; i < N; i++) {
      nums_test[i] = *(RAM_base_address + i);
    }
    if (atomic) gsi_wishbone_stop_atomic_access();

    for(int i = 0; i < N; i++) {
      if(nums[i] != nums_test[i]) {
        neorv32_uart0_printf("Data at address 0x%x is not correct, expected 0x%x, got 0x%x\n", (RAM_base_address + i), nums[i], nums_test[i]);
        failed = true;
      }
    }
    if(!failed) {
       neorv32_uart0_printf("Data transfers successfull, no data lost.\n");
       for(int i = 0; i < N; i++) {
         if(nums[i] != nums_test[i]) {
           neorv32_uart0_printf("Expected 0x%x, got 0x%x\n", nums[i], nums_test[i]);
           gsi_test_failed();
         }
       }
    }
    stop = true;
  }
}

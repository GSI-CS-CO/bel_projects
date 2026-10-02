#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>
#include <gsi_test_neorv32.h>
#include <gsi_wishbone_neorv32.h>
#include <gsi_sdb_neorv32.h>
#include <stdbool.h>

/* Defines */
#define BAUD_RATE       921600 /* 1152000 for real hardware, 921600 for simulation */
#define SEED_0          0x12345678
#define SEED_1          0x90abcdef
#define RAM_DEV_ID      0x66cfeb52
#define RAM_ADR_FIRST   0x04060000
#define RAM_ADR_SECOND  0x05060000
#define N               6
#define NUM_OF_RAMS     2

/* Prototypes */
void     run_test_first_ram(bool atomic, bool use_sdb);
void     run_test_second_ram(bool use_sdb);
uint32_t get_ram_base(uint32_t id);

/* Main */
int main(void)
{
  /* Test block write access */
  neorv32_rte_setup();
  neorv32_uart0_setup(BAUD_RATE, 0);

  /* No SDB */
  run_test_first_ram(false, false);
  run_test_first_ram(true, false);
  run_test_second_ram(false);

  /* With SDB */
  run_test_first_ram(false, true);
  run_test_first_ram(true, true);
  run_test_second_ram(true);

  /* Done */
  gsi_test_passed();
  return 0;
}

void run_test_first_ram(bool atomic, bool use_sdb)
{
  volatile int * RAM_base_address = (int*) 0x0;
  int nums[N] = {0};
  int nums_test[N] = {0};
  bool failed = false;
  bool stop = false;

  neorv32_uart0_printf("Starting the loop.\n");

  if (use_sdb) { RAM_base_address = (int*) get_ram_base(0); }
  else         { RAM_base_address = (int*) RAM_ADR_FIRST; }

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

void run_test_second_ram(bool use_sdb)
{
  volatile int * RAM_base_address = (int*) 0x0;

  if (use_sdb) { RAM_base_address = (int*) get_ram_base(1); }
  else         { RAM_base_address = (int*) RAM_ADR_SECOND; }

  *RAM_base_address = SEED_0;
  if (*RAM_base_address != SEED_0)
  {
    gsi_test_failed();
  }
}

uint32_t get_ram_base(uint32_t id)
{
  uint32_t ram_addresses[NUM_OF_RAMS];
  devices found = 0;

  /* Find both simulated RAMs in SDB order on every call. */
  uint32_t status = gsi_sdb_find_device(ram_addresses,
                                       GSI_SDB_CERN_VENDOR_ID,
                                       RAM_DEV_ID,
                                       &found, NUM_OF_RAMS);

  if (status != 0 || found != NUM_OF_RAMS || id >= NUM_OF_RAMS)
  {
    neorv32_uart0_printf("RAM lookup failed: status=%u, found=%u, id=%u\n", status, found, id);
    gsi_test_failed();
  }

  return ram_addresses[id];
}

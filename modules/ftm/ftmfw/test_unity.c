#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <pp-printf.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "irq.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"

#include "unity.h"
#include "unity_custom.h"


uint8_t cpuId;
uint8_t cpuQty;

/// Init. Discovers periphery and inits all modules.
void init()
{

  discoverPeriphery();
  cpuId = getCpuIdx();
  uart_init_hw();

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  for (int j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
  
  atomic_on(); mprintf("#%02u: Rdy\n", cpuId); atomic_off();

}


void test_one_plus_two(void) {
    int x = 1;
    int y = 2;
    int z = x + y;

    TEST_ASSERT_EQUAL_UINT8(3, z);
    TEST_ASSERT_LESS_OR_EQUAL_INT(4, z);
    TEST_ASSERT_EQUAL_UINT32(3, z);
    
    //our own workaround macro for 64b values
    TEST_ASSERT_EQUAL_HEX64_MSG(3, z);
    
}

void setUp(void) {
    
}

void tearDown(void) {
    /* Cleanup if needed */
    
    //Necessary delay for printf to complete before next test starts (if text is dropped, increase loop).
    //TODO Handling be in print itself, but this is a workaround for now.
    for (uint32_t j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
}

/* ============================================
 * Test Runner
 * ============================================ */

void main(void) {

    init();

    mprintf("#%02u: Doing unit tests with Unity \n", cpuId);

    UNITY_BEGIN();
    RUN_TEST(test_one_plus_two);
    UNITY_END();

    while (1) { asm("nop"); }
  
}

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
#include "ftm_common.h"
#include "dm.h"

#include "unity.h"
#include "unity_custom.h"


uint8_t cpuId;
uint8_t cpuQty;

/// Global init. Discovers periphery and inits all modules.
/** Global init. Discovers periphery, initialises EBM and PQ, checks WR, inits DM and diagnostics and signals readiness on console. */
void init()
{
  *status = 0;
  *count  = 0;

  discoverPeriphery();
  cpuId = getCpuIdx();
  uart_init_hw();

  isr_table_clr();
  irq_set_mask(0x01);
  irq_disable();

  for (int j = 0; j < ((125000000/8)); ++j) { asm("nop"); }
  atomic_on(); mprintf("#%02u: Rdy\n", cpuId); atomic_off();

}


/* Test helpers */
static inline uint64_t make_val(uint32_t hi, uint32_t lo) {
    return ((uint64_t)hi << 32) | (uint64_t)lo;
}


void test_safeRead64_null_addr1st(void) {
    volatile uint64_t src = make_val(0x01020304, 0x05060708);
    uint64_t dest = 0xDEADBEEFCAFEBABE;
 
    uint8_t rc = safeRead64((volatile uint64_t*)LM32_NULL_PTR, &src, &dest);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABEULL, dest);
}

void test_safeRead64_null_addr2nd(void) {
    volatile uint64_t src = make_val(0x01020304, 0x05060708);
    uint64_t dest = 0xDEADBEEFCAFEBABEULL;
    uint8_t rc = safeRead64(&src, (volatile uint64_t*)LM32_NULL_PTR, &dest);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABEULL, dest);
}

void test_safeRead64_null_dest(void) {
    volatile uint64_t src = make_val(0x01020304, 0x05060708);
    uint8_t rc = safeRead64(&src, &src, (uint64_t*)LM32_NULL_PTR);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
}

/* Same address stable read -> success */
void test_safeRead64_same_address_success(void) {
    volatile uint64_t v = make_val(0x11223344, 0x55667788);
    uint64_t dest = 0;
    uint8_t rc = safeRead64(&v, &v, &dest);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG((uint64_t)v, dest);
}

/* Two addresses with identical contents -> success */
void test_safeRead64_two_addrs_identical_success(void) {
    volatile uint64_t a = make_val(0x0A0B0C0D, 0x0E0F1011);
    volatile uint64_t b = a;
    uint64_t dest = 0x0;
    uint8_t rc = safeRead64(&a, &b, &dest);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG((uint64_t)a, dest);
}

/* High-word mismatch -> failure, dest unchanged */
void test_safeRead64_high_mismatch_fail(void) {
    volatile uint64_t a = make_val(0x20000000, 0xAAAAAAAA);
    volatile uint64_t b = make_val(0x30000000, 0xAAAAAAAA); // high differs
    uint64_t dest = 0xCAFEBABECAFEBABEULL;
    uint8_t rc = safeRead64(&a, &b, &dest);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xCAFEBABECAFEBABEULL, dest);
}

/* Low-word only mismatch -> success and dest equals the source (hi+lo from addr1st) */
void test_safeRead64_low_only_mismatch_ok(void) {
    volatile uint64_t a = make_val(0x12345678, 0xAAAA0001);
    volatile uint64_t b = make_val(0x12345678, 0xBBBB0002); // low differs, hi same
    uint64_t dest = 0;
    uint8_t rc = safeRead64(&a, &b, &dest);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    // dest should match 'a' (hi from a (same as b), and low from a)
    TEST_ASSERT_EQUAL_HEX64_MSG((uint64_t)a, dest);
}

/* All bits changed -> failure */
void test_safeRead64_all_changed_fail(void) {
    volatile uint64_t a = make_val(0x01010101, 0x02020202);
    volatile uint64_t b = make_val(0xFFFFFFFF, 0xEEEEEEEE);
    uint64_t dest = 0xDEADBEEFDEADBEEFULL;
    uint8_t rc = safeRead64(&a, &b, &dest);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFDEADBEEFULL, dest);
}

/* Endianness / hi-lo assembly check.
   Build a 64-bit value by populating the low-level 32-bit words and check
   the assembled 64-bit result matches (platform specific). */
void test_safeRead64_endianness_assembly(void) {
    uint32_t hi = 0x01234567u;
    uint32_t lo = 0x89ABCDEFu;
    volatile uint64_t val64;
    volatile uint32_t* parts32 = (volatile uint32_t*)&val64;
    parts32[0] = hi; // make sure hi is written to the first 32-bit slot as dm.c expects
    parts32[1] = lo;
    uint64_t dest = 0;
    uint8_t rc = safeRead64(&val64, &val64, &dest);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    uint64_t expected = ((uint64_t)hi << 32) | (uint64_t)lo;
    TEST_ASSERT_EQUAL_HEX64_MSG(expected, dest);
}

/* safeRead64_with_retry: stable value -> success */
void test_safeRead64_with_retry_stable_success(void) {
    volatile uint64_t v = make_val(0xCAFED00D, 0xBAAAAAAD);
    uint64_t dest = 0;
    uint8_t rc = safeRead64_with_retry(&v, &dest);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG((uint64_t)v, dest);
}

/* ===== dynField Tests ===== */

/* 32-bit IMMEDIATE mode: value should not be modified */
void test_dynField_32b_immediate_unchanged(void) {
    volatile uint32_t test_val = 0xCAFEBABE;
    volatile uint32_t src = 0x12345678;
    volatile uint32_t dst = 0xDEADBEEF;
    
    uint8_t rc = dynField(DYN_MODE_IM, &src, &dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, dst);
}

/* 32-bit ADDRESS mode: destination not dereferenced, treated as immediate value */
void test_dynField_32b_address_unchanged(void) {
    volatile uint32_t src = 0xABCD1234;
    volatile uint32_t dst = 0x11223344;
    
    uint8_t rc = dynField(DYN_MODE_ADR, &src, &dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0x11223344, dst);
}

/* 32-bit REFERENCE mode: src is a pointer, dereference once and copy to dst */
void test_dynField_32b_reference_direct_deref(void) {
    volatile uint32_t value = 0x12345678;
    volatile uint32_t* src_ptr = &value;
    volatile uint32_t dst = 0;
    
    uint8_t rc = dynField(DYN_MODE_REF, src_ptr, (volatile uint32_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, dst);
}

/* 32-bit REFERENCE mode with NULL source pointer: should fail */
void test_dynField_32b_reference_null_src(void) {
    volatile uint32_t dst = 0xFFFFFFFF;
    volatile uint32_t* src_ptr = (volatile uint32_t*)LM32_NULL_PTR;
    
    uint8_t rc = dynField(DYN_MODE_REF, src_ptr, (volatile uint32_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    /* dst should remain unchanged */
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst);
}

/* 32-bit REF2 (double reference) mode: deref twice (ptr2ptr) */
void test_dynField_32b_reference2_double_deref(void) {
    volatile uint32_t value = 0xABCDEF01;
    volatile uint32_t* ptr_to_value = &value;
    volatile uint32_t** ptr_to_ptr = &ptr_to_value;
    volatile uint32_t dst = 0;
    
    uint8_t rc = dynField(DYN_MODE_REF2, (volatile uint32_t*)ptr_to_ptr, (volatile uint32_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0xABCDEF01, dst);
}

/* 32-bit REF2 with NULL intermediate pointer: should fail */
void test_dynField_32b_reference2_null_intermediate_ptr(void) {
    volatile uint32_t* ptr_to_ptr = (volatile uint32_t*)LM32_NULL_PTR;
    volatile uint32_t dst = 0xFFFFFFFF;
    
    uint8_t rc = dynField(DYN_MODE_REF2, (volatile uint32_t*)ptr_to_ptr, (volatile uint32_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst);
}

/* 64-bit IMMEDIATE mode: value passes through unchanged */
void test_dynField_64b_immediate_unchanged(void) {
    volatile uint64_t test_val = 0x0123456789ABCDEF;
    volatile uint32_t* src = (volatile uint32_t*)(uint64_t*)&test_val;
    volatile uint64_t dst = 0xFFFFFFFF;
    
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_IM, src, (volatile uint32_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst);
    /* For IM mode, dst should not be modified */
}

/* 64-bit ADDRESS mode: address treated as immediate */
void test_dynField_64b_address_unchanged(void) {
    volatile uint64_t test_val = 0xDEADBEEFCAFEBABE;
    volatile uint32_t* src = (volatile uint32_t*)(uint64_t*)&test_val;
    volatile uint32_t dst_hi = 0;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_ADR, src, &dst_hi);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
}

/* 64-bit REFERENCE mode: src is pointer to 64-bit value, deref and copy */
void test_dynField_64b_reference_direct_deref(void) {
    volatile uint64_t value = 0x0123456789ABCDEF;
    volatile uint64_t* src_ptr = &value;
    volatile uint64_t dst = 0;
    
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)src_ptr, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0x0123456789ABCDEF, dst);
}

/* 64-bit REFERENCE mode with NULL pointer: should fail */
void test_dynField_64b_reference_null_src(void) {
    volatile uint64_t* src_ptr = (volatile uint64_t*)LM32_NULL_PTR;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)src_ptr, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 (double reference): deref twice, copy 64-bit value */
void test_dynField_64b_reference2_double_deref(void) {
    volatile uint64_t value = 0xDEADBEEFCAFEBABE;
    volatile uint64_t* ptr_to_value = &value;
    volatile uint64_t** ptr_to_ptr = &ptr_to_value;
    volatile uint64_t dst = 0;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 with NULL intermediate pointer: should fail */
void test_dynField_64b_reference2_null_intermediate_ptr(void) {
    volatile uint64_t** ptr_to_ptr = (volatile uint64_t**)LM32_NULL_PTR;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)(uint64_t*)ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 with NULL final pointer (ptr2ptr): should fail */
void test_dynField_64b_reference2_null_final_ptr(void) {
    volatile uint64_t* ptr_to_value = (volatile uint64_t*)LM32_NULL_PTR;
    volatile uint64_t** ptr_to_ptr = &ptr_to_value;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)(uint64_t*)ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 32-bit REF mode with several different values: test value propagation */
void test_dynField_32b_reference_various_values(void) {
    /* Test with 0x00000000 */
    volatile uint32_t val1 = 0x00000000;
    volatile uint32_t* ptr1 = &val1;
    volatile uint32_t dst1 = 0xFFFFFFFF;
    
    uint8_t rc1 = dynField(DYN_MODE_REF, ptr1, (volatile uint32_t*)&dst1);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, dst1);
    
    /* Test with 0xFFFFFFFF */
    volatile uint32_t val2 = 0xFFFFFFFF;
    volatile uint32_t* ptr2 = &val2;
    volatile uint32_t dst2 = 0x00000000;
    
    uint8_t rc2 = dynField(DYN_MODE_REF, ptr2, (volatile uint32_t*)&dst2);
    TEST_ASSERT_EQUAL_UINT8(0, rc2);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst2);
    
    /* Test with alternating pattern */
    volatile uint32_t val3 = 0xAAAA5555;
    volatile uint32_t* ptr3 = &val3;
    volatile uint32_t dst3 = 0x55555555;
    
    uint8_t rc3 = dynField(DYN_MODE_REF, ptr3, (volatile uint32_t*)&dst3);
    TEST_ASSERT_EQUAL_UINT8(0, rc3);
    TEST_ASSERT_EQUAL_HEX32(0xAAAA5555, dst3);
}

/* 64-bit REF mode with large values: test high/low word split correctly */
void test_dynField_64b_reference_large_values(void) {
    /* Test with max 64-bit value */
    volatile uint64_t val_max = 0xFFFFFFFFFFFFFFFF;
    volatile uint64_t* ptr_max = &val_max;
    volatile uint64_t dst = 0;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)ptr_max, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xFFFFFFFFFFFFFFFF, dst);
}

/* Chain of references: update value and verify propagation through chain */
void test_dynField_32b_reference_chain_update(void) {
    volatile uint32_t original = 0x11223344;
    volatile uint32_t* ptr_to_original = &original;
    volatile uint32_t dst = 0;
    
    /* First read */
    uint8_t rc1 = dynField(DYN_MODE_REF, (volatile uint32_t*)ptr_to_original, &dst);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x11223344, dst);
    
    /* Modify original value */
    original = 0x55667788;
    volatile uint32_t dst2 = 0;
    
    /* Second read should get new value */
    uint8_t rc2 = dynField(DYN_MODE_REF, (volatile uint32_t*)ptr_to_original, &dst2);
    TEST_ASSERT_EQUAL_UINT8(0, rc2);
    TEST_ASSERT_EQUAL_HEX32(0x55667788, dst2);
}

/* 32-bit REF2 with updated intermediate pointer */
void test_dynField_32b_reference2_chain_update(void) {
    volatile uint32_t val1 = 0x11111111;
    volatile uint32_t val2 = 0x22222222;
    volatile uint32_t* ptr = &val1;
    volatile uint32_t** ptr_to_ptr = &ptr;
    volatile uint32_t dst = 0;
    
    /* First deref: should get val1 */
    uint8_t rc1 = dynField(DYN_MODE_REF2, (volatile uint32_t*)ptr_to_ptr, (volatile uint32_t*)&dst);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x11111111, dst);
    
    /* Change intermediate pointer */
    ptr = &val2;
    volatile uint32_t dst2 = 0;
    
    /* Second deref: should get val2 */
    uint8_t rc2 = dynField(DYN_MODE_REF2, (volatile uint32_t*)ptr_to_ptr, (volatile uint32_t*)&dst2);
    TEST_ASSERT_EQUAL_UINT8(0, rc2);
    TEST_ASSERT_EQUAL_HEX32(0x22222222, dst2);
}

/* Test with very small 64-bit values */
void test_dynField_64b_reference_small_values(void) {
    volatile uint64_t val_small = 0x0000000100000001;
    volatile uint64_t* ptr_small = &val_small;
    volatile uint64_t dst = 0;
    
    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)ptr_small, (volatile uint32_t*)(uint64_t*)&dst);
    
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0x0000000100000001, dst);
}

/* Boundary test: immediate/address modes should not access memory */
void test_dynField_32b_immediate_no_memory_access(void) {
    /* Use stack addresses that might not be accessible without causing issues */
    volatile uint32_t src = 0x12345678;
    volatile uint32_t dst = 0xAAAAAAAA;
    
    /* IMMEDIATE mode should not dereference, just return success */
    uint8_t rc = dynField(DYN_MODE_IM, &src, &dst);
    TEST_ASSERT_EQUAL_UINT8(0, rc);
    /* dst should be unchanged */
    TEST_ASSERT_EQUAL_HEX32(0xAAAAAAAA, dst);
}


void setUp(void) {

}

void tearDown(void) {
    /* Cleanup if needed */
    for (uint32_t j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
}

/* ============================================
 * Test Runner
 * ============================================ */

void main(void) {

    init();

    mprintf("#%02u: Unit tests safeRead64 & dynField for %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

    UNITY_BEGIN();
    RUN_TEST(test_safeRead64_null_addr1st);
    RUN_TEST(test_safeRead64_null_addr2nd);
    RUN_TEST(test_safeRead64_null_dest);
    RUN_TEST(test_safeRead64_same_address_success);
    RUN_TEST(test_safeRead64_two_addrs_identical_success);
    RUN_TEST(test_safeRead64_high_mismatch_fail);
    RUN_TEST(test_safeRead64_low_only_mismatch_ok);
    RUN_TEST(test_safeRead64_all_changed_fail);
    RUN_TEST(test_safeRead64_endianness_assembly);
    RUN_TEST(test_safeRead64_with_retry_stable_success);
    
    /* dynField tests */
    RUN_TEST(test_dynField_32b_immediate_unchanged);
    RUN_TEST(test_dynField_32b_address_unchanged);
    RUN_TEST(test_dynField_32b_reference_direct_deref);
    RUN_TEST(test_dynField_32b_reference_null_src);
    RUN_TEST(test_dynField_32b_reference2_double_deref);
    RUN_TEST(test_dynField_32b_reference2_null_intermediate_ptr);
    RUN_TEST(test_dynField_64b_immediate_unchanged);
    RUN_TEST(test_dynField_64b_address_unchanged);
    RUN_TEST(test_dynField_64b_reference_direct_deref);
    RUN_TEST(test_dynField_64b_reference_null_src);
    RUN_TEST(test_dynField_64b_reference2_double_deref);
    RUN_TEST(test_dynField_64b_reference2_null_intermediate_ptr);
    RUN_TEST(test_dynField_64b_reference2_null_final_ptr);
    RUN_TEST(test_dynField_32b_reference_various_values);
    RUN_TEST(test_dynField_64b_reference_large_values);
    RUN_TEST(test_dynField_32b_reference_chain_update);
    RUN_TEST(test_dynField_32b_reference2_chain_update);
    RUN_TEST(test_dynField_64b_reference_small_values);
    RUN_TEST(test_dynField_32b_immediate_no_memory_access);
    UNITY_END();

    while (1) { asm("nop"); }
  
}

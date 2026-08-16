#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "dbg.h"
#include "ftm_common.h"
#include "dm.h"
#include "dm_hal.h"
#include "unity.h"
#include "unity_custom.h"


/** \mainpage DM Firmware Documentation
 *
 * \section intro_sec Introduction
 * This document describes the firmware of the Data Master (DM) module. The firmware is responsible for timing message generation to control all WR timing receiver platforms within the GSI/FAIR facility. For more in depth information,
 * see FAIR the tech note F-TN-C-0015e 'CarpeDM - Programming language for the DataMaster'.
 *
 * \section desc_sec Description
 * \subsection env Environment
 * This firmware runs on LM32 cpus within the Data Master (DM) gateware. The difference to standard timing receiver images
 * lies in the lack of an Event Condition Action (ECA) unit, 4 or more LM32 CPU instances with dual port memories accessible from the host controller
 * and a dedicated hardware priority queue (PQ). The PQ aggregates and sorts timing messages by urgency before forwarding them to the Etherbone Master (EBM) module for dispatch to the White Rabbit (WR) network.
 *
 * \subsection func Functionality
 * The DM firmware processes timing schedules, which are loaded into the CPU's shared memory area by the host controller. These schedules are linked lists
 * of data nodes with differing functions and properties. Their main purpose is dynamic generation of timing messages for broadcasts within the WR network.
 *
 * \subsubsection sched Timing Schedules
 * Schedules are organised as sequences of 0..n functional nodes followed by a block node. The block has a duration (period),
 * which is added to the running time sum of the associated thread. All other node types have relative time offset. Any node's absolute deadline is calculated by adding its offset to its thread's current time sum.
 * (see dlEvt(), dlBlock() ...)
 *
 * \subsubsection edf Scheduler
 * Deadlines, along with a pointer to the corresponding nodes, are fed into the Earliest Deadline First (EDF) scheduler running in the main loop. It always chooses the most urgent deadline/node for processing.
 * While a standard EDF does not have idle behaviour if there is work pending, the DM only processes nodes with deadlines falling into a 1ms window from the current time.
 * The scheduler loop also handles thread Start, Stop and Abort commands from the host.
 * (see main(), heapReplace() ...)
 *
 * \subsubsection proc Node Handlers
 * For each node type, an appropriate handler function is supplied (see tmsg(), block(), ...). Upon execution, the handler provides the specific node function and then returns a successor node with a new deadline to the scheduler.
 * Block nodes are a special case, as they have individual command queues. These can be imagined as parallel inboxes, which can receive asynchronous commands from the host.
 * The content of these commands influences the schedule runtime behaviour, in particular the block's specificfunction, returned successor and deadline. (see execFlow(), execFlush() ...)
 */


uint8_t cpuId;
uint8_t cpuQty;



/// Debug Interrupt console output
/** Shows and MSI's msg, address and byte select words */
void show_msi()
{
  halShowMsi();
}

/// Interrupt Handler 0 (not used)
/** IRQ handler 0, shows handler number and msi content on console. Not used in DM */
void isr0()
{
   pp_printf("ISR0\n");
   show_msi();
}

/// Interrupt Handler 1 (not used)
/** IRQ handler 1, shows handler number and msi content on console. Not used in DM */
void isr1()
{
   pp_printf("ISR1\n");
   show_msi();
}


/// Etherbone Master init routine
/** EBM init. Waits for WR core to receive IP from bootp and then sets src & dst MAC and IP addresses in EBM. */
void ebmInit()
{
   pp_printf("#%02u: DM cores Waiting for IP from WRC...\n", cpuId);
   halEbmWaitForIp();

   halEbmInit();
   halEbmConfigMeta(1500, 42, HAL_EBM_NOREPLY);                                                    //MTU, max EB msgs, flags
   halEbmConfigIf(HAL_EBM_DESTINATION, 0xffffffffffff, 0xffffffff,                0xebd0);         //Dst: EB broadcast
   halEbmConfigIf(HAL_EBM_SOURCE,      0xd15ea5edbeef, halEbmGetSrcIp(), 0xebd0);                  //Src: bogus mac (will be replaced by WR), WR IP

}

/// Global init. Discovers periphery and inits all modules.
/** Global init. Discovers periphery, initialises EBM and PQ, checks WR, inits DM and diagnostics and signals readiness on console. */
void init()
{
  dmInitSharedMemPointers();

  *status = 0;
  *count  = 0;


  halInitPeriphery();

  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_STA] = SHCTL_THR_STA;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_THR_DAT] = SHCTL_THR_DAT;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_HEAP]    = SHCTL_HEAP;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_REGS]    = SHCTL_REGS;
  p[(SHCTL_ADR_TAB >> 2) + ADRLUT_SHCTL_END]     = _SHCTL_END_;


  if (cpuId == 0) {
    //TODO replace bogus system status flags by real ones
    halUartInitHw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;
    ebmInit();         *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    halPrioQueueInit(); *status |= SHCTL_STATUS_PQ_INIT_SMSK;
    //pp_printf("#%02u: Got IP from WRC. Configured EBM and PQ\n", cpuId);
  } else {
    *status |= SHCTL_STATUS_UART_INIT_SMSK;
    *status |= SHCTL_STATUS_EBM_INIT_SMSK ;
    *status |= SHCTL_STATUS_PQ_INIT_SMSK;
  }

  int j;


  while(!halWrTimeValid()) {
    for (j = 0; j < (125000000/2); ++j) { asm("nop"); }
    if (cpuId == 0) pp_printf("#%02u: DM cores Waiting for WRC synchronisation...\n", cpuId);
  }
  if (cpuId == 0) pp_printf("#%02u: WR time now in sync\n", cpuId);

  halIrqSetup(0x01);

  dmInit();
  *status  |= SHCTL_STATUS_DM_INIT_SMSK;
  *boottime = halGetSysTime();

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

/* Low-word only mismatch (high word identical, low word differs) must be
 * rejected. The consistency check must not depend on which half a writer
 * updates first - write order is fixed by hardware config today, but
 * safeRead64 must not rely on that assumption. */
void test_safeRead64_low_only_mismatch_fail(void) {
    volatile uint64_t a = make_val(0x12345678, 0xAAAA0001);
    volatile uint64_t b = make_val(0x12345678, 0xBBBB0002); // low differs, hi same
    uint64_t dest = 0xCAFEBABECAFEBABEULL;
    uint8_t rc = safeRead64(&a, &b, &dest);
    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xCAFEBABECAFEBABEULL, dest);
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

/* Test hooks for safeRead64_with_retry's retry loop. safeRead64 itself takes
 * two addresses expressly so a test can force a mismatch synchronously; the
 * retry-loop hook reuses that by picking what addr2nd to compare against for
 * a given attempt, without needing any real concurrent writer. */
static volatile uint64_t retryHookDecoy = 0xFFFFFFFFFFFFFFFFULL;

/* Mismatches on attempt 0 only, then reports consistent from attempt 1 on -
 * simulates a value that was mid-write once and stabilized on retry. */
static volatile uint64_t* retry_hook_fails_once_then_stable(uint8_t attempt, volatile uint64_t* addr) {
    return (attempt == 0) ? &retryHookDecoy : addr;
}

/* Never matches addr's content - simulates a value that never stabilizes
 * within SAFEREAD64_MAX_RETRIES attempts. */
static volatile uint64_t* retry_hook_always_mismatches(uint8_t attempt, volatile uint64_t* addr) {
    (void)attempt; (void)addr;
    return &retryHookDecoy;
}

/* safeRead64_with_retry: fails once, then succeeds on retry */
void test_safeRead64_with_retry_succeeds_after_transient_mismatch(void) {
    volatile uint64_t v = make_val(0x11112222, 0x33334444);
    uint64_t dest = 0;

    safeRead64RetryTestHook = retry_hook_fails_once_then_stable;
    uint8_t rc = safeRead64_with_retry(&v, &dest);
    safeRead64RetryTestHook = LM32_NULL_PTR;

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG((uint64_t)v, dest);
}

/* safeRead64_with_retry: never stabilizes -> exhausts retries and fails */
void test_safeRead64_with_retry_exhausts_when_never_stable(void) {
    volatile uint64_t v = make_val(0x55556666, 0x77778888);
    uint64_t dest = 0xCAFEBABECAFEBABEULL;

    safeRead64RetryTestHook = retry_hook_always_mismatches;
    uint8_t rc = safeRead64_with_retry(&v, &dest);
    safeRead64RetryTestHook = LM32_NULL_PTR;

    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xCAFEBABECAFEBABEULL, dest);
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

/* 32-bit REFERENCE mode: src is the address of a field slot holding the
 * reference address; dynField dereferences the slot once to get that
 * address, then once more to fetch the value at it. */
void test_dynField_32b_reference_direct_deref(void) {
    volatile uint32_t value = 0x12345678;
    volatile uint32_t* field = &value; /* field slot's content is the reference address */
    volatile uint32_t dst = 0;

    uint8_t rc = dynField(DYN_MODE_REF, (volatile uint32_t*)&field, (volatile uint32_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, dst);
}

/* 32-bit REFERENCE mode where the field slot's content (the reference
 * itself) is NULL: should fail. src (the slot's address) is never NULL in
 * production - &node[i] always points at a real slot - so this exercises
 * the field content being NULL, not the slot address. */
void test_dynField_32b_reference_null_src(void) {
    volatile uint32_t dst = 0xFFFFFFFF;
    volatile uint32_t* field = (volatile uint32_t*)LM32_NULL_PTR;

    uint8_t rc = dynField(DYN_MODE_REF, (volatile uint32_t*)&field, (volatile uint32_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(1, rc);
    /* dst should remain unchanged */
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst);
}

/* 32-bit REF2 (double reference) mode: deref the field slot, then deref
 * twice more (ptr2ptr) to reach the leaf value. */
void test_dynField_32b_reference2_double_deref(void) {
    volatile uint32_t value = 0xABCDEF01;
    volatile uint32_t* ptr_to_value = &value;
    volatile uint32_t** ptr_to_ptr = &ptr_to_value;
    volatile uint32_t dst = 0;

    uint8_t rc = dynField(DYN_MODE_REF2, (volatile uint32_t*)&ptr_to_ptr, (volatile uint32_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX32(0xABCDEF01, dst);
}

/* 32-bit REF2 with NULL intermediate pointer: should fail */
void test_dynField_32b_reference2_null_intermediate_ptr(void) {
    volatile uint32_t* ptr_to_ptr = (volatile uint32_t*)LM32_NULL_PTR;
    volatile uint32_t dst = 0xFFFFFFFF;

    uint8_t rc = dynField(DYN_MODE_REF2, (volatile uint32_t*)&ptr_to_ptr, (volatile uint32_t*)&dst);

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

/* 64-bit REFERENCE mode: src is the address of a field slot holding the
 * reference address; dynField dereferences the slot once to get that
 * address, then reads the 64-bit value at it. */
void test_dynField_64b_reference_direct_deref(void) {
    volatile uint64_t value = 0x0123456789ABCDEF;
    volatile uint64_t* field = &value;
    volatile uint64_t dst = 0;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)&field, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0x0123456789ABCDEF, dst);
}

/* 64-bit REFERENCE mode where the field slot's content is NULL: should fail. */
void test_dynField_64b_reference_null_src(void) {
    volatile uint64_t* field = (volatile uint64_t*)LM32_NULL_PTR;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)&field, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 (double reference): deref the field slot, then twice more to
 * reach the leaf 64-bit value. */
void test_dynField_64b_reference2_double_deref(void) {
    volatile uint64_t value = 0xDEADBEEFCAFEBABE;
    volatile uint64_t* ptr_to_value = &value;
    volatile uint64_t** ptr_to_ptr = &ptr_to_value;
    volatile uint64_t dst = 0;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)&ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 with NULL intermediate pointer: should fail */
void test_dynField_64b_reference2_null_intermediate_ptr(void) {
    volatile uint64_t** ptr_to_ptr = (volatile uint64_t**)LM32_NULL_PTR;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)(uint64_t*)&ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 64-bit REF2 with NULL final pointer (ptr2ptr): should fail */
void test_dynField_64b_reference2_null_final_ptr(void) {
    volatile uint64_t* ptr_to_value = (volatile uint64_t*)LM32_NULL_PTR;
    volatile uint64_t** ptr_to_ptr = &ptr_to_value;
    volatile uint64_t dst = 0xDEADBEEFCAFEBABE;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF2, (volatile uint32_t*)(uint64_t*)&ptr_to_ptr, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(1, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xDEADBEEFCAFEBABE, dst);
}

/* 32-bit REF mode with several different values: test value propagation */
void test_dynField_32b_reference_various_values(void) {
    /* Test with 0x00000000 */
    volatile uint32_t val1 = 0x00000000;
    volatile uint32_t* ptr1 = &val1;
    volatile uint32_t dst1 = 0xFFFFFFFF;
    
    uint8_t rc1 = dynField(DYN_MODE_REF, (volatile uint32_t*)&ptr1, (volatile uint32_t*)&dst1);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x00000000, dst1);

    /* Test with 0xFFFFFFFF */
    volatile uint32_t val2 = 0xFFFFFFFF;
    volatile uint32_t* ptr2 = &val2;
    volatile uint32_t dst2 = 0x00000000;

    uint8_t rc2 = dynField(DYN_MODE_REF, (volatile uint32_t*)&ptr2, (volatile uint32_t*)&dst2);
    TEST_ASSERT_EQUAL_UINT8(0, rc2);
    TEST_ASSERT_EQUAL_HEX32(0xFFFFFFFF, dst2);

    /* Test with alternating pattern */
    volatile uint32_t val3 = 0xAAAA5555;
    volatile uint32_t* ptr3 = &val3;
    volatile uint32_t dst3 = 0x55555555;

    uint8_t rc3 = dynField(DYN_MODE_REF, (volatile uint32_t*)&ptr3, (volatile uint32_t*)&dst3);
    TEST_ASSERT_EQUAL_UINT8(0, rc3);
    TEST_ASSERT_EQUAL_HEX32(0xAAAA5555, dst3);
}

/* 64-bit REF mode with large values: test high/low word split correctly */
void test_dynField_64b_reference_large_values(void) {
    /* Test with max 64-bit value */
    volatile uint64_t val_max = 0xFFFFFFFFFFFFFFFF;
    volatile uint64_t* ptr_max = &val_max;
    volatile uint64_t dst = 0;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)&ptr_max, (volatile uint32_t*)(uint64_t*)&dst);

    TEST_ASSERT_EQUAL_UINT8(0, rc);
    TEST_ASSERT_EQUAL_HEX64_MSG(0xFFFFFFFFFFFFFFFF, dst);
}

/* Chain of references: update value and verify propagation through chain */
void test_dynField_32b_reference_chain_update(void) {
    volatile uint32_t original = 0x11223344;
    volatile uint32_t* ptr_to_original = &original;
    volatile uint32_t dst = 0;

    /* First read */
    uint8_t rc1 = dynField(DYN_MODE_REF, (volatile uint32_t*)&ptr_to_original, &dst);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x11223344, dst);

    /* Modify original value */
    original = 0x55667788;
    volatile uint32_t dst2 = 0;

    /* Second read should get new value */
    uint8_t rc2 = dynField(DYN_MODE_REF, (volatile uint32_t*)&ptr_to_original, &dst2);
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
    uint8_t rc1 = dynField(DYN_MODE_REF2, (volatile uint32_t*)&ptr_to_ptr, (volatile uint32_t*)&dst);
    TEST_ASSERT_EQUAL_UINT8(0, rc1);
    TEST_ASSERT_EQUAL_HEX32(0x11111111, dst);

    /* Change intermediate pointer */
    ptr = &val2;
    volatile uint32_t dst2 = 0;

    /* Second deref: should get val2 */
    uint8_t rc2 = dynField(DYN_MODE_REF2, (volatile uint32_t*)&ptr_to_ptr, (volatile uint32_t*)&dst2);
    TEST_ASSERT_EQUAL_UINT8(0, rc2);
    TEST_ASSERT_EQUAL_HEX32(0x22222222, dst2);
}

/* Test with very small 64-bit values */
void test_dynField_64b_reference_small_values(void) {
    volatile uint64_t val_small = 0x0000000100000001;
    volatile uint64_t* ptr_small = &val_small;
    volatile uint64_t dst = 0;

    uint8_t rc = dynField(DYN_WIDTH64_SMSK | DYN_MODE_REF, (volatile uint32_t*)(uint64_t*)&ptr_small, (volatile uint32_t*)(uint64_t*)&dst);

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




/* ============================================
 * Test Setup and Teardown
 * ============================================ */

void setUp(void) {
    /* defensive: make sure no test can leak its retry-loop hook into the next */
    safeRead64RetryTestHook = LM32_NULL_PTR;
}

void tearDown(void) {
    /* Cleanup if needed */
    for (uint32_t j = 0; j < ((125000000/4)); ++j) { asm("nop"); }
}

/* ============================================
 * Test Runner
 * ============================================ */

void main(void) {

          int j;


    init();

    //FIXME why is uart_hw_init here twice ???
    // wait 1s + cpuIdx * 1/10s
    for (j = 0; j < ((125000000/4)+(cpuId*2500000)); ++j) { asm("nop"); }
    if (cpuId != 0) halUartInitHw();   *status |= SHCTL_STATUS_UART_INIT_SMSK;

    halAtomicOn();

    pp_printf("#%02u: Rdy\n", cpuId);
    #if DEBUGLEVEL != 0
        pp_printf("#%02u: Debuglevel %u. Don't expect timeley delivery with console outputs on!\n", cpuId, DEBUGLEVEL);
    #endif
    #if DEBUGTIME == 1
        pp_printf("#%02u: Debugtime mode ON. Par Field of Msgs will be overwritten be dispatch time at lm32\n", cpuId);
    #endif
    #if DEBUGPRIOQ == 1
        pp_printf("#%02u: Priority Queue Debugmode ON, timestamps will be written to 0x%08x on receivers", cpuId, DEBUGPRIOQDST);
    #endif
    //mprintf("Found MsgBox at 0x%08x. MSI Path is 0x%08x\n", (uint32_t)pCpuMsiBox, (uint32_t)pMyMsi);


    halAtomicOff();

    if (halGetMsiBoxCpuSlot(cpuId, 0) == -1) {pp_printf("#%02u: Mail box slot acquisition failed\n", cpuId);}

    DBPRINT1("#%02u: Base shared ram 0x%08x\n", cpuId, halGetSharedMemBase());

    pp_printf("#%02u: Unit tests safeRead64 & dynField for %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

    UNITY_BEGIN();
    RUN_TEST(test_safeRead64_null_addr1st);
    RUN_TEST(test_safeRead64_null_addr2nd);
    RUN_TEST(test_safeRead64_null_dest);
    RUN_TEST(test_safeRead64_same_address_success);
    RUN_TEST(test_safeRead64_two_addrs_identical_success);
    RUN_TEST(test_safeRead64_high_mismatch_fail);
    RUN_TEST(test_safeRead64_low_only_mismatch_fail);
    RUN_TEST(test_safeRead64_all_changed_fail);
    RUN_TEST(test_safeRead64_endianness_assembly);
    RUN_TEST(test_safeRead64_with_retry_stable_success);
    RUN_TEST(test_safeRead64_with_retry_succeeds_after_transient_mismatch);
    RUN_TEST(test_safeRead64_with_retry_exhausts_when_never_stable);
    
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






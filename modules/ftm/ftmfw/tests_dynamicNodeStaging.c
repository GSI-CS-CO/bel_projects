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

/* ============================================
 * Test Node Setup Helpers
 * ============================================ */

static void setup_test_node(uint32_t* node, uint8_t node_type, uint32_t wordformat) {
    memset(node, 0, _MEM_BLOCK_SIZE);

    /* Set node type in flags */
    node[NODE_HASH >> 2] = 0xDEADBEEF;  /* arbitrary hash for testing */
    node[NODE_FLAGS >> 2] = (node_type & NFLG_TYPE_MSK);
    node[NODE_DEF_DEST_PTR >> 2] = 0x12345678;
    node[NODE_OPT_DYN >> 2] = wordformat;

    /* Set some data in the first 9 words (fields 0-8) */
    for (int i = 0; i < 9; i++) {
        node[i] = 0x11111111 + (i * 0x11111111);
    }
}

static void reset_mocks(void) {
    /* Reset CMock state via generated functions when needed. Here we just
     * clear the nodeFuncs table and related test-local values. */
    for (int i = 0; i < _NODE_TYPE_END_; ++i) nodeFuncs[i] = dummyNodeFunc;
}

/* Local stub: return the staging area (nodeTmp) to simulate handler returning nodeTmp */
static uint32_t* return_nodeTmp_stub(uint32_t* node, uint32_t* thrData) {
    (void)node; (void)thrData;
    return nodeTmp;
}

/* ============================================
 * Test Helpers
 * ============================================ */

static inline uint32_t make_wordformat(uint8_t field_idx, uint8_t mode, uint8_t width64) {
    uint32_t format = 0;
    format |= (mode & DYN_MODE_MSK) << (field_idx * 3);
    if (width64) {
        format |= DYN_WIDTH64_SMSK << (field_idx * 3);
    }
    return format;
}

/* ============================================
 * Capturing Stub Infrastructure
 * ============================================
 * dynamicNodeStaging() hands the node handler a *staged* copy (nodeTmp),
 * not the caller's original node, and nodeTmp is gone by the time the
 * function returns. Every assertion that only looks at the original node
 * or at the return value is blind to whether staging (dynField) actually
 * computed the right values - which is exactly where the known REF bug
 * lives. These stubs snapshot nodeTmp (and thrData) at call time so tests
 * can assert on what was actually staged. */

static uint32_t capturedStaged[_MEM_BLOCK_SIZE >> 2];
static uint32_t* capturedThrData;

static void reset_capture(void) {
    memset(capturedStaged, 0, sizeof(capturedStaged));
    capturedThrData = LM32_NULL_PTR;
}

/* Records the staged node + thrData, returns NULL (idle) like nodeNull(). */
static uint32_t* capture_stub(uint32_t* node, uint32_t* thrData) {
    memcpy(capturedStaged, node, _MEM_BLOCK_SIZE);
    capturedThrData = thrData;
    return LM32_NULL_PTR;
}

/* Records the staged node, then mutates its flags field before returning,
 * to prove a handler-side flag change is copied back into the caller's node. */
static uint32_t* capture_and_mutate_flags_stub(uint32_t* node, uint32_t* thrData) {
    (void)thrData;
    memcpy(capturedStaged, node, _MEM_BLOCK_SIZE);
    node[NODE_FLAGS >> 2] = 0xF00DF00D;
    return LM32_NULL_PTR;
}

/* Records the staged node, then mutates field 0 before returning, to prove
 * a handler-side IM/ADR field change is copied back into the caller's node. */
static uint32_t* capture_and_mutate_field0_stub(uint32_t* node, uint32_t* thrData) {
    (void)thrData;
    memcpy(capturedStaged, node, _MEM_BLOCK_SIZE);
    node[0] = 0xC001C0DE;
    return LM32_NULL_PTR;
}

/* A successor node distinct from both `node` and `nodeTmp`, used to prove a
 * handler's return value is passed through untouched when it is neither
 * NULL nor nodeTmp. */
static uint32_t sentinelSuccessor[_MEM_BLOCK_SIZE >> 2];
static uint32_t* return_sentinel_stub(uint32_t* node, uint32_t* thrData) {
    (void)node; (void)thrData;
    return sentinelSuccessor;
}

/* Per-type dispatch stubs. Each returns a distinct sentinel pointer so a
 * test can prove dynamicNodeStaging invoked the handler for the correct
 * node type - not merely that *some* handler ran. */
static uint32_t dispatchSentinelA[_MEM_BLOCK_SIZE >> 2];
static uint32_t dispatchSentinelB[_MEM_BLOCK_SIZE >> 2];
static uint32_t* dispatch_stub_a(uint32_t* node, uint32_t* thrData) { (void)node; (void)thrData; return dispatchSentinelA; }
static uint32_t* dispatch_stub_b(uint32_t* node, uint32_t* thrData) { (void)node; (void)thrData; return dispatchSentinelB; }

/* ============================================
 * Reference-mode backing storage
 * ============================================
 * REF/REF2 fields are genuinely dereferenced by dynField (memory is
 * actually read at the address the field holds). Earlier versions of these
 * tests pointed REF/REF2 fields at arbitrary integers (e.g. 0x11111111),
 * which are not valid addresses; once dynField's REF handling is fixed,
 * dereferencing those would crash/fault instead of failing a clean
 * assertion. All REF/REF2 tests below point fields at real backing storage.
 */
static uint32_t refBacking[9]; /* one target value per field index 0..8 */

static uint32_t ref2Leaf = 0xABCDEF01;
static uint32_t* ref2Mid = &ref2Leaf; /* intermediate pointer, for REF2 chains */

/* ============================================
 * Test Cases
 * ============================================ */

/* Test: NULL node pointer should return NULL */
void test_dynamicNodeStaging_null_node(void) {
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    reset_mocks();

    uint32_t* result = dynamicNodeStaging(LM32_NULL_PTR, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: all-immediate node round-trips through nodeTmp unchanged, both at
 * staging time (what the handler saw) and in the caller's node afterwards. */
void test_dynamicNodeStaging_all_immediate_fields_round_trip(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_node[_MEM_BLOCK_SIZE >> 2];

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0); /* wordformat 0 -> all fields immediate */
    memcpy(original_node, node, _MEM_BLOCK_SIZE);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, node, 9);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, capturedStaged, 9);
}

/* Test: Handler returns node itself should return original node */
void test_dynamicNodeStaging_handler_returns_nodeTmp(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    /* Arrange: stub handler to return nodeTmp (simulate handler returning nodeTmp) */
    nodeFuncs[NODE_TYPE_CNOOP] = return_nodeTmp_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    /* When handler returns nodeTmp, function should return original node */
    TEST_ASSERT_EQUAL_PTR(node, result);
}

/* Test: dynamicNodeStaging dispatches through nodeFuncs[getNodeType(...)],
 * i.e. the correct per-type handler runs, not merely *a* handler. Proven by
 * using two distinct handlers that return distinguishable sentinels. */
void test_dynamicNodeStaging_dispatches_to_correct_handler_type(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    nodeFuncs[NODE_TYPE_CNOOP] = dispatch_stub_a;
    nodeFuncs[NODE_TYPE_CFLOW] = dispatch_stub_b;

    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    uint32_t* resultCnoop = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(dispatchSentinelA, resultCnoop);

    setup_test_node(node, NODE_TYPE_CFLOW, 0);
    uint32_t* resultCflow = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(dispatchSentinelB, resultCflow);
}

/* Test: A node flag change made by the handler (on nodeTmp) is copied back
 * to the caller's node. */
void test_dynamicNodeStaging_flag_changes_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_and_mutate_flags_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(0xF00DF00D, node[NODE_FLAGS >> 2]);
}

/* Test: 64-bit field at invalid boundary (field 8) returns NULL. This is the
 * boundary-overflow check inside dynamicNodeStaging itself, not a dynField
 * failure - see test_dynamicNodeStaging_dynField_ref2_null_intermediate_fails
 * for a genuine dynField-triggered failure. */
void test_dynamicNodeStaging_64bit_field_at_invalid_boundary_fails(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    uint32_t wordformat = make_wordformat(8, DYN_MODE_IM, 1);
    node[NODE_OPT_DYN >> 2] = wordformat;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: a genuine dynField failure (REF2 with a NULL intermediate pointer)
 * propagates as NULL from dynamicNodeStaging. Distinct from the boundary
 * check above: this exercises dynField's own error path. */
void test_dynamicNodeStaging_dynField_ref2_null_intermediate_fails(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    node[0] = (uint32_t)LM32_NULL_PTR; /* the "reference edge" itself is NULL */
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_REF2, 0);

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: an IM field changed by the handler (on nodeTmp) is copied back into
 * the caller's node. */
void test_dynamicNodeStaging_immediate_field_change_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);  /* All fields immediate */

    nodeFuncs[NODE_TYPE_CNOOP] = capture_and_mutate_field0_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(0xC001C0DE, node[0]);
}

/* Test: a 32-bit REF field must be genuinely dereferenced - the value handed
 * to the node handler must be the value AT the address the field holds, not
 * the address itself.
 *
 * KNOWN BUG: dynField's 32-bit DYN_MODE_REF path (dm.c) currently reads
 * *src where src == &node[i], i.e. it just re-reads the field itself
 * (identical to IM). It never follows the address stored in the field. This
 * test is expected to FAIL against the current implementation - that failure
 * is the intended, previously-missing coverage for the known REF bug. */
void test_dynamicNodeStaging_32b_reference_field_dereferenced_correctly(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    refBacking[0] = 0xCAFEF00D;
    node[0] = (uint32_t)&refBacking[0]; /* field holds the address to dereference */
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_REF, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    /* The address itself must not appear in the staged copy */
    TEST_ASSERT_EQUAL_HEX32(refBacking[0], capturedStaged[0]);
}

/* Test: a 32-bit REF2 field must dereference twice (field -> intermediate
 * pointer -> leaf value).
 *
 * KNOWN BUG: dynField's current REF2 code performs only one real memory
 * indirection beyond reading the field (equivalent to what REF should do),
 * so it yields the intermediate pointer's address rather than the leaf
 * value. Expected to FAIL against the current implementation. */
void test_dynamicNodeStaging_32b_reference2_dereferenced_correctly(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    ref2Leaf = 0xABCDEF01;
    ref2Mid = &ref2Leaf;
    node[0] = (uint32_t)&ref2Mid; /* field holds address of the intermediate pointer */
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_REF2, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(ref2Leaf, capturedStaged[0]);
}

/* Test: Reference mode fields are NOT copied back to the original node
 * (they only ever exist transiently in nodeTmp). Uses real backing storage
 * instead of arbitrary integers, so this stays safe once REF dereferencing
 * is fixed to actually touch memory. */
void test_dynamicNodeStaging_reference_fields_not_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_node[_MEM_BLOCK_SIZE >> 2];

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    uint32_t wordformat = 0;
    for (int i = 0; i < 9; i++) {
        refBacking[i] = 0x50000000 + i;
        node[i] = (uint32_t)&refBacking[i];
        wordformat |= make_wordformat(i, DYN_MODE_REF, 0);
    }
    node[NODE_OPT_DYN >> 2] = wordformat;
    memcpy(original_node, node, _MEM_BLOCK_SIZE);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    /* Original node (the reference addresses) must remain untouched ... */
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, node, 9);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    /* ... while the handler must have seen the dereferenced values. */
    TEST_ASSERT_EQUAL_HEX32_ARRAY(refBacking, capturedStaged, 9);
}

/* Test: Address mode fields are used as-is (never dereferenced by dynField)
 * but participate in copy-back exactly like immediate fields. */
void test_dynamicNodeStaging_address_mode_field_change_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_field0;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_ADR, 0);
    original_field0 = node[0];

    nodeFuncs[NODE_TYPE_CNOOP] = capture_and_mutate_field0_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    /* staged value must be the raw field, never dereferenced */
    TEST_ASSERT_EQUAL_HEX32(original_field0, capturedStaged[0]);
    /* but the handler's change is still copied back, like IM */
    TEST_ASSERT_EQUAL_HEX32(0xC001C0DE, node[0]);
}

/* Test: Reference mode with single field, using real backing storage. */
void test_dynamicNodeStaging_single_reference_field(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_field0;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    refBacking[0] = 0x13571357;
    node[0] = (uint32_t)&refBacking[0];
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_REF, 0);
    original_field0 = node[0];

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_HEX32(original_field0, node[0]);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(refBacking[0], capturedStaged[0]);
}

/* Test: Mixed mode fields (immediate, address, reference, ref2) all staged
 * and copied back correctly in a single node. Uses real backing storage for
 * the REF/REF2 fields. */
void test_dynamicNodeStaging_mixed_field_modes_stage_values_correctly(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_field0, original_field1;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    /* Field 0: Immediate, Field 1: Address, Field 2: Reference, Field 3: Ref2 */
    refBacking[2] = 0x22222299;
    node[2] = (uint32_t)&refBacking[2];

    ref2Leaf = 0x33333399;
    ref2Mid = &ref2Leaf;
    node[3] = (uint32_t)&ref2Mid;

    uint32_t wordformat = 0;
    wordformat |= make_wordformat(0, DYN_MODE_IM,   0);
    wordformat |= make_wordformat(1, DYN_MODE_ADR,  0);
    wordformat |= make_wordformat(2, DYN_MODE_REF,  0);
    wordformat |= make_wordformat(3, DYN_MODE_REF2, 0);
    node[NODE_OPT_DYN >> 2] = wordformat;

    original_field0 = node[0];
    original_field1 = node[1];

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);

    /* IM/ADR: staged and copied back unchanged - unaffected by the REF bug */
    TEST_ASSERT_EQUAL_HEX32(original_field0, capturedStaged[0]);
    TEST_ASSERT_EQUAL_HEX32(original_field0, node[0]);
    TEST_ASSERT_EQUAL_HEX32(original_field1, capturedStaged[1]);
    TEST_ASSERT_EQUAL_HEX32(original_field1, node[1]);
    /* REF/ADR/REF2 must never be copied back to the original node */
    TEST_ASSERT_EQUAL_HEX32((uint32_t)&refBacking[2], node[2]);
    TEST_ASSERT_EQUAL_HEX32((uint32_t)&ref2Mid, node[3]);

    /* REF/REF2: known bug - these are expected to fail until dynField is fixed */
    TEST_ASSERT_EQUAL_HEX32(refBacking[2], capturedStaged[2]);
    TEST_ASSERT_EQUAL_HEX32(ref2Leaf, capturedStaged[3]);
}

/* Test: 64-bit field at boundary (field 7) succeeds and round-trips an
 * all-immediate 64-bit pair intact. */
void test_dynamicNodeStaging_64bit_field_at_boundary_success(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_field7, original_field8;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    node[NODE_OPT_DYN >> 2] = make_wordformat(7, DYN_MODE_IM, 1);
    original_field7 = node[7];
    original_field8 = node[8];

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(original_field7, capturedStaged[7]);
    TEST_ASSERT_EQUAL_HEX32(original_field8, capturedStaged[8]);
    TEST_ASSERT_EQUAL_HEX32(original_field7, node[7]);
    TEST_ASSERT_EQUAL_HEX32(original_field8, node[8]);
}

/* Test: a 64-bit REF field must be genuinely dereferenced. The field (words
 * 5+6) holds the address of a real 64-bit value; the handler must see that
 * value, not the raw field bits.
 *
 * KNOWN BUG: dynField's 64-bit DYN_MODE_REF path has the same off-by-one-
 * indirection bug as the 32-bit path (reads directly at &node[i] instead of
 * following the address stored there). Expected to FAIL today. */
void test_dynamicNodeStaging_64bit_reference_field_dereferenced_correctly(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    static uint64_t backing64;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    backing64 = 0x0123456789ABCDEFULL;
    node[5] = (uint32_t)(uintptr_t)&backing64; /* REF fields hold a plain 32b pointer at the field's base word */
    node[NODE_OPT_DYN >> 2] = make_wordformat(5, DYN_MODE_REF, 1);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX64_MSG(backing64, *(uint64_t*)&capturedStaged[5]);
}

/* Test: a TMSG node's PAR field, set up as a 64-bit REF, is staged from the
 * referenced address - here thrData's current time sum - rather than from
 * whatever bits sit in the field itself. Exercises a REF field at a node
 * type/offset actually used in production (TMSG_PAR), sourced from a real
 * thrData field instead of synthetic backing storage. */
void test_dynamicNodeStaging_tmsg_par_ref_from_thrData_currtime(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_TMSG, 0);

    *(uint64_t*)&thrData[T_TD_CURRTIME >> 2] = 0x1122334455667788ULL;
    node[TMSG_PAR >> 2] = (uint32_t)(uintptr_t)&thrData[T_TD_CURRTIME >> 2]; /* REF fields hold a plain 32b pointer at the field's base word */
    node[NODE_OPT_DYN >> 2] = make_wordformat(TMSG_PAR >> 2, DYN_MODE_REF, 1);

    nodeFuncs[NODE_TYPE_TMSG] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX64_MSG(*(uint64_t*)&thrData[T_TD_CURRTIME >> 2], *(uint64_t*)&capturedStaged[TMSG_PAR >> 2]);
    /* REF fields are never copied back to the caller's node */
    TEST_ASSERT_EQUAL_HEX32((uint32_t)(uintptr_t)&thrData[T_TD_CURRTIME >> 2], node[TMSG_PAR >> 2]);
}

/* Test: a NULL/zero 64-bit REF field IS detected and rejected by dynField.
 *
 * dynField's NULL guard on the plain (non-REF2) REF path checks `src`
 * itself, and dynamicNodeStaging now calls dynField with src=node[i] - the
 * field's own content, i.e. the reference address - so a field holding 0 is
 * genuinely NULL and the guard correctly rejects it before the node handler
 * is ever dispatched. See the 32-bit equivalent below. */
void test_dynamicNodeStaging_64bit_reference_null_field_detected(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    *(uint64_t*)&node[5] = 0ULL;
    node[NODE_OPT_DYN >> 2] = make_wordformat(5, DYN_MODE_REF, 1);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    /* handler must never have been dispatched - staging rejected before that point */
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, capturedThrData);
}

/* Test: 64-bit REF2 with a NULL intermediate field fails cleanly (no
 * dereference attempted). The positive (successful double-dereference) case
 * for 64-bit REF2 is intentionally not covered here: dynField's 64-bit REF2
 * path reinterprets an 8-byte-wide node field as a pointer-sized value,
 * which is ambiguous on a 32-bit target and cannot be exercised safely/
 * deterministically without pinning down that representation first. */
void test_dynamicNodeStaging_64bit_reference2_null_intermediate_fails(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    *(uint64_t*)&node[5] = 0ULL;
    node[NODE_OPT_DYN >> 2] = make_wordformat(5, DYN_MODE_REF2, 1);

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: every one of the 9 dynamic-capable fields is staged independently
 * and correctly, each pointing at a distinct, distinguishable backing
 * value. Directly exercises dynField's per-index handling; expected to FAIL
 * today for the same reason as the single-field REF tests above. */
void test_dynamicNodeStaging_all_fields_processed_independently(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    uint32_t wordformat = 0;
    for (int i = 0; i < 9; i++) {
        refBacking[i] = 0x60000000 + i;
        node[i] = (uint32_t)&refBacking[i];
        wordformat |= make_wordformat(i, DYN_MODE_REF, 0);
    }
    node[NODE_OPT_DYN >> 2] = wordformat;

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(refBacking, capturedStaged, 9);
}

/* Test: handler return value NULL, returned explicitly by the handler
 * itself (not via the dummyNodeFunc -> nodeNull() chain), is propagated
 * as-is and not confused with the nodeTmp special-case. */
void test_dynamicNodeStaging_handler_returns_null_explicit(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Handler gets both the staged node and the exact thrData pointer,
 * with thrData's contents intact. */
void test_dynamicNodeStaging_handler_receives_parameters(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    thrData[T_TD_CURRTIME >> 2] = 0x12345678;

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_PTR(thrData, capturedThrData);
    TEST_ASSERT_EQUAL_HEX32(0x12345678, capturedThrData[T_TD_CURRTIME >> 2]);
}

/* Test: Node hash is preserved through the function (outside the 0-8 field
 * loop, so untouched by dynField either way). */
void test_dynamicNodeStaging_preserves_node_hash(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_hash = 0xDEADBEEF;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    node[NODE_HASH >> 2] = original_hash;

    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    (void)result;
    TEST_ASSERT_EQUAL_HEX32(original_hash, node[NODE_HASH >> 2]);
}

/* Test: Node default destination pointer is preserved (outside the 0-8
 * field loop). */
void test_dynamicNodeStaging_preserves_default_dest(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_dest = 0x87654321;

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    node[NODE_DEF_DEST_PTR >> 2] = original_dest;

    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    (void)result;
    TEST_ASSERT_EQUAL_HEX32(original_dest, node[NODE_DEF_DEST_PTR >> 2]);
}

/* Test: the wordformat word itself (outside the 0-8 field loop) is never
 * touched by dynamicNodeStaging. */
void test_dynamicNodeStaging_wordformat_field_preserved(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_wordformat = 0x00249249;  /* Some arbitrary pattern */

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, original_wordformat);

    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    (void)result;
    TEST_ASSERT_EQUAL_HEX32(original_wordformat, node[NODE_OPT_DYN >> 2]);
}

/* Test: Address mode fields are staged as-is (no dereference) at several
 * different indices, and left unmodified since the handler doesn't touch them. */
void test_dynamicNodeStaging_multiple_address_fields_preserved(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_node[_MEM_BLOCK_SIZE >> 2];

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    /* Fields 0, 2, 4 are address mode */
    uint32_t wordformat = 0;
    wordformat |= make_wordformat(0, DYN_MODE_ADR, 0);
    wordformat |= make_wordformat(2, DYN_MODE_ADR, 0);
    wordformat |= make_wordformat(4, DYN_MODE_ADR, 0);
    node[NODE_OPT_DYN >> 2] = wordformat;
    memcpy(original_node, node, _MEM_BLOCK_SIZE);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32(original_node[0], capturedStaged[0]);
    TEST_ASSERT_EQUAL_HEX32(original_node[2], capturedStaged[2]);
    TEST_ASSERT_EQUAL_HEX32(original_node[4], capturedStaged[4]);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, node, 9);
}

/* Test: a NULL/zero 32-bit REF field IS detected and rejected by dynField.
 *
 * dynField's NULL guard on the plain (non-REF2) REF path checks `src`
 * itself (dm.c: `if (tmpPtr32 != LM32_NULL_PTR)`), and dynamicNodeStaging
 * now calls dynField with src=node[i] - the field's own content - so a REF
 * field whose content is 0 is genuinely NULL and gets rejected before the
 * node handler is ever dispatched. */
void test_dynamicNodeStaging_32b_reference_null_field_detected(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    node[0] = (uint32_t)LM32_NULL_PTR;
    node[NODE_OPT_DYN >> 2] = make_wordformat(0, DYN_MODE_REF, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    /* handler must never have been dispatched - staging rejected before that point */
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, capturedThrData);
}

/* Test: a fully populated node round-trips through nodeTmp intact (all
 * fields immediate, including the fields outside the 0-8 dynamic range). */
void test_dynamicNodeStaging_large_node_data_round_trip(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_node[_MEM_BLOCK_SIZE >> 2];

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    /* Fill node with distinct values for verification */
    for (int i = 0; i < (_MEM_BLOCK_SIZE >> 2); i++) {
        node[i] = 0x12340000 | i;
    }
    /* getNodeType reads the type out of the flags field, so keep it valid */
    node[NODE_FLAGS >> 2]   = NODE_TYPE_CNOOP & NFLG_TYPE_MSK;
    node[NODE_OPT_DYN >> 2] = 0; /* the fill loop above clobbered the wordformat descriptor; restore all-immediate */
    memcpy(original_node, node, _MEM_BLOCK_SIZE);

    nodeFuncs[NODE_TYPE_CNOOP] = capture_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, capturedStaged, 9);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(original_node, node, 9);
}

/* Test: a handler-returned pointer that is neither NULL nor nodeTmp (a
 * genuine successor node) is passed through unmodified. */
void test_dynamicNodeStaging_handler_return_value_passthrough(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};

    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);

    nodeFuncs[NODE_TYPE_CNOOP] = return_sentinel_stub;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(sentinelSuccessor, result);
}

/* ============================================
 * Test Setup and Teardown
 * ============================================ */

void setUp(void) {
    reset_mocks();
    reset_capture();
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

    pp_printf("#%02u: Unit test dynamicNodeStaging for %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

    UNITY_BEGIN();
    RUN_TEST(test_dynamicNodeStaging_null_node);
    RUN_TEST(test_dynamicNodeStaging_all_immediate_fields_round_trip);
    RUN_TEST(test_dynamicNodeStaging_handler_returns_nodeTmp);
    RUN_TEST(test_dynamicNodeStaging_dispatches_to_correct_handler_type);
    RUN_TEST(test_dynamicNodeStaging_flag_changes_copied_back);
    RUN_TEST(test_dynamicNodeStaging_64bit_field_at_invalid_boundary_fails);
    RUN_TEST(test_dynamicNodeStaging_dynField_ref2_null_intermediate_fails);
    RUN_TEST(test_dynamicNodeStaging_immediate_field_change_copied_back);
    RUN_TEST(test_dynamicNodeStaging_32b_reference_field_dereferenced_correctly);
    RUN_TEST(test_dynamicNodeStaging_32b_reference2_dereferenced_correctly);
    RUN_TEST(test_dynamicNodeStaging_reference_fields_not_copied_back);
    RUN_TEST(test_dynamicNodeStaging_address_mode_field_change_copied_back);
    RUN_TEST(test_dynamicNodeStaging_single_reference_field);
    RUN_TEST(test_dynamicNodeStaging_mixed_field_modes_stage_values_correctly);
    RUN_TEST(test_dynamicNodeStaging_64bit_field_at_boundary_success);
    RUN_TEST(test_dynamicNodeStaging_64bit_reference_field_dereferenced_correctly);
    RUN_TEST(test_dynamicNodeStaging_tmsg_par_ref_from_thrData_currtime);
    RUN_TEST(test_dynamicNodeStaging_64bit_reference_null_field_detected);
    RUN_TEST(test_dynamicNodeStaging_64bit_reference2_null_intermediate_fails);
    RUN_TEST(test_dynamicNodeStaging_all_fields_processed_independently);
    RUN_TEST(test_dynamicNodeStaging_handler_returns_null_explicit);
    RUN_TEST(test_dynamicNodeStaging_handler_receives_parameters);
    RUN_TEST(test_dynamicNodeStaging_preserves_node_hash);
    RUN_TEST(test_dynamicNodeStaging_preserves_default_dest);
    RUN_TEST(test_dynamicNodeStaging_wordformat_field_preserved);
    RUN_TEST(test_dynamicNodeStaging_multiple_address_fields_preserved);
    RUN_TEST(test_dynamicNodeStaging_32b_reference_null_field_detected);
    RUN_TEST(test_dynamicNodeStaging_large_node_data_round_trip);
    RUN_TEST(test_dynamicNodeStaging_handler_return_value_passthrough);
    UNITY_END();

    while (1) { asm("nop"); }

}



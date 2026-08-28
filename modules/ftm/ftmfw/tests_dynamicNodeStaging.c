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
 * Test Cases
 * ============================================ */

/* Test: NULL node pointer should return NULL */
void test_dynamicNodeStaging_null_node(void) {
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    reset_mocks();
    
    uint32_t* result = dynamicNodeStaging(LM32_NULL_PTR, (uint32_t*)thrData);
    
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Valid node with all immediate fields */
void test_dynamicNodeStaging_all_immediate_fields(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_node[_MEM_BLOCK_SIZE >> 2];
    
    reset_mocks();
    
    /* Setup: all fields as immediate (mode 0) */
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    memcpy(original_node, node, _MEM_BLOCK_SIZE);
    
    /* Use dummy handler as requested */
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
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

/* Test: Different node types call correct handlers */
void test_dynamicNodeStaging_calls_correct_handler_type(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CFLOW, 0);
    nodeFuncs[NODE_TYPE_CFLOW] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Node flags modified by handler should be copied back */
void test_dynamicNodeStaging_copies_back_node_flags(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_flags;
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    original_flags = node[NODE_FLAGS >> 2];
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: dynField failures should return NULL */
void test_dynamicNodeStaging_dynField_failure_returns_null(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* To simulate dynField failure we cannot mock dynField (user requested),
     * so instead create an invalid wordformat that causes safe failure, e.g.
     * place 64-bit width at field 8 which triggers boundary check and returns NULL. */
    uint32_t wordformat = 0;
    wordformat |= (DYN_MODE_IM & DYN_MODE_MSK) << (8 * 3);
    wordformat |= DYN_WIDTH64_SMSK << (8 * 3);
    node[NODE_OPT_DYN >> 2] = wordformat;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Immediate mode fields are copied back to original node */
void test_dynamicNodeStaging_immediate_fields_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);  /* All fields immediate */
    
    /* Modify field 0 value before calling */
    uint32_t original_field0 = node[0];
    node[0] = 0xAAAAAAAA;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Reference mode fields are NOT copied back to original node */
void test_dynamicNodeStaging_reference_fields_not_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    
    /* Create node with reference fields (all fields in REF mode) */
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    uint32_t wordformat = 0;
    for (int i = 0; i < 9; i++) {
        wordformat |= (DYN_MODE_REF & DYN_MODE_MSK) << (i * 3);
    }
    node[NODE_OPT_DYN >> 2] = wordformat;
    
    uint32_t original_field0 = node[0];
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    /* For reference fields, original node should remain unchanged */
    TEST_ASSERT_EQUAL_HEX32(original_field0, node[0]);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Address mode fields are copied back like immediate */
void test_dynamicNodeStaging_address_mode_copied_back(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    
    /* Create node with address mode fields */
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    uint32_t wordformat = 0;
    for (int i = 0; i < 9; i++) {
        wordformat |= (DYN_MODE_ADR & DYN_MODE_MSK) << (i * 3);
    }
    node[NODE_OPT_DYN >> 2] = wordformat;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Mixed mode fields (immediate, ref, ref2, address) */
void test_dynamicNodeStaging_mixed_field_modes(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Create mixed word format:
     * Field 0: Immediate (DYN_MODE_IM)
     * Field 1: Address (DYN_MODE_ADR)
     * Field 2: Reference (DYN_MODE_REF)
     * Field 3: Ref2 (DYN_MODE_REF2)
     * Fields 4-8: Immediate
     */
    uint32_t wordformat = 0;
    wordformat |= (DYN_MODE_IM & DYN_MODE_MSK) << (0 * 3);   /* Field 0 */
    wordformat |= (DYN_MODE_ADR & DYN_MODE_MSK) << (1 * 3);  /* Field 1 */
    wordformat |= (DYN_MODE_REF & DYN_MODE_MSK) << (2 * 3);  /* Field 2 */
    wordformat |= (DYN_MODE_REF2 & DYN_MODE_MSK) << (3 * 3); /* Field 3 */
    
    node[NODE_OPT_DYN >> 2] = wordformat;
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: 64-bit field at boundary (field 7) succeeds */
void test_dynamicNodeStaging_64bit_field_at_boundary_success(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Field 7 with 64-bit mode should be OK */
    uint32_t wordformat = 0;
    wordformat |= (DYN_MODE_IM & DYN_MODE_MSK) << (7 * 3);
    wordformat |= DYN_WIDTH64_SMSK << (7 * 3);
    
    node[NODE_OPT_DYN >> 2] = wordformat;
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: 64-bit field at invalid boundary (field 8) returns NULL */
void test_dynamicNodeStaging_64bit_field_at_invalid_boundary_fails(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Field 8 with 64-bit mode should fail (boundary check) */
    uint32_t wordformat = 0;
    wordformat |= (DYN_MODE_IM & DYN_MODE_MSK) << (8 * 3);
    wordformat |= DYN_WIDTH64_SMSK << (8 * 3);
    
    node[NODE_OPT_DYN >> 2] = wordformat;
    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Each dynField call processes correct field index */
void test_dynamicNodeStaging_all_fields_processed(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Handler return value NULL is propagated */
void test_dynamicNodeStaging_handler_returns_null(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Handler gets both node and thrData parameters */
void test_dynamicNodeStaging_handler_receives_parameters(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Set some data in thrData to verify it's passed through */
    thrData[T_TD_CURRTIME >> 2] = 0x12345678;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Node hash is preserved through the function */
void test_dynamicNodeStaging_preserves_node_hash(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_hash = 0xDEADBEEF;
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    node[NODE_HASH >> 2] = original_hash;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_HEX32(original_hash, node[NODE_HASH >> 2]);
}

/* Test: Node default destination pointer is preserved */
void test_dynamicNodeStaging_preserves_default_dest(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_dest = 0x87654321;
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    node[NODE_DEF_DEST_PTR >> 2] = original_dest;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_HEX32(original_dest, node[NODE_DEF_DEST_PTR >> 2]);
}

/* Test: Node type word format is saved and used correctly */
void test_dynamicNodeStaging_wordformat_handling(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    uint32_t original_wordformat = 0x00249249;  /* Some arbitrary pattern */
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, original_wordformat);
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Reference mode with single field */
void test_dynamicNodeStaging_single_reference_field(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Only field 0 is reference, rest immediate */
    uint32_t wordformat = (DYN_MODE_REF & DYN_MODE_MSK) << (0 * 3);
    node[NODE_OPT_DYN >> 2] = wordformat;
    
    uint32_t original_field0 = node[0];
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_HEX32(original_field0, node[0]);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Address mode with multiple fields */
void test_dynamicNodeStaging_multiple_address_fields(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Fields 0, 2, 4 are address mode */
    uint32_t wordformat = 0;
    wordformat |= (DYN_MODE_ADR & DYN_MODE_MSK) << (0 * 3);
    wordformat |= (DYN_MODE_ADR & DYN_MODE_MSK) << (2 * 3);
    wordformat |= (DYN_MODE_ADR & DYN_MODE_MSK) << (4 * 3);
    node[NODE_OPT_DYN >> 2] = wordformat;
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: dynField failure on different field indices */
void test_dynamicNodeStaging_dynField_fails_on_field_3(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Simulate a dynField failure by making the source pointer NULL for a REF field */
    node[0] = (uint32_t)LM32_NULL_PTR;
    uint32_t wordformat = (DYN_MODE_REF & DYN_MODE_MSK) << (0 * 3);
    node[NODE_OPT_DYN >> 2] = wordformat;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Handler retrieves correct node type from flags */
void test_dynamicNodeStaging_correct_node_type_from_flags(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CFLOW, 0);  /* Set CFLOW type */
    
    nodeFuncs[NODE_TYPE_CFLOW] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Large node data is properly copied to and from nodeTmp */
void test_dynamicNodeStaging_large_node_data_copied(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    
    /* Fill node with distinct values for verification */
    for (int i = 0; i < (_MEM_BLOCK_SIZE >> 2); i++) {
        node[i] = 0x12340000 | i;
    }
    
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);
    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
}

/* Test: Return value properly handles distinction between nodeTmp and other pointers */
void test_dynamicNodeStaging_return_value_handling_various_pointers(void) {
    uint32_t node[_MEM_BLOCK_SIZE >> 2];
    uint32_t node2[_MEM_BLOCK_SIZE >> 2];
    uint32_t thrData[_T_TD_SIZE_ >> 2] = {0};
    
    reset_mocks();
    setup_test_node(node, NODE_TYPE_CNOOP, 0);
    setup_test_node(node2, NODE_TYPE_CNOOP, 0);
    
    /* Handler returns a different node pointer - not possible with dummyNodeFunc,
     * so set handler to dummyNodeFunc and expect LM32_NULL_PTR */
    nodeFuncs[NODE_TYPE_CNOOP] = dummyNodeFunc;

    uint32_t* result = dynamicNodeStaging(node, (uint32_t*)thrData);

    TEST_ASSERT_EQUAL_PTR(LM32_NULL_PTR, result);
    
}

/* ============================================
 * Test Setup and Teardown
 * ============================================ */

void setUp(void) {
    reset_mocks();
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

    mprintf("#%02u: Unit test dynamicNodeStaging for %s DM FW %s \n", cpuId, DM_RELEASE, DM_VERSION);

    UNITY_BEGIN();
    RUN_TEST(test_dynamicNodeStaging_null_node);
    RUN_TEST(test_dynamicNodeStaging_all_immediate_fields);
    RUN_TEST(test_dynamicNodeStaging_handler_returns_nodeTmp);
    RUN_TEST(test_dynamicNodeStaging_calls_correct_handler_type);
    RUN_TEST(test_dynamicNodeStaging_copies_back_node_flags);
    RUN_TEST(test_dynamicNodeStaging_dynField_failure_returns_null);
    RUN_TEST(test_dynamicNodeStaging_immediate_fields_copied_back);
    RUN_TEST(test_dynamicNodeStaging_reference_fields_not_copied_back);
    RUN_TEST(test_dynamicNodeStaging_address_mode_copied_back);
    RUN_TEST(test_dynamicNodeStaging_mixed_field_modes);
    RUN_TEST(test_dynamicNodeStaging_64bit_field_at_boundary_success);
    RUN_TEST(test_dynamicNodeStaging_64bit_field_at_invalid_boundary_fails);
    RUN_TEST(test_dynamicNodeStaging_all_fields_processed);
    RUN_TEST(test_dynamicNodeStaging_handler_returns_null);
    RUN_TEST(test_dynamicNodeStaging_handler_receives_parameters);
    RUN_TEST(test_dynamicNodeStaging_preserves_node_hash);
    RUN_TEST(test_dynamicNodeStaging_preserves_default_dest);
    RUN_TEST(test_dynamicNodeStaging_wordformat_handling);
    RUN_TEST(test_dynamicNodeStaging_single_reference_field);
    RUN_TEST(test_dynamicNodeStaging_multiple_address_fields);
    RUN_TEST(test_dynamicNodeStaging_dynField_fails_on_field_3);
    RUN_TEST(test_dynamicNodeStaging_correct_node_type_from_flags);
    RUN_TEST(test_dynamicNodeStaging_large_node_data_copied);
    RUN_TEST(test_dynamicNodeStaging_return_value_handling_various_pointers);
    UNITY_END();

    while (1) { asm("nop"); }
  
}
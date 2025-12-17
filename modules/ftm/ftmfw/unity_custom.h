#ifndef UNITY_CUSTOM_H
#define UNITY_CUSTOM_H

#include "unity.h"

#define TEST_ASSERT_EQUAL_HEX64_MSG(exp, act)            \
    do {                                                  \
        uint32_t exp_hi = (uint32_t)((exp) >> 32);       \
        uint32_t exp_lo = (uint32_t)((exp) & 0xFFFFFFFF);\
        uint32_t act_hi = (uint32_t)((act) >> 32);       \
        uint32_t act_lo = (uint32_t)((act) & 0xFFFFFFFF);\
                                                         \
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(exp_hi, act_hi, "64b high word differs"); \
        TEST_ASSERT_EQUAL_HEX32_MESSAGE(exp_lo, act_lo, "64b low word differs");  \
    } while(0)

#endif // UNITY_CUSTOM_H
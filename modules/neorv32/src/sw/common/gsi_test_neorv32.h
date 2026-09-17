#ifndef __GSI_TEST
#define __GSI_TEST

#include <neorv32_gpio.h>

#define GSI_TEST_FAILED_GPIO 30
#define GSI_TEST_PASSED_GPIO 31

void gsi_test_failed();
void gsi_test_passed();

#endif

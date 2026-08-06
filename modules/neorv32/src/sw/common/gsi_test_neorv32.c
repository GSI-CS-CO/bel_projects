#include <gsi_test_neorv32.h>

void gsi_test_failed()
{
  neorv32_gpio_pin_set(GSI_TEST_FAILED_GPIO, 1);
}

void gsi_test_passed()
{
  neorv32_gpio_pin_set(GSI_TEST_PASSED_GPIO, 1);
}

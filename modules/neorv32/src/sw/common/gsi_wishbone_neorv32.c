#include <gsi_wishbone_neorv32.h>

void gsi_wishbone_start_atomic_access()
{
  neorv32_gpio_pin_set(GSI_WISHBONE_ATOMIC_ACCESS_GPIO, 1);
}

void gsi_wishbone_stop_atomic_access()
{
  neorv32_gpio_pin_set(GSI_WISHBONE_ATOMIC_ACCESS_GPIO, 0);
}

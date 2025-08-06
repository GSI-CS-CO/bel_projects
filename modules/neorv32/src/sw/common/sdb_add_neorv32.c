#include <sdb_add_neorv32.h>

unsigned int sdb_add(void)
{
  return neorv32_gpio_port_get();
}

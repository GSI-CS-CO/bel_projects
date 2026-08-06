#ifndef __GSI_WISHBONE
#define __GSI_WISHBONE

#include <neorv32_gpio.h>

#define GSI_WISHBONE_ATOMIC_ACCESS_GPIO 0

void gsi_wishbone_start_atomic_access();
void gsi_wishbone_stop_atomic_access();

#endif

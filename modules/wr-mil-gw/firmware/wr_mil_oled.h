#ifndef WR_MIL_OLED_H_
#define WR_MIL_OLED_H_

#include <inttypes.h>
#include <stdint.h>
#include "wr_mil_config.h"

#define OLED_PID 0x93a6f3c4

// update OLED
void oled_array(volatile WrMilConfig *config, volatile uint32_t *oled);

#endif


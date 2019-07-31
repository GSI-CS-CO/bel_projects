#ifndef WR_MIL_OLED_H_
#define WR_MIL_OLED_H_

#include <inttypes.h>
#include <stdint.h>
#include "wr_mil_config.h"

#define OLED_PID 0x93a6f3c4


// this function is intended to be called from the main loop. Most of the time it does nothing,
// but after ~4 sec, it updates the display. This happens one character at a time, i.e. one 
// character per function call. 
// It was done like this to minimize the waiting time for the function to return 
// so that the real-time behavior of the system is not compromised.
//
// returns 1 if a character was written, 0 otherwise
int oled_loop(volatile WrMilConfig *config, volatile uint32_t *oled);

void oled_write_one_char(volatile WrMilConfig *config, volatile uint32_t *oled, int ch);


#endif


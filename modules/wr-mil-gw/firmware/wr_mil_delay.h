#ifndef WR_MIL_DELAY_H_
#define WR_MIL_DELAY_H_

#include <stdint.h>

// Function for delay of 32ns times the argument plus a constant offset.
// It is written in assembler to ensure the same behavior under different 
// compiler flags (e.g. -O -Os ...) 
// arguments:
//   n:  number of 32ns loops
void delay_96plus32n_ns(uint32_t n);

// some convenience macros for fixed delay time
#define DELAY05us   delay_96plus32n_ns(13)
#define DELAY1us    delay_96plus32n_ns(28)
#define DELAY2us    delay_96plus32n_ns(60)
#define DELAY5us    delay_96plus32n_ns(153)
#define DELAY10us   delay_96plus32n_ns(310)
#define DELAY20us   delay_96plus32n_ns(622)
#define DELAY40us   delay_96plus32n_ns(1247)
#define DELAY50us   delay_96plus32n_ns(1560)
#define DELAY100us  delay_96plus32n_ns(3122)
#define DELAY1000us delay_96plus32n_ns(31220)




#endif


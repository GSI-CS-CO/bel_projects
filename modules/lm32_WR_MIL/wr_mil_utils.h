#ifndef WR_MIL_UTILS_H_
#define WR_MIL_UTILS_H_

#include <stdint.h>

// convert 64-bit TAI from WR into an array of five MIL events (EVT_UTC_1/2/3/4/5 events 0xE0 - 0xE4)
// tai is the 64-bit WR-TAI value
// EVT_UTC points to an array of 5 uint32_t
void make_mil_timestamp(uint64_t tai, uint32_t *EVT_UTC);



void delay_96plus32n_ns(uint32_t n);
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

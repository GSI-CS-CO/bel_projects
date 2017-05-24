#ifndef WR_MIL_UTILS_H_
#define WR_MIL_UTILS_H_

#include <stdint.h>
#include "wr_mil_value64bit.h"
#include "wr_mil_eca_ctrl.h"

// convert 64-bit TAI from WR into an array of five MIL events (EVT_UTC_1/2/3/4/5 events with evtNr 0xE0 - 0xE4)
// arguments:
//   TAI:     a 64-bit WR-TAI value
//   EVT_UTC: points to an array of 5 uint32_t and will be filled 
//            with valid special MIL events:
//                            EVT_UTC_1 = EVT_UTC[0] =  ms[ 9: 2]          , code = 0xE0
//                            EVT_UTC_2 = EVT_UTC[1] =  ms[ 1: 0] s[30:25] , code = 0xE1
//                            EVT_UTC_3 = EVT_UTC[2] =   s[24:16]          , code = 0xE2
//                            EVT_UTC_4 = EVT_UTC[3] =   s[15: 8]          , code = 0xE3
//                            EVT_UTC_5 = EVT_UTC[4] =   s[ 7: 0]          , code = 0xE4
//            where s is a 30 bit number (seconds since 2008) and ms is a 10 bit number
//            containing the  milisecond fraction.
void make_mil_timestamp(uint64_t TAI, uint32_t *EVT_UTC);


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


// keep processor in an idle loop that will end at a specified TAI value (that has to be in the future)
// arguments:
//    eca     : valid pointer to ECA registers as obtained by EcaCtrl_init()
//    stopTAI : the TAI value when the function should return
//
// return value: 0 if the function returns at the specified stopTAI. The jitter of the return time 
//                 was measured and is < 120 ns
//               1 if the specified stopTAI was too soon. In this case, the function
//                 returns at a time after the specified stopTAI
uint32_t wait_until_tai(volatile ECACtrlRegs *eca, uint64_t stopTAI);


#endif

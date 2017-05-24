#include "wr_mil_utils.h"

void make_mil_timestamp(uint64_t tai, uint32_t *EVT_UTC) // EVT_UTC_1 = EVT_UTC[0] =  ms[ 9: 2]          , code = 0xE0
                                                         // EVT_UTC_2 = EVT_UTC[1] =  ms[ 1: 0] s[30:25] , code = 0xE1
                                                         // EVT_UTC_3 = EVT_UTC[2] =   s[24:16]          , code = 0xE2
                                                         // EVT_UTC_4 = EVT_UTC[3] =   s[15: 8]          , code = 0xE3
                                                         // EVT_UTC_5 = EVT_UTC[4] =   s[ 7: 0]          , code = 0xE4
{
  uint64_t msNow  = tai / UINT64_C(1000000); // conversion from ns to ms (since 1970)
  uint64_t ms2008 = UINT64_C(1199142000000); // miliseconds at 01/01/2008  (since 1970)
                                             // the number was caluclated using: date --date='01/01/2008' +%s
  uint64_t mil_timestamp_ms = msNow - ms2008;
  uint32_t mil_ms           = mil_timestamp_ms % 1000;
  uint32_t mil_sec          = mil_timestamp_ms / UINT64_C(1000);

  EVT_UTC[0]  = (mil_ms>>2)   & 0x000000ff;  // mil_ms[9:2]    to EVT_UTC_1[7:0]
  EVT_UTC[1]  = (mil_ms<<6)   & 0x000000C0;  // mil_ms[1:0]    to EVT_UTC_2[7:5]
  EVT_UTC[1] |= (mil_sec>>24) & 0x0000003f;  // mil_sec[29:24] to EVT_UTC_2[5:0]
  EVT_UTC[2]  = (mil_sec>>16) & 0x000000ff;  // mil_sec[23:16] to EVT_UTC_3[7:0]
  EVT_UTC[3]  = (mil_sec>>8)  & 0x000000ff;  // mil_sec[15:8]  to EVT_UTC_4[7:0]
  EVT_UTC[4]  =  mil_sec      & 0x000000ff;  // mil_sec[7:0]   to EVT_UTC_5[7:0]

  // shift time information to the upper bits [15:8] and add code number
  EVT_UTC[0] = (EVT_UTC[0] << 8) | 0x22; //0x20 ;// 0xE0;
  EVT_UTC[1] = (EVT_UTC[1] << 8) | 0x22; //0x21 ;// 0xE1;
  EVT_UTC[2] = (EVT_UTC[2] << 8) | 0x22; //0x22 ;// 0xE2;
  EVT_UTC[3] = (EVT_UTC[3] << 8) | 0x22; //0x23 ;// 0xE3;
  EVT_UTC[4] = (EVT_UTC[4] << 8) | 0x22; //0x24 ;// 0xE4;
}

// the implementation of thif function is in file "wr_mil_delay_96plus32n_ns.s" and is implemented
// in assembly to make sure that the delay is not changed by compiler version or settings
//void delay_96plus32n_ns(uint32_t n)
//{
//  while(n--) asm("nop"); // if asm("nop") is missing, compiler will optimize the loop away
//}

#ifndef WR_MIL_VALUE64BIT_H_
#define WR_MIL_VALUE64BIT_H_

/* provides union TAI_t to facilitate access and manipulation of 64-bit TAI values */
#include <stdint.h>

typedef struct 
{
  uint32_t hi;
  uint32_t lo;
} HiLo_t;

typedef union 
{
  // LM32 is big endian high part first (i.e. at pos[0])
  // uint32_t pos[2]; // pos[0] contains high bit part;   pos[1] contains low bit part
  HiLo_t   part;
  uint64_t value;
} Value64Bit_t;

typedef Value64Bit_t TAI_t;
typedef Value64Bit_t EvtId_t;

#endif

#ifndef WR_MIL_TAI_H_
#define WR_MIL_TAI_H_

/* provides union TAI_t to facilitate access and manipulation of 64-bit TAI values */
#include <stdint.h>

typedef struct 
{
  uint32_t hi;
  uint32_t lo;
} HiLo_t;

typedef union 
{
  // uint32_t pos[2]; // pos[0] contains high bit part;   pos[1] contains low bit part
  HiLo_t   part;
  uint64_t value;
} TAI_t;

#endif

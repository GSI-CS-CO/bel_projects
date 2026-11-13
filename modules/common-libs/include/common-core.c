/********************************************************************************************
 *  common-core.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 13-nov-2026
 *
 *  common routines for x86 and ecpu firmware; common-lib and common-fwlib provide
 *  wrappers to this code 
 * 
 *  see common-core.h for version, license and documentation
 *
 ********************************************************************************************/
#include <stdint.h>
#include <common-defs.h>

// helper routine converting single precision float as 32bit to native float
float comcore_u2f(uint32_t u)
{
  fdat_t fdat;

  fdat.data = u;

  return fdat.f;
} // comcore_u2f


// helper routine converting single precision float to 32bit
uint32_t comcore_f2u(float f)                      // was declared 'float': the bit pattern went through a
{                                                  // numeric float conversion and lost its low 8 bits
  fdat_t fdat;

  fdat.f = f;

  return fdat.data;
} // comcore_f2u


uint16_t comcore_float2half(float f)
{
  uint32_t x;                                      // float number as uint32_t
  uint32_t mag;                                    // magnitude (sign removed)
  uint32_t e;                                      // exponent
  uint16_t s;                                      // sign bit

  x   = comcore_f2u(f);
  s   = (x >> 16) & 0x8000;                        // sign bit, kept for zero, infinity and overflow
  mag = x & 0x7fffffff;

  if (mag >= 0x7f800000) {                         // infinity or NaN
    if (mag == 0x7f800000) return s | 0x7c00;      // +-infinity
    return 0xfcff;                                 // any NaN; same encoding as before for compatibility
  } // if inf or NaN

  mag += 0x00000fff + ((mag >> 13) & 0x1);         // round to nearest, ties to even

  e = mag >> 23;                                   // biased exponent after rounding
  if (e > 127 + 15) return s | 0x7c00;             // overflow -> +-infinity 
  if (e < 127 - 14) return s;                      // no subnormals: flush to signed zero

  return s | (uint16_t)((e - 127 + 15) << 10) | (uint16_t)((mag >> 13) & 0x03ff);
} //comcore_float2half


float comcore_half2float(uint16_t h){
  uint32_t e;                                       // exponent
  uint32_t m;                                       // mantissa
  uint32_t s;                                       // sign bit
  float    f;                                       // result

  s  = (uint32_t)(h & 0x8000) << 16;                // sign bit, shifted to single precision position
  e  = (h >> 10) & 0x1f;                            // exponent
  m  = h & 0x03ff;                                  // mantissa

  if (h == 0xfcff) return comcore_u2f(0x7fc00000);  // NaN encoding used by comcore_float2half
  if (e == 0x1f)   return comcore_u2f(s | 0x7f800000 | (m << 13)); // +-infinity, other NaNs
  if (e == 0) {                                     // zero or subnormal
    f = (float)m / 16777216.0f;                     // m * 2^-24, exact 
    return s ? -f : f;
  } // if e

  f = comcore_u2f(s | ((e + 127 - 15) << 23) | (m << 13));

  return f;
} // comcore_half2float


// unsigned integer division, rounding half up; returns 0xffffffff for d == 0
uint32_t comcore_intdiv(uint32_t n, uint32_t d){
  uint32_t q;
  uint32_t r;

  if (d == 0) return 0xffffffff;                    // division by 0

  q = n / d;
  r = n % d;
  if (r >= d - r) q++;                              // same rounding as (n + d/2) / d; but without overflow

  return q;
} // comcore_intdiv

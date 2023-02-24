/********************************************************************************************
 *  common-core.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 23-Feb-2023
 *
 *  common routines for x86 and ecpu firmware; common-lib and common-fwlib provide
 *  wrappers to this code 
 * 
 *  see common-core.h for version, license and documentation
 *
 ********************************************************************************************/
#include <common-defs.h>

// helper routine converting single precision float as 32bit to native float
float comcore_u2f(uint32_t u)
{
  fdat_t fdat;

  fdat.data = u;

  return fdat.f;
} // comcore_u2f


// helper routine converting single precision float to 32bit
float comcore_f2u(float f)
{
  fdat_t fdat;

  fdat.f = f;

  return fdat.data;
} // comcore_f2u


uint16_t comcore_float2half(float f)
{
  uint32_t x;                                      // float number as uin32_t
  uint16_t e;                                      // exponent
  uint16_t m;                                      // mantissa
  uint16_t s;                                      // sign bit
  uint16_t h;                                      // result

  x = comcore_f2u(f);                              // float to uint32

  switch (x) {
    case 0x00000000 : return  0x0000; break;       //  0
    case 0x80000000 : return  0x8000; break;       // -0
    case 0xffc00001 : return  0xfcff; break;       // NaN
    case 0xff800001 : return  0xfcff; break;       // NaN
    case 0x7fc00000 : return  0xfcff; break;       // NaN
    case 0x7f800000 : return  0x7c00; break;       // +infinity
    case 0xff800000 : return  0xfc00; break;       // -infinity
    default    : 
      x += 0x00001000;                             // round to nearest even
      
      s  = (x >> 16) & 0x8000;                     // get sign bit
      
      e  =  x >> 23;                               // exponent; here: no support for subnormal numbers
      e &= 0xff;                                   // mask relevant bits
      if (e >  127 + 15) return 0x7c00;            // largest supported exponent is 15
      if (e <  127 - 14) return 0x8000;            // smallest supported exponent is -14
      e -= 127;                                    // subtract exponent bias 127 (single precision)
      e +=  15;                                    // add exponent bias 15 (half precision)
      e  = e << 10;                                // shift to relevant position
      
      m  = x >> 13;                                // shift mantissa
      m &= 0x03ff;                                 // mask relevant bits

      h  = s | e | m;

      break;
  } // switch x

  return h;
} //comcore_float2half


float comcore_half2float(uint16_t h){
  uint32_t e;                                       // exponent
  uint32_t m;                                       // mantissa
  uint32_t s;                                       // sign bit
  float    f;                                       // result

  switch (h) {
    case 0x0000 : return comcore_u2f(0x00000000); break; //  0
    case 0x8000 : return comcore_u2f(0x80000000); break; // -0
    case 0xfcff : return comcore_u2f(0x7fc00000); break; // NaN
    case 0x7c00 : return comcore_u2f(0x7f800000); break; // +infinity
    case 0xfc00 : return comcore_u2f(0xff800000); break; // -infinity
    default: 
      s  = h & 0x8000;                              // get sign bit
      s  = s << 16;                                 // shift to relevant position
      
      e  = h >> 10;                                 // exponent; no support for subnormal numbers
      e &= 0x1f;                                    // mask relevant bits
      e += 0x7f;                                    // add exponent bias 127, single precision
      e -= 0x0f;                                    // subtract exponent bias 15, half precision
      e  = e << 23;                                 // shift to relevant position
      
      m  = h & 0x03ff;                              // get relevant bits
      m  = m << 13;                                 // shift to relevant position

      f  = comcore_u2f(s | e | m);

      break;
  } // switch h

  return f;
} // comcore_half2float

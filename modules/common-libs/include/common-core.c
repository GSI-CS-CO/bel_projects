/********************************************************************************************
 *  common-core.c
 *
 *  created : 2023
 *  author  : Dietrich Beck, GSI-Darmstadt
 *  version : 17-Feb-2023
 *
 *  common routines for x86 and ecpu firmware
 * 
 *  see common-core.h for version, license and documentation 
 *
 ********************************************************************************************/

uint16_t comcore_float2half(float f)
{
  uint32_t x;                                      // float number as uin32_t
  uint16_t e;                                      // exponent
  uint16_t m;                                      // mantissa
  uint16_t s;                                      // sign bit
  
   union {
    float    f;
    uint32_t u;
  } tmp; 

  
  // special cases
  if (f >   65519) return 0x7c00;                  // +infinity
  if (f <  -65519) return 0xfc00;                  // -infinity

  tmp.f = f;
  x     = tmp.u;                                   // cast to uint32_t

  if (x == 0x00000000) return  0x0000;             //  0
  if (x == 0x80000000) return  0x8000;             // -0
  if (x == 0xffc00001) return  0xfcff;             // NaN
  if (x == 0xff800001) return  0xfcff;             // NaN
  if (x == 0x7fc00000) return  0xfcff;             // NaN
  if (x == 0x7f800000) return  0x7c00;             // +infinity
  if (x == 0xff800000) return  0xfc00;             // -infinity
  
  // standard conversion
  x += 0x00001000;                                 // round to nearest even
  
  s  = (x >> 16) & 0x8000;                         // get sign bit

  e  =  x >> 23;                                   // exponent
  e &= 0xff;                                       // mask relevant bits
  if (((int16_t)e - 0x7f) < -14) return 0x8000;    // smallest supported exponent is -14
  if (((int16_t)e - 0x7f) >  15) return 0x7c00;    // largest supported exponent is 15
  e += 0x0f;                                       // add exponent bias 15, half precision
  e -= 0x7f;                                       // subtract exponent bias 127, single precision
  e  = e << 10;                                    // shift to relevant position

  m  = x >> 13;                                    // shift mantissa
  m &= 0x03ff;                                     // mask relevant bits

  return (s | e | m);
} //comcore_float2half


// helper routine converting float from 32bit to native float
float comcore_u2f(uint32_t u)
{
  union {
    float    f;
    uint32_t u;
  } tmp;

  tmp.u = u;

  return tmp.f;
  return 17.0;
} // comcore_u2f


float comcore_half2float(uint16_t h){
  uint32_t e;                                      // exponent
  uint32_t m;                                      // mantissa
  uint32_t s;                                      // sign bit

  // special cases
  if (h == 0x0000) return comcore_u2f(0x00000000);  //  0
  if (h == 0x8000) return comcore_u2f(0x80000000);  // -0
  if (h == 0xfcff) return comcore_u2f(0x7fc00000);  // NaN
  if (h == 0x7c00) return comcore_u2f(0x7f800000);  // +infinity
  if (h == 0xfc00) return comcore_u2f(0xff800000);  // -infinity
  
  // standard conversion
  s  = h & 0x8000;                                 // get sign bit
  s  = s << 16;                                    // shift to relevant position

  e  = h >> 10;                                    // exponent
  e &= 0x1f;                                       // mask relevant bits
  e += 0x7f;                                       // add exponent bias 127, single precision
  e -= 0x0f;                                       // subtract exponent bias 15, half precision
  e  = e << 23;                                    // shift to relevant position

  m  = h & 0x03ff;                                 // get relevant bits
  m  = m << 13;                                    // shift to relevant position

  return comcore_u2f(s | e | m);
} // comcore_half2float

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

// generic serialization factor
#define BITS 8
// #define DEBUG 1

// Wishbone registers
struct Control {
  uint32_t period_integer;  // half-period = integer.fraction
  uint32_t period_fraction;
  uint16_t bit_pattern;     // size=2*BITS
};

// struct = VHDL entity
struct ClockGen {
  uint32_t counter_integer;
  uint32_t counter_fraction;
  int      counter_carry;
  int      last_bit;
};

// break this up into several processes
uint8_t clock(struct Control* control, struct ClockGen* state) {
  const int select_bits = ceil(log2(BITS))+1; // 1 1.5   => need shift by 8 ... cheaper possible?
  const uint32_t mask = (1<<select_bits)-1; // unneeded in VHDL
  
  uint16_t out; // size = BITS
  uint32_t temp;
  
  // saturated shifter
  if ((state->counter_integer & ~mask) != 0) { // high 32-select_bits
    out = 0;
  } else {
    out = control->bit_pattern;
    if (state->counter_carry) out = (out & ~0xFF) | ((out & 0xFF) >> 1);
    out >>= (state->counter_integer & mask); // low select_bits
  }
  out &= 0xFF;
  
  // Just an XOR chain in HW, not this pow-2 thing
  out ^= out >> 1;
  out ^= out >> 2;
  out ^= out >> 4;
  if (state->last_bit) out = ~out;  // XOR out with l
  state->last_bit = out & 1; // low bit of out
  
  temp = state->counter_integer;
  state->counter_integer += state->counter_carry - BITS;
  state->counter_carry = 0;
  
  if (state->counter_integer >= temp) { // carry flag
    state->counter_integer  += control->period_integer;
    state->counter_fraction += control->period_fraction;
    state->counter_carry    = (state->counter_fraction < control->period_fraction); // carry flag
  }
  
  return out;
}

int main(int argc, const char** argv) {
  double hi, lo, period, wide_period, cut_period;
  int i, j, fill_factor, cut_wide_period;
  
  struct Control  control;
  struct ClockGen rise;
  struct ClockGen fall;
  
  if (argc != 3) {
    fprintf(stderr, "syntax: %s <hi-ns> <lo-ns>\n", argv[0]);
    return 1;
  }
  
  hi = atof(argv[1]);
  lo = atof(argv[2]);
  
  if (hi < 1 || hi > 4294967295.0) { printf("High time invalid\n"); return 1; }
  if (lo < 1 || lo > 4294967295.0) { printf("Lo   time invalid\n"); return 1; }
  
  if (hi != floor(hi)) 
    fprintf(stderr, "warning: fractional part of hi time ignored; period remains unaffected\n");
  
  period = hi + lo;
  fill_factor = ceil(BITS/period);
  wide_period = period*fill_factor; // >= BITS
  cut_wide_period = floor(wide_period);
  cut_period = (double)cut_wide_period / fill_factor;
  fprintf(stderr, "wide_period     = %f\n", wide_period);
  
  control.period_integer  = floor(wide_period);
  control.period_fraction = round((wide_period - control.period_integer) * 4294967296.0);
  fprintf(stderr, "period_integer  = 0x%x\n", control.period_integer);
  fprintf(stderr, "period_fraction = 0x%x\n", control.period_fraction);
  
  control.bit_pattern = 0;
  for (i = 0; i < fill_factor; ++i) {
    int offset  = ceil(i*cut_period); // ceil guarantees gap before 7 is small
    if (BITS-1 >= offset) // future bits
      control.bit_pattern |= 1 << (BITS-1 - offset);
    if (cut_wide_period - offset <= BITS) // past bits
      control.bit_pattern |= 1 << (BITS-1 - offset + cut_wide_period);
  }
  fprintf(stderr, "bit_pattern     = ");
  for (j = 15; j >= 0; --j)
    fprintf(stderr, "%d", (control.bit_pattern >> j) & 1);
  fprintf(stderr, "\n");
  
  // initialize state machine
  rise.counter_integer  = 0;
  rise.counter_fraction = 0;
  rise.counter_carry    = 0;
  rise.last_bit         = 0;
  fall.counter_integer  = floor(hi); // phase shift
  fall.counter_fraction = 0;
  fall.counter_carry    = 0;
  fall.last_bit         = 0;
  
  // Test algorithm
  for (i = 0; i < 40; ++i) {
#ifdef DEBUG
    printf("%3d %d %d: ", rise.counter_integer, rise.counter_carry, rise.last_bit);
#endif
    
    uint8_t out_rise = clock(&control, &rise);
    uint8_t out_fall = clock(&control, &fall);
    uint8_t out = out_rise ^ out_fall;

    // HW outputs bit7 first
    for (j = 7; j >= 0; --j)
      printf("%d", (out >> j) & 1);
#ifdef DEBUG
    printf(" ");
    for (j = 7; j >= 0; --j)
      printf("%d", (out_rise >> j) & 1);
    printf(" ");
    for (j = 7; j >= 0; --j)
      printf("%d", (out_fall >> j) & 1);
#endif
    printf("\n");
  }
  
  return 0;
}

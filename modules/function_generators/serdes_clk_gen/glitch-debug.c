#include <stdio.h>

int main() {
  int i;
  int count = 0;
  int last = 1;
  
  int ones  = -1;
  int zeros = -1;
  int big_zeros = 0;
  int small_zeros = 0;
  
  int trace = 0;
  
  while ((i = fgetc(stdin)) != EOF) {
    if (i != '0' && i != '1') continue;
    if ((i == '0' && last == 0) ||
        (i == '1' && last == 1)) {
      ++count;
      continue;
    }
    
    if (last == 1) {
      if (ones == -1) ones = count;
      if (ones != count)
        fprintf(stderr, "Anomaly: %d ones (not %d)\n", count, ones);
    } else {
      if (zeros == -1 || zeros-1 == count) {
        zeros = count;
        big_zeros = 0;
        small_zeros = 0;
      } else {
        if (count == zeros) ++small_zeros;
        if (count == zeros+1) ++big_zeros;
        if (zeros != count && zeros+1 != count)
          fprintf(stderr, "Anomaly: %d zeros (not %d/%d)\n", count, zeros, zeros+1);
      }
    }
    last = i - '0';
    count = 1;
    
    if (++trace == 10000) {
      printf("Hi: %d, Lo: %.17g\n", ones, zeros + big_zeros*1.0/(big_zeros+small_zeros));
      trace = 0;
    }
  }
  printf("Hi: %d, Lo: %.17g\n", ones, zeros + big_zeros*1.0/(big_zeros+small_zeros));
  return 0;
}

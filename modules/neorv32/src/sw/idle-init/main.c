#include <stddef.h>
#include <stdlib.h>
#include <neorv32.h>
#include <neorv32_uart.h>

#define BAUD_RATE 921600

int add_test(int a, int b);

int main(void)
{
  while(true) {
    int i = 0; // this instruction is only here so that the program counter has to advance noticeably for debugging
  }
}

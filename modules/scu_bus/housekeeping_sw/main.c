#include <stdint.h>

#include <inttypes.h>
#include <stdarg.h>

#include "mprintf.h"
#include "board.h"
#include "uart.h"
#include "w1.h"

extern struct w1_bus wrpc_w1_bus;

volatile unsigned short* scu_reg = (unsigned short*)BASE_SCU_REG;

void usleep(int x)
{
  int i;
  for (i = x * CPU_CLOCK/1000/4; i > 0; i--) asm("# noop");
}

void msDelay(int msecs) {
	int i;
	for(i = msecs * CPU_CLOCK/4; i > 0; i--)
		asm("# noop");
}

void ReadTempDevices(int bus) {
  struct w1_dev *d;
  int i;
  int tvalue;
  wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
  if (w1_scan_bus(&wrpc_w1_bus) > 0) {
    for (i = 0; i < W1_MAX_DEVICES; i++) {
      d = wrpc_w1_bus.devs + i;
        if (d->rom) {
          mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
          scu_reg[0] = (unsigned short)(d->rom >> 48);
          scu_reg[1] = (unsigned short)(d->rom >> 32);
          scu_reg[2] = (unsigned short)(d->rom >> 16);
          scu_reg[3] = (unsigned short)d->rom;
          if ((char)d->rom == 0x42) {
            tvalue = w1_read_temp(d, 0);
            scu_reg[4] = (unsigned short)(tvalue >> 12);
            mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
          }
          mprintf("\n");
        }
    }
  } else {
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
  }
}

void init() {

	uart_init_hw();
	uart_write_string("Debug Port\n");
 
  // Find the device(s)
  // on the ADDAC card are two ow ports
  wrpc_w1_init();
  ReadTempDevices(0); 
	
} //end of init()

int main(void)
{
	init();
	
	while(1) {
    ReadTempDevices(0); 
	}
}

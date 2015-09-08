#include <stdint.h>

#include <inttypes.h>
#include <stdarg.h>

#include "syscon.h"
#include "hw/memlayout.h"
#include "mprintf.h"
#include "board.h" //from WR
#include "hk_board.h"
#include "uart.h"
#include "w1.h"
#include "mini_sdb.h"


extern struct w1_bus wrpc_w1_bus;
volatile unsigned short* scu_reg;
volatile unsigned int* aru_base;
volatile unsigned int* asmi_base;


//getSysTime() needs extra hardware from ftm cluster
//void msDelayBig(uint64_t ms)
//{
//  uint64_t later = getSysTime() + ms * 1000000ULL / 8;
//  while(getSysTime() < later) {asm("# noop");}
//}

void msDelay(uint32_t msecs) {
  usleep(1000 * msecs);
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

int main(void)
{
  discoverPeriphery();
  aru_base      = (unsigned int *)find_device_adr(GSI, WB_REMOTE_UPDATE);
  scu_reg       = (unsigned short *)find_device_adr(GSI, WB_SCU_REG);
  asmi_base     = (unsigned int *)find_device_adr(GSI, WB_ASMI);
  BASE_ONEWIRE  = (unsigned char *)find_device_adr(CERN, WR_1Wire);

  if (!BASE_UART) {
    while (1) {};
  }
  uart_init_hw();
  uart_write_string("Debug Port\n");

  mprintf("aru_base: 0x%x\n", aru_base);
  mprintf("scu_reg: 0x%x\n", scu_reg);
  mprintf("asmi_base: 0x%x\n", asmi_base);
  mprintf("BASE_UART: 0x%x\n", BASE_UART);
  mprintf("BASE_ONEWIRE: 0x%x\n", BASE_ONEWIRE);

  if (!aru_base) {
    mprintf("no remote update controller found!\n");
    while (1) {};
  }
  if (!scu_reg) {
    mprintf("no WB to SCU memory found!\n");
    while (1) {};
  }
  if (!asmi_base) {
    mprintf("no ASMI controller found!\n");
    while (1) {};
  }
  if (!BASE_ONEWIRE) {
    mprintf("no 1Wire controller found!\n");
    while (1) {};
  }
 
  wrpc_w1_init();

  if (aru_base[CONFIG_SRC] == 0) {  //PowerUp
    aru_base[PAGE_SEL] = 0x90000; // start address for Application image
    aru_base[CONFIG_MODE] = 0x1;  // set to Application mode
    //aru_base[CONFIG] = 0x1; // trigger reconfiguration
  }

    
	while(1) {
    ReadTempDevices(0); 
	}
}

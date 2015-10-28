#include "scu_bus.h"
#include "display.h"
#include "w1.h"
#include "inttypes.h"
#include "mprintf.h"

//#define DEBUG


/*  for every found slave the slotnumber is added to the slave array
    e.g. [2,3] means slaves in slot 2 and 3
*/
void probe_scu_bus(volatile unsigned short* bus_addr, unsigned short system_addr, unsigned short group_addr, int* slaves) {
  int slot;
  unsigned short cid_sys, cid_group;
  for (slot = 1; slot <= MAX_SCU_SLAVES; slot++) {
    cid_sys = bus_addr[(slot<<16) + CID_SYS];     //CID system addr from slave
    cid_group = bus_addr[(slot<<16) + CID_GROUP]; //CID group addr from slave
    if (cid_sys == system_addr && cid_group == group_addr) 
      *(slaves++) = slot;  
  }
  *slaves = 0; // end of device list 
}

void ReadTempDevices(int bus, uint64_t *id, uint32_t *temp) {
  struct w1_dev *d;
  int i;
  int tvalue;
  wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
  if (w1_scan_bus(&wrpc_w1_bus) > 0) {
    for (i = 0; i < W1_MAX_DEVICES; i++) {
      d = wrpc_w1_bus.devs + i;
        if (d->rom) {
          #ifdef DEBUG
          mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
          #endif
          if ((char)d->rom == 0x42) {
            *id = d->rom;
            tvalue = w1_read_temp(d, 0);
            *temp = (tvalue >> 12); //full precision with 1/16 degree C
            #ifdef DEBUG
            mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
            #endif
          }
          #ifdef DEBUG
          mprintf("\n");
          #endif
        }
    }
  } else {
    #ifdef DEBUG
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
    #endif
  }
} 

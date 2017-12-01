#include "scu_bus.h"
#include "display.h"
#include "w1.h"
#include "inttypes.h"
#include "mprintf.h"
#include "dow_crc.h"
#include "scu_mil.h"
#include "daq.h"

#define DEBUG

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
          if ((calc_crc((int)(d->rom >> 32), (int)d->rom)) != 0)
            continue;
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

void scan_scu_bus(volatile unsigned short *scub_adr, uint32_t *fglist, uint32_t *daqlist) {
  unsigned short ext_clk_reg;
  short ifa_id, ifa_vers, fg_vers;
  unsigned char ifa_adr;
  int slot;
  int cid_group;
  int cid_sys;
  int slave_version;
  int fg_ver;
  int chn;

  // scu bus slaves
  for (slot = 1; slot <= MAX_SCU_SLAVES; slot++) {
    scub_adr[OFFS(slot)+ 0x10] = 0; //clear echo reg
    if (scub_adr[OFFS(slot) + 0x10] != 0xdead) {

      cid_group     = scub_adr[OFFS(slot) + CID_GROUP];
      cid_sys       = scub_adr[OFFS(slot) + CID_SYS];
      slave_version = scub_adr[OFFS(slot) + SLAVE_VERSION];
      fg_ver        = scub_adr[OFFS(slot) + FG1_BASE + FG_VER];

      ext_clk_reg = scub_adr[OFFS(slot) + SLAVE_EXT_CLK];          //read clk status from slave
      if (ext_clk_reg & 0x1)
        scub_adr[OFFS(slot) + SLAVE_EXT_CLK] = 0x1;                //switch clk to sys clk from scu bus

      // if slave is a sio3, scan for ifa cards
      if (cid_sys == SYS_CSCO && (cid_group == GRP_SIO3 || cid_group == GRP_SIO2)) {
        // reset all taskslots by reading value back
        scub_reset_mil(scub_adr, slot);

        for (ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++) {
          if (scub_read_mil(scub_adr, slot, &ifa_id, IFA_ID << 8 | ifa_adr) != OKAY)     continue;
          if (scub_read_mil(scub_adr, slot, &ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY) continue;
          if (scub_read_mil(scub_adr, slot, &fg_vers, 0xa6 << 8 | ifa_adr) != OKAY)      continue;

          if (((0xffff & fg_vers) >= 0x2) && ((0xffff & ifa_id) == 0xfa00) && ((0xffff & ifa_vers) >= 0x1900)) {
            add_to_fglist(DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, 0xffff & fg_vers, fglist);
          }
        }
      } else {
        /* search for daq channels */
        chn = 0;
        while (scub_adr[OFFS(slot) + DAQ_CNTRL(chn++)] != 0xdead);
        if (chn-1)
          add_to_daqlist(slot, chn-1, cid_sys, cid_group, daqlist); 

        add_to_fglist(slot, ifa_adr, cid_sys, cid_group, fg_ver, fglist);
      }
    }
  }
}

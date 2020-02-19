#include <fg.h>
#include <scu_bus.h>
#include <string.h>
#include <unistd.h>
#include <mprintf.h>
#include <scu_mil.h>
#include <mini_sdb.h>

#define OFFS(SLOT) ((SLOT) * (1 << 16))

int add_to_fglist(int slot, int dev, int cid_sys, int cid_group, int fg_ver, uint32_t *fglist) {
  int found = 0;
  int count = 0;

  /* find first free slot */
  while ((fglist[count] != 0) && (count < MAX_FG_MACROS)) {
    count++;
  }
  
  if (cid_sys == SYS_CSCO || cid_sys == SYS_PBRF || cid_sys == SYS_LOEP) {
    if (cid_group == GRP_ADDAC1 ||
        cid_group == GRP_ADDAC2 ||
        cid_group == GRP_DIOB) {
      /* two FGs */
      if (count < MAX_FG_MACROS) {
        fglist[count] = 16;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= 0x0 << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
      if (count < MAX_FG_MACROS) {
        fglist[count] = 16;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= 0x1 << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
     
    /* ACU/MFU */
    } else if (cid_group == GRP_MFU) {
      /* two FGs */
      if (count < MAX_FG_MACROS) {
        fglist[count] = 20;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= 0x0 << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
      if (count < MAX_FG_MACROS) {
        fglist[count] = 20;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= 0x1 << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
    
    /* FIB */ 
    } else if (cid_group == GRP_FIB_DDS) {
      /* one FG */
      if (count < MAX_FG_MACROS) {
        fglist[count] = 32;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= 0x0 << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
    /* IFA8 */
    } else if (cid_group == GRP_IFA8) {
      if (count < MAX_FG_MACROS) {
        fglist[count] = 16;           // output bits
        fglist[count] |= fg_ver << 8; // version
        fglist[count] |= dev << 16;   // device: macro number inside of the slave card
        fglist[count] |= slot << 24;  // slot
        count++;                      // next macro
        found++;
      }
    }
  }

  return count; //return number of found fgs
}
void scan_scu_bus(volatile unsigned short *scub_adr, volatile unsigned int *mil_addr, uint32_t *fglist, uint64_t *ext_id) {
  int i, j = 0;
  unsigned short ext_clk_reg;
  short ifa_id, ifa_vers, fg_vers;
  unsigned char ifa_adr;
  int slot;
  int cid_group;
  int cid_sys;
  int slave_version;
  int fg_ver;

  // scu bus slaves
  for (i = 1; i <= MAX_SCU_SLAVES; i++) {
    scub_adr[OFFS(i) + 0x10] = 0; //clear echo reg
    if (scub_adr[OFFS(i) + 0x10] != 0xdead) {

      slot          = i;
      cid_group     = scub_adr[OFFS(i) + CID_GROUP];
      cid_sys       = scub_adr[OFFS(i) + CID_SYS];
      slave_version = scub_adr[OFFS(i) + SLAVE_VERSION];
      fg_ver        = scub_adr[OFFS(i) + FG1_BASE + FG_VER];

      ext_clk_reg = scub_adr[OFFS(i) + SLAVE_EXT_CLK];          //read clk status from slave
      if (ext_clk_reg & 0x8)                                    //test for local clk running
        scub_adr[OFFS(i) + SLAVE_EXT_CLK] = 0x1;                //switch clk to sys clk from scu bus

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
            //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
          }
        }
      } else {
        add_to_fglist(slot, ifa_adr, cid_sys, cid_group, fg_ver, fglist);
      }
    }
  }
  /* check only for ifks, if there is a macro found and a mil extension attached to the baseboard */
  /* mil extension is recognized by a valid 1wire id                                              */
  /* mil extension has a 1wire temp sensor with family if 0x42                                    */
  slot = 0;
  if (((int)mil_addr != ERROR_NOT_FOUND) && (((int)*ext_id & 0xff) == 0x42)) {
    // reset all taskslots by reading value back
    reset_mil(mil_addr);

    for (ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++) {
      if (read_mil(mil_addr, &ifa_id, IFA_ID << 8 | ifa_adr) != OKAY)     continue; 
      if (read_mil(mil_addr, &ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY) continue; 
      if (read_mil(mil_addr, &fg_vers, 0xa6 << 8 | ifa_adr) != OKAY)      continue; 
      
      if (((0xffff & fg_vers) >= 0x2) && ((0xffff & ifa_id) == 0xfa00) && ((0xffff & ifa_vers) >= 0x1900)) {
        add_to_fglist(DEV_MIL_EXT | slot, ifa_adr, SYS_CSCO, GRP_IFA8, 0xffff & fg_vers, fglist);
        //write_mil(mil_addr, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
      }
    }
  }
}



/* init the buffers for MAX_FG_CHANNELS */
void init_buffers(struct channel_regs *cr, int channel, uint32_t *fg_macros,  volatile unsigned short* scub_base, volatile unsigned int* devb_base) {
  uint32_t slot;
  uint32_t dev;
  uint32_t macro;
  if(channel >= 0 && channel < MAX_FG_CHANNELS) {
    cr[channel].wr_ptr = 0;
    cr[channel].rd_ptr = 0;
    cr[channel].state = 0;
    cr[channel].ramp_count = 0;
    //reset hardware
    reset_mil(devb_base);
    scub_reset_mil(scub_base, slot);
    if (cr[channel].macro_number >= 0) {    //there is a macro assigned to that channel
      macro = cr[channel].macro_number;
      slot = fg_macros[macro] >> 24;
      dev = (fg_macros[macro] >> 16) & 0xff;
      //mprintf("reset fg %d in slot %d\n", device, slot);
      /* scub slave */
      if ((slot & 0xf0) == 0) {
        if (dev == 0) {
          scub_base[OFFS(slot) + FG1_BASE + FG_CNTRL] = 0x1; // reset fg
        } else if (dev == 1) {
          scub_base[OFFS(slot) + FG2_BASE + FG_CNTRL] = 0x1; // reset fg
        }
      /* mil extension */
      } else if (slot & DEV_MIL_EXT) {
        write_mil(devb_base, 0x1, FC_CNTRL_WR | dev); // reset fg 
      } else if (slot & DEV_SIO) {
        scub_write_mil(scub_base, slot & 0xf, 0x1, FC_CNTRL_WR | dev); // reset fg
      }
    }
  }
}

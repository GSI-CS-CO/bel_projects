#include <fg.h>
#include <scu_bus.h>
#include <string.h>
#include <unistd.h>
#include <mprintf.h>
#include <scu_mil.h>
#include <mini_sdb.h>


int scan_scu_bus(struct scu_bus *bus, uint64_t id, volatile unsigned short *scub_adr, volatile unsigned int *mil_addr) {
  int i, j = 0;
  unsigned short ext_clk_reg;
  short data;
  unsigned char adr;
  memset(bus->slaves, 0, sizeof(bus->slaves));
  bus->unique_id = id;

  for (i = 1; i <= MAX_SCU_SLAVES; i++) {
    scub_adr[i * (1<<16) + 0x10] = 0; //clear echo reg
    if (scub_adr[i * (1<<16) + 0x10] != 0xdead) {
      bus->slaves[j].unique_id = (uint64_t)(scub_adr[i * (1<<16) + 0x40]) << 48;
      bus->slaves[j].unique_id |= (uint64_t)(scub_adr[i * (1<<16) + 0x41]) << 32;
      bus->slaves[j].unique_id |= (uint64_t)(scub_adr[i * (1<<16) + 0x42]) << 16;
      bus->slaves[j].unique_id |= (uint64_t)(scub_adr[i * (1<<16) + 0x43]);

      bus->slaves[j].slot = i;
      bus->slaves[j].cid_group = scub_adr[i * (1<<16) + CID_GROUP];
      bus->slaves[j].cid_sys = scub_adr[i * (1<<16) + CID_SYS];
      bus->slaves[j].version = scub_adr[i * (1<<16) + SLAVE_VERSION];
      bus->slaves[j].fg_ver = scub_adr[i * (1<<16) + FG1_BASE + FG_VER]; 

      ext_clk_reg = scub_adr[(i << 16) + SLAVE_EXT_CLK];          //read clk status from slave
      if (ext_clk_reg & 0x1)
        scub_adr[(i << 16) + SLAVE_EXT_CLK] = 0x1;                //switch clk to sys clk from scu bus

      j++; /* next found slave */
    }
  }

  if ((int)mil_addr != ERROR_NOT_FOUND) {
    clear_receive_flag(mil_addr);
    for (adr = 0; adr < IFK_MAX_ADR; adr++) {
      if (read_mil(mil_addr, &data, 0xa6 << 8 | adr) == OKAY) {
        if ((0xffff & data) == 0x2)
          mprintf("found ifk with fg at 0x%x, data: 0x%x\n", adr, 0xffff & data);
          bus->slaves[j].fg_ver = 0xffff & data;
          bus->slaves[j].unique_id = adr;
          bus->slaves[j].slot = i;
          bus->slaves[j].cid_sys = SYS_CSCO;
          bus->slaves[j].cid_group = GRP_IFA8;
          if (read_mil(mil_addr, &data, 0xcc << 8 | adr) == OKAY)
            bus->slaves[j].version = 0xffff & data;
          j++;    
      }
    }
  } 

  bus->slaves[j].unique_id = 0; /* end of list */
  return j; /* return number of slaves found */
}

int scan_for_fgs(struct scu_bus *bus, uint32_t *fglist) {
  int i = 0, j = 0;

  while(bus->slaves[i].unique_id) {
    if (bus->slaves[i].cid_sys == SYS_CSCO || bus->slaves[i].cid_sys == SYS_PBRF || bus->slaves[i].cid_sys == SYS_LOEP) {
      if (bus->slaves[i].cid_group == GRP_ADDAC1 ||
          bus->slaves[i].cid_group == GRP_ADDAC2 ||
          bus->slaves[i].cid_group == GRP_DIOB) {
        /* two FGs */
        if (j < MAX_FG_MACROS) {
          fglist[j] = 16;                             //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;  //version
          fglist[j] |= 0x0 << 16;                     //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;   //slot
          j++;                                        //next macro
        }
        if (j < MAX_FG_MACROS) {
          fglist[j] = 16;                             //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;  //version
          fglist[j] |= 0x1 << 16;                     //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;   //slot
          j++;                                        //next macro
        }
       
      /* ACU/MFU */
      } else if (bus->slaves[i].cid_group == GRP_MFU) {
        /* two FGs */
        if (j < MAX_FG_MACROS) {
          fglist[j] = 20;                             //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;  //version
          fglist[j] |= 0x0 << 16;                     //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;   //slot
          j++;                                        //next macro
        }
        if (j < MAX_FG_MACROS) {
          fglist[j] = 20;                             //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;  //version
          fglist[j] |= 0x1 << 16;                     //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;   //slot
          j++;                                        //next macro
        }
      
      /* FIB */ 
      } else if (bus->slaves[i].cid_group == GRP_FIB_DDS) {
        /* one FG */
        if (j < MAX_FG_MACROS) {
          fglist[j] = 32;                             //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;  //version
          fglist[j] |= 0x0 << 16;                     //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;   //slot
          j++;                                        //next macro
        }
      /* IFA8 */
      } else if (bus->slaves[i].cid_group == GRP_IFA8) {
        if (j < MAX_FG_MACROS) {
          fglist[j] = 16;                                 //output bits
          fglist[j] |= (bus->slaves[i].fg_ver) << 8;      //version
          fglist[j] |= (bus->slaves[i].unique_id) << 16;  //device: macro number inside of the slave card
          fglist[j] |= (bus->slaves[i].slot) << 24;       //slot
          j++;                                            //next macro
        }
      }
    }
    i++; // next scu bus slot
  }

  fglist[j] = 0;  //set next macro to 0 to mark end of list
  return j; //return number of found fgs
}


/* init the buffers for MAX_FG_CHANNELS */
void init_buffers(struct channel_regs *cr, int channel, uint32_t *fg_macros,  volatile unsigned short* scub_base) {
  uint32_t slot;
  uint32_t device;
  uint32_t macro;
  if(channel >= 0 && channel < MAX_FG_CHANNELS) {
    cr[channel].wr_ptr = 0;
    cr[channel].rd_ptr = 0;
    cr[channel].state = 0;
    //reset hardware
    if (cr[channel].macro_number >= 0) {    //there is a macro assigned to that channel
      macro = cr[channel].macro_number;
      slot = fg_macros[macro] >> 24;
      device = (fg_macros[macro] >> 16) & 0xff;
      //mprintf("reset fg %d in slot %d\n", device, slot);
      if (device == 0) {
        scub_base[(slot << 16) + FG1_BASE + FG_CNTRL] = 0x1; // reset fg
      } else if (device == 1) {
        scub_base[(slot << 16) + FG2_BASE + FG_CNTRL] = 0x1; // reset fg
      }
    }
  }
}

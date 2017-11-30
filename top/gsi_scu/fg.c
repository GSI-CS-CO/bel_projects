////////////////////////////////////////////////////////////////////////////////
//
// filename: fg.c
// desc: helper functions scanning for fgs
// creation date:
// last modified: 30.11.2017
// author: Stefan Rauch
//
// Copyright (C) 2017 GSI Helmholtz Centre for Heavy Ion Research GmbH
//
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////////
#include <fg.h>
#include <scu_bus.h>
#include <string.h>
#include <unistd.h>
#include <mprintf.h>
#include <mini_sdb.h>
#include <scu_mil.h>



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

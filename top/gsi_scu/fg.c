/*!
 *  @file fg.c
 *  @brief SCU-Function generator module for LM32.
 *
 *  @date 10.07.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch perhaps...
 *  @revision Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#if !defined(__lm32__) && !defined(__DOXYGEN__) && !defined(__DOCFSM__)
  #error This module is for the target LM32 only!
#endif

#include <fg.h>
#include <scu_bus.h>
#include <mprintf.h>
#include <scu_mil.h>
#include <mini_sdb.h>

#define OFFS(SLOT) ((SLOT) * (1 << 16))

int add_to_fglist(int slot, int dev, int cid_sys, int cid_group, int fg_ver, uint32_t *fglist)
{
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

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-Bus for functions generators which are
 *        connected via SCU-bus-to-MIL-adapter
 */
static inline
void scanScuBusFgsViaMil( volatile uint16_t *scub_adr, uint32_t *fglist )
{
   SCUBUS_SLAVE_FLAGS_T slotFlags;

   slotFlags  = scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO2 );
   slotFlags |= scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO3 );
   if( slotFlags == 0 )
      return;

   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( slotFlags, slot ) )
         continue;
      scub_reset_mil( scub_adr, slot );
      for( uint16_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
      {
         int16_t ifa_id, ifa_vers, fg_vers;
         if( scub_read_mil(scub_adr, slot, &ifa_id, IFA_ID << 8 | ifa_adr) != OKAY )
            continue;
         if( scub_read_mil(scub_adr, slot, &ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY )
            continue;
         if( scub_read_mil(scub_adr, slot, &fg_vers, 0xa6 << 8 | ifa_adr) != OKAY )
            continue;

         if( ((0xffff & fg_vers) >= 0x2) &&
             ((0xffff & ifa_id) == 0xfa00) &&
             ((0xffff & ifa_vers) >= 0x1900) )
         {
            add_to_fglist( DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, 0xffff & fg_vers, fglist);
           // mprintf( "M-slot: %d\n", slot );
          //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
         }
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus direct to the SCU-bus connected
 *        function generators
 */
static inline
void scanScuBusFgsDirect( volatile uint16_t *scub_adr, uint32_t *fglist )
{
   SCUBUS_SLAVE_FLAGS_T slotFlags;
   slotFlags  = scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, 38 );
   if( slotFlags == 0 )
      return;
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( slotFlags, slot ) )
         continue;
      add_to_fglist( slot, 0, SYS_CSCO, 38,
                     scub_adr[OFFS(slot) + FG1_BASE + FG_VER], fglist );
     // mprintf( "S-slot: %d, group = %d\n", slot, 38 );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus for all kinda of function generators.
 */
static inline
void scanScuBusFgs( volatile uint16_t *scub_adr, uint32_t *fglist )
{

#if 1
   scanScuBusFgsDirect( scub_adr, fglist );
   scanScuBusFgsViaMil( scub_adr, fglist );
#else
   int slot;
   int cid_group;
   int cid_sys;
   int fg_ver;
   uint16_t ifa_adr;
   uint16_t ext_clk_reg;
   int16_t ifa_id, ifa_vers, fg_vers;
   for( slot = 1; slot <= MAX_SCU_SLAVES; slot++ )
   {
      scub_adr[OFFS(slot) + 0x10] = 0; //clear echo reg
      if( scub_adr[OFFS(slot) + 0x10] == SCUBUS_INVALID_VALUE )
         continue;
      cid_group     = scub_adr[OFFS(slot) + CID_GROUP];
      cid_sys       = scub_adr[OFFS(slot) + CID_SYS];
//      slave_version = scub_adr[OFFS(i) + SLAVE_VERSION];
      fg_ver        = scub_adr[OFFS(slot) + FG1_BASE + FG_VER];

      ext_clk_reg = scub_adr[OFFS(slot) + SLAVE_EXT_CLK];          //read clk status from slave
      if(ext_clk_reg & 0x1)
         scub_adr[OFFS(slot) + SLAVE_EXT_CLK] = 0x1;                //switch clk to sys clk from scu bus

     if( cid_sys != SYS_CSCO )
        continue;

     // if slave is a sio3, scan for ifa cards
      if( cid_group == GRP_SIO3 || cid_group == GRP_SIO2 )
      {
        // reset all taskslots by reading value back
         scub_reset_mil(scub_adr, slot);
         for( ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
         {
            if( scub_read_mil(scub_adr, slot, &ifa_id, IFA_ID << 8 | ifa_adr) != OKAY)     continue;
            if( scub_read_mil(scub_adr, slot, &ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY) continue;
            if( scub_read_mil(scub_adr, slot, &fg_vers, 0xa6 << 8 | ifa_adr) != OKAY)      continue;

            if (((0xffff & fg_vers) >= 0x2) && ((0xffff & ifa_id) == 0xfa00) && ((0xffff & ifa_vers) >= 0x1900))
            {
               add_to_fglist(DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, 0xffff & fg_vers, fglist);
               mprintf( "M-slot: %d\n", slot );
             //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
            }
         }
         continue;
      }
      mprintf( "S-slot: %d, group = %d\n", slot, cid_group );
      add_to_fglist( slot, ifa_adr, cid_sys, cid_group, fg_ver, fglist);
   }
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the MIL extension for function generators
 */
static inline
void scanExtMilFgs( volatile unsigned int *mil_addr, uint32_t *fglist, uint64_t *ext_id )
{
   int16_t ifa_id, ifa_vers, fg_vers;
   uint16_t ifa_adr;
   /* check only for ifks, if there is a macro found and a mil extension attached to the baseboard */
   /* mil extension is recognized by a valid 1wire id                                              */
   /* mil extension has a 1wire temp sensor with family if 0x42                                    */

   if( !(((int)mil_addr != ERROR_NOT_FOUND) && (((int)*ext_id & 0xff) == 0x42)) )
      return;

    // reset all taskslots by reading value back
   reset_mil( mil_addr );

   for( ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
   {
      if( read_mil( mil_addr, &ifa_id, IFA_ID << 8 | ifa_adr) != OKAY )
         continue;
      if( read_mil( mil_addr, &ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY )
         continue;
      if( read_mil(mil_addr, &fg_vers, 0xa6 << 8 | ifa_adr) != OKAY )
         continue;
      if( ((0xffff & fg_vers) >= 0x2) && ((0xffff & ifa_id) == 0xfa00) &&
         ((0xffff & ifa_vers) >= 0x1900) )
      {
         add_to_fglist( DEV_MIL_EXT, ifa_adr, SYS_CSCO, GRP_IFA8, 0xffff & fg_vers, fglist);
        //write_mil(mil_addr, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
      }
   }
}

/*! ---------------------------------------------------------------------------
 * @brief scans the whole SCU for connected function generators.
 */
void scan_all_fgs( volatile uint16_t *scub_adr,
                   volatile unsigned int *mil_addr,
                   uint32_t *fglist, uint64_t *ext_id )
{
   scanScuBusFgs( scub_adr, fglist );
   scanExtMilFgs( mil_addr, fglist, ext_id );
}

/*! ---------------------------------------------------------------------------
 * @brief  init the buffers for MAX_FG_CHANNELS
 */
void init_buffers(struct channel_regs *cr, int channel, uint32_t *fg_macros,
                  volatile unsigned short* scub_base, volatile unsigned int* devb_base)
{
  uint32_t slot;
  uint32_t dev;
  uint32_t macro;
  if( channel >= 0 && channel < MAX_FG_CHANNELS) {
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
      if ((slot & (DEV_MIL_EXT | DEV_SIO)) == 0) {
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

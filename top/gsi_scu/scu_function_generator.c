/*!
 *  @file scu_function_generator.c
 *  @brief SCU-Function generator module for LM32.
 *
 *  @date 21.10.2019
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

#include <scu_function_generator.h>
#include <scu_bus.h>
#include <mprintf.h>
#include <scu_mil.h>
#include <mini_sdb.h>

#define OFFS(SLOT) ((SLOT) * (1 << 16))

/*
 * Maybe a bug in the obsolete DOXYGEN 1.8.5 in the ASL-cluster,
 * otherwise the local functions of this module will not
 * documented by DOXYGEN. :-/
 */
#ifdef __DOXYGEN__
  #define STATIC
#else
  #define STATIC static
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC void fgInitMacro( FG_MACRO_T* pMacro,
                         const uint8_t outputBits,
                         const uint8_t version,
                         const uint8_t device, /* mil extension */
                         const uint8_t socket )
{
   pMacro->outputBits  = outputBits;
   pMacro->version     = version;
   pMacro->device      = device;
   pMacro->socket      = socket;
}

/*! ---------------------------------------------------------------------------
 */
STATIC int add_to_fglist( const uint8_t socked, const uint8_t dev,
                          const uint16_t cid_sys, const uint16_t cid_group,
                          const uint8_t fg_ver,
                          FG_MACRO_T* fglist )
{
   int count = 0;

   /* find first free slot */
   while( (fglist[count].outputBits != 0) && (count < MAX_FG_MACROS) )
      count++;

   if( !(cid_sys == SYS_CSCO || cid_sys == SYS_PBRF || cid_sys == SYS_LOEP) )
      return count;

   switch( cid_group )
   {
      case GRP_ADDAC1: /* directly to next case */
      case GRP_ADDAC2: /* directly to next case */
      case GRP_DIOB:
      {  /* two FG */
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 16, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 16, fg_ver, 1, socked );
         /* ACU/MFU */
         break;
      }
      case GRP_MFU: /* two FGs */
      {
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 20, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 20, fg_ver, 1, socked );
         /* FIB */
         break;
      }
      case GRP_FIB_DDS: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 32, fg_ver, 0, socked );
         /* IFA8 */
         break;
      }
      case GRP_IFA8: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgInitMacro( &fglist[count++], 16, fg_ver, dev, socked );
         break;
      }
   }
   return count; //return number of found fgs
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-Bus for functions generators which are
 *        connected via SCU-bus-to-MIL-adapter
 */
STATIC inline
void scanScuBusFgsViaMil( volatile uint16_t *scub_adr, FG_MACRO_T* fglist )
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
STATIC inline
void scanScuBusFgsDirect( volatile uint16_t *scub_adr, FG_MACRO_T* fglist )
{
   const  SCUBUS_SLAVE_FLAGS_T slotFlags = scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, 38 );
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
STATIC inline
void scanScuBusFgs( volatile uint16_t *scub_adr, FG_MACRO_T* fglist )
{
   scanScuBusFgsDirect( scub_adr, fglist );
   scanScuBusFgsViaMil( scub_adr, fglist );
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the MIL extension for function generators
 */
STATIC inline
void scanExtMilFgs( volatile unsigned int *mil_addr,
                    FG_MACRO_T* fglist, uint64_t *ext_id )
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
                   FG_MACRO_T* fglist, uint64_t *ext_id )
{
   for( unsigned int i = 0; i < MAX_FG_MACROS; i++ )
   {
      fglist[i].socket     = 0;
      fglist[i].device     = 0;
      fglist[i].version    = 0;
      fglist[i].outputBits = 0;
   }

   scanScuBusFgs( scub_adr, fglist );
   scanExtMilFgs( mil_addr, fglist, ext_id );
}

/*! ---------------------------------------------------------------------------
 * @brief  init the buffers for MAX_FG_CHANNELS
 */
void init_buffers( FG_CHANNEL_REG_T* cr, const unsigned int channel,
                   FG_MACRO_T* fg_macros,
                   volatile uint16_t* scub_base,
                   volatile unsigned int* devb_base )
{
   if( channel > MAX_FG_CHANNELS )
      return;

   cr[channel].wr_ptr = 0;
   cr[channel].rd_ptr = 0;
   cr[channel].state = 0;
   cr[channel].ramp_count = 0;

      // Is a macro assigned to that channel?
   const int32_t macro = cr[channel].macro_number;
   if( macro < 0 )
      return; // No


   const uint8_t socket = fg_macros[macro].socket;
   const uint8_t dev    = fg_macros[macro].device;

   //reset hardware
   reset_mil( devb_base );
   scub_reset_mil(scub_base, socket);

   //mprintf("reset fg %d in socked %d\n", device, socked);
   /* scub slave */
   if( (socket & (DEV_MIL_EXT | DEV_SIO)) == 0 )
   {
      if( dev == 0 )
      {
         scub_base[OFFS(socket) + FG1_BASE + FG_CNTRL] = 0x1; // reset fg
         return;
      }
      if( dev == 1 )
      {
         scub_base[OFFS(socket) + FG2_BASE + FG_CNTRL] = 0x1; // reset fg
      }
      return;
   }

   /* mil extension */
   if (socket & DEV_MIL_EXT)
   {
      write_mil(devb_base, 0x1, FC_CNTRL_WR | dev); // reset fg
      return;
   }

   if (socket & DEV_SIO)
   {
      scub_write_mil(scub_base, socket & 0xf, 0x1, FC_CNTRL_WR | dev); // reset fg
   }
}

/*================================== EOF ====================================*/


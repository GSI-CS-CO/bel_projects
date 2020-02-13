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
#include <scu_main.h>
#include <mprintf.h>
#ifdef CONFIG_MIL_FG
#include <scu_mil.h>
#endif
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
 * @brief Prints all found function generators.
 */
void printFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_macros ); i++ )
   {
      FG_MACRO_T* pFgMacro = &g_shared.fg_macros[i];

      /*
       * Is the end of list been reached?
       */
      if( pFgMacro->outputBits == 0 )
         break;

      mprintf( "fg-%d-%d\tver: %d output-bits: %d\n",
               pFgMacro->socket,
               pFgMacro->device,
               pFgMacro->version,
               pFgMacro->outputBits
             );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Print the values and states of all channel registers.
 */
void print_regs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.fg_regs ); i++ )
   {
      FG_CHANNEL_REG_T* pFgReg = &g_shared.fg_regs[i];
      mprintf("Registers of channel %d:\n", i );
      mprintf("\twr_ptr:\t%d\n",       pFgReg->wr_ptr );
      mprintf("\trd_ptr:\t%d\n",       pFgReg->rd_ptr );
      mprintf("\tmbx_slot:\t0x%x\n",   pFgReg->mbx_slot );
      mprintf("\tmacro_number:\t%d\n", pFgReg->macro_number );
      mprintf("\tramp_count:\t%d\n",   pFgReg->ramp_count );
      mprintf("\ttag:\t%d\n",          pFgReg->tag );
      mprintf("\tstate:\t%d\n\n",      pFgReg->state );
   }
}

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

   /*
    * Climbing to the first free list item.
    */
   while( (count < MAX_FG_MACROS) && (fglist[count].outputBits != 0) )
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

#ifdef CONFIG_MIL_FG

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

   /*
    * At least one MIL-function-generator connected via SCU-bus was found.
    */
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( slotFlags, slot ) )
         continue;
      /*
       * MIL-bus adapter was in the current slot found.
       * Proofing whether MIL function generators connected to this adapter.
       */
      scub_reset_mil( scub_adr, slot );
      for( uint32_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
      {
         uint16_t ifa_id, ifa_vers, fg_vers;
         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_id ) );
         if( scub_read_mil(scub_adr, slot, (short*)&ifa_id, IFA_ID << 8 | ifa_adr) != OKAY )
            continue;
         if( ifa_id != 0xfa00 )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_vers ) );
         if( scub_read_mil(scub_adr, slot, (short*)&ifa_vers, IFA_VERS << 8 | ifa_adr) != OKAY )
            continue;
         if( ifa_vers < 0x1900 )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( fg_vers ) );
         if( scub_read_mil(scub_adr, slot, (short*)&fg_vers, 0xa6 << 8 | ifa_adr) != OKAY )
            continue;
         if( (fg_vers < 2) || (fg_vers > 0x00FF) )
            continue;

         /*
          * All three proves has been passed, so we can add it to the FG-list.
          */
         add_to_fglist( DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, fg_vers, fglist);
         //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
      }
   }
}
#endif // ifdef CONFIG_MIL_FG

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
#ifdef CONFIG_MIL_FG
   scanScuBusFgsViaMil( scub_adr, fglist );
#endif
}

#ifdef CONFIG_MIL_FG
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

#endif /* CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_function_generator.h
 */
void scan_all_fgs( volatile uint16_t *scub_adr,
               #ifdef CONFIG_MIL_FG
                   volatile unsigned int *mil_addr,
               #endif
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
#ifdef CONFIG_MIL_FG
   scanExtMilFgs( mil_addr, fglist, ext_id );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see scu_function_generator.h
 */
void init_buffers( FG_CHANNEL_REG_T* cr, const unsigned int channel,
                   FG_MACRO_T* fg_macros,
                   volatile uint16_t* scub_base
              #ifdef CONFIG_MIL_FG
                   , volatile unsigned int* devb_base
              #endif
                 )
{
   if( channel > MAX_FG_CHANNELS )
      return;

   cr[channel].wr_ptr = 0;
   cr[channel].rd_ptr = 0;
   cr[channel].state = 0;
   cr[channel].ramp_count = 0;

   /*
    *  Is a macro assigned to that channel by SAFTLIB?
    *  FunctionGeneratorImpl::acquireChannel
    */
   const int32_t macro = cr[channel].macro_number;
   if( macro < 0 )
      return; /* No */


   const uint8_t socket = fg_macros[macro].socket;
   const uint8_t dev    = fg_macros[macro].device;

   //reset hardware
#ifdef CONFIG_MIL_FG
   reset_mil( devb_base );
   scub_reset_mil( scub_base, socket );
#endif
   //mprintf("reset fg %d in socked %d\n", device, socked);
   /* scub slave */
   if( isNonMilFg( socket ) )
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
#ifdef CONFIG_MIL_FG
   /* mil extension */
   if( isMilExtentionFg( socket ) )
   {
      write_mil( devb_base, 0x1, FC_CNTRL_WR | dev ); // reset fg
      return;
   }

   if( isMilScuBusFg( socket ) )
   {
      scub_write_mil( scub_base, getFgSlotNumber( socket ),
                      0x1, FC_CNTRL_WR | dev); // reset fg
   }
#endif
}

/*================================== EOF ====================================*/


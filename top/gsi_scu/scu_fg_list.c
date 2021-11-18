/*!
 * @file scu_fg_list.c
 * @brief Module for scanning the SCU for function generators and initializing
 *        the function generator list in the shared memory.
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 21.10.2019
 * Renamed from scu_function_generator.c 21.10.2019
 */
#if !defined(__lm32__) && !defined(__DOXYGEN__) && !defined(__DOCFSM__)
  #error This module is for the target LM32 only!
#endif

#include <scu_fg_list.h>
#include <scu_main.h>
#include <eb_console_helper.h>
#ifdef CONFIG_MIL_FG
 #include <scu_mil.h>
#endif
#include <mini_sdb.h>
#include "scu_fg_macros.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <daq_main.h>
#endif

#define IFA_ID_VAL         0xfa00
#define IFA_MIN_VERSION    0x1900
#define FG_MIN_VERSION     2

/*! ---------------------------------------------------------------------------
 * @brief Prints all found function generators.
 */
void printFgs( void )
{
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.oSaftLib.oFg.aMacros ); i++ )
   {
      FG_MACRO_T* pFgMacro = &g_shared.oSaftLib.oFg.aMacros[i];

      /*
       * Is the end of list been reached?
       */
      if( pFgMacro->outputBits == 0 )
         break;

      mprintf( ESC_FG_CYAN ESC_BOLD
               "fg-%d-%d\tver: %d output-bits: %d\n" ESC_NORMAL,
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
   for( unsigned int i = 0; i < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ); i++ )
   {
      FG_CHANNEL_REG_T* pFgReg = &g_shared.oSaftLib.oFg.aRegs[i];
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
 * @see scu_fg_list.h
 */
void fgListReset( FG_MACRO_T* pFgList )
{
   for( unsigned int i = 0; i < MAX_FG_MACROS; i++ )
   {
      pFgList[i].socket     = 0;
      pFgList[i].device     = 0;
      pFgList[i].version    = 0;
      pFgList[i].outputBits = 0;
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void fgListInitItem( FG_MACRO_T* pMacro,
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
STATIC int fgListAdd( const uint8_t socked,
                      const uint8_t dev,
                      const uint16_t cid_sys,
                      const uint16_t cid_group,
                      const uint8_t fg_ver,
                      FG_MACRO_T* pFgList )
{
   int count = 0;

   /*
    * Climbing to the first free list item.
    */
   while( (count < MAX_FG_MACROS) && (pFgList[count].outputBits != 0) )
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
            fgListInitItem( &pFgList[count++], 16, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 16, fg_ver, 1, socked );
         /* ACU/MFU */
         break;
      }
      case GRP_MFU: /* two FGs */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 20, fg_ver, 0, socked );
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 20, fg_ver, 1, socked );
         /* FIB */
         break;
      }
      case GRP_FIB_DDS: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 32, fg_ver, 0, socked );
         /* IFA8 */
         break;
      }
      case GRP_IFA8: /* one FG */
      {
         if( count < MAX_FG_MACROS )
            fgListInitItem( &pFgList[count++], 16, fg_ver, dev, socked );
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
ONE_TIME_CALL
void scanScuBusFgsViaMil( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags =
               scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO2 )
             | scuBusFindSpecificSlaves( (void*)scub_adr, SYS_CSCO, GRP_SIO3 );

   if( slotFlags == 0 )
      return;

   /*
    * At least one MIL-function-generator connected via SCU-bus was found.
    */
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( slotFlags, slot ) )
         continue;

   #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
      scuBusEnableSlaveInterrupt( (void*)scub_adr, slot );
   #endif

      /*
       * MIL-bus adapter was in the current slot found.
       * Proofing whether MIL function generators connected to this adapter.
       */
      
      /*
       * Resetting all task-slots of this SCU-bis slave.
       */
      scub_reset_mil( scub_adr, slot );

      for( uint32_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
      {
         uint16_t ifa_id, ifa_vers, fg_vers;
         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_id ) );
         if( scub_read_mil( scub_adr, slot, (short*)&ifa_id, IFA_ID << 8 | ifa_adr ) != OKAY )
            continue;
         if( ifa_id != IFA_ID_VAL )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( ifa_vers ) );
         if( scub_read_mil( scub_adr, slot, (short*)&ifa_vers, IFA_VERS << 8 | ifa_adr ) != OKAY )
            continue;
         if( ifa_vers < IFA_MIN_VERSION )
            continue;

         STATIC_ASSERT( sizeof( short ) == sizeof( fg_vers ) );
         if( scub_read_mil( scub_adr, slot, (short*)&fg_vers, 0xa6 << 8 | ifa_adr ) != OKAY )
            continue;
         if( (fg_vers < FG_MIN_VERSION) || (fg_vers > 0x00FF) )
            continue;

         /*
          * All three proves has been passed, so we can add it to the FG-list.
          */
         fgListAdd( DEV_SIO | slot, ifa_adr, SYS_CSCO, GRP_IFA8, fg_vers, pFgList );
         //scub_write_mil(scub_adr, slot, 0x100, 0x12 << 8 | ifa_adr); // clear PUR
      }
   }
}
#endif // ifdef CONFIG_MIL_FG

/*! ---------------------------------------------------------------------------
 * @brief Adds a found function generator to the function generator list.
 */
#ifndef CONFIG_SCU_DAQ_INTEGRATION
STATIC inline
#endif
void addAddacToFgList( const void* pScuBusBase,
                       const unsigned int slot,
                       FG_MACRO_T* pFGlist )
{
   FG_ASSERT( pFGlist != NULL );
   fgListAdd( slot,
              0,
              SYS_CSCO,
              GRP_ADDAC2,
              getFgFirmwareVersion( pScuBusBase, slot ),
              pFGlist );
}

#ifndef CONFIG_SCU_DAQ_INTEGRATION
/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus direct to the SCU-bus connected
 *        function generators
 * @param pScuBusBase Base address of SCU bus
 * @param pFGlist Start pointer of function generator list.
 */
ONE_TIME_CALL
void scanScuBusFgsDirect( const void* pScuBusBase, FG_MACRO_T* pFGlist )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags = scuBusFindSpecificSlaves( pScuBusBase,
                                                                    SYS_CSCO,
                                                                    GRP_ADDAC2 )
                                        | scuBusFindSpecificSlaves( pScuBusBase,
                                                                    SYS_CSCO,
                                                                    GRP_ADDAC1 );

   if( slotFlags == 0 )
      return;

   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( scuBusIsSlavePresent( slotFlags, slot ) )
      {
         addAddacToFgList( pScuBusBase, slot, pFGlist );
      #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
         scuBusEnableSlaveInterrupt( pScuBusBase, slot );
      #endif
      }
   }

}
#endif /* ifndef CONFIG_SCU_DAQ_INTEGRATION */

#ifdef CONFIG_NON_DAQ_FG_SUPPORT
/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU bus for specific slaves having a
 *        function generator and add it to the function generator list if
 *        any found.
 * @param pScuBusBase Base address of SCU bus
 * @param pFgList Start pointer of function generator list.
 * @param systemAddr System address
 * @param groupAddr  Group address
 */
STATIC void scanScuBusForFg( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList,
                             SLAVE_SYSTEM_T systemAddr, SLAVE_GROUP_T groupAddr )
{
   const SCUBUS_SLAVE_FLAGS_T slotFlags = scuBusFindSpecificSlaves( (void*)scub_adr,
                                                                    systemAddr,
                                                                    groupAddr );
   if( slotFlags == 0 )
      return;

   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( scuBusIsSlavePresent( slotFlags, slot ) )
      {
         fgListAdd( slot,
                    0,
                    systemAddr,
                    groupAddr,
                    getFgFirmwareVersion( (void*)scub_adr, slot ),
                    pFgList );
      #ifndef _CONFIG_IRQ_ENABLE_IN_START_FG
         scuBusEnableSlaveInterrupt( (void*)scub_adr, slot );
      #endif
      }      
   }
}

#define CONFIG_DIOB_WITHOUT_DAQ

/*! ---------------------------------------------------------------------------
 * @brief Scans the SCU- bus for function generators which doesn't have DAQs.
 * @param pScuBusBase Base address of SCU bus
 * @param pFgList Start pointer of function generator list.
 */
ONE_TIME_CALL
void scanScuBusFgsWithoutDaq( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList )
{
   scanScuBusForFg( scub_adr, pFgList, SYS_PBRF, GRP_FIB_DDS );
 #ifdef CONFIG_DIOB_WITHOUT_DAQ
   scanScuBusForFg( scub_adr, pFgList, SYS_CSCO, GRP_DIOB );
 #endif
}
#endif /* ifdef CONFIG_NON_DAQ_FG_SUPPORT */

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU-bus for all kinda of function generators.
 */
ONE_TIME_CALL
void scanScuBusFgs( volatile uint16_t *scub_adr, FG_MACRO_T* pFgList )
{
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   scuDaqInitialize( &g_scuDaqAdmin, pFgList );
#else
   scanScuBusFgsDirect( (void*)scub_adr, pFgList );
#endif
#if defined( CONFIG_MIL_FG ) || defined( CONFIG_NON_DAQ_FG_SUPPORT )
 #ifdef CONFIG_SCU_DAQ_INTEGRATION
   if( daqBusIsAcuDeviceOnly( &g_scuDaqAdmin.oDaqDevs ) )
   { /*
      * When a ACU device has been recognized, it's not allow to made
      * further scans on the SCU bus!
      */
      return;
   }
 #endif
 #ifdef CONFIG_NON_DAQ_FG_SUPPORT
   scanScuBusFgsWithoutDaq( scub_adr, pFgList );
 #endif
 #ifdef CONFIG_MIL_FG
   scanScuBusFgsViaMil( scub_adr, pFgList );
 #endif
#endif
}

#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 * @brief Scans the MIL extension for function generators
 */
ONE_TIME_CALL
void scanExtMilFgs( volatile unsigned int *mil_addr,
                    FG_MACRO_T* pFgList, uint64_t *ext_id )
{
  /*
   * Check only for "ifks", if there is a macro found and a mil extension
   * attached to the baseboard.
   * + mil extension is recognized by a valid 1wire id
   * + mil extension has a 1wire temp sensor with family if 0x42
   */
   if( !(((int)mil_addr != ERROR_NOT_FOUND) && (((int)*ext_id & 0xff) == 0x42)) )
      return;

   /*
    * reset all task-slots by reading value back
    */
   reset_mil( mil_addr );

   /*
    * Probing of all potential MIL-function-generatirs.
    */
   for( uint32_t ifa_adr = 0; ifa_adr < IFK_MAX_ADR; ifa_adr++ )
   {
      uint16_t ifa_id, ifa_vers, fg_vers;

      STATIC_ASSERT( sizeof( short ) == sizeof( ifa_id ) );
      if( read_mil( mil_addr, (short*)&ifa_id, IFA_ID << 8 | ifa_adr ) != OKAY )
         continue;
      if( ifa_id != IFA_ID_VAL )
         continue;

      STATIC_ASSERT( sizeof( short ) == sizeof( ifa_vers ) );
      if( read_mil( mil_addr, (short*)&ifa_vers, IFA_VERS << 8 | ifa_adr ) != OKAY )
         continue;
      if( ifa_vers < IFA_MIN_VERSION )
         continue;

      STATIC_ASSERT( sizeof( short ) == sizeof( fg_vers ) );
      if( read_mil( mil_addr, (short*)&fg_vers, 0xa6 << 8 | ifa_adr ) != OKAY )
         continue;
      if( (fg_vers < FG_MIN_VERSION) || (fg_vers > 0x00FF) )
         continue;

      /*
       * All three proves has been passed, so we can add it to the FG-list.
       */
      fgListAdd( DEV_MIL_EXT, ifa_adr, SYS_CSCO, GRP_IFA8, fg_vers, pFgList );
   }
}

#endif /* CONFIG_MIL_FG */

/*! ---------------------------------------------------------------------------
 * @see scu_function_generator.h
 */
void fgListFindAll( volatile uint16_t *scub_adr,
               #ifdef CONFIG_MIL_FG
                   volatile unsigned int *mil_addr,
               #endif
                   FG_MACRO_T* pFgList, uint64_t *ext_id )
{
   fgListReset( pFgList );
   scanScuBusFgs( scub_adr, pFgList );
#ifdef CONFIG_MIL_FG
   scanExtMilFgs( mil_addr, pFgList, ext_id );
#endif
}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_list.h
 */
void fgResetAndInit( FG_CHANNEL_REG_T* pChannelRegisters,
                     const unsigned int channel,
                     FG_MACRO_T* pFgList,
                     const void* pScuBus
                   #ifdef CONFIG_MIL_FG
                   , const void* pMilBus
                   #endif
                   )
{
   if( channel > MAX_FG_CHANNELS )
      return;

   pChannelRegisters[channel].wr_ptr = 0;
   pChannelRegisters[channel].rd_ptr = 0;
   pChannelRegisters[channel].state = STATE_STOPPED;
   pChannelRegisters[channel].ramp_count = 0;

   const int32_t macroNumber = pChannelRegisters[channel].macro_number;

   /*
    *  Is a macro assigned to that channel by SAFTLIB?
    *  FunctionGeneratorImpl::acquireChannel
    */
   if( macroNumber < 0 )
      return; /* No */


   const unsigned int socket = pFgList[macroNumber].socket;
   const unsigned int dev    = pFgList[macroNumber].device;

#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
      getFgRegisterPtr( pScuBus, socket, dev )->cntrl_reg.i16 = FG_RESET;
#ifdef CONFIG_MIL_FG
      return;
   }

   if( isMilScuBusFg( socket ) )
   {
      const unsigned int slot = getFgSlotNumber( socket );
      scub_reset_mil( (unsigned short*)pScuBus, slot );
      scub_write_mil( (unsigned short*)pScuBus, slot, FG_RESET, FC_CNTRL_WR | dev );
      return;
   }

   /* MIL- extension */
   FG_ASSERT( isMilExtentionFg( socket ) );
   reset_mil( (unsigned int*)pMilBus );
   write_mil( (unsigned int*)pMilBus, FG_RESET, FC_CNTRL_WR | dev );
#endif
}

/*================================== EOF ====================================*/


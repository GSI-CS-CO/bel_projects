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
#include <scu_fg_handler.h>
#include <scu_main.h>
#include <eb_console_helper.h>
#include <scu_fg_handler.h>
#ifdef CONFIG_MIL_FG
 #include <scu_mil.h>
 #include "scu_mil_fg_handler.h"
#endif
#include <mini_sdb.h>
#include "scu_fg_macros.h"
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #include <daq_main.h>
#endif

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

      scuLog( LM32_LOG_INFO, ESC_FG_CYAN ESC_BOLD
              "fg-%d-%d\tver: %d output-bits: %d\n" ESC_NORMAL,
              pFgMacro->socket,
              pFgMacro->device,
              pFgMacro->version,
              pFgMacro->outputBits );
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
int fgListAdd( const uint8_t socked,
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

/*! ---------------------------------------------------------------------------
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

#ifdef CONFIG_DIOB_WITH_DAQ
/*! ---------------------------------------------------------------------------
 */
void addDiobToFgList( const void* pScuBusBase,
                      const unsigned int slot,
                      FG_MACRO_T* pFGlist )
{
   FG_ASSERT( pFGlist != NULL );
   fgListAdd( slot,
              0,
              SYS_CSCO,
              GRP_DIOB,
              getFgFirmwareVersion( pScuBusBase, slot ),
              pFGlist );
}
#endif

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


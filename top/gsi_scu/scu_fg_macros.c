/*!
 * @file scu_fg_macros.c
 * @brief Module for handling MIL and non MIL
 *        function generator macros
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/ScuFgDoc
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 04.02.2020
 * Outsourced from scu_main.c
 */
#include "scu_fg_macros.h"
#include "scu_fg_handler.h"
#include "scu_fg_list.h"
#ifdef CONFIG_MIL_FG
   #include "scu_mil_fg_handler.h"
#endif

#define CONFIG_DISABLE_FEEDBACK_IN_DISABLE_IRQ

extern volatile uint16_t*     g_pScub_base;
extern volatile uint32_t*     g_pScub_irq_base;

#ifdef CONFIG_MIL_FG
extern volatile unsigned int* g_pScu_mil_base;
extern volatile uint32_t*     g_pMil_irq_base;

STATIC_ASSERT( sizeof( *g_pScu_mil_base ) == sizeof( uint32_t ) );

typedef enum
{
   MIL_INL = 0x00,
   MIL_DRY = 0x01,
   MIL_DRQ = 0x02
} MIL_T;
#endif /* ifdef CONFIG_MIL_FG */

/*!
 * @brief Memory space of sent function generator data.
 *        Non shared memory part for each function generator channel.
 */
FG_CHANNEL_T g_aFgChannels[MAX_FG_CHANNELS] =
{
   {
    #ifdef CONFIG_USE_FG_MSI_TIMEOUT
      .timeout      = 0L,
    #endif
    #ifdef CONFIG_USE_SENT_COUNTER
      .param_sent   = 0,
    #endif
      .last_c_coeff = 0
   }
};

STATIC_ASSERT( ARRAY_SIZE(g_aFgChannels) == MAX_FG_CHANNELS );

#ifdef CONFIG_USE_FG_MSI_TIMEOUT
/*! ---------------------------------------------------------------------------
 */
void wdtPoll( void )
{
   const uint64_t currentTime = getWrSysTimeSafe();
   for( unsigned int i = 0; i < ARRAY_SIZE( g_aFgChannels ); i++ )
   {
      criticalSectionEnter();
      volatile const uint64_t timeout = g_aFgChannels[i].timeout;
      criticalSectionExit();

      if( timeout == 0LL )
         continue;
      if( currentTime <= timeout )
         continue;

      criticalSectionEnter();
      g_aFgChannels[i].timeout = 0LL;
      criticalSectionExit();

   #ifdef CONFIG_USE_LM32LOG
      lm32Log( LM32_LOG_ERROR, ESC_ERROR
               "ERROR: MSI-timeout on fg-%u-%u channel: %u !\n" ESC_NORMAL,
               getSocket(i), getDevice(i), i );
   #else
      mprintf( ESC_ERROR
               "ERROR: MSI-timeout on fg-%u-%u channel: %u !\n" ESC_NORMAL,
               getSocket(i), getDevice(i), i );
   #endif
   }
}
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgEnableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_aFgChannels ) );


   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %u ): fg-%u-%u\n" ESC_NORMAL,
            __func__, channel, socket, dev );


#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtReset( channel );
#endif

   FG_REGISTER_T* pAddagFgRegs = NULL;

#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
      STATIC_ASSERT( sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) == sizeof( uint32_t ) );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_low ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
      STATIC_ASSERT( sizeof( pAddagFgRegs->tag_high ) == sizeof( g_shared.oSaftLib.oFg.aRegs[0].tag ) / 2 );
      /*
       * Note: In the case of ADDAC/ACU-FGs the socket-number is equal
       *       to the slot number.
       */
      pAddagFgRegs = addacFgPrepare( (void*)g_pScub_base,
                                     socket, dev,
                                     g_shared.oSaftLib.oFg.aRegs[channel].tag );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      if( milHandleClearHandlerState( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket ) )
         return;

      milFgPrepare( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket, dev );
   }
#endif


   FG_PARAM_SET_T pset;
  /*
   * Fetch first parameter set from buffer
   */
   if( cbReadSave( &g_shared.oSaftLib.oFg.aChannelBuffers[0], &g_shared.oSaftLib.oFg.aRegs[0], channel, &pset ) != 0 )
   {
   #ifdef CONFIG_MIL_FG
      if( pAddagFgRegs != NULL )
      {
   #endif
         addacFgStart( pAddagFgRegs, &pset, channel );
   #ifdef CONFIG_MIL_FG
      }
      else
      {
         milFgStart( (void*)g_pScub_base,
                     (void*)g_pScu_mil_base,
                      &pset,
                      socket, dev, channel );
      }
   #endif /* CONFIG_MIL_FG */
   #ifdef CONFIG_USE_SENT_COUNTER
      g_aFgChannels[channel].param_sent++;
   #endif
   } /* if( cbRead( ... ) != 0 ) */


   sendSignalArmed( channel );


}

/*! ---------------------------------------------------------------------------
 * @see scu_fg_macros.h
 */
void fgDisableChannel( const unsigned int channel )
{
   FG_ASSERT( channel < ARRAY_SIZE( g_shared.oSaftLib.oFg.aRegs ) );

#ifdef CONFIG_USE_FG_MSI_TIMEOUT
   wdtDisable( channel );
#endif

   FG_CHANNEL_REG_T* pFgRegs = &g_shared.oSaftLib.oFg.aRegs[channel];

   if( pFgRegs->macro_number == SCU_INVALID_VALUE )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   lm32Log( LM32_LOG_DEBUG, ESC_DEBUG "%s( %u ): fg-%u-%u\n" ESC_NORMAL,
            __func__, channel, socket, dev );

#ifdef CONFIG_MIL_FG
   int status;
   if( isAddacFg( socket ) )
   {
#endif
      /*
       * Note: In the case if ADDAC/ACU- function generator the slot number
       *       is equal to the socket number.
       */
      addacFgDisable( (void*)g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
   }
   else
   {
      status = milFgDisable( (void*)g_pScub_base,
                             (void*)g_pScu_mil_base,
                             socket, dev );
      if( status != OKAY )
         return;

   }
#endif /* CONFIG_MIL_FG */

   if( pFgRegs->state == STATE_ACTIVE )
   {    // hw is running
      hist_addx( HISTORY_XYZ_MODULE, "flush circular buffer", channel );
      lm32Log( LM32_LOG_DEBUG, ESC_DEBUG
               "Flush circular buffer of fg-%u-%u channel: %u\n" ESC_NORMAL,
               socket, dev, channel );
      pFgRegs->rd_ptr = pFgRegs->wr_ptr;
   }
   else
   {
      pFgRegs->state = STATE_STOPPED;
      sendSignal( IRQ_DAT_DISARMED, channel );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief enables MSI generation for the specified channel.
 *
 * Messages from the SCU bus are send to the MSI queue of this CPU with the
 * offset 0x0. \n
 * Messages from the MIL extension are send to the MSI queue of this CPU with
 * the offset 0x20. \n
 * A hardware macro is used, which generates MSIs from legacy interrupts.
 *
 * @todo Replace this awful naked index-numbers by well documented
 *       and meaningful constants!
 *
 * @param channel number of the channel between 0 and MAX_FG_CHANNELS-1
 * @see fgDisableInterrupt
 */
void scuBusEnableMeassageSignaledInterrupts( const unsigned int channel )
{
   const unsigned int socket = getSocket( channel );
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) || isMilScuBusFg( socket ) )
   {
#endif
      FG_ASSERT( getFgSlotNumber( socket ) >= SCUBUS_START_SLOT );
      const uint16_t slot = getFgSlotNumber( socket ) - SCUBUS_START_SLOT;
      ATOMIC_SECTION()
      {
         g_pScub_base[GLOBAL_IRQ_ENA] = 0x20;
         g_pScub_irq_base[MSI_CHANNEL_SELECT] = slot;
         g_pScub_irq_base[MSI_SOCKET_NUMBER]  = slot;
         g_pScub_irq_base[MSI_DEST_ADDR]      = (uint32_t)&((MSI_LIST_T*)pMyMsi)[0];
         g_pScub_irq_base[MSI_ENABLE]         = (1 << slot);
      }
#ifdef CONFIG_MIL_FG
      return;
   }

   if( !isMilExtentionFg( socket ) )
      return;

   ATOMIC_SECTION()
   {
      g_pMil_irq_base[MSI_CHANNEL_SELECT] = MIL_DRQ;
      g_pMil_irq_base[MSI_SOCKET_NUMBER]  = MIL_DRQ;
      //g_pMil_irq_base[MSI_DEST_ADDR]      = (uint32_t)pMyMsi + 0x20;
      g_pMil_irq_base[MSI_DEST_ADDR]      = (uint32_t)&((MSI_LIST_T*)pMyMsi)[2];
      g_pMil_irq_base[MSI_ENABLE]         = (1 << MIL_DRQ);
   }
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief disables the generation of irqs for the specified channel
 *  SIO and MIL extension stop generating irqs
 *  @param channel number of the channel from 0 to MAX_FG_CHANNELS-1
 * @see scuBusEnableMeassageSignaledInterrupts
 */
void fgDisableInterrupt( const unsigned int channel )
{
   if( channel >= MAX_FG_CHANNELS )
      return;

   const unsigned int socket = getSocket( channel );
   const unsigned int dev    = getDevice( channel );

   //mprintf("IRQs for slave %d disabled.\n", socket);
#ifdef CONFIG_MIL_FG
   if( isAddacFg( socket ) )
   {
#endif
     /*
      * In the case of ADDAC/ACU-FGs the socket-number is equal to the
      * slot number, so it's not necessary to extract the slot number here.
      */
      addacFgDisableIrq( (void*)g_pScub_base, socket, dev );
#ifdef CONFIG_MIL_FG
      return;
   }

   milFgDisableIrq( (void*)g_pScub_base, (void*)g_pScu_mil_base, socket, dev );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Send signal REFILL to the SAFTLIB when the fifo level has
 *        the threshold reached. Helper function of function handleMacros().
 * @see handleMacros
 * @param channel Channel of concerning function generator.
 */
void sendRefillSignalIfThreshold( const unsigned int channel )
{
   if( cbgetCountSave( &g_shared.oSaftLib.oFg.aRegs[0], channel ) == FG_REFILL_THRESHOLD )
   {
     // mprintf( "*" ); //!!
      sendSignal( IRQ_DAT_REFILL, channel );
   }
}

/*================================== EOF ====================================*/

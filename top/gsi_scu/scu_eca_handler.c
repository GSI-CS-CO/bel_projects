/*!
 * @file scu_eca_handler.c
 * @brief Handler of Event Conditioned Action for SCU function-generators
 * @date 31.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * Outsourced from scu_main.c
 */

#include "scu_main.h"
#include "scu_fg_list.h"
#include "scu_eca_handler.h"

extern volatile unsigned int* g_pScu_mil_base;
extern volatile uint16_t*     g_pScub_base;

#define MIL_BROADCAST 0x20ff //TODO Who the fuck is 0x20ff documented!

/*!
 * @brief Global object for ECA (event condition action) handler.
 * @see initEcaQueue
 * @see ecaHandler
 */
ECA_OBJ_T g_eca =
{
  .tag    = 0xDEADBEEF, /*!<@brief just define a tag for ECA actions we want to receive */
  .pQueue = NULL
#ifdef _CONFIG_ECA_BY_MSI
  , .pControl = NULL
#endif
};

/*! ---------------------------------------------------------------------------
 * @brief Find the ECA queue of LM32
 */
void initEcaQueue( void )
{
#ifdef _CONFIG_ECA_BY_MSI
   #warning ECA by MSI not ready yet!
   g_eca.pControl = ecaControlGetRegisters();
   if( g_eca.pControl == NULL )
   {
      mprintf( ESC_ERROR "\nERROR: Can't find ECA control register!"
                         " system stopped!"ESC_NORMAL"\n" );
      while( true )
         NOP();
   }
 //TODO
 //  ecaControlSetMsiLM32TargetAddress( pEcaCtl, (void*)pMyMsi, true );
 //  ecaControlGetAndResetChannelValidCount( pEcaCtl );
 //  ECA_CONTROL_ACCESS( pEcaCtl, channelNumberSelect );
#endif
   g_eca.pQueue = ecaGetLM32Queue();
   if( g_eca.pQueue == NULL )
   {
      mprintf( ESC_ERROR "\nERROR: Can't find ECA queue for LM32,"
                        " system stopped!"ESC_NORMAL"\n" );
      while( true )
         NOP();
   }
   //!@todo Check this story with ECA-tag...
   //g_eca.tag = g_eca.pQueue->tag;
   mprintf( ESC_FG_MAGENTA
            "ECA queue found at: 0x%p.\n"
            "\tWaiting for actions with tag 0x%08X...\n"
            ESC_NORMAL,
            g_eca.pQueue, g_eca.tag );
}

#define OFFS(SLOT) ((SLOT) * (1 << 16))

#ifdef _CONFIG_ECA_BY_MSI
  #define ECA_ATOMIC_SECTION() 
#else
  #define ECA_ATOMIC_SECTION() ATOMIC_SECTION()
#endif
/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Event Condition Action (ECA) handler
 * @see schedule
 */
inline void ecaHandler
                    #ifdef _CONFIG_ECA_BY_MSI
                      ( void )
                    #else
                      ( register TASK_T* pThis FG_UNUSED )
                    #endif
{
#ifndef _CONFIG_ECA_BY_MSI
   FG_ASSERT( pThis->pTaskData == NULL );
#endif
   FG_ASSERT( g_eca.pQueue != NULL );

   if( !ecaTestTagAndPop( g_eca.pQueue, g_eca.tag ) )
      return;

#ifdef DEBUG_SAFTLIB
   mprintf( "* " );
#endif
   bool                 isMilDevArmed = false;
   SCUBUS_SLAVE_FLAGS_T active_sios   = 0; /* bitmap with active sios */

   /*
    * Check if there are armed SCI SIO MIL or extention MIL
    * function generator(s).
    */
   for( unsigned int channel = 0; channel < ARRAY_SIZE(g_shared.fg_regs); channel++ )
   {
      if( g_shared.fg_regs[channel].state != STATE_ARMED )
      { /*
         * This function generator is not armed, skip and
         * go to the next function generator channel.
         */
         continue;
      }

      /*
       * Armed function generator found...
       */
      const unsigned int socket = getSocket( channel );
      if( isMilExtentionFg( socket ) )
      {
         isMilDevArmed = true;
         continue;
      }

      const unsigned int slot = getFgSlotNumber( socket );
      if( (slot != 0) && isMilScuBusFg( socket ) )
         active_sios |= scuBusGetSlaveFlag( slot );
   }

   if( isMilDevArmed )
      g_pScu_mil_base[MIL_SIO3_TX_CMD] = MIL_BROADCAST;

   /*!
    * Send broadcast start to active SIO SCI-BUS-slaves
    * @todo Remove this indexed SCU-bus access by encapsulated
    *       hardware access!
    */
   if( active_sios != 0 ) ECA_ATOMIC_SECTION()
   {  /*
       * Select active SIO slaves.
       */
      g_pScub_base[OFFS(0) + MULTI_SLAVE_SEL] = active_sios;
      /*
       * Send broadcast.
       */
      g_pScub_base[OFFS(13) + MIL_SIO3_TX_CMD] = MIL_BROADCAST;
   }
}

/* ================================= EOF ====================================*/

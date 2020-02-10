/*!
 * @file scu_eca_handler.c
 * @brief Handler of Event Conditioned Action for SCU function-generators
 * @date 31.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * Outsourced from scu_main.c
 */

#include "scu_main.h"
#include "scu_eca_handler.h"

#ifdef CONFIG_MIL_FG
   extern volatile unsigned int* g_pScu_mil_base;
#endif
extern volatile uint16_t*     g_pScub_base;

#define MIL_BROADCAST 0x20ff //TODO Who the fuck is 0x20ff documented!

/*!
 * @brief Global object for ECA (event condition action) handler.
 * @see initEcaQueue
 * @see ecaHandler
 */
ECA_OBJ_T g_eca = { 0, NULL };

/*! ---------------------------------------------------------------------------
 * @brief Find the ECA queue of LM32
 */
void initEcaQueue( void )
{
   g_eca.pQueue = ecaGetLM32Queue();
   if( g_eca.pQueue == NULL )
   {
      mprintf( ESC_ERROR"\nERROR: Can't find ECA queue for LM32,"
                        " system stopped!"ESC_NORMAL"\n" );
      while( true )
         asm volatile ("nop");
   }
   g_eca.tag = g_eca.pQueue->tag;
   mprintf("\nECA queue found at: 0x%08x."
           " Waiting for actions with tag 0x%08x\n\n",
            (unsigned int)g_eca.pQueue, g_eca.tag );
}

/*! ---------------------------------------------------------------------------
 * @ingroup TASK
 * @brief Event Condition Action (ECA) handler
 * @see schedule
 */
void ecaHandler( register TASK_T* pThis FG_UNUSED )
{
   FG_ASSERT( pThis->pTaskData == NULL );
   FG_ASSERT( g_eca.pQueue != NULL );

   if( !ecaTestTagAndPop( g_eca.pQueue, g_eca.tag ) )
      return;

#ifdef DEBUG_SAFTLIB
   mprintf( "* " );
#endif
#ifdef CONFIG_MIL_FG
   bool isMilDevArmed = false;
#endif
   SCUBUS_SLAVE_FLAGS_T active_sios   = 0; /* bitmap with active sios */

   /* check if there are armed fgs */
   for( unsigned int i = 0; i < ARRAY_SIZE(g_shared.fg_regs); i++ )
   { // only armed fgs
      if( g_shared.fg_regs[i].state != STATE_ARMED )
         continue;

      const uint8_t socket = getSocket( i );
   #ifdef CONFIG_MIL_FG
      if( isMilExtentionFg( socket ) )
      {
         isMilDevArmed = true;
         continue;
      }
   #endif
      if( isMilScuBusFg( socket ) && (getFgSlotNumber( socket ) != 0) )
         active_sios |= (1 << (getFgSlotNumber( socket ) - 1));
   }

#ifdef CONFIG_MIL_FG
   if( isMilDevArmed )
      g_pScu_mil_base[MIL_SIO3_TX_CMD] = MIL_BROADCAST;
#endif

   // send broadcast start to active sio slaves
   if( active_sios != 0 )
   {  // select active sio slaves
      g_pScub_base[OFFS(0) + MULTI_SLAVE_SEL] = active_sios;
      // send broadcast
   #ifdef CONFIG_MIL_FG
      g_pScub_base[OFFS(13) + MIL_SIO3_TX_CMD] = MIL_BROADCAST;
   #endif
   }
}

/* ================================= EOF ====================================*/

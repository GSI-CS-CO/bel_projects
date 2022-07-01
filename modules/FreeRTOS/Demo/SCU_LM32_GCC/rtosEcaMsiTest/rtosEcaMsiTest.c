/*!
 * @file rtosEcaMsiTest.c
 * @brief Test program for ECA -Interrupts associated with FreeRTOS
 *
 * @see https://www-acc.gsi.de/wiki/Timing/TimingSystemHowSoftCPUHandleECAMSIs
 * @copyright 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @date 09.04.2020
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * This example demonstrates handling of message-signaled interrupts (MSI)
 * caused by ECA channel.\n
 * ECA is capable to send MSIs on certain conditions such as producing actions
 * on reception of timing messages.\n
 *
 * Chose SCU: export SCU_URL=scuxl4711
 * build:     make \n
 * load:      make load \n
 * debug:     eb-console tcp/$SCU_URL
 *
 *  To run example:\n
 *  - set ECA rules for eCPU action channel
 * @code
 *  saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 * @endcode
 *  - debug firmware output
 * @code
 *  eb-console dev/wbm0
 * @endcode
 *  - inject timing message (invoke on the second terminal)
 * @code
 *  saft-ctl tr0 inject 0x1122334455667788 0x8877887766556642 0
 * @endcode
 *
 */
#include "eb_console_helper.h"
#include "lm32signal.h"
#include "scu_msi.h"
#include "eca_queue_type.h"
#include "scu_lm32Timer.h"
#include "FreeRTOS.h"
#include "queue.h"

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

/*!
 * @brief ECA actions tagged for this LM32 CPU
 *
 * @code
 * saft-ecpu-ctl tr0 -d -c 0x1122334455667788 0xFFFFFFFFFFFFFFFF 0 0x42
 *                                                                 ====
 * @endcode
 */
#define MY_ACT_TAG 0x42

/*!
 * @brief WB address of ECA control register set.
 */
ECA_CONTROL_T* g_pEcaCtl;

/*!
 * @brief WB address of ECA queue
 */
ECA_QUEUE_ITEM_T* g_pEcaQueue;

#if 0
/*! ---------------------------------------------------------------------------
 * @brief Reorders the interrupt priority.
 *
 * This gives the possibility to reorder the interrupt priority if really
 * necessary.\n
 * In this example its just for test purposes.
 *
 * @note When this function is implemented, then the default function
 *       implemented in module lm32interrupts.c will
 *       overwritten by this function.
 */
unsigned int _irqReorderPriority( const unsigned int prio )
{
   /*
    * Here the the interrupt priorities of the timer interrupt and the
    * ECA-Interrupt becomes exchanged.
    */
   switch( prio )
   {
      case ECA_INTERRUPT_NUMBER: return TIMER_IRQ;
      case TIMER_IRQ:            return ECA_INTERRUPT_NUMBER;
   }
   return prio;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by LM32 when an exception appeared.
 */
void _onException( const uint32_t sig )
{
   ATOMIC_SECTION()
   {
      char* str;
      #define _CASE_SIGNAL( S ) case S: str = #S; break;
      switch( sig )
      {
         _CASE_SIGNAL( SIGINT )
         _CASE_SIGNAL( SIGTRAP )
         _CASE_SIGNAL( SIGFPE )
         _CASE_SIGNAL( SIGSEGV )
         default: str = "unknown"; break;
      }
      mprintf( ESC_ERROR "%s( %d ): %s\n" ESC_NORMAL, __func__, sig, str );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Clear pending valid actions for LM32
 */
STATIC inline void clearActions( void )
{
   uint32_t valCnt = ecaControlGetAndResetLM32ValidCount( g_pEcaCtl );
   if( valCnt != 0 )
   {
      mprintf( "pending actions: %d\n", valCnt );
      valCnt = ecaClearQueue( g_pEcaQueue, valCnt ); // pop pending actions
      mprintf( "cleared actions: %d\n", valCnt );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Configure ECA to send MSI to embedded soft-core LM32.
 *
 * - ECA action channel for LM32 is selected and MSI target address of LM32 is
 *   set in the ECA MSI target register
 *
 */
STATIC inline void configureEcaMsiForLM32( void )
{
   clearActions();   // clean ECA queue and channel from pending actions
   ecaControlSetMsiLM32TargetAddress( g_pEcaCtl, (void*)pMyMsi, true );
   mprintf( "MSI path (ECA -> LM32)           : enabled\n"
           "\tECA channel = %d\n\tdestination = 0x%08X\n",
            ECA_SELECT_LM32_CHANNEL, (uint32_t)pMyMsi);
}

/*! ---------------------------------------------------------------------------
 * @brief Interrupt function: Handle MSIs sent by ECA
 *
 * If interrupt was caused by a valid action, then MSI has value of (4<<16|num).\n
 * Both ECA action channel and ECA queue connected to that channel must be handled:\n
 * - read and clear the valid counter value of ECA action channel for LM32 and,\n
 * - pop pending actions from ECA queue connected to this action channel
 */
STATIC void onIrqEcaEvent( const unsigned int intNum,
                           const void* pContext )
{
   MSI_ITEM_T m;

   //irqMsiCopyObjectAndRemove( &m, intNum );
   while( irqMsiCopyObjectAndRemoveIfActive( &m, intNum ) )
      xQueueSendToFrontFromISR( (QueueHandle_t) pContext, &m, NULL );
}

/*! ---------------------------------------------------------------------------
 * @brief Pop pending embedded CPU actions from an ECA queue and handle them
 */
STATIC inline void ecaHandler( void )
{
   static unsigned int s_count = 0;

   const unsigned int pending = ecaControlGetAndResetLM32ValidCount( g_pEcaCtl );
   mprintf( "valid:\t%d\n", pending );
   for( unsigned int i = 0; i < pending; i++ )
   {
      if( !ecaIsValid( g_pEcaQueue ) )
         continue;

      const ECA_QUEUE_ITEM_T ecaItem = *g_pEcaQueue;

      ecaPop( g_pEcaQueue );

      /*
       * here: do something according to action
       */
      if( ecaItem.tag != MY_ACT_TAG )
      {
         mprintf( "%s: unknown tag: %d\n", __func__, ecaItem.tag );
         continue;
      }

      mprintf( ESC_FG_YELLOW ESC_BOLD
               "%s: id: 0x%08x%08x\n"
               "deadline:       0x%08X%08X\n"
               "param:          0x%08X%08X\n"
               "flag:           0b%08b\n"
               ESC_FG_BLUE
               "count:          %u\n" ESC_NORMAL,
               __func__,
               ecaItem.eventIdH,  ecaItem.eventIdL,
               ecaItem.deadlineH, ecaItem.deadlineL,
               ecaItem.paramH,    ecaItem.paramL,
               ecaItem.flags,
               ++s_count
             );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief The task main function!
 */
STATIC void vTaskEcaMain( void* pvParameters UNUSED )
{
   mprintf( ESC_BOLD ESC_FG_CYAN "Function \"%s()\" of task \"%s\" started\n"
            ESC_NORMAL,
            __func__, pcTaskGetName( NULL ) );

   if( pEca == NULL )
   {
      mprintf( ESC_ERROR "Could not find the ECA event input. Exit!\n" ESC_NORMAL);
      vTaskEndScheduler();
   }
   mprintf("ECA event input                  @ 0x%p\n", pEca );
   mprintf("MSI destination addr for LM32    : 0x%p\n", pMyMsi );

   g_pEcaCtl = ecaControlGetRegisters();
   if( g_pEcaCtl == NULL )
   {
      mprintf( ESC_ERROR "Could not find the ECA channel control. Exit!\n"
               ESC_NORMAL );
      vTaskEndScheduler();
   }
   mprintf( "ECA channel control              @ 0x%p\n", g_pEcaCtl );

   g_pEcaQueue = ecaGetLM32Queue();
   if( g_pEcaQueue == NULL )
   {
      mprintf( ESC_ERROR "Could not find the ECA queue connected"
                         " to eCPU action channel. Exit!\n" ESC_NORMAL );
      vTaskEndScheduler();
   }
   mprintf( "ECA queue to LM32 action channel @ 0x%08p\n", g_pEcaQueue );

   mprintf( "Creating the MSI OS-message queue\n" );
   QueueHandle_t xMsiQueue = xQueueCreate( 5, sizeof( MSI_ITEM_T ) );
   if( xMsiQueue == NULL )
   {
      mprintf( ESC_ERROR "Could not create OS message queue!\n" ESC_NORMAL );
      vTaskEndScheduler();
   }

   configureEcaMsiForLM32();

   irqRegisterISR( ECA_INTERRUPT_NUMBER, xMsiQueue, onIrqEcaEvent );
   mprintf( "Installing of ECA interrupt is done.\n" );

   unsigned int i = 0;
   const char fan[] = { '|', '/', '-', '\\' };
   mprintf( ESC_BOLD "Entering task main loop and waiting for MSI ...\n" ESC_NORMAL );
   while( true )
   {
      MSI_ITEM_T m;

      /*!
       * Waiting until a message is in the queue.
       */
      if( xQueueReceive( xMsiQueue, &m, pdMS_TO_TICKS( 250 ) ) == pdPASS )
      { /*
         * At least one message has been received...
         */
         mprintf( ESC_FG_MAGENTA
                  "\nMSI:\t0x%08X\n"
                  "Adr:\t0x%08X\n"
                  "Sel:\t0x%02X\n"
                  ESC_NORMAL,
                  m.msg,  m.adr,  m.sel );

         /*!
          * Are valid actions pending?
          */
         if( (m.msg & ECA_VALID_ACTION) != 0 )
         { /*
            * Yes, calling the ecaHandler.
            */
            ecaHandler();
         }
      }
      else
      { /*
         * Receive timeout: Showing a software fan as still alive.
         */
         mprintf( ESC_BOLD "\r%c" ESC_NORMAL, fan[i++] );
         i %= ARRAY_SIZE( fan );
      }
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   static const char* taskName = "ECA-Handler";
   BaseType_t status;
   mprintf( "Creating task \"%s\"\n", taskName );
   status = xTaskCreate( vTaskEcaMain,
                         taskName,
                         configMINIMAL_STACK_SIZE * 2,
                         NULL,
                         tskIDLE_PRIORITY + 1,
                         NULL
                       );
   if( status != pdPASS )
      return status;


   vTaskStartScheduler();

   return pdPASS;
}

/*! ---------------------------------------------------------------------------
 */
void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS ECA-MSI test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n" );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}
/*================================== EOF ====================================*/

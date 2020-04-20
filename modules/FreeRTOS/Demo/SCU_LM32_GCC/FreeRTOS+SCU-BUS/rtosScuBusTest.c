/*!
 * @file rtosScuBusTest.c
 * @brief FreeRtos test program on SCU making accesses ti SCU bus slaves
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      15.04.2020
 */
#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "generated/shared_mmap.h"
#include "scu_lm32_macros.h"
#include "scu_bus.h"
#include "scu_bus_defines.h"
#include "FreeRTOS.h"
#include "task.h"

#define MAX_TEST_SLAVES MAX_SCU_SLAVES

typedef struct
{
   unsigned int slot;
   void*        pAddress;
} SLAVE_T;


/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

/*! ---------------------------------------------------------------------------
 */
STATIC void vTaskScuBusSlave( void* pvParameters )
{
   SLAVE_T* pSlave = (SLAVE_T*) pvParameters;
   uint16_t count = 0;
   while( true )
   {
      count++;
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void vTaskMain( void* pvParameters UNUSED )
{
   mprintf( "Task \"%s\" started\n", __func__ );

   void* pScuBusBase = find_device_adr( GSI, SCU_BUS_MASTER );
   if( pScuBusBase == (void*)ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "Can't find SCU bus master\n" ESC_NORMAL );
      vTaskEndScheduler();
   }

   mprintf( "Scanning SCU bus...\n" );
   const SCUBUS_SLAVE_FLAGS_T slavePersentFlags = scuBusFindAllSlaves( pScuBusBase );
   const unsigned int numberOfSlaves = scuBusGetNumberOfSlaves( slavePersentFlags );
   mprintf( "%d slaves found.\n", numberOfSlaves );
   if( numberOfSlaves < MAX_TEST_SLAVES )
   {
      mprintf( ESC_ERROR "This test needs at least two SCU bus slaves!" ESC_NORMAL);
      vTaskEndScheduler();
   }

   SLAVE_T slaves[MAX_TEST_SLAVES];
   unsigned int m = 0;
   for( unsigned int i = SCUBUS_START_SLOT; i <= MAX_SCU_SLAVES; i++ )
   {
      if( !scuBusIsSlavePresent( slavePersentFlags, i ) )
         continue;

      slaves[m].slot = i;
      slaves[m].pAddress = scuBusGetAbsSlaveAddr( pScuBusBase, i );
      mprintf( "Use slave in slot %u; address: 0x%08x\n",
               slaves[m].slot, slaves[m].pAddress );
      //TODO
      m++;
      if( m >= ARRAY_SIZE( slaves ) )
         break;

   }

   TickType_t xLastExecutionTime = xTaskGetTickCount();
   const unsigned int Y = 10;
   unsigned int secs = 0;
   mprintf( "Enter main loop...\n" );
   while( true )
   {
      //TODO
      
      unsigned int i;
      for( i = 0; i < m; i++ )
      {
         mprintf( ESC_XY( "1", "%d" ) ESC_CLR_LINE
                  "Slot: %02d: ",
                  Y+i, slaves[i].slot );
      }
      mprintf( ESC_XY( "1", "%d" ) ESC_CLR_LINE
               "Seconds: %u", Y+i+1, secs++ );
      /*
       * Task will suspend for 1000 ms.
       */
      vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 1000 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Initializing the main task and start it.
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   BaseType_t status = xTaskCreate( vTaskMain,
                         "Main task",
                         configMINIMAL_STACK_SIZE * 4,
                         NULL,
                         tskIDLE_PRIORITY + 1,
                         NULL
                       );
   if( status != pdPASS )
      return status;

   vTaskStartScheduler();
   portENABLE_INTERRUPTS();

   return pdPASS;
}


void main( void )
{
   init();
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS SCU-BUS test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n" );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

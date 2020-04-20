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

//#define CONFIG_SCU_ATOMIC_SECTION

typedef struct
{
   unsigned int slot;
   void*        pAddress;
   TaskHandle_t xCreatedTask;
   uint16_t     lastCount;
   bool         decerment;
} SLAVE_T;

#ifdef CONFIG_SCU_ATOMIC_SECTION
   #define SCU_ATOMIC_SECTION() ATOMIC_SECTION()
#else
   #define SCU_ATOMIC_SECTION()
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

/*! ---------------------------------------------------------------------------
 * @brief Task function which represents a single SCU-bus slave.
 *
 * It reads the slaves echo register, increment or decrement it and write it
 * back.
 */
STATIC void vTaskScuBusSlave( void* pvParameters )
{
   SLAVE_T* pSlave = (SLAVE_T*) pvParameters;
   volatile  uint16_t count = 0;

   /*
    * The size of the slaves echo register is 16 bit only. Therefore
    * a delay counter is used to throttle the up respectively down counting.
    */
   unsigned int delay = 0;

   while( true )
   {
      SCU_ATOMIC_SECTION()
         count = scuBusGetSlaveValue16( pSlave->pAddress, Echo_Register );

      if( delay == 1000 )
      {
         delay = 0;
         if( pSlave->decerment )
            count--;
         else
            count++;
      }
      else
         delay++;

      SCU_ATOMIC_SECTION()
         scuBusSetSlaveValue16( pSlave->pAddress, Echo_Register, count );
      //portYIELD();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief The main task.
 *
 * Scans the whole SCU-bus end creates a task for each found slave.
 */
STATIC void vTaskMain( void* pvParameters UNUSED )
{
   mprintf( ESC_FG_MAGNETA "Task \"%s\" started\n", __func__ );

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

   SLAVE_T slaves[MAX_TEST_SLAVES];
   unsigned int m = 0;
   for( unsigned int i = SCUBUS_START_SLOT; i <= MAX_SCU_SLAVES; i++ )
   {
      if( !scuBusIsSlavePresent( slavePersentFlags, i ) )
         continue;

      slaves[m].slot = i;
      slaves[m].pAddress = scuBusGetAbsSlaveAddr( pScuBusBase, i );
      slaves[m].decerment = (m % 2) != 0;
      SCU_ATOMIC_SECTION()
         scuBusSetSlaveValue16( slaves[m].pAddress, Echo_Register, 0 );
      int status = xTaskCreate( vTaskScuBusSlave,
                                "SCU-Slave",
                                configMINIMAL_STACK_SIZE,
                                (void*)&slaves[m],
                                tskIDLE_PRIORITY + 1,
                                &slaves[m].xCreatedTask );
      if( status != pdPASS )
      {
         mprintf( ESC_ERROR
                  "Unable to start child task for slot: %d, status: %d\n"
                  ESC_NORMAL,
                   slaves[m].slot, status );
         vTaskEndScheduler();
      }

      mprintf( "Task for slave in slot %u; address: 0x%08x; ID: %u started.\n",
               slaves[m].slot, slaves[m].pAddress, slaves[m].xCreatedTask );
      slaves[m].lastCount = 0;
      m++;
      if( m >= ARRAY_SIZE( slaves ) )
         break;
   }

   TickType_t xLastExecutionTime = xTaskGetTickCount();
   const unsigned int Y = m + 10;
   unsigned int secs = 0;
   mprintf( "Enter main loop...\n" ESC_NORMAL );
   while( true )
   {
      unsigned int i;
      for( i = 0; i < m; i++ )
      {
         uint16_t counter;
         SCU_ATOMIC_SECTION()
            counter = scuBusGetSlaveValue16( slaves[i].pAddress, Echo_Register );

         mprintf( ESC_XY( "1", "%d" ) ESC_CLR_LINE ESC_FG_CYAN ESC_BOLD
                  "Slot: %02d: echo register: %05u, 0x%04x, %s delta: %u"
                  ESC_NORMAL,
                  Y+i, slaves[i].slot,
                  counter, counter,
                  slaves[i].decerment? "dec" : "inc",
                  0xFFFF & (slaves[i].decerment?
                             (slaves[i].lastCount - counter):
                             (counter - slaves[i].lastCount) ) );

         slaves[i].lastCount = counter;
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

   portENABLE_INTERRUPTS();
   vTaskStartScheduler();

   return pdPASS;
}

/*! ---------------------------------------------------------------------------
 * @brief The main function, what else...
 */
void main( void )
{
   init();
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS SCU-BUS test\n"
            "Compiler:  " COMPILER_VERSION_STRING "\n"
            "Tick rate: " TO_STRING( configTICK_RATE_HZ ) " Hz\n"
         #ifdef CONFIG_SCU_ATOMIC_SECTION
            "Using atomic sections.\n"
         #endif
          );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

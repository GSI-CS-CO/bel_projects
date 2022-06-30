/*!
 * @file rtosScuBusTest.c
 * @brief FreeRtos test program on SCU making accesses to SCU bus slaves
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      15.04.2020
 *
 * This test-program scans the SCU-bus and creates a task for each found
 * SCU-bus slave.
 * Each created task increments or decrements its echo register in a alternating
 * manner depending of the slave position in the scu-bus slot.
 */
#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "scu_lm32_macros.h"
#include "scu_bus.h"

#include "FreeRTOS.h"
#include "task.h"

#define MAX_TEST_SLAVES MAX_SCU_SLAVES

 #define CONFIG_SCU_ATOMIC_SECTION

#ifdef CONFIG_SCU_ATOMIC_SECTION
   #define SCU_ATOMIC_SECTION() ATOMIC_SECTION()
#else
   #define SCU_ATOMIC_SECTION()
#endif

/*! ---------------------------------------------------------------------------
 * @brief Object type for each found SCU-bus slave.
 */
typedef struct
{
   unsigned int slot;         /*!<@brief Slot number                         */
   void*        pAddress;     /*!<@brief Slave address                       */
   TaskHandle_t xCreatedTask; /*!<@brief Task ID                             */
   uint16_t     lastCount;    /*!<@brief Last counter value                  */
   bool         decerment;    /*!<@brief Decrements or increments the counter*/
} SLAVE_T;

/*! ---------------------------------------------------------------------------
 * @brief Task function which represents a single SCU-bus slave.
 *
 * It reads the slaves echo register, increment or decrement it and write it
 * back.
 */
NO_INLINE STATIC void vTaskScuBusSlave( void* pvParameters )
{
   const SLAVE_T* pSlave = (SLAVE_T*) pvParameters;
   volatile  uint16_t count;

   /*
    * The size of the slaves echo register is 16 bit only. Therefore
    * a delay counter is used to throttle the up respectively down counting.
    */
   unsigned int delay = 0;

   while( true )
   {
      count = 0;
      SCU_ATOMIC_SECTION()
         count = scuBusGetSlaveValue16( pSlave->pAddress, Echo_Register );

      scuBusSetSlaveValue16( pSlave->pAddress, Echo_Register, count ); //!!

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
   mprintf( ESC_FG_BLUE ESC_BOLD "Task \"%s\" started\n%d tasks running\n",
            pcTaskGetName( NULL ), uxTaskGetNumberOfTasks() );

   void* pScuBusBase = find_device_adr( GSI, SCU_BUS_MASTER );
   if( pScuBusBase == (void*)ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "Can't find SCU bus master\n" ESC_NORMAL );
      vTaskEndScheduler();
   }

   mprintf( "Scanning SCU bus...\n" );
   const SCUBUS_SLAVE_FLAGS_T slavePersentFlags = scuBusFindAllSlaves( pScuBusBase );
   if( slavePersentFlags == 0 )
   {
      mprintf( ESC_ERROR
               "No slave(s) found on SCU-bus so this test isn't meaningful!\n"
               ESC_NORMAL );
      vTaskEndScheduler();
   }

   mprintf( "%d slaves found.\n", scuBusGetNumberOfSlaves( slavePersentFlags ) );

   SLAVE_T slaves[MAX_TEST_SLAVES];
   unsigned int dev = 0;
   /*
    * For all slots.
    */
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( slavePersentFlags, slot ) )
      { /*
         * No slave in this slot.
         * Jump to the next slot...
         */
         continue;
      }

      /*
       * In this slot is a slave.
       */
      slaves[dev].slot      = slot;
      slaves[dev].pAddress  = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
      slaves[dev].decerment = (dev % 2) != 0;
      slaves[dev].lastCount = 0;

      SCU_ATOMIC_SECTION()
         scuBusSetSlaveValue16( slaves[dev].pAddress, Echo_Register, 0 );

      /*
       * Creating a task for the current slave.
       */
      int status = xTaskCreate( vTaskScuBusSlave,
                                "SCU-Slave",
                                configMINIMAL_STACK_SIZE,
                                (void*)&slaves[dev],
                                tskIDLE_PRIORITY + 1,
                                &slaves[dev].xCreatedTask );
      if( status != pdPASS )
      {
         mprintf( ESC_ERROR
                  "Unable to start child task for slot: %u, status: %d\n"
                  ESC_NORMAL,
                   slaves[dev].slot, status );
         vTaskEndScheduler();
      }

      mprintf( "Task \"%s\" for slave in slot %2u; address: 0x%p; ID: %u started.\n",
               pcTaskGetName( slaves[dev].xCreatedTask ),
               slaves[dev].slot, slaves[dev].pAddress, slaves[dev].xCreatedTask );

      dev++;
      if( dev >= ARRAY_SIZE( slaves ) )
         break;
   }

   TickType_t xLastExecutionTime = xTaskGetTickCount();
   const unsigned int Y = dev + 12;
   unsigned int secs = 0;
   mprintf( "%u tasks running.\nEnter main loop...\n" ESC_NORMAL,
            uxTaskGetNumberOfTasks() );
   while( true )
   {
      unsigned int i;
      for( i = 0; i < dev; i++ )
      {
         uint16_t counter;
         SCU_ATOMIC_SECTION()
            counter = scuBusGetSlaveValue16( slaves[i].pAddress, Echo_Register );

         mprintf( ESC_XY( "1", "%u" ) ESC_CLR_LINE ESC_FG_CYAN ESC_BOLD
                  "Slot: %2u: echo register: %5u, 0x%04X, %s delta: %u"
                  ESC_NORMAL,
                  Y+i, slaves[i].slot,
                  counter, counter,
                  slaves[i].decerment? "dec" : "inc",
                  0xFFFF & (slaves[i].decerment?
                             (slaves[i].lastCount - counter):
                             (counter - slaves[i].lastCount) ) );

         slaves[i].lastCount = counter;
      }
      mprintf( ESC_XY( "1", "%u" ) ESC_CLR_LINE
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

   return pdPASS;
}
extern const char build_id_rom[];
/*! ---------------------------------------------------------------------------
 * @brief The main function, what else...
 */
void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR ESC_NORMAL "FreeRTOS SCU-BUS test\n"
            "Compiler:  " COMPILER_VERSION_STRING "\n"
            "Tick rate: " TO_STRING( configTICK_RATE_HZ ) " Hz\n"
         #ifdef CONFIG_SCU_ATOMIC_SECTION
            "Using atomic sections.\n"
         #endif
            "IRQ-nesting count: %d\n", irqGetAtomicNestingCount()
          );

   mprintf( "build-id: %p\n", build_id_rom );
   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

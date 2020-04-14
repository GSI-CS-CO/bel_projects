/*!
 * @file rtosDdr3Test.c
 * @brief FreeRtos testprogram on SCU using DDR3 RAM
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      14.04.2020
 */
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "generated/shared_mmap.h"
#include "scu_lm32_macros.h"
#include "scu_ddr3.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

//#define CONFIG_YIELD

#define DDR3_INDEX64_1   0
#define DDR3_INDEX64_2   1

DDR3_T g_oDdr3;


/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

#ifndef CONFIG_YIELD
/*! ---------------------------------------------------------------------------
 * @brief Simulating of any task activities. Prevents that the task main loop
 *        with the atomic section will not be to tight.
 *
 * In other words:\n
 * A task main loop shall not consist a atomic section only,
 * which is the case in practice.\n
 * Otherwise the time gap for enabled interrupts would be to small.
 */
STATIC void doSomething( void )
{
   for( unsigned int i = 0; i < 10000; i++ )
      NOP();
}
#endif


/*! ---------------------------------------------------------------------------
 * @brief Two instances of this task will created by the main-task.
 */
STATIC void vTaskWriteDdr3( void* pvParameters )
{
   const unsigned int ddr3Index = (unsigned int) pvParameters;

   /*
    * To distinguish both tasks, will task 2 initialized with
    * a offset of 1000.
    */
   DDR3_PAYLOAD_T pl =
   {
      .ad32[0] = (ddr3Index == DDR3_INDEX64_1)? 0: 1000,
      .ad32[1] = (ddr3Index == DDR3_INDEX64_1)? 0: 1000
   };

   while( true )
   {
      ATOMIC_SECTION() ddr3write64( &g_oDdr3, ddr3Index, &pl );

      pl.ad32[0]++;
      pl.ad32[1]--;

     /*
      * The loop has to be filled with further activities outside of the
      * atomic section, otherwise the gap for the activated timer interrupt
      * would be to tight.
      * If the atomic section is the only activity in this task,
      * then the vPortYield function must be used.
      */
   #ifdef CONFIG_YIELD
      vPortYield();
   #else
      doSomething();
   #endif
   }
}

/*! ---------------------------------------------------------------------------
 */
STATIC void vTaskMain( void* pvParameters UNUSED )
{
   mprintf( "Task \"%s\" started\n", __func__ );

   if( ddr3init( &g_oDdr3 ) < 0 )
   {
      mprintf( ESC_ERROR "Unable to initialize DDR3 RAM!\n" ESC_NORMAL );
      vTaskEndScheduler();
   }

   BaseType_t status;
   status = xTaskCreate( vTaskWriteDdr3,
                         "Task 1",
                         configMINIMAL_STACK_SIZE,
                         (void*)DDR3_INDEX64_1,
                         tskIDLE_PRIORITY + 1,
                         NULL );
   if( status != pdPASS )
   {
      mprintf( ESC_ERROR "Unable to start child task 1: %d\n" ESC_NORMAL,
               status );
      vTaskEndScheduler();
   }
   mprintf( "Child task 1 started.\n" );

   status = xTaskCreate( vTaskWriteDdr3,
                         "Task 2",
                         configMINIMAL_STACK_SIZE,
                         (void*)DDR3_INDEX64_2,
                         tskIDLE_PRIORITY + 1,
                         NULL );
   if( status != pdPASS )
   {
      mprintf( ESC_ERROR "Unable to start child task 2: %d\n" ESC_NORMAL,
               status );
      vTaskEndScheduler();
   }
   mprintf( "Child task 2 started.\n" );

   uint32_t count = 0;
   TickType_t xLastExecutionTime = xTaskGetTickCount();
   while( true )
   {
      DDR3_PAYLOAD_T d1, d2;
      ATOMIC_SECTION() ddr3read64( &g_oDdr3, &d1, DDR3_INDEX64_1 );
      ATOMIC_SECTION() ddr3read64( &g_oDdr3, &d2, DDR3_INDEX64_2 );

      mprintf( ESC_XY( "1",  "10" ) ESC_CLR_LINE "Task 1: up: %u"
               ESC_XY( "26", "10" ) "down: %u", d1.ad32[0], d1.ad32[1] );
      mprintf( ESC_XY( "1",  "11" ) ESC_CLR_LINE "Task 2: up: %u"
               ESC_XY( "26", "11" ) "down: %u", d2.ad32[0], d2.ad32[1] );
      mprintf( ESC_XY( "1",  "12" ) ESC_CLR_LINE "secs: %u", ++count );
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
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS DDR3 test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n"
            "Task frequency: " TO_STRING( configTICK_RATE_HZ ) " Hz\n"
#ifdef CONFIG_YIELD
            "Yield variant\n"
#endif
          );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

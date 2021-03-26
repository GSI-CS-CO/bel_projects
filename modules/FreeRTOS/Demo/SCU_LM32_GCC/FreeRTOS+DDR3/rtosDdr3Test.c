/*!
 * @file rtosDdr3Test.c
 * @brief FreeRtos testprogram on SCU using DDR3 RAM
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      14.04.2020
 */
#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif
//#define CONFIG_YIELD
//#define CONFIG_MUTEX

#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "generated/shared_mmap.h"
#include "scu_lm32_macros.h"
#include "scu_ddr3.h"
#include "FreeRTOS.h"
#include "task.h"

#ifdef CONFIG_MUTEX
 #include "semphr.h"
#endif

#define DDR3_INDEX64_1   0
#define DDR3_INDEX64_2   1

DDR3_T g_oDdr3;

#ifdef CONFIG_MUTEX
SemaphoreHandle_t g_semaDdr3;
uint32_t g_mutexFail = 0;
#endif


inline void ddr3Lock( void )
{
#ifdef CONFIG_MUTEX
   if( xSemaphoreTake( g_semaDdr3, 10 ) != pdPASS )
   {
      ATOMIC_SECTION() g_mutexFail++;
   }
#else
   //criticalSectionEnter();
#endif
}

inline void ddr3Unlock( void )
{
#ifdef CONFIG_MUTEX
   xSemaphoreGive( g_semaDdr3 );
#else
   //criticalSectionExit();
#endif
}

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
   for( unsigned int i = 0; i < 1000; i++ )
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
      ddr3write64( &g_oDdr3, ddr3Index, &pl );

      /*
       * The 32 bit position of the test counters in the 64 bit payload
       * size of the DDR3 RAM are reversed for each task.
       * That makes the discovery of possible race condition easier.
       */
      if( ddr3Index == DDR3_INDEX64_1 )
      {
         pl.ad32[0]++;
         pl.ad32[1]--;
      }
      else
      {
         pl.ad32[0]--;
         pl.ad32[1]++;
      }

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

#ifdef CONFIG_MUTEX
   g_semaDdr3 = xSemaphoreCreateMutex();
   if( g_semaDdr3 == NULL )
   {
      mprintf( ESC_ERROR "Unable to create mutex for DDR3 RAM!\n" ESC_NORMAL );
      vTaskEndScheduler();
   }
#endif

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

   uint32_t lastCountUp1, lastCountUp2, lastCountDown1, lastCountDown2;
   uint32_t count = 0;
   TickType_t xLastExecutionTime = xTaskGetTickCount();
   DDR3_PAYLOAD_T d1 = { .d64 = 0UL }, d2 = { .d64 = 0UL };
   while( true )
   {
      lastCountUp1   = d1.ad32[0];
      lastCountDown1 = d1.ad32[1];
      lastCountUp2   = d2.ad32[1];
      lastCountDown2 = d2.ad32[0];

      ddr3read64( &g_oDdr3, &d1, DDR3_INDEX64_1 );
      ddr3read64( &g_oDdr3, &d2, DDR3_INDEX64_2 );

      mprintf( ESC_XY( "1",  "10" ) ESC_CLR_LINE "Task 1: up: %u"
               ESC_XY( "26", "10" ) "delta: %u;"
               ESC_XY( "46", "10" ) "down: %u"
               ESC_XY( "66", "10" ) "delta: %u",
               d1.ad32[0],
               d1.ad32[0] - lastCountUp1,
               d1.ad32[1],
               lastCountDown1 - d1.ad32[1]
             );

      mprintf( ESC_XY( "1",  "11" ) ESC_CLR_LINE "Task 2: up: %u"
               ESC_XY( "26", "11" ) "delta: %u;"
               ESC_XY( "46", "11" ) "down: %u"
               ESC_XY( "66", "11" ) "delta: %u",
               d2.ad32[1],
               d2.ad32[1] - lastCountUp2,
               d2.ad32[0],
               lastCountDown2 - d2.ad32[0]
             );
   #ifdef CONFIG_MUTEX
      uint32_t mt;
      ATOMIC_SECTION() mt = g_mutexFail;
      mprintf( ESC_XY( "1",  "12" ) ESC_CLR_LINE "Mutex timeouts: %u", mt );
   #endif
      mprintf( ESC_XY( "1",  "13" ) ESC_CLR_LINE "Seconds: %u", ++count );

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


void main( void )
{
   init();
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS DDR3 test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n"
         #if (configUSE_PREEMPTION == 1)
            "Task frequency: " TO_STRING( configTICK_RATE_HZ ) " Hz\n"
         #endif
         #ifdef CONFIG_YIELD
            "Yield variant\n"
         #endif
         #ifdef CONFIG_MUTEX
            "Using mutex\n"
         #endif
          );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

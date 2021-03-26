/*!
 * @file   up-down_count.c
 * @brief  Test of three tasks of FreeRTOS in LM32 of SCU
 *
 * Task 1: Up-counter
 * Task 2: Down-counter
 * Task 3: Counter monitor
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      24.03.2020
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include "eb_console_helper.h"
#include "mini_sdb.h"
#include "generated/shared_mmap.h"
#include "scu_lm32_macros.h"
#include "shared_memory_helper.h"
#include "FreeRTOS.h"
#include "task.h"

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif


/*! ---------------------------------------------------------------------------
 */
STATIC inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

#define TEST_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )

typedef struct PACKED_SIZE
{
   volatile uint32_t countUp;
   volatile uint32_t countDowm;
   volatile uint32_t countTick;
   volatile uint32_t countIdle;
   volatile uint32_t countMonitor;
} SHARED_DATA_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( SHARED_DATA_T, countUp )    == 0 * sizeof(uint32_t));
STATIC_ASSERT( offsetof( SHARED_DATA_T, countDowm )  == 1 * sizeof(uint32_t));
STATIC_ASSERT( offsetof( SHARED_DATA_T, countTick )  == 2 * sizeof(uint32_t));
STATIC_ASSERT( offsetof( SHARED_DATA_T, countIdle )  == 3 * sizeof(uint32_t));
STATIC_ASSERT( offsetof( SHARED_DATA_T, countMonitor ) == 4 * sizeof(uint32_t));
STATIC_ASSERT( sizeof( SHARED_DATA_T ) == SHARED_SIZE );
#endif

SHARED_DATA_T SHARED g_shared = { 0, 0, 0, 0, 0 };

//#define CONFIG_YIELD

#if (configUSE_TICK_HOOK == 0)
  #error configUSE_TICK_HOOK has to be enabled in this application!
#endif
#if (configUSE_IDLE_HOOK == 0)
  #error configUSE_TICK_HOOK has to be enabled in this application!
#endif

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
 * @brief Callback function becomes invoked by each timer interrupt.
 *
 * @note This Function is for debug purposes only.
 *       In this case the macro configUSE_TICK_HOOK must be 1.
 */
void vApplicationTickHook( void )
{
   /*
    * To put this in a atomic section is only necessary when nested interrupts
    * becomes supported.
    */
   ATOMIC_SECTION() g_shared.countTick++;
}

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by each timer interrupt.
 */
void vApplicationIdleHook( void )
{
   ATOMIC_SECTION() g_shared.countIdle++;
}

/*! ---------------------------------------------------------------------------
 * @brief Count up task function.
 */
STATIC void vTaskCountUp( void* pvParameters UNUSED )
{
   while( true )
   {
      ATOMIC_SECTION() g_shared.countUp++;
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
     // vTaskDelay(pdMS_TO_TICKS(1));
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Count down task function.
 */
STATIC void vTaskCuontDown( void* pvParameters UNUSED )
{
   while( true )
   {
      ATOMIC_SECTION() g_shared.countDowm--;
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
     // vTaskDelay(pdMS_TO_TICKS(1));
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Main function of monitor task.
 *
 * It's not necessary to put the slowly mprintf-function in a atomic section
 * as long as mprintf is only in this single task like here.
 */
STATIC void vTaskMonitor( void* pvParameters UNUSED )
{
   uint32_t countUp,   lastCountUp;
   uint32_t countDown, lastCountDown;
   uint32_t countTick, lastCountTick;
   uint32_t countIdle, lastCountIdle;
   uint32_t jitter = 0;
   bool first = true;
   TickType_t xLastExecutionTime = xTaskGetTickCount();

   while( true )
   {
     /*
      * Keep atomic sections as short as possible.
      */
      ATOMIC_SECTION()
      {
         countUp   = g_shared.countUp;
         countDown = g_shared.countDowm;
         countTick = g_shared.countTick;
         countIdle = g_shared.countIdle;
      }
      if( first )
      {
         first = false;
         lastCountUp   = countUp;
         lastCountDown = countDown;
         lastCountTick = countTick;
         lastCountIdle = countIdle;
      }

      const uint32_t deltaTick = countTick - lastCountTick;
      if( deltaTick != configTICK_RATE_HZ )
         jitter++;

      mprintf( ESC_XY( "1", "15" ) ESC_CLR_LINE
               "Up counter:   %u"
               ESC_XY( "30", "15" ) "Delta: %u",
               countUp, countUp - lastCountUp
             );
      lastCountUp = countUp;

      mprintf( ESC_XY( "1", "16" ) ESC_CLR_LINE
               "Down counter: %u"
               ESC_XY( "30", "16" ) "Delta: %u",
               countDown, lastCountDown - countDown
             );
      lastCountDown = countDown;

      mprintf( ESC_XY( "1", "17" ) ESC_CLR_LINE
               "Tick counter: %u"
               ESC_XY( "30", "17" ) "Delta: %u"
               ESC_XY( "50", "17" ) "Jitter: %u",
               countTick, deltaTick, jitter
             );
      lastCountTick = countTick;

      mprintf( ESC_XY( "1", "18" ) ESC_CLR_LINE
               "Idle counter: %u"
               ESC_XY( "30", "18" ) "Delta: %u",
               countIdle, countIdle - lastCountIdle
             );
      lastCountIdle = countIdle;

      mprintf( ESC_XY( "1", "19" ) ESC_CLR_LINE
               "Monitor:      %u", g_shared.countMonitor++ );
      /*
       * Task will suspend for 1000 ms.
       */
      vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 1000 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Creating of all tasks and start of the scheduler.
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   BaseType_t status;

   mprintf( "Creating task \"Count up\"\n" );
   status = xTaskCreate( vTaskCountUp,
                         "Count up",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         TEST_TASK_PRIORITY,
                         NULL
                       );
   if( status != pdPASS )
      return status;

   mprintf( "Creating task \"Count down\"\n" );
   status = xTaskCreate( vTaskCuontDown,
                         "Count down",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         TEST_TASK_PRIORITY,
                         NULL
                       );
   if( status != pdPASS )
      return status;

   mprintf( "Creating task \"Monitor\"\n" );
   status = xTaskCreate( vTaskMonitor,
                         "Monitor",
                         configMINIMAL_STACK_SIZE * 4,
                         NULL,
                         TEST_TASK_PRIORITY,
                         NULL
                       );
   if( status != pdPASS )
      return status;

   vTaskStartScheduler();

   return status;
}


/*! ---------------------------------------------------------------------------
 */
void main( void )
{
   init();
#if 0
   uint32_t* pCpuRamExternal = shmGetRelatedEtherBoneAddress(SHARED_OFFS);
   if( pCpuRamExternal == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Could not find external WB address of my own RAM !\n" ESC_NORMAL);
      while( true );
   }
#endif
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS count up/down test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n"
          #ifdef CONFIG_YIELD
            "Task yield enabled\n"
          #endif
            "Tick-rate:               %d Hz\n"
            "Minimal stack size:      %d bytes\n"
            "Total heap size:         %d bytes\n"
            "Address of countUp:      0x%08x\n"
            "Address of countDown:    0x%08x\n"
            "Address of countTick:    0x%08x\n"
            "Address of countIdle:    0x%08x\n"
            "Address of countMonitor: 0x%08x\n",
            configTICK_RATE_HZ,
            configMINIMAL_STACK_SIZE * sizeof(StackType_t),
            configTOTAL_HEAP_SIZE,
            (unsigned int) &g_shared.countUp,
            (unsigned int) &g_shared.countDowm,
            (unsigned int) &g_shared.countTick,
            (unsigned int) &g_shared.countIdle,
            (unsigned int) &g_shared.countMonitor
          );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

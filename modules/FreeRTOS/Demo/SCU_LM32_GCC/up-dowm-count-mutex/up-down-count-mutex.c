/*!
 * @file   up-down-count-mutex.c
 * @brief  Test of three tasks of FreeRTOS in LM32 of SCU with mutexes
 *
 * Task 1: Up-counter
 * Task 2: Down-counter
 * Task 3: Counter monitor
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      31.03.2020
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
#include "helper_macros.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

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

volatile uint32_t g_countUp   = 0;
volatile uint32_t g_countDown = 0;

SemaphoreHandle_t g_semaCountUp   = NULL;
SemaphoreHandle_t g_semaCountDown = NULL;

//#define CONFIG_YIELD

/*! ---------------------------------------------------------------------------
 * @brief Count up task function.
 */
STATIC void vTaskCountUp( void* pvParameters UNUSED )
{
   while( true )
   {
      xSemaphoreTake( g_semaCountUp, portMAX_DELAY );
      g_countUp++;
      xSemaphoreGive( g_semaCountUp );
   #ifdef CONFIG_YIELD
      vPortYield();
   #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Count down task function.
 */
STATIC void vTaskCuontDown( void* pvParameters UNUSED )
{
   while( true )
   {
      xSemaphoreTake( g_semaCountDown, portMAX_DELAY );
      g_countDown--;
      xSemaphoreGive( g_semaCountDown );
   #ifdef CONFIG_YIELD
      vPortYield();
   #endif
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
   uint32_t countUp, lastCountUp = 0, countDown, lastCountDown = 0;

   TickType_t xLastExecutionTime = xTaskGetTickCount();

   while( true )
   {
      xSemaphoreTake( g_semaCountUp, portMAX_DELAY );
      countUp   = g_countUp;
      xSemaphoreGive( g_semaCountUp );

      xSemaphoreTake( g_semaCountDown, portMAX_DELAY );
      countDown = g_countDown;
      xSemaphoreGive( g_semaCountDown );

      mprintf( ESC_XY( "1", "12" ) ESC_CLR_LINE
               "Up counter:   %u"
               ESC_XY( "30", "12" ) "Delta: %u",
               countUp, countUp - lastCountUp
             );
      lastCountUp = countUp;

      mprintf( ESC_XY( "1", "13" ) ESC_CLR_LINE
               "Down counter: %u"
               ESC_XY( "30", "13" ) "Delta: %u",
               countDown, lastCountDown - countDown
             );
      lastCountDown = countDown;

      /*
       * Task will suspend for 1000 ms.
       */
      vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 1000 ) );
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Creating of all tasks and mutexes and start of the scheduler.
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   BaseType_t status;

   mprintf( "Creating mutex for \"Count up\"\n" );
   g_semaCountUp = xSemaphoreCreateMutex();
   if( g_semaCountUp == NULL )
      return pdFAIL;

   mprintf( "Creating mutex for \"Count down\"\n" );
   g_semaCountDown = xSemaphoreCreateMutex();
   if( g_semaCountDown == NULL )
      return pdFAIL;


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

   portENABLE_INTERRUPTS();
   vTaskStartScheduler();

   return status;
}


/*! ---------------------------------------------------------------------------
 */
void main( void )
{
   init();
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR "FreeRTOS count up/down test\n"
            "Compiler: " COMPILER_VERSION_STRING "\n"
          #ifdef CONFIG_YIELD
            "Task yield enabled\n"
          #endif
            "Tick-rate:          %d Hz\n"
            "Minimal stack size: %d bytes\n"
            "Total heap size:    %d bytes\n",
            configTICK_RATE_HZ,
            configMINIMAL_STACK_SIZE * sizeof(StackType_t),
            configTOTAL_HEAP_SIZE
          );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

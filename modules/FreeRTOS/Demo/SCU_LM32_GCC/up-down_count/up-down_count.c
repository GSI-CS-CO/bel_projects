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
#include "helper_macros.h"
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

volatile uint32_t g_countUp   = 0;
volatile uint32_t g_countDown = 0;

#define MONITOR_RATE (uint32_t)400000

/*! ---------------------------------------------------------------------------
 * @brief Count up task function.
 */
STATIC void vTaskCountUp( void* pvParameters UNUSED )
{
   while( true )
   {
      ATOMIC_SECTION() g_countUp++;
      //vPortYield();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Count down task function.
 */
STATIC void vTaskCuontDown( void* pvParameters UNUSED )
{
   while( true )
   {
      ATOMIC_SECTION() g_countDown--;
      //vPortYield();
   }
}

/*! ---------------------------------------------------------------------------
 * @brief
 */
STATIC void vTaskMonitor( void* pvParameters UNUSED )
{
   uint32_t countUp, lastCountUp = 0, countDown, lastCountDown = 0;

   while( true )
   {
      ATOMIC_SECTION()
      {
         countUp   = g_countUp;
         countDown = g_countDown;
      }
      if( (countUp - lastCountUp) >= MONITOR_RATE )
      {
         gotoxy( 1, 7 );
         mprintf( "Up counter:   %u", countUp );
         lastCountUp = countUp;
      }
      if( (lastCountDown - countDown) >= MONITOR_RATE )
      {
         gotoxy( 1, 8 );
         mprintf( "Down counter: %u", countDown );
         lastCountDown = countDown;
      }
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
                         configMINIMAL_STACK_SIZE,
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
   clrscr();
   gotoxy( 1, 1 );
   mprintf( "FreeRTOS count up/down test\nCompiler: " COMPILER_VERSION_STRING "\n" );

   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/*================================== EOF ====================================*/

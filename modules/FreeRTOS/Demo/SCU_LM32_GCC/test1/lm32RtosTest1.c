/*!
 * @file   lm32RtosTest1.c
 * @brief  Very simple test program using FreeRTOS in LM32 of SCU
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      15.01.2020
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


static inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

#define HELLO_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )
#define HELLO_DELAY            ( (TickType_t) 1000 / configTICK_RATE_HZ )

static void vHelloTask( void* pvParameters )
{
   /*
    * Initialise xLastExecutionTime so the first call to vTaskDelayUntil() works correctly
    */
   TickType_t xLastExecutionTime = xTaskGetTickCount();

   while( true )
   {
      mprintf( "Hello World! User data: %d\n", *((int*)pvParameters) );
      /*
       * Delay mainCHECK_DELAY milliseconds.
       */
      vTaskDelayUntil( &xLastExecutionTime, HELLO_DELAY );
   }
}

const int userTascData = 4711;

int asmTest( int a )
{
   int ret;
   asm volatile (
      "addi %0, %0, 1\n\t"
      : "=r" (a)
      : "r" (userTascData)
      :
       );
   return a;
}

int main( void )
{
   init();
   mprintf( "freeRTOS-test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   mprintf( "ASM return: %d\n", asmTest(42) );
#if 1
   BaseType_t xReturned = xTaskCreate(
                vHelloTask,               /* Function that implements the task. */
                "Hello",                  /* Text name for the task. */
                configMINIMAL_STACK_SIZE, /* Stack size in words, not bytes. */
                (void*)&userTascData,     /* Parameter passed into the task. */
                HELLO_TASK_PRIORITY,      /* Priority at which the task is created. */
                NULL                      /* Used to pass out the created task's handle. */
              );
   if( xReturned != pdPASS )
   {
      mprintf( "Error %d by creating task!\n", xReturned );
      return 0;
   }

   vTaskStartScheduler();
#endif
   while( true );
   return 0;
}

/*================================== EOF ====================================*/

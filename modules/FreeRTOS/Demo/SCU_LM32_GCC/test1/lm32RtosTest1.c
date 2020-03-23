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

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

static inline void init( void )
{
   discoverPeriphery(); // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();      // init UART, required for printf...
}

#define TEST_TASK_PRIORITY    ( tskIDLE_PRIORITY + 1 )

static void vTask( void* pvParameters )
{
   /*
    * Initialize xLastExecutionTime so the first call to vTaskDelayUntil() works correctly
    */
#ifndef CONFIG_NO_RTOS_TIMER
   TickType_t xLastExecutionTime = xTaskGetTickCount();
#endif

   ATOMIC_SECTION() mprintf( "*** Once! ***\n" );
   unsigned int count = 0;
   while( true )
   {
     // ATOMIC_SECTION()
     // irqDisable();
      portENTER_CRITICAL();
      {
         mprintf( "Task main function, count: %d, user data: \"%s\"\n",
                   ++count,
                  (const char*)pvParameters );
         mprintf( "IRQ: 0x%08x nesting: %d\n", irqGetEnableRegister(), irqGetAtomicNestingCount() );
      }
      portEXIT_CRITICAL();
     // irqEnable();
      //SCU_ASSERT( irqIsEnabled() );
      /*
       * Delay mainCHECK_DELAY milliseconds.
       */
   #ifdef CONFIG_NO_RTOS_TIMER
      /*
       * Very crude delay loop... not fine! I know...
       */
      unsigned int d = configCPU_CLOCK_HZ / 8;
      while( d-- != 0 )
         portNOP();
   #else
      vTaskDelayUntil( &xLastExecutionTime, pdMS_TO_TICKS( 20 ) );
   #endif
   #if configUSE_PREEMPTION == 0
      vPortYield();
      mprintf( "after vPortYield(): \"%s\"\n\n", (const char*)pvParameters );
   #endif
   }
}

const char* userTaskData1 = ESC_FG_CYAN"Donald"ESC_NORMAL;
const char* userTaskData2 = ESC_FG_RED"Dagobert"ESC_NORMAL;


void main( void )
{
   init();
   mprintf( "freeRTOS-test\nCompiler: " COMPILER_VERSION_STRING "\n" );
   BaseType_t xReturned;
#if 1
   xReturned = xTaskCreate(
                vTask,               /* Function that implements the task. */
                "TASK 1",                  /* Text name for the task. */
                configMINIMAL_STACK_SIZE, /* Stack size in words, not bytes. */
                (void*)userTaskData1,     /* Parameter passed into the task. */
                TEST_TASK_PRIORITY,      /* Priority at which the task is created. */
                NULL                      /* Used to pass out the created task's handle. */
              );
   if( xReturned != pdPASS )
   {
      mprintf( ESC_ERROR "Error %d: by creating task 1!\n"ESC_NORMAL, xReturned );
      while( true );
   }
#endif
   xReturned = xTaskCreate(
                vTask,               /* Function that implements the task. */
                "task 2",                  /* Text name for the task. */
                configMINIMAL_STACK_SIZE, /* Stack size in words, not bytes. */
                (void*)userTaskData2,     /* Parameter passed into the task. */
                TEST_TASK_PRIORITY,      /* Priority at which the task is created. */
                NULL                      /* Used to pass out the created task's handle. */
              );
   if( xReturned != pdPASS )
   {
      mprintf( ESC_ERROR "Error %d: by creating task 2!\n"ESC_NORMAL, xReturned );
      while( true );
   }

   portENABLE_INTERRUPTS();
   vTaskStartScheduler();


   mprintf( ESC_ERROR "Error: This point shall never be reached!\n" ESC_NORMAL );
   while( true );
}

/*================================== EOF ====================================*/

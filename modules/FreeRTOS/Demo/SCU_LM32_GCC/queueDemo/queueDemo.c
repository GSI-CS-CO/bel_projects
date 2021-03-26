/*!
 * @file   queueDemo.c
 * @brief  FreeRTOS queue test on LM32 in SCU
 *
 * Sending of message queues from normal tasks and form a interrupt context.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      03.04.2020
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

#include "FreeRTOS.h"
#include "queue.h"

#ifndef CONFIG_RTOS
   #error "This project provides FreeRTOS"
#endif

#if (configUSE_TICK_HOOK == 0)
  #error configUSE_TICK_HOOK has to be enabled in this application!
#endif

#define QUEUE_LENGTH 5
#define ITEM_SIZE    sizeof( char* )

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
/*
 * Structure that will hold the TCB of the task being created.
 */
STATIC StaticTask_t g_xPrintTaskBuffer1;
STATIC StackType_t g_xStackPrintTask1[128];

STATIC StaticTask_t g_xPrintTaskBuffer2;
STATIC StackType_t g_xStackPrintTask2[128];

STATIC StaticTask_t g_xUartGatekeeperTaskBuffer;
STATIC StackType_t g_xStackUartGatekeeperTask[128];

/*
 * The variable used to hold the queue's data structure.
 */
STATIC StaticQueue_t g_xStaticQueue;
/*
 * The array to use as the queue's storage area.  This must be at least
 * uxQueueLength * uxItemSize bytes.
 */
STATIC uint8_t g_ucQueueStorageArea[ QUEUE_LENGTH * ITEM_SIZE ];
#endif

/*!
 * Declare a variable of type QueueHandle_t. The queue is used to send messages
 * from the print tasks and the tick interrupt to the gatekeeper task.
 */
QueueHandle_t g_xPrintQueue;

/*
 * Define the strings that the tasks and interrupt will print out via the
 * gatekeeper.
 */
STATIC char* g_pcStringsToPrint[] =
{
   ESC_BOLD ESC_FG_YELLOW  "Task 1 ****************************************************\n" ESC_NORMAL,
   ESC_BOLD ESC_FG_MAGENTA "Task 2 ----------------------------------------------------\n" ESC_NORMAL,
   ESC_BOLD ESC_FG_CYAN    "Message printed from the tick hook interrupt ##############\n" ESC_NORMAL
};

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by each timer interrupt.
 *
 * @note This Function is for debug purposes only.
 *       In this case the macro configUSE_TICK_HOOK must be 1.
 */
void vApplicationTickHook( void )
{
   static unsigned int iCount = 0;

   /*
    * Print out a message every second. The message is not written out directly,
    * but sent to the gatekeeper task.
    */
   iCount++;
   if( iCount >= pdMS_TO_TICKS(1000) )
   { /*
      * As xQueueSendToFrontFromISR() is being called from the tick hook, it is
      * not necessary to use the xHigherPriorityTaskWoken parameter (the third
      * parameter), and the parameter is set to NULL.
      */
      xQueueSendToFrontFromISR( g_xPrintQueue, &g_pcStringsToPrint[2], NULL );

     /*
      * Reset the count ready to print out the string again in 200 ticks time.
      */
      iCount = 0;
   }
}

/*! ---------------------------------------------------------------------------
 * @brief A task which has a massage to print.
 */
STATIC void prvPrintTask( void* pvParameters )
{ /*
   * Two instances of this task are created. The task parameter is used to
   * pass an index into an array of strings into the task. Cast this to the
   * required type.
   */
   const unsigned int iIndexToString = (int) pvParameters;

   while( true )
   { /*
      * Print out the string, not directly, but instead by passing a pointer to
      * the string to the gatekeeper task via a queue. The queue is created
      * before the scheduler is started so will already exist by the time this
      * task executes for the first time.
      * A block time is not specified because there should always be space in
      * the queue.
      */
      xQueueSendToBack( g_xPrintQueue, &g_pcStringsToPrint[iIndexToString], 0 );

      /*
       * Wait for a while depending on the task parameter
       */
      vTaskDelay( pdMS_TO_TICKS( (iIndexToString == 0)? 1000 : 500) );
   }
}

/*! ---------------------------------------------------------------------------
 */
static void prvUartGatekeeperTask( void* pvParameters UNUSED )
{
   char* pcMessageToPrint;

   /*
    * This is the only task that is allowed to write to standard out.
    * Any other task wanting to write a string to the output does not access
    * UART directly, but instead sends the string to this task.
    * As only this task accesses the UART there are no mutual exclusion
    * or serialization issues to consider within the implementation
    * of the task itself.
    */
   while( true )
   { /*
      * Wait for a message to arrive. An indefinite block time is specified so
      * there is no need to check the return value – the function will only
      * return when a message has been successfully received.
      */
      xQueueReceive( g_xPrintQueue, &pcMessageToPrint, portMAX_DELAY );

     /*
      * Output the received string.
      */
      puts( pcMessageToPrint );

     /*
      * Loop back to wait for the next message.
      */
   }
}

/*! ---------------------------------------------------------------------------
 * @brief Creating of all tasks and the message queue and start of
 *        the scheduler.
 */
STATIC inline BaseType_t initAndStartRTOS( void )
{
   mprintf( "Creating the print-queue\n" );
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
   g_xPrintQueue = xQueueCreateStatic( QUEUE_LENGTH,
                                       ITEM_SIZE,
                                       g_ucQueueStorageArea,
                                       &g_xStaticQueue );
#else
   g_xPrintQueue = xQueueCreate( QUEUE_LENGTH, ITEM_SIZE );
#endif
   if( g_xPrintQueue == NULL )
      return pdFAIL;

   mprintf( "Creating task \"prvUartGatekeeperTask\"\n" );
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
   if( xTaskCreateStatic( prvUartGatekeeperTask,
                          "Gate-keeper",
                          ARRAY_SIZE( g_xStackUartGatekeeperTask ),
                          NULL,
                          0,
                          g_xStackUartGatekeeperTask,
                          &g_xUartGatekeeperTaskBuffer
     ) == NULL )
      return pdFAIL;
#else
   BaseType_t status;
   status = xTaskCreate( prvUartGatekeeperTask,
                         "Gate-keeper",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         0,
                         NULL
                       );
   if( status != pdPASS )
      return status;
#endif

   mprintf( "Creating task \"prvPrintTask 1\"\n" );
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
   if( xTaskCreateStatic( prvPrintTask,
                          "prvPrintTask 1",
                          ARRAY_SIZE( g_xStackPrintTask1 ),
                          (void*) 0,
                          1,
                          g_xStackPrintTask1,
                          &g_xPrintTaskBuffer1
     ) == NULL )
      return pdFAIL;
#else
   status = xTaskCreate( prvPrintTask,
                         "prvPrintTask 1",
                         configMINIMAL_STACK_SIZE,
                         (void*) 0,
                         1,
                         NULL
                       );
   if( status != pdPASS )
      return status;
#endif

   mprintf( "Creating task \"prvPrintTask 2\"\n" );
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 0 )
   if( xTaskCreateStatic( prvPrintTask,
                          "prvPrintTask 2",
                          ARRAY_SIZE( g_xStackPrintTask2 ),
                          (void*) 1,
                          2,
                          g_xStackPrintTask2,
                          &g_xPrintTaskBuffer2
     ) == NULL )
      return pdFAIL;
#else
   status = xTaskCreate( prvPrintTask,
                         "prvPrintTask 2",
                         configMINIMAL_STACK_SIZE,
                         (void*) 1,
                         2,
                         NULL
                       );
   if( status != pdPASS )
      return status;
#endif

   vTaskStartScheduler();

   return pdPASS;
}

/*! ===========================================================================
 * @brief Main function, what else...
 */
void main( void )
{
   mprintf( ESC_XY( "1", "1" ) ESC_CLR_SCR
            "FreeRTOS message queue test\n"
            "Compiler:  " COMPILER_VERSION_STRING "\n"
            "Tick-rate: %d Hz\n", configTICK_RATE_HZ );


   const BaseType_t status = initAndStartRTOS();
   mprintf( ESC_ERROR "Error: This point shall never be reached!\n"
                      "Status: %d\n" ESC_NORMAL, status );
   while( true );
}

/* ================================= EOF ====================================*/

/*!
 *  @file port.c
 *  @brief LM32 port for FreeRtos LM32.
 *
 *  @date 14.01.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *  Origin:  Richard Barry.(V4.7.0)
 *  @todo A lot!
 */
/*
 * FreeRTOS Kernel V10.1.1
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 */
#ifndef __DOXYGEN__
 #ifndef __GNUC__
   #error This module is for GNU compiler only!
 #endif
 #ifndef __lm32__
   #error This module is for the target Latice micro32 (LM32) only!
 #endif
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifndef CONFIG_NO_RTOS_TIMER
 #include "lm32Timer.h"
 #include "lm32Interrupts.h"
#endif
#include "eb_console_helper.h"
#include "mini_sdb.h"


/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the MICO32 port.
 *----------------------------------------------------------*/
#ifndef CONFIG_NO_RTOS_TIMER
static void prvSetupTimer( void );
#endif

#if (configAPPLICATION_ALLOCATED_HEAP == 1)
  uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

volatile static unsigned int uxCriticalNesting = 0;

/* ----------------------------------------------------------------------------
 * See header file for description. 
 */
portSTACK_TYPE* pxPortInitialiseStack( portSTACK_TYPE* pxTopOfStack,
                                       TaskFunction_t pxCode,
                                       void* pvParameters )
{  /*
    * Place a 4 bytes of known values on the bottom of the stack.
    * This is just useful for debugging.
    */
   *pxTopOfStack = 0x12345678;
   pxTopOfStack--;

   /* The compiler expects R0 to be 0. */

   /*
    * Place the parameter on the stack in the expected location.
    */
   *pxTopOfStack = (portSTACK_TYPE) pvParameters;  /* R1 */
   pxTopOfStack--;

   /*
    * Now the remaining registers.
    */
   *pxTopOfStack = (portSTACK_TYPE) 0x02; /* R2 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x03; /* R3 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x04; /* R4 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x05; /* R5 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x06; /* R6 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x07; /* R7 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x08; /* R8 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x09; /* R9 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x10; /* R10 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x11; /* R11 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x12; /* R12 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x13; /* R13 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x14; /* R14 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x15; /* R15 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x16; /* R16 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x17; /* R17 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x18; /* R18 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x19; /* R19 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x20; /* R20 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x21; /* R21 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x22; /* R22 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x23; /* R23 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x24; /* R24 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x25; /* R25 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x26; /* R26 */
   pxTopOfStack--;
   *pxTopOfStack = (portSTACK_TYPE) 0x27; /* R27 */
   pxTopOfStack--;

   /*
    * The return address
    */
   *pxTopOfStack = (portSTACK_TYPE) pxCode;
   pxTopOfStack--;

   /*
    * The exception (interrupt) return address
    * - which in this case is the start of the task.
    */
   *pxTopOfStack = (portSTACK_TYPE) pxCode;
   pxTopOfStack--;

   /*
    * Status information.
    */
   *pxTopOfStack = (portSTACK_TYPE) 0x00;
   pxTopOfStack--;

   return pxTopOfStack;
}

/*! ---------------------------------------------------------------------------
 */
portBASE_TYPE xPortStartScheduler( void )
{
#ifndef CONFIG_NO_RTOS_TIMER
   /*
    * Setup the hardware to generate the tick.
    */
   prvSetupTimer();
#endif
   /*
    * Kick off the first task.
    */
   vStartFirstTask();

   /*
    * Should not get here as the tasks are now running!
    */
   return pdTRUE;
}

/*! ---------------------------------------------------------------------------
 */
void vPortEndScheduler( void )
{
   /* It is unlikely that the LM32 port will get stopped.  */
}

#ifndef CONFIG_NO_RTOS_TIMER
/*! ---------------------------------------------------------------------------
 */
static void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   xTaskIncrementTick();
#if configUSE_PREEMPTION == 1
   vTaskSwitchContext();
#endif

   /* Clear Timer Status */
   ((LM32_TIMER_T*)pContext)->status = 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Setup timer to generate a tick interrupt.
 */
static void prvSetupTimer( void )
{
 //  LM32_TIMER_T* pTimer = (LM32_TIMER_T*)TIMER_BASE_ADDRESS;

   volatile LM32_TIMER_T* pTimer = (LM32_TIMER_T*) find_device_adr( GSI, CPU_TIMER_CTRL_IF );
   if( (unsigned int)pTimer == ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found!\n" ESC_NORMAL );
      while( true );
   }

   /* stop the timer first and ack any pending interrupts */
   pTimer->control = TIMER_CONTROL_STOP_BIT_MASK;
   pTimer->status  = 0;

   /* Register Interrupt Service Routine */
   registerISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );

   pTimer->period = configCPU_CLOCK_HZ / configTICK_RATE_HZ;

   /* start the timer                               */
   pTimer->control = TIMER_CONTROL_START_BIT_MASK |
                     TIMER_CONTROL_INT_BIT_MASK   |
                     TIMER_CONTROL_CONT_BIT_MASK;
}
#else
 #if configUSE_PREEMPTION == 1
   #error In preemtion mode is the timer essential!
 #endif
 #warning Timer for FreeRTOS will not implenented! Some tick related functions will not work!
#endif

/*
 * Critical section management.
 * When the compiler and linker has LTO ability so the following inline
 * declarations will be really inline.
 */
/*! ---------------------------------------------------------------------------
 */
inline void vPortEnterCritical( void )
{
   portDISABLE_INTERRUPTS();
   uxCriticalNesting++;
}

/*! ---------------------------------------------------------------------------
 */
inline void vPortExitCritical( void )
{
   configASSERT( uxCriticalNesting > 0 );
   uxCriticalNesting--;
   if( uxCriticalNesting == 0 )
   {
      portENABLE_INTERRUPTS();
   }
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/*! ---------------------------------------------------------------------------
 * configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task.
 */
void vApplicationGetIdleTaskMemory( StaticTask_t** ppxIdleTaskTCBBuffer,
                                    StackType_t** ppxIdleTaskStackBuffer,
                                    uint32_t* pulIdleTaskStackSize )
{
   /*
    * If the buffers to be provided to the Idle task are declared inside this
    * function then they must be declared static – otherwise they will be allocated on
    * the stack and so not exists after this function exits.
    */
   static StaticTask_t xIdleTaskTCB;
   static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

   /*
    * Pass out a pointer to the StaticTask_t structure in which the Idle task’s
    * state will be stored.
    */
   *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

   /*
    * Pass out the array that will be used as the Idle task’s stack.
    */
   *ppxIdleTaskStackBuffer = uxIdleTaskStack;

   /*
    * Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configMINIMAL_STACK_SIZE is specified in words, not bytes.
    */
   *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

#if (configUSE_TIMERS == 1)
/*
 * configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task.
 */
void vApplicationGetTimerTaskMemory( StaticTask_t** ppxTimerTaskTCBBuffer,
                                     StackType_t** ppxTimerTaskStackBuffer,
                                     uint32_t* pulTimerTaskStackSize )
{
   /*
    * If the buffers to be provided to the Timer task are declared inside this
    * function then they must be declared static – otherwise they will be allocated on
    * the stack and so not exists after this function exits.
    */
   static StaticTask_t xTimerTaskTCB;
   static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

   /*
    * Pass out a pointer to the StaticTask_t structure in which the Timer
    * task’s state will be stored.
    */
   *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

   /*
    * Pass out the array that will be used as the Timer task’s stack.
    */
   *ppxTimerTaskStackBuffer = uxTimerTaskStack;

   /*
    * Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    * Note that, as the array is necessarily of type StackType_t,
    * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes.
    */
   *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif /* #if (configUSE_TIMERS == 1) */
#endif /* #if (configSUPPORT_STATIC_ALLOCATION == 1) */

/*================================== EOF ====================================*/
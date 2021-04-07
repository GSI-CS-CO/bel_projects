/*!
 * @file port.c FreeRTOS backed for Lattice Micro32 (LM32) within the SCU.
 * @brief LM32 port for FreeRtos LM32.
 *
 * @date 14.01.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see portmacro.h
 * @see portasm.S
 * @see crt0FreeRTOS.S
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
 #ifndef CONFIG_RTOS
   #error Compiler switch CONFIG_RTOS has to be define in Makefile!
 #endif
#endif
#include "FreeRTOS.h"
#include "task.h"

#ifndef CONFIG_NO_RTOS_TIMER
 #include "scu_lm32Timer.h"
#endif
#include "eb_console_helper.h"

#ifdef CONFIG_SCU
 #include "mini_sdb.h"
#endif

#if (configAPPLICATION_ALLOCATED_HEAP == 1)
  uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

/* ----------------------------------------------------------------------------
 * See header file for description. 
 */
portSTACK_TYPE* pxPortInitialiseStack( portSTACK_TYPE* pxTopOfStack,
                                       TaskFunction_t pxCode,
                                       void* pvParameters )
{  /*
    * Place a 4 bytes of known values on the bottom of the stack.
    * This is just useful for debugging.
    * Position of "r0" this will not updated its always zero!
    * The compiler expects "r0" to be always 0!
    */
   *pxTopOfStack = 0xC0FEAD03;
   pxTopOfStack--;

   /*
    * Place the parameter on the stack in the expected location.
    * Position of "r1"
    */
   *pxTopOfStack = (portSTACK_TYPE) pvParameters;
   pxTopOfStack--;

   /*
    * The registers from "r2" to "r27" can have
    * a arbitrary value.
    */
   for( portSTACK_TYPE i = 2; i <= 27; i++ )
   {
      *pxTopOfStack = i;
      pxTopOfStack--;
   }

   /*!
    * The return address
    * Position of register "r29" alias "ra"
    * @see portasm.S
    */
   *pxTopOfStack = (portSTACK_TYPE) pxCode;
   pxTopOfStack--;

   /*!
    * The exception (interrupt) return address
    * - which in this case is the start of the task.
    * Position of register "r30" alias "ea"
    * @see portasm.S
    */
   *pxTopOfStack = (portSTACK_TYPE) pxCode;
   pxTopOfStack--;

   /*!
    * Status information.
    * Position of context switch cause flag.
    * @see portasm.S
    * @see __cscf
    */
   *pxTopOfStack = (portSTACK_TYPE) 0;
   pxTopOfStack--;

   return pxTopOfStack;
}

#ifndef CONFIG_NO_RTOS_TIMER
/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @ingroup SCU_LM32_TIMER
 * @brief The timer interrupt function!
 * @param intNum Interrupt number of this interrupt (not used in this case)
 * @param pContext User tunnel  (start address of timer registers but
 *                 not used in this case)
 */
STATIC void onTimerInterrupt( const unsigned int intNum, const void* pContext )
{
   xTaskIncrementTick();
#if configUSE_PREEMPTION == 1
   vTaskSwitchContext();
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @ingroup SCU_LM32_TIMER
 * @brief Setup timer to generate a tick interrupt.
 */
ONE_TIME_CALL void prvSetupTimer( void )
{
#ifdef CONFIG_SCU
   SCU_LM32_TIMER_T* pTimer = lm32TimerGetWbAddress();
   if( pTimer == (SCU_LM32_TIMER_T*)ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "ERROR: Timer not found or not implemented!\n" ESC_NORMAL );
      while( true );
   }
#else
   #ifndef LM32_TIMER_BASE_ADDR
     #error Macro LM32_TIMER_BASE_ADDR is not defined!
   #endif
   SCU_LM32_TIMER_T* pTimer = (SCU_LM32_TIMER_T*) LM32_TIMER_BASE_ADDR;
#endif

   #if configTICK_RATE_HZ == 0
     #error configTICK_RATE_HZ is defined by zero!
   #endif
   /*
    * CPU frequency has to be divisible by the task tick frequency!
    */
   STATIC_ASSERT( (configCPU_CLOCK_HZ % configTICK_RATE_HZ) == 0 );

   lm32TimerSetPeriod( pTimer, configCPU_CLOCK_HZ / configTICK_RATE_HZ );
   lm32TimerEnable( pTimer );
   /*
    * Register Interrupt Service Routine
    */
   irqRegisterISR( TIMER_IRQ, (void*)pTimer, onTimerInterrupt );
}

#else /* ifndef CONFIG_NO_RTOS_TIMER */
 #if configUSE_PREEMPTION == 1
   #error In preemtion mode is the timer essential!
 #endif
 #warning Timer for FreeRTOS will not implenented! Some tick related functions will not work!
#endif /* else ifndef CONFIG_NO_RTOS_TIMER */

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
    * Kick off the first task, resets the critical section nesting counter
    * and enables the global interrupt.
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
   configASSERT( false );
}

#if (configSUPPORT_STATIC_ALLOCATION == 1) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task.
 */
OVERRIDE
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
   static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

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

#if (configUSE_TIMERS == 1) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task.
 */
OVERRIDE
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
   static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

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

#if ( configCHECK_FOR_STACK_OVERFLOW != 0 ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Becomes invoked in the case of a stack overflow.
 * @note Use this function for develop and debug purposes only!
 * @see https://www.freertos.org/Stacks-and-stack-overflow-checking.html
 * @see FreeRTOSConfig.h
 */
OVERRIDE
void vApplicationStackOverflowHook( TaskHandle_t xTask UNUSED, char* pcTaskName )
{
   mprintf( ESC_ERROR "Error: Stack overflow in task \"%s\"!\n"
                      "Method: %d\n" ESC_NORMAL,
                       pcTaskName,
                       configCHECK_FOR_STACK_OVERFLOW
          );
   configASSERT( false );
}
#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW != 0 ) */

#if ( configUSE_MALLOC_FAILED_HOOK != 0 ) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Becomes invoked when a memory allocation was not successful.
 * @note Use this function for develop and debug purposes only!
 * @see https://www.freertos.org/a00016.html
 * @see FreeRTOSConfig.h
 */
OVERRIDE
void vApplicationMallocFailedHook( void )
{
   mprintf( ESC_ERROR "Error: Memory allocation failed!\n" ESC_NORMAL );
   configASSERT( false );
}
#endif /* #if ( configUSE_MALLOC_FAILED_HOOK != 0 ) */

/*================================== EOF ====================================*/

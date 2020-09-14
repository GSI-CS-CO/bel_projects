/*!
 *  @file portmacro.h
 *  @brief LM32 portmacro for FreeRtos LM32.
 *
 *  @date 14.10.2020
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
 *
 * 1 tab == 4 spaces!
 */
#ifndef PORTMACRO_H
#define PORTMACRO_H
#if !defined(__lm32__) && !defined(__DOXYGEN__)
  #error This module is for the target LM32 only!
#endif
#include <irq.h>
/*
Changes from V1.2.3

   + portCPU_CLOSK_HZ definition changed to 8MHz base 10, previously it
      base 16.
*/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  void*
#define portBASE_TYPE   int32_t

typedef portSTACK_TYPE StackType_t;
typedef int32_t        BaseType_t;
typedef uint32_t       UBaseType_t;

#if ( configUSE_16_BIT_TICKS == 1 )
   typedef uint16_t TickType_t;
   #define portMAX_DELAY ( TickType_t ) 0xffff
#else
   typedef uint32_t TickType_t;
   #define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif
/*-----------------------------------------------------------*/

/*! ---------------------------------------------------------------------------
 * @brief Enables all interrupts.
 */
#define portENABLE_INTERRUPTS()   irq_enable()

/*! ---------------------------------------------------------------------------
 * @brief Disables all interrupts.
 */
#define portDISABLE_INTERRUPTS()  irq_disable()

/*! ---------------------------------------------------------------------------
 * @brief Macro for the begin of a critical respectively atomic section.
 *
 * Disable interrupts before incrementing the count of critical section nesting.
 * The nesting count is maintained so we know when interrupts should be
 * re-enabled.  Once interrupts are disabled the nesting count can be accessed
 * directly.  Each task maintains its own nesting count.
 */
#define portENTER_CRITICAL()                       \
{                                                  \
   extern volatile UBaseType_t g_criticalNesting;  \
                                                   \
   portDISABLE_INTERRUPTS();                       \
   g_criticalNesting++;                            \
}

/*! ---------------------------------------------------------------------------
 * @brief Macro for the end of a critical respectively atomic section.
 *
 * Interrupts are disabled so we can access the nesting count directly.  If the
 * nesting is found to be 0 (no nesting) then we are leaving the critical
 * section and interrupts can be re-enabled.
 */
#define portEXIT_CRITICAL()                        \
{                                                  \
   extern volatile UBaseType_t g_criticalNesting;  \
                                                   \
   configASSERT( g_criticalNesting != 0 );         \
   g_criticalNesting--;                            \
   if( g_criticalNesting == 0 )                    \
   {                                               \
      portENABLE_INTERRUPTS();                     \
   }                                               \
}

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH   ( -1 )
#define portTICK_PERIOD_MS ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT 4
#define portNOP()          asm volatile ( "nop" )

/*-----------------------------------------------------------*/

/* Kernel utilities. */
extern void vPortYield( void ); //  __attribute__ ( ( naked ) );
#define portYIELD() vPortYield()
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) \
   void vFunction( void* pvParameters )

#define portTASK_FUNCTION( vFunction, pvParameters ) \
   void vFunction( void* pvParameters )

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */
/*================================== EOF ====================================*/

/*!
 * @file portmacro.h FreeRTOS backed for Lattice Micro32 (LM32) within the SCU.
 * @brief LM32 portmacro for FreeRtos LM32.
 *        Port specific definitions. \n
 *        The settings in this file configure FreeRTOS correctly \n
 *        for the Gnu-compiler for Lattice Micro32 (LM32)
 *
 * @date 14.10.2020
 * @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see port.c
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

#ifndef PORTMACRO_H
#define PORTMACRO_H
#if !defined(__lm32__) && !defined(__DOXYGEN__)
  #error This module is for the target LM32 only!
#endif
#include "generated/shared_mmap.h"
#include "lm32Interrupts.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       int16_t
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   int32_t

typedef portSTACK_TYPE StackType_t;
typedef int32_t        BaseType_t;
typedef uint32_t       UBaseType_t;

#if (configUSE_16_BIT_TICKS == 1)
   typedef uint16_t TickType_t;
   #define portMAX_DELAY (TickType_t) 0xffff
#else
   typedef uint32_t TickType_t;
   #define portMAX_DELAY (TickType_t) 0xffffffffUL
#endif

/*-----------------------------------------------------------*/
/*!
 * @brief Port Disable Interrupts
 */
#define portDISABLE_INTERRUPTS() irqDisable()

/*!
 * @brief Port Enable Interrupts
 */
#define portENABLE_INTERRUPTS()  irqEnable()

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH      ( -1 )
#define portTICK_RATE_MS      ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT    4
#define portNOP()             NOP()

/*-----------------------------------------------------------*/

/* Critical section management. */
#define portENTER_CRITICAL() criticalSectionEnter()
#define portEXIT_CRITICAL()  criticalSectionExit()

#define portYIELD()          vPortYield()


/*-----------------------------------------------------------*/

/*!
 * Task function macros as described on the FreeRTOS.org WEB site.
 */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

void vStartFirstTask( void );
void vPortYield( void );


#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

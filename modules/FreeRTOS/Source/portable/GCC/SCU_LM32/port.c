/*!
 *  @file port.c
 *  @brief LM32 port for FreeRtos LM32.
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
 */
#ifndef __DOXYGEN__
 #ifndef __GNUC__
   #error This module is for GNU compiler only!
 #endif
 #ifndef __lm32__
   #error This module is for the target Latice micro32 (LM32) only!
 #endif
#endif

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

/*!
 * @brief Critical nesting depth counter.
 *
 * Calls to portENTER_CRITICAL() can be nested.  When they are nested the
 * critical section should not be left (i.e. interrupts should not be re-enabled)
 * until the nesting depth reaches 0.  This variable simply tracks the nesting
 * depth.  Each task maintains it's own critical nesting depth variable so
 * uxCriticalNesting is saved and restored from the task stack during a context
 * switch.
 *
 * @see portENTER_CRITICAL
 * @see portEXIT_CRITICAL
 */
volatile UBaseType_t g_criticalNesting = 0;

/* Start tasks with interrupts enables. */

#if 0
#define portFLAGS_INT_ENABLED                 ( StackType_t ) 0x80 )

/* Hardware constants for timer 1. */
#define portCLEAR_COUNTER_ON_MATCH            ( uint8_t ) 0x08 )
#define portPRESCALE_64                       ( ( uint8_t ) 0x03 )
#define portCLOCK_PRESCALER                   ( ( uint32_t ) 64 )
#define portCOMPARE_MATCH_A_INTERRUPT_ENABLE  ( ( uint8_t ) 0x10 )
#endif

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void TCB_t;
extern volatile TCB_t * volatile pxCurrentTCB;

#if (configAPPLICATION_ALLOCATED_HEAP == 1)
  uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

/*-----------------------------------------------------------*/

/*! 
 * @brief Macro to save all the general purpose registers, the save the stack pointer
 * into the TCB.  
 * 
 * The first thing we do is save the flags then disable interrupts.  This is to 
 * guard our stack against having a context switch interrupt after we have already 
 * pushed the registers onto the stack - causing the 32 registers to be on the 
 * stack twice. 
 * 
 * r1 is set to zero as the compiler expects it to be thus, however some
 * of the math routines make use of R1. 
 * 
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer. 
 */
#define portSAVE_CONTEXT()                       \
   asm volatile ( "addi sp, sp, -120             \n\t" \
                  "sw   (sp+116), r1             \n\t" \
                  "sw   (sp+112), r2             \n\t" \
                  "sw   (sp+108), r3             \n\t" \
                  "sw   (sp+104), r4             \n\t" \
                  "sw   (sp+100), r5             \n\t" \
                  "sw   (sp+96),  r6             \n\t" \
                  "sw   (sp+92),  r7             \n\t" \
                  "sw   (sp+88),  r8             \n\t" \
                  "sw   (sp+84),  r9             \n\t" \
                  "sw   (sp+80),  r10            \n\t" \
                  "sw   (sp+76),  r11            \n\t" \
                  "sw   (sp+72),  r12            \n\t" \
                  "sw   (sp+68),  r13            \n\t" \
                  "sw   (sp+64),  r14            \n\t" \
                  "sw   (sp+60),  r15            \n\t" \
                  "sw   (sp+56),  r16            \n\t" \
                  "sw   (sp+52),  r17            \n\t" \
                  "sw   (sp+48),  r18            \n\t" \
                  "sw   (sp+44),  r19            \n\t" \
                  "sw   (sp+40),  r20            \n\t" \
                  "sw   (sp+36),  r21            \n\t" \
                  "sw   (sp+32),  r22            \n\t" \
                  "sw   (sp+28),  r23            \n\t" \
                  "sw   (sp+24),  r24            \n\t" \
                  "sw   (sp+20),  r25            \n\t" \
                  "sw   (sp+16),  r26            \n\t" \
                  "sw   (sp+12),  r27            \n\t" \
                  "sw   (sp+8),   r28            \n\t" \
                  "sw   (sp+4),   ra             \n\t" \
                  "xor  r1, r1, r1               \n\t" \
                  "mvhi r1, hi(pxCurrentTCB)     \n\t" \
                  "ori  r1, r1, lo(pxCurrentTCB) \n\t" \
                  "lw   r1, (r1+0)               \n\t" \
                  "sw   (r1+0), sp               \n\t" \
                  : : : \
                )

/*! ---------------------------------------------------------------------------
 * @brief Opposite to portSAVE_CONTEXT().
 *
 *  Interrupts will have been disabled during
 *  the context save so we can write to the stack pointer.
 */
#define portRESTORE_CONTEXT()                              \
   asm volatile (  "xor    sp, sp, sp                \n\t" \
                   "mvhi   sp, hi(pxCurrentTCB)      \n\t" \
                   "ori    sp, sp, lo(pxCurrentTCB)  \n\t" \
                   "lw     sp, (sp+0)                \n\t" \
                   "lw     sp, (sp+0)                \n\t" \
                   "lw     ra, (sp+4)                \n\t" \
                   "lw     r28, (sp+8)               \n\t" \
                   "lw     r27, (sp+12)              \n\t" \
                   "lw     r26, (sp+16)              \n\t" \
                   "lw     r25, (sp+20)              \n\t" \
                   "lw     r24, (sp+24)              \n\t" \
                   "lw     r23, (sp+28)              \n\t" \
                   "lw     r22, (sp+32)              \n\t" \
                   "lw     r21, (sp+36)              \n\t" \
                   "lw     r20, (sp+40)              \n\t" \
                   "lw     r19, (sp+44)              \n\t" \
                   "lw     r18, (sp+48)              \n\t" \
                   "lw     r17, (sp+52)              \n\t" \
                   "lw     r16, (sp+56)              \n\t" \
                   "lw     r15, (sp+60)              \n\t" \
                   "lw     r14, (sp+64)              \n\t" \
                   "lw     r13, (sp+68)              \n\t" \
                   "lw     r12, (sp+72)              \n\t" \
                   "lw     r11, (sp+76)              \n\t" \
                   "lw     r10, (sp+80)              \n\t" \
                   "lw     r9, (sp+84)               \n\t" \
                   "lw     r8, (sp+88)               \n\t" \
                   "lw     r7, (sp+92)               \n\t" \
                   "lw     r6, (sp+96)               \n\t" \
                   "lw     r5, (sp+100)              \n\t" \
                   "lw     r4, (sp+104)              \n\t" \
                   "lw     r3, (sp+108)              \n\t" \
                   "lw     r2, (sp+112)              \n\t" \
                   "lw     r1, (sp+116)              \n\t" \
                   : : :                                   \
                )

#if 0
void vPortYield( void )
{
// The following 3 lines are automatically generated from the gcc Compiler
  addi sp,sp,-4         Decrement Stack
  sw (sp+4),ra          Store Return Address on Stack
  sw (sp+0),sp          Store content of Stack on Stack

// macro portSAVE_CONTEXT();
  addi sp,sp,-120       Decrement Stack by size of registers which are stored on the stack
  sw (sp+116),r1        Store content of first register on Stack
  ....                  Store remaining registers on Stack
  sw (sp+4),ra          Store Return Address on Stack

  and r1,r0,r0          Clear register R1
  mvhi r1,hi(pxCurrentTCB) Load High-Halfword of CurrentTCB
  ori  r1,r1,lo(pxCurrentTCB) Load Low-Halfword of CurrentTCB
  lw r1,(r1+0)          Load Pointer from CurrentTCB
  sw (r1+0), sp         Store actual StackPointer into current CurrentTCB - TopOfStack

  calli vTaskSwitchContext   Call vTaskSwitchContext() Function

// macro portRESTORE_CONTEXT();
  and sp,r0,r0          Clear StackPointer
  mvhi sp,hi(pxCurrentTCB) Load High-Halfword of CurrentTCB
  ori  sp,sp,lo(pxCurrentTCB)  Load Low-Halfword of CurrentTCB
  lw sp,(sp+0)          Load Pointer to CurrentTCB
  lw sp,(sp+0)          Load content from CurrentTCB - TopOfStack

  lw ra,(sp+4)          Load Return Address from Stack
  ......                Load remaining registers on Stack
  lw r1,(sp+116)Load content of first register from Stack

// The following 3 lines are automatically generated from the gcc Compiler
  lw ra,(sp+4)          Load Return Address from Stack
  addi sp,sp,4          Increment Stack
  ret                   Return from Subroutine
}

#endif
/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from timer 1, compare match A.
 */
static void prvSetupTimerInterrupt( void );
/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    uint32_t usAddress;
#if 0
	/* Place a few bytes of known values on the bottom of the stack. 
	This is just useful for debugging. */

	*pxTopOfStack = 0x11;
	pxTopOfStack--;
	*pxTopOfStack = 0x22;
	pxTopOfStack--;
	*pxTopOfStack = 0x33;
	pxTopOfStack--;

	/* Simulate how the stack would look after a call to vPortYield() generated by 
	the compiler. */

	/*lint -e950 -e611 -e923 Lint doesn't like this much - but nothing I can do about it. */

	/* The start of the task code will be popped off the stack last, so place
	it on first. */
	usAddress = ( uint32_t ) pxCode;
	*pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	usAddress >>= 8;
	*pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	/* Next simulate the stack as if after a call to portSAVE_CONTEXT().  
	portSAVE_CONTEXT places the flags on the stack immediately after r0
	to ensure the interrupts get disabled as soon as possible, and so ensuring
	the stack use is minimal should a context switch interrupt occur. */
	*pxTopOfStack = ( StackType_t ) 0x00;	/* R0 */
	pxTopOfStack--;
	*pxTopOfStack = portFLAGS_INT_ENABLED;
	pxTopOfStack--;


	/* Now the remaining registers.   The compiler expects R1 to be 0. */
	*pxTopOfStack = ( StackType_t ) 0x00;	/* R1 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x02;	/* R2 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x03;	/* R3 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x04;	/* R4 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x05;	/* R5 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x06;	/* R6 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x07;	/* R7 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x08;	/* R8 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x09;	/* R9 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x10;	/* R10 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x11;	/* R11 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x12;	/* R12 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x13;	/* R13 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x14;	/* R14 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x15;	/* R15 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x16;	/* R16 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x17;	/* R17 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x18;	/* R18 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x19;	/* R19 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x20;	/* R20 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x21;	/* R21 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x22;	/* R22 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x23;	/* R23 */
	pxTopOfStack--;

	/* Place the parameter on the stack in the expected location. */
	usAddress = ( uint16_t ) pvParameters;
	*pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	usAddress >>= 8;
	*pxTopOfStack = ( StackType_t ) ( usAddress & ( uint16_t ) 0x00ff );
	pxTopOfStack--;

	*pxTopOfStack = ( StackType_t ) 0x26;	/* R26 X */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x27;	/* R27 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x28;	/* R28 Y */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x29;	/* R29 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x30;	/* R30 Z */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) 0x031;	/* R31 */
	pxTopOfStack--;

	/*lint +e950 +e611 +e923 */
#endif
	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
	/* Setup the hardware to generate the tick. */
	prvSetupTimerInterrupt();

	/* Restore the context of the first task that is going to run. */
	portRESTORE_CONTEXT();

	/* Simulate a function call end as generated by the compiler.  We will now
	jump to the start of the task the context of which we have just restored. */
	asm volatile ( "ret" );

	/* Should not get here. */
	return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
   /*
    * It is unlikely that the AVR port will get stopped.  If required simply
    * disable the tick interrupt here.
    */
}

/*! ---------------------------------------------------------------------------
 * @brief  Manual context switch.
 *
 * The first thing we do is save the registers so we
 * can use a naked attribute.
 */
void vPortYield( void )
{
   portSAVE_CONTEXT();
   vTaskSwitchContext();
   portRESTORE_CONTEXT();
}

/*! ---------------------------------------------------------------------------
 * Context switch function used by the tick.  This must be identical to 
 * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
 * difference from vPortYield() is the tick count is incremented as the
 * call comes from the tick ISR.
 */
void vPortYieldFromTick( void )
{
   portSAVE_CONTEXT();
   if( xTaskIncrementTick() != pdFALSE )
   {
      vTaskSwitchContext();
   }
   portRESTORE_CONTEXT();
}

/*! ---------------------------------------------------------------------------
 * Setup timer 1 compare match A to generate a tick interrupt.
 */
static void prvSetupTimerInterrupt( void )
{
#if 0
uint32_t ulCompareMatch;
uint8_t ucHighByte, ucLowByte;

	/* Using 16bit timer 1 to generate the tick.  Correct fuses must be
	selected for the configCPU_CLOCK_HZ clock. */

	ulCompareMatch = configCPU_CLOCK_HZ / configTICK_RATE_HZ;

	/* We only have 16 bits so have to scale to get our required tick rate. */
	ulCompareMatch /= portCLOCK_PRESCALER;

	/* Adjust for correct value. */
	ulCompareMatch -= ( uint32_t ) 1;

	/* Setup compare match value for compare match A.  Interrupts are disabled 
	before this is called so we need not worry here. */
	ucLowByte = ( uint8_t ) ( ulCompareMatch & ( uint32_t ) 0xff );
	ulCompareMatch >>= 8;
	ucHighByte = ( uint8_t ) ( ulCompareMatch & ( uint32_t ) 0xff );
	OCR1AH = ucHighByte;
	OCR1AL = ucLowByte;

	/* Setup clock source and compare match behaviour. */
	ucLowByte = portCLEAR_COUNTER_ON_MATCH | portPRESCALE_64;
	TCCR1B = ucLowByte;

	/* Enable the interrupt - this is okay as interrupt are currently globally
	disabled. */
	ucLowByte = TIMSK;
	ucLowByte |= portCOMPARE_MATCH_A_INTERRUPT_ENABLE;
	TIMSK = ucLowByte;
#endif
}
/*-----------------------------------------------------------*/

#if configUSE_PREEMPTION == 1
/*
 * Tick ISR for preemptive scheduler.  We can use a naked attribute as
 * the context is saved at the start of vPortYieldFromTick().  The tick
 * count is incremented after the context is saved.
 */
void SIG_OUTPUT_COMPARE1A( void ) __attribute__ ( ( signal, naked ) );
void SIG_OUTPUT_COMPARE1A( void )
{
   vPortYieldFromTick();
   asm volatile ( "reti" );
}
#else
/*
 * Tick ISR for the cooperative scheduler.  All this does is increment the
 * tick count.  We don't need to switch context, this can only be done by
 * manual calls to taskYIELD();
 */
void SIG_OUTPUT_COMPARE1A( void ) __attribute__ ( ( signal ) );
void SIG_OUTPUT_COMPARE1A( void )
{
   xTaskIncrementTick();
}
#endif

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/*! ---------------------------------------------------------------------------
 * configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task.
 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
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
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
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

/*================================== EOF =====================================*/

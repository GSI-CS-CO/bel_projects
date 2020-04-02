/*!
 * @file portmacro.h FreeRTOS backed for Lattice Micro32 (LM32) within the SCU.
 * @brief LM32 portmacro for FreeRtos LM32.
 *        Port specific definitions. \n
 *        The settings in this file configure FreeRTOS correctly \n
 *        for the Gnu-compiler for Lattice Micro32 (LM32)
 *
 * @date 14.01.2020
 * @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see port.c
 * @see portasm.S
 * @see crt0FreeRTOS.S
 */
#ifndef _PORTMACRO_H
#define _PORTMACRO_H
#if !defined(__lm32__) && !defined(__DOXYGEN__)
  #error This module is for the target LM32 only!
#endif

#include <stdbool.h>
#include <stdint.h>
#include "lm32Interrupts.h"

/* Define to trap errors during development. */
#ifdef CONFIG_RTOS_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define configASSERT SCU_ASSERT
#else
   #define configASSERT(__e)
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef char           portCHAR;
typedef float          portFLOAT;
typedef double         portDOUBLE;
typedef long           portLONG;
typedef int16_t        portSHORT;
typedef uint32_t       portSTACK_TYPE;
typedef int32_t        portBASE_TYPE;
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

#define configSTACK_DEPTH_TYPE           uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Port Disable Interrupts
 * @see irqDisable
 */
#define portDISABLE_INTERRUPTS() irqDisable()

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Port Enable Interrupts
 * @see irqEnable
 */
#define portENABLE_INTERRUPTS()  irqEnable()

/*-----------------------------------------------------------*/

/* Architecture specifics. */
#ifndef USRCPUCLK
   #error Macro USRCPUCLK not defined in Makefile!
#endif
#define configCPU_CLOCK_HZ    (USRCPUCLK * 1000)

#define portSTACK_GROWTH      ( -1 )
#define portTICK_RATE_MS      ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT    4
#define portNOP()             NOP()
#define portFORCE_INLINE      ALWAYS_INLINE

/*-----------------------------------------------------------*/

/* Critical section management. */

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @see criticalSectionEnter
 */
#define portENTER_CRITICAL() criticalSectionEnter()

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @see criticalSectionExit
 */
#define portEXIT_CRITICAL()  criticalSectionExit()

#define portYIELD()          vPortYield()


/*-----------------------------------------------------------*/

/*!
 * Task function macros as described on the FreeRTOS.org WEB site.
 */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) \
   void vFunction( void *pvParameters )

#define portTASK_FUNCTION( vFunction, pvParameters ) \
   void vFunction( void *pvParameters )

/*! ---------------------------------------------------------------------------
 * @brief Declaration of assembly function implemented in portasm.S
 * @see portasm.S
 */
void vStartFirstTask( void );

/*! ---------------------------------------------------------------------------
 * @brief Declaration of assembly function implemented in portasm.S
 * @see portasm.S
 */
void vPortYield( void );

#ifdef __cplusplus
}
#endif
#endif /* _PORTMACRO_H */
/*================================== EOF ====================================*/

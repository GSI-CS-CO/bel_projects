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
 * @see crt0ScuLm32.S
 */
#ifndef _PORTMACRO_H
#define _PORTMACRO_H
#if !defined(__lm32__) && !defined(__DOXYGEN__)
  #error This module is for the target LM32 only!
#endif

#include <stdbool.h>
#include <stdint.h>
#include <lm32Interrupts.h>

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

/*!
 * Time is measured in ‘ticks’ – which is the number of times the tick
 * interrupt has executed since the RTOS kernel was started.\n
 * The tick count is held in a variable of type TickType_t.\n
 * Defining configUSE_16_BIT_TICKS as 1 causes TickType_t to be defined
 * (typedef’ed) as an unsigned 16bit type.\n
 * Defining configUSE_16_BIT_TICKS as 0 causes TickType_t to be defined
 * (typedef’ed) as an unsigned 32bit type.\n
 * Using a 16 bit type will greatly improve performance on 8 and 16 bit
 * architectures, but limits the maximum specifiable time period to
 * 65535 ‘ticks’.\n Therefore, assuming a tick frequency of 250Hz,
 * the maximum time a task can delay or block when a 16bit counter
 * is used is 262 seconds, compared to 17179869 seconds when
 * using a 32bit counter.\n
 *
 * The LM32 is a 32 bit architecture, therefore a 32 bit counter will
 * used.
 */
#define configUSE_16_BIT_TICKS 0

#if (configUSE_16_BIT_TICKS == 1)
   typedef uint16_t TickType_t;
   #define portMAX_DELAY (TickType_t) 0xFFFF
#else
   typedef uint32_t TickType_t;
   #define portMAX_DELAY (TickType_t) 0xFFFFFFFFUL
#endif

#ifndef configSUPPORT_DYNAMIC_ALLOCATION
  #ifdef CONFIG_RTOS_HEAP
    #define configSUPPORT_DYNAMIC_ALLOCATION  1
  #else
    #define configSUPPORT_DYNAMIC_ALLOCATION  0
  #endif
#endif

#define configSTACK_DEPTH_TYPE            uint32_t
#define configMESSAGE_BUFFER_LENGTH_TYPE  size_t

/*!
 * @ingroup OVERWRITABLE
 * @brief The size of the stack used by the idle task.
 *
 * Generally this should not be reduced from the value set in the
 * FreeRTOSConfig.h file provided with the demo application
 * for the port you are using.\n
 * Like the stack size parameter to the xTaskCreate() and xTaskCreateStatic()
 * functions, the stack size is specified in words, not bytes.
 * If each item placed on the stack is 32-bits, then a stack size of
 * 100 means 400 bytes (each 32-bit stack item consuming 4 bytes).
 */
#ifndef configMINIMAL_STACK_SIZE
  #define configMINIMAL_STACK_SIZE        128
#endif
#if (configMINIMAL_STACK_SIZE < 128)
  #error configMINIMAL_STACK_SIZE has to be at least 128
#endif

/*!
 * @ingroup OVERWRITABLE
 * @brief The maximum permissible length of the descriptive name given to
 *        a task when the task is created.
 *
 * The length is specified in the number of characters including the
 * NULL termination byte.
 */
#ifndef configMAX_TASK_NAME_LEN
  #define configMAX_TASK_NAME_LEN         16
#endif

/*!
 * @ingroup OVERWRITABLE
 * @brief The frequency of the RTOS tick interrupt.
 *
 * The tick interrupt is used to measure time.
 * Therefore a higher tick frequency means time can be measured to a higher
 * resolution. However, a high tick frequency also means that the RTOS kernel
 * will use more CPU time so be less efficient.\n
 * The RTOS demo applications all use a tick rate of 1000Hz.\n
 * This is used to test the RTOS kernel and is higher than would normally
 * be required.
 *
 * More than one task can share the same priority.\n
 * The RTOS scheduler will share processor time between tasks of the same
 * priority by switching between the tasks during each RTOS tick.\n
 * A high tick rate frequency will therefore also have the effect of reducing
 * the ‘time slice’ given to each task.
 *
 * This value will shown in the SCU-LM32 build-id as well.
 */
#ifndef configTICK_RATE_HZ
  #define configTICK_RATE_HZ             1000
#endif

/*!
 * @ingroup OVERWRITABLE
 * @brief The number of priorities available to the application tasks.
 *
 * Any number of tasks can share the same priority.\n
 * Co-routines are prioritised separately.
 * @see configMAX_CO_ROUTINE_PRIORITIES
 * Each available priority consumes RAM within the RTOS kernel so this value\n
 * should not be set any higher than actually required by your application.
 */
#ifndef configMAX_PRIORITIES
  #define configMAX_PRIORITIES          3
#endif



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
#define portFORCE_INLINE      inline ALWAYS_INLINE

#define configPRINTF( a )     mprintf a
#define configMIN             min
#define configMAX             max

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
#define portENABLE_INTERRUPTS()  _irqEnable()

/*-----------------------------------------------------------*/

/*!
 * Task function macros as described on the FreeRTOS.org WEB site.
 */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) \
   void vFunction( void *pvParameters )

#define portTASK_FUNCTION( vFunction, pvParameters ) \
   void vFunction( void *pvParameters )

/*! ---------------------------------------------------------------------------
 * @brief Start first task and enable global interrupt
 *
 * Declaration of assembly function implemented in portasm.S
 * @see portasm.S
 */
void vStartFirstTask( void );

/*! ---------------------------------------------------------------------------
 * @brief Function for context switching it enables the IRQs as well.
 *
 * Declaration of assembly function implemented in portasm.S
 * @see portasm.S
 */
void vPortYield( void );

//#define portYIELD()          vPortYield()
STATIC inline ALWAYS_INLINE void portYIELD( void )
{
   vPortYield();
}

#ifdef __cplusplus
}
#endif
#endif /* _PORTMACRO_H */
/*================================== EOF ====================================*/

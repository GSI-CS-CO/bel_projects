/*!
 * @file   lm32Interrupts.h
 * @brief  Administration of the interrupt callback functions for LM32
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      21.01.2020
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
#ifndef INTERRUPTS_HEADER_FILE
#define INTERRUPTS_HEADER_FILE

#if !defined(__lm32__) && !defined(__DOXYGEN__)
  #error This module is for the target LM32 only!
#endif

#include <stdbool.h>
#include <stdint.h>
#include <scu_lm32_macros.h>

/*!
 * @defgroup INTERRUPT Interrupt administration of LM32
 */

/*!
 * @ingroup INTERRUPT
 * @defgroup ATOMIC Helper functions and macros for critical
 *                  uninterruptible code segments.
 */

#ifdef CONFIG_INTERRUPT_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define IRQ_ASSERT SCU_ASSERT
#else
   #define IRQ_ASSERT(__e)
#endif

#ifndef MAX_LM32_INTERRUPTS
/*!
 * @ingroup INTERRUPT
 * @brief Maximum number of possible interrupt sources.
 */
#define MAX_LM32_INTERRUPTS 32
#endif
#if ( MAX_LM32_INTERRUPTS > 32 )
 #error Macro MAX_LM32_INTERRUPTS is to large! Allowed maximum are 32 !
#endif

#ifdef __cplusplus
extern "C" {
#endif


/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Signature of interrupt callback function.
 * @see registerISR
 * @param intNum Number of interrupt from 0 to MAX_LM32_INTERRUPTS-1
 *        first parameter of registerISR().
 * @param pContext User context, second parameter of registerISR().
 */
typedef void(*ISRCallback)( const unsigned int intNum, const void* pContext );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Registers and de-registers interrupt-handler routine.
 *
 * To register, pass a valid function pointer to the Callback parameter.
 * To deregister, pass 0 as the callback parameter.
 *
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @param pContext Pointer provided by user that will be passed to the
 *                 interrupt-handler callback.
 * @param ISRCallback User-provided interrupt-handler routine. If this
 *                    value NULL then the interrupt becomes de-registered.
 */ 
void irqRegisterISR( const unsigned int intNum, void* pContext,
                     ISRCallback Callback );


/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Enables a specific interrupt
 *        Counterpart of irqDisableSpecific
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @see irqDisableSpecific
 */
void irqEnableSpecific( const unsigned int intNum );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Disables a specific interrupt.
 *        Counterpart of irqEnableSpecific
 *
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @see irqEnableSpecific
 */
void irqDisableSpecific( const unsigned int intNum );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Global enabling of all registered and activated interrupts.
 *        Counterpart of irqDisable().
 * @see irqDisable
 */
STATIC inline void irqEnable( void )
{
   const uint32_t ie = 0x00000001;
   asm volatile ( "wcsr ie, %0"::"r"(ie) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Global disabling of all interrupts.
 *        Counterpart of irqEnable()
 * @see irqEnable
 */
STATIC inline void irqDisable( void )
{
   asm volatile ( "wcsr ie, r0" );
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Function shall be invoked immediately before a critical respectively
 *        atomic section begins.
 *
 * Counterpart to criticalSectionLeave.
 * @see criticalSectionLeave
 */
void criticalSectionEnter( void );

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Function shall be invoked immediately after the end of a critical
 *        respectively atomic section.
 *
 * Counterpart to criticalSectionEnter
 * @see criticalSectionEnter
 */
void criticalSectionExit( void );

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Helper function for macro ATOMIC_SECTION feeding the pseudo for-loop
 * @see ATOMIC_SECTION
 */
STATIC inline bool __criticalSectionEnter( void )
{
   criticalSectionEnter();
   return true;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Establishes a atomic respectively critical section between the
 *        following enclosing curly braces.
 *
 * All interrupts within the body of the atomic section are locked.
 * @note <b>CAUTION:</b> Do not use the keywords "brake" or "return" within
 *       the atomic body! Its a for-loop!\n
 * @note Nested atomic sections are possible.
 * @note Keep atomic sections as short as possible, otherwise the danger of
 *       jittering grows when using a real time OS.
 *
 * Example:
 * @code
 * ATOMIC_SECTION()
 * { // Atomic body
 *    foo();
 *    bar();
 * }
 * @endcode
 */
#define ATOMIC_SECTION()                             \
   for( bool __c__ = __criticalSectionEnter();       \
        __c__;                                       \
        __c__ = false, criticalSectionExit() )

#ifdef __cplusplus
}
#endif

#endif
/* ================================= EOF ====================================*/

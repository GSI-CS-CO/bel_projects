/*!
 * @file   lm32Interrupts.h
 * @brief  General administration of the interrupt handling and
 *         critical resp. atomic sections for LM32
 *
 * @note This module is suitable for FreeRTOS-port.
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
#ifndef _LM32INTERRUPTS_H
#define _LM32INTERRUPTS_H

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

#if !defined( MAX_LM32_INTERRUPTS ) || defined(__DOXYGEN__)
/*!
 * @ingroup INTERRUPT
 * @brief Maximum number of possible interrupt sources.
 *
 * The maximum number of items of the table depends on this macro
 * which can be overwritten by the makefile.
 * The default and maximum value of MAX_LM32_INTERRUPTS is 32.
 *
 * @see ISREntryTable
 */
#define MAX_LM32_INTERRUPTS 32
#endif
#if ( MAX_LM32_INTERRUPTS > 32 )
 #error Macro MAX_LM32_INTERRUPTS is to large! Allowed maximum is 32 !
#endif

#ifdef __cplusplus
extern "C" {
namespace gsi
{
#endif

/*!
 * @ingroup INTERRUPT
 * @brief Interrupt enable bit mask of interrupt control register.
 */
#define IRQ_IE ((uint32_t)0x00000001)

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Signature of interrupt callback function.
 * @see registerISR
 * @param intNum Number of interrupt from 0 to MAX_LM32_INTERRUPTS-1
 *        first parameter of registerISR().
 * @param pContext User context, second parameter of registerISR().
 */
typedef void(*ISRCallback)( const unsigned int intNum, const void* pContext );

#ifdef CONFIG_USE_INTERRUPT_TIMESTAMP
/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the white rabbit time-stamp of the last occurred interrupt.
 */
uint64_t irqGetTimestamp( void );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the elapsed time in nanoseconds sonce the last interrupt.
 */
uint64_t irqGetTimeSinceLastInterrupt( void );

#endif /* ifdef CONFIG_USE_INTERRUPT_TIMESTAMP */

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT OVERWRITABLE
 * @brief Reordering the interrupt priority.
 *
 * By default the interrupt number is equal to the interrupt priority.
 * @note It's possible to overwrite this function for the case
 *       that the interrupt number isn't equal to the interrupt priority.
 * @param prio Interrupt priority.
 * @return Interrupt number.
 */
unsigned int _irqReorderPriority( const unsigned int prio );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT OVERWRITABLE
 * @brief Returns the interrupt flag mask calculated by the given
 *        interrupt number
 * @note It's possible to overwrite this function for the case
 *       that the standard interrupt input line will changed.
 * @param intNum Interrupt number.
 * @return Interrupt pending mask.
 */
uint32_t _irqGetPendingMask( const unsigned int intNum );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the critical- respectively atomic- section nesting counter.
 *
 * @note Usually for debug purposes only.
 * @note Outside of a critical section and/or interrupt routine the return
 *       value has to be always zero.\n
 *       Inside of a critical section greater or equal one depending on the
 *       nesting depth.
 */
unsigned int irqGetAtomicNestingCount( void );

void irqPresetAtomicNestingCount( void );


#ifndef CONFIG_RTOS
/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Enables the global interrupt.
 * @note This function is only for non FreeRTOS applications available! \n
 *       If a Free-RTOS application is used, so the Free-RTOS function
 *       vTaskStartScheduler() respectively the included assembler routine
 *       vStartFirstTask() implemented in portasm.S will accomplished this.
 */
void irqEnable( void );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Registers and un-registers interrupt-handler routine.
 *
 * To register, pass a valid function pointer to the Callback parameter.
 * To deregister, pass 0 as the callback parameter.
 *
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @param pContext Pointer provided by user that will be passed to the
 *                 interrupt-handler callback.
 * @param pfCallback User-provided interrupt-handler routine. If this
 *                   value NULL then the interrupt becomes de-registered.
 */ 
void irqRegisterISR( const unsigned int intNum, void* pContext,
                     ISRCallback pfCallback );


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
 * @brief Returns the current value of the LM32 interrupt pending register.
 * @return Value of the interrupt pending register.
 */
STATIC inline ALWAYS_INLINE
uint32_t irqGetPendingRegister( void )
{
   uint32_t ip;
   asm volatile ( "rcsr %0, ip" :"=r"(ip) );
   return ip;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Resets the bits in the LM32 interrupt pending register.
 * @param ip Bit must to reset the corresponding pending bit.
 * @note The clearing of the bits in the pending register will accomplished
 *       by writing a one in the concerning bit-position!
 */
STATIC inline ALWAYS_INLINE
void irqResetPendingRegister( const uint32_t ip )
{
   asm volatile ( "wcsr ip, %0" ::"r"(ip) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the current value of the LM32 interrupt pending register and
 *        reset it.
 * @return Value of the interrupt pending register before reset.
 */
STATIC inline
uint32_t irqGetAndResetPendingRegister( void )
{  /*
    * CAUTION! Its absolutely necessary that the interrupt pending register
    * becomes cleared immediately after reading. That means immediately at the
    * next CPU cycle.
    * We don't know exactly how the C/C++-compiler will translate this,
    * depending on the optimization level, version and so on.
    * Therefore its better to implement this in assembler.
    */
   uint32_t pending;
   asm volatile ( "rcsr %0, ip\n" \
                  "wcsr ip, %0\n" \
                  :"=r"(pending) );
   return pending;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the global interrupt enable register of the LM32.
 */
STATIC inline ALWAYS_INLINE uint32_t irqGetEnableRegister( void )
{
   uint32_t ie;
   asm volatile ( "rcsr %0, ie" :"=r"(ie) );
   return ie;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Sets the global interrupt enable register of the LM32.
 */
STATIC inline ALWAYS_INLINE void irqSetEnableRegister( register const uint32_t ie )
{
   asm volatile ( "wcsr ie, %0" ::"r"(ie) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Returns the current value of the LM32 interrupt mask register
 * @return Current value of the interrupt mask register.
 */
STATIC inline ALWAYS_INLINE
uint32_t irqGetMaskRegister( void )
{
   uint32_t im;
   asm volatile ( "rcsr %0, im" :"=r"(im) );
   return im;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Sets the interrupt mask register by the given value.
 * @param im Value to set.
 */
STATIC inline ALWAYS_INLINE
void irqSetMaskRegister( const uint32_t im )
{
   asm volatile ( "wcsr im, %0" ::"r"(im) );
}

/*! --------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief
 */
STATIC inline ALWAYS_INLINE bool irqIsEnabled( void )
{
   return (irqGetEnableRegister() & IRQ_IE) != 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Global enabling of all registered and activated interrupts.
 *        Counterpart of irqDisable().
 * @note Use this for one time in your application only, for further
 *       interrupt enable/disable actions prefer the functions \n
 *       criticalSectionEnter() and criticalSectionExit() respectively
 *       the macro ATOMIC_SECTION().
 * @see irqDisable
 */
STATIC inline ALWAYS_INLINE void _irqEnable( void )
{
#ifdef CONFIG_RTOS___
   irqSetEnableRegister( IRQ_IE );
#else
   irqSetEnableRegister( irqGetEnableRegister() | IRQ_IE );
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Global disabling of all interrupts.
 *        Counterpart of irqEnable()
 * @note CAUTION! Prefer the functions criticalSectionEnter()
 *       and criticalSectionExit() respectively the macro ATOMIC_SECTION()
 *       rather than this function in your application!
 * @see irqEnable
 */
STATIC inline ALWAYS_INLINE void irqDisable( void )
{
#ifndef CONFIG_DISABLE_CRITICAL_SECTION
 #ifdef CONFIG_RTOS
   irqSetEnableRegister( 0 );
 //  volatile const uint32_t im = irqGetMaskRegister();
 //  irqSetMaskRegister( 0 );
 //  irqSetEnableRegister( irqGetEnableRegister() & ~IRQ_IE );
 //  irqSetMaskRegister( im );
 #else
    irqSetEnableRegister( irqGetEnableRegister() & ~IRQ_IE );
 #endif
#else
 #warning "CAUTION: Critical sections disabled this could lead to uncontrolled program-behaviour!"
   NOP();
#endif
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Function shall be invoked immediately before a critical respectively
 *        atomic section begins.
 *
 * Counterpart to criticalSectionExit.
 *
 * @note Keep atomic sections as short as possible, otherwise the danger of
 *       jittering grows when using the real time OS FreeRTOS.
 *
 * @see criticalSectionExit
 */
void criticalSectionEnter( void );

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Backward compatibility
 */
STATIC inline ALWAYS_INLINE void atomic_on( void )
{
   criticalSectionEnter();
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Function shall be invoked immediately after the end of a critical
 *        respectively atomic section.
 *
 * Counterpart to criticalSectionEnter
 *
 * @note Keep atomic sections as short as possible, otherwise the danger of
 *       jittering grows when using the real time OS FreeRTOS.
 *
 * @see criticalSectionEnter
 */
void criticalSectionExit( void );

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Backward compatibility
 */
STATIC inline ALWAYS_INLINE void atomic_off( void )
{
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Helper function for macro ATOMIC_SECTION feeding the pseudo for-loop
 * @see ATOMIC_SECTION
 */
ALWAYS_INLINE
STATIC inline bool __criticalSectionEnter( void )
{
   criticalSectionEnter();
   return true;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Helper function for macro ATOMIC_SECTION feeding the pseudo for-loop
 * @see ATOMIC_SECTION
 */
ALWAYS_INLINE
STATIC inline bool __criticalSectionExit( void )
{
   criticalSectionExit();
   return false;
}

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief Establishes a atomic respectively critical section between the
 *        following enclosing curly braces.
 *
 * @note <b>CAUTION:</b> Do not use the keywords "break" or "return" within
 *       the atomic body! Its a for-loop!\n
 * @note Nested atomic sections are possible.\n
 * @note Keep atomic sections as short as possible, otherwise the danger of
 *       jittering grows when using the real time OS FreeRTOS.
 *
 * Example:
 * @code
 * ATOMIC_SECTION()
 * { // Atomic body
 *    foo();
 *    bar();
 * }
 * //...
 * ATOMIC_SECTION() baz();
 * @endcode
 */
#define ATOMIC_SECTION()                             \
   for( bool __c__ = __criticalSectionEnter();       \
        __c__;                                       \
        __c__ = __criticalSectionExit() )

#if defined(__cplusplus ) || defined(__DOXYGEN__)
} /* namespace gsi */
} /* extern "C" */
namespace gsi
{

/*! ---------------------------------------------------------------------------
 * @ingroup ATOMIC
 * @brief An object of this class preforms a automatic call of
 *        criticalSectionExit() by the destructor when the scope of this
 *        object will left.
 *
 * @note For C++ only!
 * @see ATOMIC_SECTION
 *
 * Example:
 * @code
 * {
 *    gsi::AtomicSection myAtomicSection;
 *    foo();
 *    bar();
 * }
 * @endcode
 */
class AtomicSection
{
public:
   AtomicSection( void )
   {
      criticalSectionEnter();
   }

   ~AtomicSection( void )
   {
      criticalSectionExit();
   }
};

} /* namespace gsi */
#endif /* ifdef __cplusplus */
#endif /* ifndef _LM32INTERRUPTS_H */
/* ================================= EOF ====================================*/

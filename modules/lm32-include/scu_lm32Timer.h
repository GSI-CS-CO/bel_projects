/*!
 * @file   lm32Timer.h
 * @brief  Definitions for using the Micro32 (LM32) timer SCU specific only.
 *
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/Lm32IRQTimer
 *
 * @note Header only!
 * @note This module is suitable for FreeRTOS-port.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      27.02.2020
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
#ifndef _SCU_LM32TIMER_H
#define _SCU_LM32TIMER_H

#include <mini_sdb.h>
#include <scu_lm32_macros.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
namespace gsi
{
#endif

/*!
 * @defgroup SCU_LM32_TIMER Very simple timer for LM32 in the SCU.
 * @note This module is suitable for FreeRTOS-port.
 */

/*!
 * @ingroup INTERRUPT
 * @ingroup SCU_LM32_TIMER
 * @brief Interrupt number of timer.
 */
#define TIMER_IRQ 1

/*!
 * @ingroup SCU_LM32_TIMER
 * @brief Second argument of "find_device_adr( GSI, WB_TIMER )"
 * @note First argument is "GSI"
 * @see find_device_adr
 */
#define WB_TIMER 0xD8BAAA13

/*!
 * @ingroup SCU_LM32_TIMER
 * @brief Registers of the SCU_LM32_TIMER Timer.
 */
typedef struct HW_IMAGE
{
   /*!
    * @brief Control register, setting to 1 enables the timer.
    */
   volatile uint32_t control;

   /*!
    * @brief Initializing value of the timers count down register.
    */
   volatile uint32_t period;
} SCU_LM32_TIMER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( SCU_LM32_TIMER_T, control ) == 0 * sizeof(uint32_t) );
STATIC_ASSERT( offsetof( SCU_LM32_TIMER_T, period  ) == 1 * sizeof(uint32_t) );
STATIC_ASSERT( sizeof( SCU_LM32_TIMER_T ) == 2 * sizeof( uint32_t ) );
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup PATCH
 * @ingroup SCU_LM32_TIMER
 * @brief Accomplishes a access to the timer registers.
 * @param p Pointer of type SCU_LM32_TIMER_T* base address of timer.
 * @param m Register name respectively name of member variable.
 */
#define TIMER_ACCESS( p, m ) __WB_ACCESS( SCU_LM32_TIMER_T, uint32_t, p, m )

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Returns the wishbone address of the timer macro.
 * @retval ERROR_NOT_FOUND Timer macro not found else pointer to the timers
 *         register set.
 */
STATIC inline SCU_LM32_TIMER_T* lm32TimerGetWbAddress( void )
{
   return (SCU_LM32_TIMER_T*) find_device_adr( GSI, WB_TIMER );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Enables the countdown of the timer.
 * @param pTimer Wishbone start address of timer register set.
 */
STATIC inline void lm32TimerEnable( SCU_LM32_TIMER_T* pTimer )
{
   TIMER_ACCESS( pTimer, control ) = 1;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Disables the timer.
 * @param pTimer Wishbone start address of timer register set.
 */
STATIC inline void lm32TimerDisable( SCU_LM32_TIMER_T* pTimer )
{
   TIMER_ACCESS( pTimer, control ) = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Returns true when the timer is enabled.
 * @param pTimer Wishbone start address of timer register set.
 */
STATIC inline bool lm32TimerIsEnabled( SCU_LM32_TIMER_T* pTimer )
{
   return TIMER_ACCESS( pTimer, control ) != 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Sets the duration of interrupts in CPU-clock cycles.
 * @note After this function call the timer is disabled!
 * @param pTimer Wishbone start address of timer register set.
 * @param period Initializing value for countdown, respectively
 *              duration of interrupt periods in CPU clock cycles.
 */
STATIC inline void lm32TimerSetPeriod( SCU_LM32_TIMER_T* pTimer,
                                       const uint32_t period )
{
   lm32TimerDisable( pTimer );
   TIMER_ACCESS( pTimer, period ) = period;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_LM32_TIMER
 * @brief Returns the by lm32TimerSetPeriod() set period.
 * @param pTimer Wishbone start address of timer register set.
 * @return Adjusted period.
 */
STATIC inline uint32_t lm32TimerGetPeriod( SCU_LM32_TIMER_T* pTimer )
{
   return TIMER_ACCESS( pTimer, period );
}

#ifdef __cplusplus
} /* namespace gsi */
} /* extern "C" */
#endif

#endif /* ifndef _SCU_LM32TIMER_H */
/*================================== EOF ====================================*/

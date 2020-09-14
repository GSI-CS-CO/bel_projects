/*!
 * @file   lm32Timer.h
 * @brief  Definitions for using the Micro32 (LM32) timer
 *
 * @see LatticeMicoTimer31.pdf
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
#ifndef _LM32TIMER_H
#define _LM32TIMER_H

#include <helper_macros.h>
#include <stdint.h>

/*!
 * @brief When the internal counter reaches zero, the timeout (TO) bit
 *        is set to 1.
 *
 * After it has been set, the TO bit remains set until it is cleared
 * by a master component. You can clear the TO bit by writing zero to
 * the status register.
 */
#define TIMER_STATUS_TO_BIT_MASK   (1 << 0)

/*!
 * @brief  When the internal counter is running, the RUN bit is read as 1.
 *
 * When the internal counter is not running, it is read as 0. A write
 * operation to the status register does not change the RUN bit.
 */
#define TIMER_STATUS_RUN_BIT_MASK  (1 << 1)


/*!
 * @brief Specifies the interrupt enable signal.
 *
 * It is active high. The default is 0. Write 1 to enable interrupt requests
 * and 0 to disable them.
 */
#define TIMER_CONTROL_INT_BIT_MASK   (1 << 0)

/*!
 * @brief The continuous (CONT) bit determines how the internal counter
 *        behaves when it reaches zero.
 *
 * If the CONT bit is 1, the counter
 * will keep running until it is stopped by the STOP bit. If the CONT
 * bit is 0, the counter will stop when it reaches zero. When it
 * reaches zero, the counter reloads with the 32-bit value stored in
 * the period register, regardless of the CONT bit. When
 * START_STOP_CONTROL is turned off, the timer keeps running
 * and is not affected by the value of this bit.
 */
#define TIMER_CONTROL_CONT_BIT_MASK  (1 << 1)

/*!
 * @brief The START bit enables the counter when a write operation is
 *        performed.
 *
 * Writing a 1 to the START bit causes the internal counter to begin
 * counting down. If the timer is stopped before reaching zero,
 * writing a 1 to the START bit will cause the timer to restart counting
 * from the number currently held in its counter.\n
 * If the timer is already running, writing a value to START will have
 * no effect.
 */
#define TIMER_CONTROL_START_BIT_MASK (1 << 2)

/*!
 * @brief The STOP bit disables the counter when a write operation is
 *        performed.
 *
 * Writing a 1 to the STOP bit causes the internal counter to stop.
 * The STOP bit has no effect if the timer is already stopped,
 * if a 0 is written to the STOP bit, or if the timer
 * hardware has been configured with the START_STOP_CONTROL
 * bit option turned off.
 * An undefined result is produced when a 1 is written to both the START and
 * STOP bits simultaneously.
 */
#define TIMER_CONTROL_STOP_BIT_MASK  (1 << 3)

#define TIMER_BASE_ADDRESS 0xA0000600 // TODO not the correct address yet!

#define TIMER_IRQ 1   //TODO not the correct number yet!

/*!
 * @brief Registers of the LatticeMocro Timer.
 * @see LatticeMicoTimer31.pdf
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Status register
    * @see TIMER_STATUS_TO_BIT_MASK
    * @see TIMER_STATUS_RUN_BIT_MASK
    */
   volatile uint32_t status;

   /*!
    * @brief Timer control register.
    * @see TIMER_CONTROL_INT_BIT_MASK
    * @see TIMER_CONTROL_CONT_BIT_MASK
    * @see TIMER_CONTROL_START_BIT_MASK
    * @see TIMER_CONTROL_STOP_BIT_MASK
    */
   volatile uint32_t control;

   /*!
    * @brief Period register
    *
    * Controls the period register and the width of the internal counter.
    * The period width is the width of the adder-subtractor used
    * to implement the counter.
    * The internal counter is loaded with the period_width-bit value stored
    * in the period register whenever a write operation to the
    * period register occurs or the internal counter reaches zero.
    */
   volatile uint32_t period;

   /*!
    * @brief Specifies the snapshot value of the internal counter.
    *
    * The internal counter is loaded with the period_width-bit value stored
    * in the period register whenever a write operation to the
    * period register occurs or the internal counter reaches zero.
    */
   volatile uint32_t snapshot;
} LM32_TIMER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( LM32_TIMER_T ) == 4 * sizeof( uint32_t ) );
#endif

#endif /* ifndef _LM32TIMER_H */
/*================================== EOF ====================================*/

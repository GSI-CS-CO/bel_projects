/*!
 * @file   lm32Timer.h
 * @brief  Definitions for using the Micrl32 (LM32) timer
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

#define TIMER_CONTROL_INT_BIT_MASK   (1 << 0)
#define TIMER_CONTROL_CONT_BIT_MASK  (1 << 1)
#define TIMER_CONTROL_START_BIT_MASK (1 << 2)
#define TIMER_CONTROL_STOP_BIT_MASK  (1 << 3)

#define TIMER_BASE_ADDRESS 0xA0000600 // TODO not the correct address yet!

#define TIMER_IRQ 1   //TODO not the correct number yet!

typedef struct PACKED_SIZE
{
   volatile uint32_t status;
   volatile uint32_t control;
   volatile uint32_t period;
   volatile uint32_t napshot;
} LM32_TIMER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( LM32_TIMER_T ) == 4 * sizeof( uint32_t ) );
#endif

#endif /* ifndef _LM32TIMER_H */
/*================================== EOF ====================================*/

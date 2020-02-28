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

/*!
 * @defgroup INTERRUPT Interrupt administration of LM32
 */

/*!
 * @ingroup INTERRUPT
 * @brief Maximum number of possible interrupt sources.
 */
#define MAX_LM32_INTERRUPTS 32

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
 *
 * @retval false Success
 * @retval true Wrong interrupt number
 */ 
bool registerISR( const unsigned int intNum, void* pContext,
                  ISRCallback Callback );

/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Disables a specific interrupt
 * 
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @retval false Success
 * @retval true Wrong interrupt number
 */
bool disableSpecificInterrupt( const unsigned int intNum );


/*! ---------------------------------------------------------------------------
 * @ingroup INTERRUPT
 * @brief Enables a specific interrupt
 *
 * @param intNum Interrupt line number that your component is
 *               connected to (0 to 31).
 * @retval false Success
 * @retval true Wrong interrupt number
 */
bool enableSpecificInterrupt( const unsigned int intNum );


#ifdef __cplusplus
}
#endif

#endif
/* ================================= EOF ====================================*/

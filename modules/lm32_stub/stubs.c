/*!
 * @brief Weak function definitions for the case if they
 *        won't use in the project but the linker expect them. \n
 *        They could be overwritten if necessary.
 * @file  stups.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#ifndef __lm32__
  #error This module is for the target Latice micro32 (LM32) only!
#endif
#include <stdint.h>
#include "helper_macros.h"

/*! --------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function becomes invoked immediately before the function main().
 * @see crt0ScuLm32.S
 */
void OVERRIDE __init( void )
{
}

/*! --------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function becomes invoked immediately after the function main().
 * @see crt0ScuLm32.S
 */
void OVERRIDE __exit( void )
{
}

/*! --------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function becomes invoked when a interrupt appears.
 * @see crt0ScuLm32.S
 */
void OVERRIDE _irq_entry(void)
{
}

/*! --------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function becomes invoked for all LM32 exceptions except the
 *        interrupt and system-call.
 * @see crt0ScuLm32.S
 */
void OVERRIDE _onException( const uint32_t sig )
{
}

/*! --------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function
 * @param sp Actual value of stack-pointer.
 * @see crt0ScuLm32.S
 */
void OVERRIDE _onSysCall( const uint32_t sp )
{
}

#ifdef CONFIG_CPLUSPLUS_MODULE_PRESENT
/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Dummy function for the case a virtual function pointer is not filled.
 *
 * The linker expect this if abstract C++ classes will used. \n
 * Workaround not nice, I know... :-/
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 24.01.2019
 */
void OVERRIDE __cxa_pure_virtual( void )
{
}

#endif /* ifdef CONFIG_CPLUSPLUS_MODULE_PRESENT */

#if defined(__SSP__) || defined(__SSP_ALL__) || defined(__SSP_STRONG__)

/*! ---------------------------------------------------------------------------
 * @ingroup OVERWRITABLE
 * @brief Rewritable function becomes invoked in the case of stack overflow.
 *        "Stack Smashing Protector" (SSP)
 */
void OVERRIDE __stack_chk_fail( void )
{
}

/*!
 * @ingroup OVERWRITABLE
 * @brief Will used for the stack smashing protector (SSP).
 */
uintptr_t OVERRIDE __stack_chk_guard;

#endif /* defined(__SSP__) || defined(__SSP_ALL__) */
/*================================== EOF ====================================*/

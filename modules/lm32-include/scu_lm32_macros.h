/*!
 *
 * @brief     Some additional helpful macro definitions specific for SCU LM32
 *
 *            Extension of file helper_macros.h
 *
 * @file      scu_lm32_macros.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      04.01.2019
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
 *******************************************************************************
 */
#ifndef _SCU_LM32_MACROS_H
#define _SCU_LM32_MACROS_H
#ifndef __lm32__
   #error This header file is specific for LM32 targets in GSI-SCU only!
#endif

#include <helper_macros.h>

/*! ---------------------------------------------------------------------------
 * @brief Modifier macro forces data variables declared by this macro in
 *        to the shared memory area.
 *
 * @note For a clean design this macro should only appear once in the project!
 */
#define SHARED __attribute__((section(".shared")))

#endif /* ifndef _SCU_LM32_MACROS_H */
/*================================== EOF ====================================*/

/*!
 * @file scu_mmu_tag.h
 * @brief Definition of memory-block identifier (tag) for
 *        Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note Header only
 * @note This source code is suitable for LM32 and Linux.
 * 
 * @see       scu_mmu.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      20.04.2022
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
#ifndef _SCU_MMU_TAG_H
#define _SCU_MMU_TAG_H
#include <scu_mmu.h>

#ifdef __cplusplus
namespace Scu
{
namespace mmu
{
#endif

/*!
 * @ingroup SCU_MMU
 * @brief Memory block identifier for ADDAC-DAQ
 */
STATIC const MMU_TAG_T TAG_ADDAC_DAQ = 0xFF01;

/*!
 * @ingroup SCU_MMU
 * @brief Memory block identifier for MIL-DAQ
 */
STATIC const MMU_TAG_T TAG_MIL_DAQ   = 0xFF02;

/*!
 * @ingroup SCU_MMU
 * @brief Memory block identifier for LM32-log messages.
 */
STATIC const MMU_TAG_T TAG_LM32_LOG  = 0xFC01;

#ifdef __cplusplus
} /* namespace mmu */
} /* namespace Scu */
#endif
#endif /* ifndef _SCU_MMU_TAG_H */
/*================================== EOF ====================================*/

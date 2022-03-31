/*!
 * @file scu_mmu_lm32.h
 * @brief LM32 part of Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note This source code is suitable for LM32 ony.
 * 
 * @see       scu_mmu.h
 * @see       scu_mmu_lm32.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      31.03.2022
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
#ifndef _SCU_MMU_LM32_H
#define _SCU_MMU_LM32_H
#ifndef __lm32__
  #error This module is for LM32 only! 
#endif

#include <scu_mmu.h>

#define MMU_ASSERT DDR_ASSERT

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace mmu
{
#endif

typedef DDR3_T MMU_OBJ_T;
   
MMU_OBJ_T* mmuGetObject( void );
   
MMU_STATUS_T mmuInit( MMU_OBJ_T* pMuObj );

#ifdef __cplusplus
} /* namespace mmu */
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* ifndef _SCU_MMU_LM32_H */
/*================================== EOF ====================================*/

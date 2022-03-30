/*!
 * @file scu_mmu.c
 * @brief Memory Management Unit of SCU
 * 
 * Administration of the shared memory (for SCU3 using DDR3) between 
 * Linux host and LM32 application.
 * 
 * @note This source code is suitable for LM32 and Linux.
 * 
 * @see       scu_mmu.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      30.03.2022
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
#include <scu_mmu.h>

/*!
 * @brief Identifier for the bigin of the patition list.
 */
const uint32_t MMU_MAGIC = 0xAAFF0055;

/*!
 * @brief Start index of the first partition list item.
 */
const MMU_ADDR_T MMU_LIST_START = 0;

#ifdef CONFIG_SCU_USE_DDR3
const MMU_ADDR_T MMU_MAX_INDEX = DDR3_MAX_INDEX64;
#endif

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
bool mmuIsPresent( void )
{
   RAM_PAYLOAD_T probe;

   mmuRead( MMU_LIST_START, &probe, 1 );
   return ( probe.ad32[0] == MMU_MAGIC );
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
unsigned int mmuGetNumberOfBlocks( void )
{
   return 0; //TODO
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
MMU_STATUS_T mmuAlloc( MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t len )
{
   return OK; //TODO
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
MMU_STATUS_T mmuGet( MMU_TAG_T tag, MMU_ADDR_T* pStartAddr, size_t* pLen )
{
   return OK; //TODO
}


/*================================== EOF ====================================*/

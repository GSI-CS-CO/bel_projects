/*!
 * @brief     Some helper functions for ease using shared memory in LM32.
 *
 * @file      shared_memory_helper.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      08.01.2019
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
#include <scu_assert.h>
#include <helper_macros.h>
#include <shared_memory_helper.h>

/*! ---------------------------------------------------------------------------
 * @see shared_memory_helper.h
 */
uint32_t* shmGetRelatedEtherBoneAddress( const uint32_t sharedOffset )
{
   uint32_t idx;
   sdb_location aFoundSdb[10]; //! @todo Check array size!
   sdb_location foundClu;
   const unsigned int cpuId = getCpuIdx();
   mprintf( "%d\n", cpuId );

   idx = 0;
   find_device_multi( &foundClu, &idx, 1, GSI, LM32_CB_CLUSTER );
   idx = 0;

   find_device_multi_in_subtree( &foundClu, aFoundSdb, &idx,
                                 ARRAY_SIZE(aFoundSdb), GSI, LM32_RAM_USER);
   if( idx < cpuId )
      return NULL;

   SCU_ASSERT( cpuId < ARRAY_SIZE(aFoundSdb) );
   return (uint32_t*)(getSdbAdr(&aFoundSdb[cpuId]) + sharedOffset
                                                   - LM32_INTERN_OFFSET);
}

/*================================== EOF ====================================*/

/*!
 * @brief     Some helper functions for ease using shared memory in LM32.
 *
 * @file      shared_memory_helper.h
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
#ifndef _SHARED_MEMORY_HELPER_H
#define _SHARED_MEMORY_HELPER_H
#include "mini_sdb.h"
//#include "aux.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t getCpuID( void )  { return *pCpuId; }
static inline uint32_t getCpuIdx( void ) { return getCpuID() & 0xff; }

#define LM32_INTERN_OFFSET 0x80000000

/*! ---------------------------------------------------------------------------
 * @brief Gets the pointer to shared memory for
 *        external perspective from host bridge.
 * @retval !=NULL Base pointer for external access via host
 * @retval ==NULL Error
 */
uint32_t* shmGetRelatedEtherBoneAddress( const uint32_t sharedOffset );

#ifdef __cplusplus
}
#endif

#endif
/*================================== EOF ====================================*/

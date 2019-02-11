/*!
 *  @file scu_ramBuffer.c
 *  @brief Abstraction layer for handling RAM buffer for DAQ data blocks.
 *
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 07.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#include <scu_ramBuffer.h>

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
RAM_RING_INDEX_T ramRingGetSize( RAM_RING_INDEXES_T* pThis )
{
   if( pThis->end == pThis->capacity) /* Is ring-buffer full? */
      return pThis->capacity;
   if( pThis->end >= pThis->start )
      return pThis->end - pThis->start;
   mprintf( "***\n" );
   return (pThis->capacity - pThis->start) + pThis->end;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetRemainingCapacity( pThis ) >= toAdd );
   RAM_ASSERT( pThis->end < pThis->capacity );

   pThis->end = (pThis->end + toAdd) % pThis->capacity;

   if( pThis->end == pThis->start )
      pThis->end = pThis->capacity; /* Ring buffer is full. */
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetSize( pThis ) >= toAdd );

   if( (toAdd != 0) && (pThis->end == pThis->capacity) )  /* Is ring-buffer full? */
      pThis->end = pThis->start;

   pThis->start = (pThis->start + toAdd) % pThis->capacity;
}

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
int ramInit( register RAM_SCU_T* pThis, RAM_RING_INDEXES_T* pRingIndexes )
{
   pThis->pRingIndexes = pRingIndexes;
#ifdef CONFIG_SCU_USE_DDR3
   return ddr3init( &pThis->ddr3 );
#endif
}

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! ---------------------------------------------------------------------------
 * @see scu_ramBuffer.h
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel )
{
   RAM_ASSERT( pThis != NULL );
   RAM_ASSERT( pDaqChannel != NULL );
   RAM_ASSERT( daqChannelGetDaqFifoWords( pDaqChannel ) > DAQ_DISCRIPTOR_WORD_SIZE );
   return 0;
}

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */

/*================================== EOF ====================================*/

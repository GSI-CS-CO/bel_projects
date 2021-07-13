/*!
 * @file   circular_index.c
 * @brief  Administration of memory read- and write indexes for circular
 *         buffers resp. ring buffers and FiFos.
 *
 * @note Suitable for LM32 and Linux.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      17.08.2020
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

#include <circular_index.h>

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
RAM_RING_INDEX_T ramRingGetSize( const RAM_RING_INDEXES_T* pThis )
{
   if( pThis->end == pThis->capacity ) /* Is ring-buffer full? */
      return pThis->capacity;

   if( pThis->end >= pThis->start )
      return pThis->end - pThis->start;

   return (pThis->capacity - pThis->start) + pThis->end;
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetRemainingCapacity( pThis ) >= toAdd );
   RAM_ASSERT( pThis->end < pThis->capacity );

   if( toAdd == 0 )
      return;

   pThis->end = (pThis->end + toAdd) % pThis->capacity;

   if( pThis->end == pThis->start ) /* Is buffer full? */
   { /*
      * To distinguish between buffer empty and full,
      * in the case of full the write index will set to a value out of
      * valid range. Here: the maximum capacity.
      */
      pThis->end = pThis->capacity;
   }
}

/*! ---------------------------------------------------------------------------
 * @see circular_index.h
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd )
{
   RAM_ASSERT( ramRingGetSize( pThis ) >= toAdd );

   if( toAdd == 0 )
      return;

   /* Was ring-buffer full? */
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * Here the buffer is still full and write index will set back to a valid
      * value.
      */
      pThis->end = pThis->start;
   }
   pThis->start = (pThis->start + toAdd) % pThis->capacity;
}

#ifdef CONFIG_CIRCULAR_DEBUG
/*! ---------------------------------------------------------------------------
 * @brief Prints the values of the members of RAM_RING_INDEXES_T
 */
#ifndef __lm32__
  #include  <stdio.h>
  #define mprintf printf
#endif
void ramRingDbgPrintIndexes( const RAM_RING_INDEXES_T* pThis,
                                                             const char* txt )
{
   if( txt != NULL )
     mprintf( "DBG: %s\n", txt );
   mprintf( "  DBG: offset:   %d\n"
            "  DBG: capacity: %d\n"
            "  DBG: start:    %d\n"
            "  DBG: end:      %d\n"
            "  DBG: used:     %d\n"
            "  DBG: free      %d\n\n",
            pThis->offset,
            pThis->capacity,
            pThis->start,
            pThis->end,
            ramRingGetSize( pThis ),
            ramRingGetRemainingCapacity( pThis )
          );
}
#endif

/*================================== EOF ====================================*/

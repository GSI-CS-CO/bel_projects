/*!
 * @file sw_queue.c
 * @brief Simple software queue respectively software fifo for small devices.
 * @see sw_queue.h
 * @date 30.04.2021
 * @copyright (C) 2021 GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * 
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
#include <string.h>
#include <sw_queue.h>
#if defined(__lm32__) || defined(__DOXYGEN__)
  #include <lm32Interrupts.h>
#endif


/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
void queueCreateOffset( SW_QUEUE_T* pThis,
                        uint8_t* pBuffer,
                        const unsigned int offset,
                        const size_t itemSize, 
                        const unsigned int capacity )
{
   pThis->pBuffer = pBuffer;
   pThis->indexes.offset = offset;
   pThis->itemSize = itemSize;
   pThis->indexes.capacity = capacity;
   queueReset( pThis );
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePush( SW_QUEUE_T* pThis, const void* pItem )
{
   if( queueIsFull( pThis ) )
      return false;

   memcpy( &pThis->pBuffer[ramRingGetWriteIndex(&pThis->indexes) * pThis->itemSize],
           pItem, pThis->itemSize );
   ramRingIncWriteIndex( &pThis->indexes );

   return true;
}

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePushSave( SW_QUEUE_T* pThis, const void* pItem )
{
   bool ret;

   criticalSectionEnter();
   ret = queuePush( pThis, pItem );
   criticalSectionExit();

   return ret;
}
#endif

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePop( SW_QUEUE_T* pThis, void* pItem )
{
   if( queueIsEmpty( pThis ) )
      return false;

   memcpy( pItem, 
           &pThis->pBuffer[ramRingGetReadIndex(&pThis->indexes) * pThis->itemSize],
           pThis->itemSize );
   ramRingIncReadIndex( &pThis->indexes );

   return true;
}

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
void queueResetSave( SW_QUEUE_T* pThis )
{
   criticalSectionEnter();
   queueReset( pThis );
   criticalSectionExit();
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
bool queuePopSave( SW_QUEUE_T* pThis, void* pItem )
{
   bool ret;

   criticalSectionEnter();
   ret = queuePop( pThis, pItem );
   criticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
unsigned int queueGetSizeSave( SW_QUEUE_T* pThis )
{
   unsigned int ret;

   criticalSectionEnter();
   ret = queueGetSize( pThis );
   criticalSectionExit();

   return ret;
}

/*! ---------------------------------------------------------------------------
 * @see sw_queue.h
 */
unsigned int queueGetRemainingCapacitySave( SW_QUEUE_T* pThis )
{
   unsigned int ret;

   criticalSectionEnter();
   ret = queueGetRemainingCapacity( pThis );
   criticalSectionExit();

   return ret;
}

#endif /* ifdef __lm32__ */

/*================================== EOF ====================================*/

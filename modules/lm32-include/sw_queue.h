/*!
 * @file sw_queue.h
 * @brief Simple software queue respectively software fifo for small devices.
 * @see sw_queue.c
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
#ifndef _SW_QUEUE_H
#define _SW_QUEUE_H

#include <circular_index.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*! ---------------------------------------------------------------------------
 * @brief Administration object type for a software queue.
 */
typedef struct
{  /*!
    * @brief Administration of buffer write and read indexes and capacity.
    */
   RAM_RING_INDEXES_T indexes;

   /*!
    * @brief Size in bytes of a single element in the queue.
    */
   size_t             itemSize;

   /*!
    * @brief Pointer to the reserved memory area in bytes for this object.
    * @note CAUTION! Be sure that the reserved memory area are big enough!
    */
   uint8_t*           pBuffer;
} SW_QUEUE_T;

/*! ---------------------------------------------------------------------------
 * @brief Creates and initialized a software queue with a offset in
 *        itemSize- units.
 * @param pThis Pointer to the concerned queue object. 
 * @param pBuffer Pointer to the reserved memory area in bytes.
 * @param offset Offset in (bytes * itenSize) units in reserved memory area
 *               of pBuffer.
 * @param itemSize Size in bytes for a single queue item.
 * @param capacity Maximum number of queue items.
 */
void queueCreateOffset( SW_QUEUE_T* pThis,
                        uint8_t* pBuffer,
                        const unsigned int offset,
                        const size_t itemSize, 
                        const unsigned int capacity );

/*! ---------------------------------------------------------------------------
 * @brief Creates and initialized a software queue with a offset in
 *        itemSize- units.
 *
 * Example for making a queue with 42 items:
 * @code
 * typedef struct
 * {
 *    int a;
 *    int b;
 *    int c;
 * } MY_ITEM_T;
 * 
 * #define MY_CAPACITY 42
 * uint8_t myBuffer[MY_CAPACITY * sizeof(MY_ITEM_T)];
 * SW_QUEUE_T myQueue;
 * 
 * queueCreate( &myQueue, myBuffer, sizeof(MY_ITEM_T), MY_CAPACITY );
 * 
 * // Queue is ready to use. 
 * @endcode
 * @param pThis Pointer to the concerned queue object. 
 * @param pBuffer Pointer to the reserved memory area in bytes.
 * @param itemSize Size in bytes for a single queue item.
 * @param capacity Maximum number of queue items.
 */
ALWAYS_INLINE STATIC inline
void queueCreate( SW_QUEUE_T* pThis,
                  uint8_t* pBuffer,
                  const size_t itemSize, 
                  const unsigned int capacity )
{
   queueCreateOffset( pThis, pBuffer, 0, itemSize, capacity );
}

/*! ---------------------------------------------------------------------------
 * @brief Clears the queue.
 * @param pThis Pointer to the concerned queue object. 
 */
ALWAYS_INLINE STATIC inline
void queueReset(  SW_QUEUE_T* pThis )
{
   return ramRingReset( &pThis->indexes );
}

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Clears the queue within a atomic section.
 * @param pThis Pointer to the concerned queue object. 
 */
void queueResetSave(  SW_QUEUE_T* pThis );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Write a item in the queue if there is still enough space.
 *        Counterpart to queuePop.
 * @see queuePop
 * @param pThis Pointer to the concerned queue object.
 * @param pItem Pointer to the item to be written.
 * @retval true Action was successful.
 * @retval false Queue already full, item discarded.
 */
bool queuePush( SW_QUEUE_T* pThis, const void* pItem );

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Write a item in the queue within a atomic section if there is still
 *        enough space.\n
 *        Counterpart to queuePop.
 * @see queuePopSave
 * @param pThis Pointer to the concerned queue object.
 * @param pItem Pointer to the item to be written.
 * @retval true Action was successful.
 * @retval false Queue already full, item discarded.
 */
bool queuePushSave( SW_QUEUE_T* pThis, const void* pItem );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Copies the oldest item of the queue, and remove it.
 *        counterpart to queuePush().
 * @see queuePush
 * @param pThis Pointer to the concerned queue object. 
 * @param pItem Destination pointer in which will copied.
 */
bool queuePop( SW_QUEUE_T* pThis, void* pItem );

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Copies the oldest item of the queue, and remove it within a
 *        atomic section.\n
 *        counterpart to queuePushSave().
 * @see queuePushSave
 * @param pThis Pointer to the concerned queue object. 
 * @param pItem Destination pointer in which will copied.
 */
bool queuePopSave( SW_QUEUE_T* pThis, void* pItem );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items which are currently in the queue.
 * @param pThis Pointer to the concerned queue object.
 * @return Number of items in the queue.
 */
ALWAYS_INLINE STATIC inline
unsigned int queueGetSize( SW_QUEUE_T* pThis )
{
   return ramRingGetSize( &pThis->indexes );
}

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items which are currently in the queue within
 *        a atomic section.
 * @param pThis Pointer to the concerned queue object.
 * @return Number of items in the queue.
 */
unsigned int queueGetSizeSave( SW_QUEUE_T* pThis );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns true when the queue is empty.
 * @param pThis Pointer to the concerned queue object.
 * @retval true Queue is empty.
 * @retval false Queue is not empty. 
 */
ALWAYS_INLINE STATIC inline
bool queueIsEmpty( SW_QUEUE_T* pThis )
{
   return queueGetSize( pThis ) == 0;
}

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Returns true when the queue is empty within a atomic section.
 * @param pThis Pointer to the concerned queue object.
 * @retval true Queue is empty.
 * @retval false Queue is not empty. 
 */
ALWAYS_INLINE STATIC inline
bool queueIsEmptySave( SW_QUEUE_T* pThis )
{
   return queueGetSizeSave( pThis ) == 0;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items that still fit in the queue.
 * @param pThis Pointer to the concerned queue object.
 * @return Number of currently free places.
 */
ALWAYS_INLINE STATIC inline
unsigned int queueGetRemainingCapacity( SW_QUEUE_T* pThis )
{
   return ramRingGetRemainingCapacity( &pThis->indexes );
}

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items that still fit in the queue within
 *        a atomic section.
 * @param pThis Pointer to the concerned queue object.
 * @return Number of currently free places.
 */
unsigned int queueGetRemainingCapacitySave( SW_QUEUE_T* pThis );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the maximum capacity in items op this queue. 
 * @param pThis Pointer to the concerned queue object.
 * @return Maximum number of items.
 */
ALWAYS_INLINE STATIC inline
unsigned int queueGetMaxCapacity( SW_QUEUE_T* pThis )
{
   return pThis->indexes.capacity;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns true when the queue is full.
 * @param pThis Pointer to the concerned queue object.
 * @retval true Queue is full.
 * @retval false Queue is not full.
 */
ALWAYS_INLINE STATIC inline
bool queueIsFull( SW_QUEUE_T* pThis )
{
   return queueGetRemainingCapacity( pThis ) == 0;
}

#ifdef __lm32__
/*! ---------------------------------------------------------------------------
 * @brief Returns true when the queue is full within a atomic section.
 * @param pThis Pointer to the concerned queue object.
 * @retval true Queue is full.
 * @retval false Queue is not full.
 */
ALWAYS_INLINE STATIC inline
bool queueIsFullSave( SW_QUEUE_T* pThis )
{
   return queueGetRemainingCapacitySave( pThis ) == 0;
}
#endif

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif
#endif /* ifndef _SW_QUEUE_H */
/*================================== EOF ====================================*/

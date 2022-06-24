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
 * @brief Generates the memory buffer name from the given queue name.
 * @param qeueName Name of the queue.
 */
#define QUEUE_GET_MEM_NAME( qeueName ) __ ## qeueName ## _mem__

/*! ---------------------------------------------------------------------------
 * @brief Reservation of static memory for the queue containment.
 * @param name Name of the queue.
 * @param maxCapacity Maximum number of payload items.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_ALLOC_STATIC_MEM( name, maxCapacity, ITEM_TYP )                 \
   uint8_t QUEUE_GET_MEM_NAME( name )[ maxCapacity * sizeof(ITEM_TYP) ]

/*! ---------------------------------------------------------------------------
 * @brief Static initializer for a queue object.
 * @param name Name of the queue.
 * @param maxCapacity Maximum number of payload items.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_STATIC_INITIALIZER( name, maxCapacity, ITEM_TYP )               \
   {                                                                          \
      .indexes.offset   = 0,                                                  \
      .indexes.capacity = maxCapacity,                                        \
      .indexes.start    = 0,                                                  \
      .indexes.end      = 0,                                                  \
      .itemSize         = sizeof(ITEM_TYP),                                   \
      .pBuffer          = QUEUE_GET_MEM_NAME( name )                          \
   }

/*! ---------------------------------------------------------------------------
 * @brief Initializer for queues as member object in a structure.
 * @see QUEUE_IMPLEMENT 
 * @param name Name of the queue.
 * @param maxCapacity Maximum number of payload items.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_STATIC_MEMBER_INITIALIZER( name, maxCapacity, ITEM_TYP )        \
   .name = QUEUE_STATIC_INITIALIZER( name,  maxCapacity, ITEM_TYP )

/*! ---------------------------------------------------------------------------
 * @brief Implements a queue object as member object of a structure.
 *
 * Example: Implementing and initializing a queue as member object of a
 *          structure with 42 payload items.
 * @code
 * typedef struct
 * {
 *    int a;
 *    int b;
 *    int c;
 * } MY_ITEM_T;
 *
 * typedef struct
 * {
 *    QUEUE_IMPLEMENT( myQueue, 42, MY_ITEM_T );
 *    int some;
 *    int others;
 * } MY_STRUCT_WITH_QUEUE_T;
 *
 * MY_STRUCT_WITH_QUEUE_T myStruct = 
 * {
 *    QUEUE_STATIC_MEMBER_INITIALIZER( myQueue, 42, ITEM_TYP ),
 *    some = 4711,
 *    others = 57 
 * };
 * @endcode
 * @param name Name of the queue to create.
 * @param maxCapacity Maximum number of payload items.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_IMPLEMENT( name, maxCapacity, ITEM_TYP )                        \
   QUEUE_ALLOC_STATIC_MEM( name, maxCapacity, ITEM_TYP );                     \
   SW_QUEUE_T name

/*! ---------------------------------------------------------------------------
 * @brief Creates a queue object in the .data memory segment.
 * 
 * @note CAUTION: This object becomes initialized during compile time. \n
 *                Therefore after a reset the object keeps the old values! \n
 *                Therefore don't forget to call the function queueReset()
 *                before the queue will used.
 * @see  queueReset
 * 
 * Example: Creating a queue with a capacity of 42 payload items.
 * @code
 * typedef struct
 * {
 *    int a;
 *    int b;
 *    int c;
 * } MY_ITEM_T;
 *
 * QUEUE_CREATE_STATIC( myQueue, 42, MY_ITEM_T );
 *
 * void main( void )
 * {
 *    queueReset( &myQueue );
 *    // Do something with this queue...
 * }
 * @endcode 
 * @param name Name of the queue to create.
 * @param maxCapacity Maximum number of payload items.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_CREATE_STATIC( name, maxCapacity, ITEM_TYP )                    \
   QUEUE_IMPLEMENT( name, maxCapacity, ITEM_TYP ) =                           \
      QUEUE_STATIC_INITIALIZER( name, maxCapacity, ITEM_TYP )

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
 * @brief Creates and initialized a software queue with a capacity in
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
 * @see QUEUE_INIT_MEMBER
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

#define QUEUE_CREATE( pThis, aBuffer, ITEM_TYP )       \
   queueCreate( pThis, aBuffer, sizeof(ITEM_TYP),     \
                sizeof(aBuffer) / sizeof(ITEM_TYP) )

/*! ---------------------------------------------------------------------------
 * @brief Runtime initialization when the queue is a member of a structure.
 * 
 * Macro invokes the function queueCreate by specific parameters.
 * @see queueCreate \n
 * Example:
 * @code
 * typedef struct
 * {
 *    int a;
 *    int b;
 *    int c;
 * } MY_ITEM_T;
 *
 * typedef struct
 * {
 *    QUEUE_IMPLEMENT( myQueue, 42, MY_ITEM_T );
 *    int some;
 *    int others;
 * } MY_STRUCT_WITH_QUEUE_T;
 *
 * void main( void )
 * {
 *    MY_STRUCT_WITH_QUEUE_T myStructWithQueue;
 *
 *    myStructWithQueue.some   = 4711;
 *    myStructWithQueue.others = -815;
 *    QUEUE_INIT_MEMBER( &myStructWithQueue, myQueue, MY_ITEM_T );
 *
 *    // Do something...
 * }
 * @endcode
 * @param parentPtr Pointer of the parent object which includes the queue.
 * @param qeueName Name of the queue member variable.
 * @param ITEM_TYP Data type of payload item.
 */
#define QUEUE_INIT_MEMBER( parentPtr, qeueName, ITEM_TYP )                    \
   queueCreate( &((parentPtr)->qeueName),                                     \
                (parentPtr)->QUEUE_GET_MEM_NAME( qeueName ),                  \
                sizeof( ITEM_TYP ),                                           \
                sizeof( (parentPtr)->QUEUE_GET_MEM_NAME( qeueName )) /        \
                     sizeof( ITEM_TYP ) )

/*! ---------------------------------------------------------------------------
 * @brief Clears the queue.
 * @param pThis Pointer to the concerned queue object. 
 */
ALWAYS_INLINE STATIC inline
void queueReset( SW_QUEUE_T* pThis )
{
   return ramRingReset( &pThis->indexes );
}

#if defined(__lm32__) || defined(__DOXYGEN__)
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

#if defined(__lm32__) || defined(__DOXYGEN__)
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
 * @retval true Data valid, at least one item was in queue.
 * @retval false Date invalid, queue was empty.
 */
bool queuePop( SW_QUEUE_T* pThis, void* pItem );

#if defined(__lm32__) || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Copies the oldest item of the queue, and remove it within a
 *        atomic section.\n
 *        counterpart to queuePushSave().
 * @see queuePushSave
 * @param pThis Pointer to the concerned queue object. 
 * @param pItem Destination pointer in which will copied.
 * @retval true Data valid, at least one item was in queue.
 * @retval false Date invalid, queue was empty.
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

#if defined(__lm32__) || defined(__DOXYGEN__)
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

#if defined(__lm32__) || defined(__DOXYGEN__)
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

#if defined(__lm32__) || defined(__DOXYGEN__)
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

#if defined(__lm32__) || defined(__DOXYGEN__)
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

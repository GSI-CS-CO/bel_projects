/*!
 * @file   circular_index.h
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
#ifndef _CIRCULAR_INDEX_H
#define _CIRCULAR_INDEX_H

#include <stdint.h>
#include <helper_macros.h>

#ifdef CONFIG_RAM_PEDANTIC_CHECK
   /* CAUTION:
    * Assert-macros could be expensive in memory consuming and the
    * latency time can increase as well!
    * Especially in embedded systems with small resources.
    * Therefore use them for bug-fixing or developing purposes only!
    */
   #include <scu_assert.h>
   #define RAM_ASSERT SCU_ASSERT
#else
   #define RAM_ASSERT(__e) ((void)0)
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

/*!
 * @brief Data type for read, write, offset and capacity for
 *        ring buffer access.
 */
typedef uint32_t RAM_RING_INDEX_T;

/*! ---------------------------------------------------------------------------
 * @brief Data type of ring buffer indexes.
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief offset in alignment units of physical memory
    */
   RAM_RING_INDEX_T offset;

   /*!
    * @brief Maximum capacity of ring- buffer
    */
   RAM_RING_INDEX_T capacity;

   /*!
    * @brief Read- index of ring buffer
    */
   RAM_RING_INDEX_T start;

   /*!
    * @brief Write- index of ring buffer
    */
   RAM_RING_INDEX_T end;
} RAM_RING_INDEXES_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(RAM_RING_INDEXES_T) == 4 * sizeof(RAM_RING_INDEX_T));
STATIC_ASSERT( offsetof( RAM_RING_INDEXES_T, start ) <
               offsetof( RAM_RING_INDEXES_T, end ));
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of currently used memory items
 * @param pThis Pointer to the ring index object
 * @return Actual number written items
 */
RAM_RING_INDEX_T ramRingGetSize( const RAM_RING_INDEXES_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Returns the remaining free items of the currently used memory
 * @param pThis Pointer to the ring index object
 * @return Number of free memory items.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetRemainingCapacity( const RAM_RING_INDEXES_T* pThis )
{
   return pThis->capacity - ramRingGetSize( pThis );
}

/*! ---------------------------------------------------------------------------
 * @brief Adds a value to the write index.
 * @param pThis Pointer to the ring index object
 * @param value to add to the write index.
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd );

/*!----------------------------------------------------------------------------
 * @brief Increments the write-index.
 * @param pThis Pointer to the ring index object
 */
STATIC inline
void ramRingIncWriteIndex( RAM_RING_INDEXES_T* pThis )
{
   ramRingAddToWriteIndex( pThis, 1 );
}

/*! ---------------------------------------------------------------------------
 * @brief Adds a value to the read index.
 * @param pThis Pointer to the ring index object
 * @param value to add to the read index.
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, const RAM_RING_INDEX_T toAdd );

/*! ---------------------------------------------------------------------------
 * @brief Increments the read-index.
 * @param pThis Pointer to the ring index object
 */
STATIC inline
void ramRingIncReadIndex( RAM_RING_INDEXES_T* pThis )
{
   ramRingAddToReadIndex( pThis, 1 );
}

/*! ---------------------------------------------------------------------------
 * @brief Resets respectively clears the ring buffer
 * @param pThis Pointer to the ring index object
 */
STATIC inline void ramRingReset( register RAM_RING_INDEXES_T* pThis )
{
   pThis->start = 0;
   pThis->end   = 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the current absolute read index for a read access to the
 *        physical memory.
 * @param pThis Pointer to the ring index object
 * @return Index value for read access.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetReadIndex( register RAM_RING_INDEXES_T* pThis )
{
   return pThis->start + pThis->offset;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items beginning at the read index until to the
 *        upper border  of the used memory buffer belonging to this object.
 *
 * Value range:  {1 <= return <= max capacity}
 *
 * @param pThis Pointer to the ring index object
 * @return Number of items which can read until the upper border of the buffer.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetUpperReadSize( register RAM_RING_INDEXES_T* pThis )
{
   RAM_ASSERT( pThis->capacity > 0 );
   return pThis->capacity - pThis->start;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of items beginning at the read index until to the
 *        upper border  of the used memory buffer belonging to this object.
 *
 * Value range:  {1 <= return <= max capacity}
 *
 * @param pThis Pointer to the ring index object
 * @return Number of items which can write until the upper border of the buffer.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetUpperWriteSize( register RAM_RING_INDEXES_T* pThis )
{
   RAM_ASSERT( pThis->capacity > 0 );
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * But in this case the read index has the correct value.
      */
      return ramRingGetUpperReadSize( pThis );
   }
   return pThis->capacity - pThis->end;
}


/*! ---------------------------------------------------------------------------
 * @brief Returns the current absolute write-index for a write access to the
 *        physical memory.
 * @param pThis Pointer to the ring index object
 * @return Index value for write access.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetWriteIndex( register RAM_RING_INDEXES_T* pThis )
{ /*
   * Is the buffer full?
   */
   if( pThis->end == pThis->capacity )
   { /*
      * In the case the buffer was full the write index has been set to a
      * invalid value (maximum capacity) to distinguishing between full
      * and empty.
      * But in this case the read index has the correct value.
      */
      return ramRingGetReadIndex( pThis );
   }
   return pThis->end + pThis->offset;
}

/*============= Index administration for shared memory ======================*/
/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Data type of ring buffer indexes for shared buffers between Linux
 *        and LM32.
 */
typedef struct PACKED_SIZE
{  /*!
    * @brief Administration of write and read index.
    */
   RAM_RING_INDEXES_T indexes;

   /*!
    * @brief Holds the number of memory items which has been read by the
    *        client.
    * 
    * This will also need for a handshaking transfer.
    */
   RAM_RING_INDEX_T   wasRead;
} RAM_RING_SHARED_INDEXES_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( RAM_RING_SHARED_INDEXES_T, indexes ) == 0 );
STATIC_ASSERT( offsetof( RAM_RING_SHARED_INDEXES_T, wasRead ) == sizeof( RAM_RING_INDEXES_T ));
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the number of currently used memory items
 * @param pThis Pointer to the shared ring indexes object.
 * @return Actual number written items
 */
STATIC inline
RAM_RING_INDEX_T ramRingSharedGetSize( const RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetSize( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the remaining free items of the currently used memory
 * @param pThis Pointer to the shared ring indexes object.
 * @return Number of free memory items.
 */
STATIC inline
RAM_RING_INDEX_T ramRingSharedGetRemainingCapacity( const RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetRemainingCapacity( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Adds a value to the write index.
 * @param pThis Pointer to the shared ring indexes object.
 * @param toAdd to add to the write index.
 */
STATIC inline
void ramRingSharedAddToWriteIndex( RAM_RING_SHARED_INDEXES_T* pThis,
                                   const RAM_RING_INDEX_T toAdd )
{
   ramRingAddToWriteIndex( &pThis->indexes, toAdd );
}

/*!----------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Increments the write-index.
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline
void ramRingSharedIncWriteIndex( RAM_RING_SHARED_INDEXES_T* pThis )
{
   ramRingIncWriteIndex( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Adds a value to the read index.
 * @param pThis Pointer to the shared ring indexes object.
 * @param toAdd to add to the read index.
 */
STATIC inline
void ramRingSharedAddToReadIndex( RAM_RING_SHARED_INDEXES_T* pThis,
                                  const RAM_RING_INDEX_T toAdd )
{
   ramRingAddToReadIndex( &pThis->indexes, toAdd );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Synchronizes the read index of the number of items which has been
 *        read by the client.
 * @note This shall be the job of the server only, to prevent possible
 *       race conditions.
 * 
 * In this way a handshaking transfer becomes possible.
 *
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline
void ramRingSharedSynchonizeReadIndex( RAM_RING_SHARED_INDEXES_T* pThis )
{
   ramRingSharedAddToReadIndex( pThis, pThis->wasRead );
   pThis->wasRead = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Increments the read-index.
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline
void ramRingSharedIncReadIndex( RAM_RING_SHARED_INDEXES_T* pThis )
{
   ramRingIncReadIndex( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Resets respectively clears the ring buffer
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline void ramRingSharedReset( RAM_RING_SHARED_INDEXES_T* pThis )
{
   ramRingReset( &pThis->indexes );
   pThis->wasRead = 0;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the current absolute read index for a read access to the
 *        physical memory.
 * @param pThis Pointer to the shared ring indexes object.
 * @return Index value for read access.
 */
STATIC inline 
RAM_RING_INDEX_T ramRingSharedGetReadIndex( RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetReadIndex( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the number of items beginning at the read index until to the
 *        upper border  of the used memory buffer belonging to this object.
 *
 * Value range:  {1 <= return <= max capacity}
 *
 * @param pThis Pointer to the shared ring indexes object.
 * @return Number of items which can read until the upper border of the buffer.
 */
STATIC inline
RAM_RING_INDEX_T ramRingSharedGetUpperReadSize( RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetUpperReadSize( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the number of items beginning at the read index until to the
 *        upper border  of the used memory buffer belonging to this object.
 *
 * Value range:  {1 <= return <= max capacity}
 *
 * @param pThis Pointer to the shared ring indexes object.
 * @return Number of items which can write until the upper border of the buffer.
 */
STATIC inline
RAM_RING_INDEX_T ramRingSharedGetUpperWriteSize( RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetUpperWriteSize( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the current absolute write-index for a write access to the
 *        physical memory.
 * @param pThis Pointer to the shared ring indexes object.
 * @return Index value for write access.
 */
STATIC inline 
RAM_RING_INDEX_T ramRingSharedGetWriteIndex( RAM_RING_SHARED_INDEXES_T* pThis )
{
   return ramRingGetWriteIndex( &pThis->indexes );
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Sets the number of items which was read by the client and which the
 *        server shall synchronize.
 * @param pThis Pointer to the shared ring indexes object.
 * @param wasRead Number of memory items which the client has been read.
 */
STATIC inline
void ramRingSharedSetWasRead( RAM_RING_SHARED_INDEXES_T* pThis, 
                              const RAM_RING_INDEX_T wasRead )
{
   pThis->wasRead = wasRead;
}

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Returns the number of memory items which has been read by the client.
 * @param pThis Pointer to the shared ring indexes object.
 * @return Number of memory items which the client has been read.
 */
STATIC inline
RAM_RING_INDEX_T ramRingSharedGetWasRead( const RAM_RING_SHARED_INDEXES_T* pThis )
{
   return pThis->wasRead;
}

#ifdef CONFIG_CIRCULAR_DEBUG
/*! ---------------------------------------------------------------------------
 * @brief Prints the values of the members of RAM_RING_INDEXES_T
 */
void ramRingDbgPrintIndexes( const RAM_RING_INDEXES_T* pThis,
                                                             const char* txt );
#else
#define ramRingDbgPrintIndexes( __a, __b )
#endif

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* ifndef _CIRCULAR_INDEX_H */
/*================================== EOF ====================================*/

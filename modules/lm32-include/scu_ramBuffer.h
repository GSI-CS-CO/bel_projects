/*!
 *  @file scu_ramBuffer.h
 *  @brief Abstraction layer for handling RAM buffer for DAQ data blocks.
 *
 *  @see scu_ramBuffer.c
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
#ifndef _SCU_RAMBUFFER_H
#define _SCU_RAMBUFFER_H

#ifdef CONFIG_SCU_USE_DDR3
#include <scu_ddr3.h>
#else
#error Unknown memory type!
#endif

#ifdef __lm32__
#include <daq.h>
#endif

/*!
 * @defgroup SCU_RAM_BUFFER
 * @brief Abstraction layer for handling SCU RAM-Buffer
 *
 * @note At the moment its DDR3 only.
 * @see SCU_DDR3
 * @{
 */

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
#endif

#ifdef CONFIG_SCU_USE_DDR3

/*!
 * @defgroup SCU_RING_BUFFER_INDEXES
 * @brief Administration of write and read indexes for
 *        a ring-buffer.
 * @{
 */


#ifndef RAM_DAQ_MIN_INDEX
   /*!
    * @brief Minimum value for Ring read and write index
    */
   #define RAM_DAQ_MIN_INDEX 0
#endif
#ifndef RAM_DAQ_MAX_INDEX
   /*!
    * @brief Maximum value for Ring read and write index
    */
   #define RAM_DAQ_MAX_INDEX DDR3_MAX_INDEX64
#endif

#endif



typedef uint32_t RAM_RING_INDEX_T;

/*! ---------------------------------------------------------------------------
 * @brief Data type of fifo pointers.
 * @note The implementation has to be within the shared memory!
 *       They must be visible in the LM32 and in the Linux side.
 */
typedef struct PACKED_SIZE
{
   RAM_RING_INDEX_T offset;   /*!<@brief offset in alignment units of physical memory */
   RAM_RING_INDEX_T capacity; /*!<@brief Maximum capacity of ring- buffer */
   RAM_RING_INDEX_T start;    /*!<@brief Start index of ring buffer */
   RAM_RING_INDEX_T end;      /*!<@brief End- index of ring buffer */
} RAM_RING_INDEXES_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(RAM_RING_INDEXES_T) == 4 * sizeof(RAM_RING_INDEX_T));
#endif

/*!
 * @brief Initializer for object of data type RAM_RING_INDEXES_T
 */
#define RAM_RING_INDEXES_INITIALIZER                  \
{                                                     \
   .offset   = RAM_DAQ_MIN_INDEX,                     \
   .capacity = RAM_DAQ_MAX_INDEX - RAM_DAQ_MIN_INDEX, \
   .start    = 0,                                     \
   .end      = 0                                      \
}

/*!
 * @brief Generalized object type for SCU RAM buffer
 */
typedef struct
{
#ifdef CONFIG_SCU_USE_DDR3
   /*!
    * @brief SCU DDR3 administration object.
    */
   DDR3_T   ddr3;
#else
   //TODO maybe in the future will use a other memory type
#endif
   /*!
    * @brief Administration of fifo- indexes.
    * @note The memory space of this object has to be within the shared
    *       memory. \n
    *       Therefore its a pointer in this object.
    */
   RAM_RING_INDEXES_T* volatile pRingIndexes;
} RAM_SCU_T;

/*! ---------------------------------------------------------------------------
 * @brief Resets respectively clears the ring buffer
 * @param pThis Pointer to the ring index object
 */
static inline void ramRingReset( RAM_RING_INDEXES_T* pThis )
{
   pThis->start = 0;
   pThis->end   = 0;
}

#define RAM_RING_GET_CAPACITY() (RAM_DAQ_MAX_INDEX - RAM_DAQ_MIN_INDEX)

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of currently used memory items
 * @param pThis Pointer to the ring index object
 * @return Actual number written items
 */
RAM_RING_INDEX_T ramRingGetSize( RAM_RING_INDEXES_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Returns the remaining free items of the currently used memory
 * @param pThis Pointer to the ring index object
 * @return Number of free memory items.
 */
static inline
RAM_RING_INDEX_T ramRingGetRemainingCapacity( RAM_RING_INDEXES_T* pThis )
{
   return pThis->capacity - ramRingGetSize( pThis );
}

/*! ---------------------------------------------------------------------------
 * @brief Adds a value to the write index.
 * @param pThis Pointer to the ring index object
 * @param value to add to the write index.
 */
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd );

/*! ---------------------------------------------------------------------------
 * @brief Adds a value to the read index.
 * @param pThis Pointer to the ring index object
 * @param value to add to the read index.
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd );

/*! ---------------------------------------------------------------------------
 * @brief Gets the actual write-index for a write access to the physical
 *        memory.
 * @param pThis Pointer to the ring index object
 * @return Index value for write access.
 */
static inline
RAM_RING_INDEX_T ramRingGetWriteIndex( register RAM_RING_INDEXES_T* pThis )
{
   if( pThis->end == pThis->capacity )
      return pThis->start + pThis->offset;
   return pThis->end + pThis->offset;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the actual read index for a read access to the physical memory.
 * @param pThis Pointer to the ring index object
 * @return Index value for read access.
 */
static inline
RAM_RING_INDEX_T ramRingGeReadIndex( register RAM_RING_INDEXES_T* pThis )
{
   return pThis->start + pThis->offset;
}

/*! @} */ // End SCU_RING_BUFFER_INDEXES

/*! ---------------------------------------------------------------------------
 * @brief Initializing SCU RAM buffer ready to use.
 * @param pThis Pointer to the RAM object.
 * @param pRingIndexes Pointer to the fifo administration in shared memory.
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ramInit( register RAM_SCU_T* pThis, RAM_RING_INDEXES_T* pRingIndexes );

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! ----------------------------------------------------------------------------
 * @brief Exchanges the order of devicedeskriptor and payload so that the
 *        devicedescriptor appears at first of the given DAQ channel during
 *        writing in the ring buffer. \n
 *        If not enough free space in the ring buffer, so the oldest
 *        DAQ data blocks becomes deleted until its enough space.
 * @param pThis Pointer to the RAM object object.
 * @param pDaqChannel Pointer of the concerning DAQ-channel-object.
 * @return Number of deleted old data blocks.
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis, DAQ_CANNEL_T* pDaqChannel );

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */

/*! @} */ //End of group SCU_RAM_BUFFER
#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_RAMBUFFER_H */
/* ================================= EOF ====================================*/

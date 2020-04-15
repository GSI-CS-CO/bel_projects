/*!
 * @file daq_ring_admin.h
 * @brief Administration of the indexes for a ring-buffer.
 *
 * @note This module is suitable for LM32 and Linux
 * @see scu_ramBuffer.h
 *
 * @see scu_ddr3.h
 * @see scu_ddr3.c
 * @date 19.06.2019
 * @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
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
#ifndef _DAQ_RING_ADMIN_H
#define _DAQ_RING_ADMIN_H

#include <helper_macros.h>

#ifdef CONFIG_SCU_USE_DDR3
#include <scu_ddr3.h>
#else
#error Unknown memory type!
#endif

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace daq
{
#endif

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

/*!
 * @defgroup SCU_RING_BUFFER_INDEXES
 * @brief Administration of write and read indexes for
 *        a ring-buffer.
 * @note A problem in administration of ring- buffer indexes is,
 *       that read index and write index are equal in the case of
 *       buffer is empty and full! \n
 *       To distinguish this both cases the write index will set to the
 *       invalid value of the maximum capacity in the case when the
 *       buffer is full.
 * @{
 */

//#define RAM_SDAQ_MAX_INDEX 2816
#ifndef RAM_SDAQ_MAX_INDEX
   /*!
    * @brief Maximum value for ring buffer read and write index
    */
   #ifdef CONFIG_MIL_DAQ_USE_RAM
      #define RAM_SDAQ_MAX_INDEX (DDR3_MAX_INDEX64 / 2)
      #define RAM_MDAQ_MAX_INDEX DDR3_MAX_INDEX64
   #else
      #define RAM_SDAQ_MAX_INDEX DDR3_MAX_INDEX64
   #endif
#endif

#ifndef RAM_SDAQ_MIN_INDEX
   /*!
    * @brief Minimum value for ring buffer read and write index
    */
   #define RAM_SDAQ_MIN_INDEX 0
   #ifdef CONFIG_MIL_DAQ_USE_RAM
      #define RAM_MDAQ_MIN_INDEX (DDR3_MAX_INDEX64 / 2)
   #endif
#endif

/*!
 * @brief Maximum capacity of ring buffer in RAM_DAQ_PAYLOAD_T
 */
#define RAM_SDAQ_MAX_CAPACITY (RAM_SDAQ_MAX_INDEX - RAM_SDAQ_MIN_INDEX)

#ifdef CONFIG_MIL_DAQ_USE_RAM
  #define RAM_MDAQ_MAX_CAPACITY (RAM_MDAQ_MAX_INDEX - RAM_MDAQ_MIN_INDEX)
#endif

/*!
 * @brief Data type for read, write, offset and capacity for
 *        ring buffer access.
 */
typedef uint32_t RAM_RING_INDEX_T;

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Data type of ring buffer indexes.
 */
typedef struct PACKED_SIZE
{
   RAM_RING_INDEX_T offset;   /*!<@brief offset in alignment units of physical
                               *         memory */
   RAM_RING_INDEX_T capacity; /*!<@brief Maximum capacity of ring- buffer */
   RAM_RING_INDEX_T start;    /*!<@brief Start index of ring buffer */
   RAM_RING_INDEX_T end;      /*!<@brief End- index of ring buffer */
} RAM_RING_INDEXES_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(RAM_RING_INDEXES_T) == 4 * sizeof(RAM_RING_INDEX_T));
STATIC_ASSERT( offsetof( RAM_RING_INDEXES_T, start ) <
               offsetof( RAM_RING_INDEXES_T, end ));
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Initializer for object of data type RAM_RING_INDEXES_T
 */
#define RAM_RING_INDEXES_SDAQ_INITIALIZER                                     \
{                                                                             \
   .offset   = RAM_SDAQ_MIN_INDEX,                                            \
   .capacity = RAM_SDAQ_MAX_CAPACITY,                                         \
   .start    = 0,                                                             \
   .end      = 0                                                              \
}

#ifdef CONFIG_MIL_DAQ_USE_RAM
 #define RAM_RING_INDEXES_MDAQ_INITIALIZER                                    \
 {                                                                            \
    .offset   = RAM_MDAQ_MIN_INDEX,                                           \
    .capacity = RAM_MDAQ_MAX_CAPACITY,                                        \
    .start    = 0,                                                            \
    .end      = 0                                                             \
 }
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Data type for data residing in the shared memory for the
 *        communication between uC server and Linux client.
 * @note The implementation has to be within the shared memory!
 *       They must be visible in the LM32 and in the Linux side.
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Flag becomes 1 by the server if he has modified
    *        the ring indexes.
    *
    * To avoid partial register access by LM32, the size of the following
    * flag is equal to the LM32 register size.
    */
   uint32_t           serverHasWritten;
   uint32_t           ramAccessLock;
   RAM_RING_INDEXES_T ringIndexes;
} RAM_RING_SHARED_OBJECT_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(RAM_RING_SHARED_OBJECT_T) ==
               sizeof(RAM_RING_INDEXES_T) +
               2 * sizeof(uint32_t) );
STATIC_ASSERT( offsetof( RAM_RING_SHARED_OBJECT_T, ramAccessLock ) <
               offsetof( RAM_RING_SHARED_OBJECT_T, ringIndexes ));
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Initializer of the shared object for the communication between
 *        server and Linux client.
 */
#define RAM_RING_SHARED_SDAQ_OBJECT_INITIALIZER                               \
{                                                                             \
   .serverHasWritten = false,                                                 \
   .ramAccessLock    = false,                                                 \
   .ringIndexes      = RAM_RING_INDEXES_SDAQ_INITIALIZER                      \
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

#define RAM_RING_GET_CAPACITY() (RAM_SDAQ_MAX_INDEX - RAM_SDAQ_MIN_INDEX)

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
void ramRingAddToWriteIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd );

/*! ---------------------------------------------------------------------------
 * @brief Adds a value to the read index.
 * @param pThis Pointer to the ring index object
 * @param value to add to the read index.
 */
void ramRingAddToReadIndex( RAM_RING_INDEXES_T* pThis, RAM_RING_INDEX_T toAdd );

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
 * @brief Returns the current absolute write-index for a write access to the
 *        physical memory.
 * @param pThis Pointer to the ring index object
 * @return Index value for write access.
 */
STATIC inline
RAM_RING_INDEX_T ramRingGetWriteIndex( register RAM_RING_INDEXES_T* pThis )
{
   if( pThis->end == pThis->capacity )
      return ramRingGetReadIndex( pThis );
   return pThis->end + pThis->offset;
}

//#define CONFIG_DAQ_DEBUG

#ifdef CONFIG_DAQ_DEBUG
/*! ---------------------------------------------------------------------------
 * @brief Prints the values of the members of RAM_RING_INDEXES_T
 */

void ramRingDbgPrintIndexes( const RAM_RING_INDEXES_T* pThis,
                                                             const char* txt );
#else
#define ramRingDbgPrintIndexes( __a, __b ) ((void)0)
#endif

/*! @} */ // End SCU_RING_BUFFER_INDEXES

#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* _DAQ_RING_ADMIN_H */
/*================================== EOF ====================================*/

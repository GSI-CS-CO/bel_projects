/*!
 * @file daq_ring_admin.h
 * @brief Administration of the indexes for a ring-buffer.
 *
 * @note This module is suitable for LM32 and Linux
 * @note Header only
 *
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

#include <scu_control_config.h>
#include <circular_index.h>

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

/*!
 * @brief Maximum byffer capacity of ADDAC/ACU and MIL DAQs
 */
#define DAQ_MAX_INDEX (DDR3_MAX_INDEX64 / 100)

//#define RAM_SDAQ_MAX_INDEX 2816
#ifndef RAM_SDAQ_MAX_INDEX
   /*!
    * @brief Maximum value for ring buffer read and write index
    */
   #ifdef CONFIG_MIL_DAQ_USE_RAM
      #define RAM_SDAQ_MAX_INDEX (DAQ_MAX_INDEX / 2)
      #define RAM_MDAQ_MAX_INDEX DAQ_MAX_INDEX
   #else
      #define RAM_SDAQ_MAX_INDEX DAQ_MAX_INDEX
   #endif
#endif

#ifndef RAM_SDAQ_MIN_INDEX
   /*!
    * @brief Minimum value for ring buffer read and write index
    */
   #define RAM_SDAQ_MIN_INDEX 0
   #ifdef CONFIG_MIL_DAQ_USE_RAM
      #define RAM_MDAQ_MIN_INDEX (DAQ_MAX_INDEX / 2)
   #endif
#endif

/*!
 * @brief Maximum capacity of ring buffer in RAM_DAQ_PAYLOAD_T
 */
#define RAM_SDAQ_MAX_CAPACITY (RAM_SDAQ_MAX_INDEX - RAM_SDAQ_MIN_INDEX)

#ifdef CONFIG_MIL_DAQ_USE_RAM
  #define RAM_MDAQ_MAX_CAPACITY (RAM_MDAQ_MAX_INDEX - RAM_MDAQ_MIN_INDEX)
#endif

//#endif
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

#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Data type for data residing in the shared memory for the
 *        communication between uC server and Linux client.
 * @note The implementation has to be within the shared memory!
 *       They must be visible in the LM32 and in the Linux side.
 */
typedef struct PACKED_SIZE
{  /*!
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
#endif // ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Initializer of the shared object for the communication between
 *        server and Linux client.
 */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ

#define RAM_RING_SHARED_SDAQ_OBJECT_INITIALIZER                               \
{                                                                             \
   .indexes = RAM_RING_INDEXES_SDAQ_INITIALIZER,                              \
   .wasRead = 0                                                               \
}

#else
#define RAM_RING_SHARED_SDAQ_OBJECT_INITIALIZER                               \
{                                                                             \
   .serverHasWritten = false,                                                 \
   .ramAccessLock    = false,                                                 \
   .ringIndexes      = RAM_RING_INDEXES_SDAQ_INITIALIZER                      \
}
#endif

#define RAM_RING_GET_CAPACITY() (RAM_SDAQ_MAX_INDEX - RAM_SDAQ_MIN_INDEX)

//#define CONFIG_DAQ_DEBUG

/*! @} */ // End SCU_RING_BUFFER_INDEXES

#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* _DAQ_RING_ADMIN_H */
/*================================== EOF ====================================*/

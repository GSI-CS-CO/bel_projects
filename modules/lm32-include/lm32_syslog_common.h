/*!
 * @file lm32_syslog_common.h
 * @brief Common data definitions for LM32 and Linux for
 *        LM32 version of syslog.
 *
 * @note Header only
 * @note This file is suitable for Linux and LM32.
 *
 * @see       lm32_syslog.h
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      22.04.2022
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
#ifndef _LM32_SYSLOG_COMMON_H
#define _LM32_SYSLOG_COMMON_H

#include <circular_index.h>

#ifdef CONFIG_SCU_USE_DDR3
   #include <scu_ddr3.h>
#else
   #error Unknown memory type!
#endif

/*!
 * @defgroup LM32_LOG Logging system for LM32
 */

#ifdef __cplusplus
extern "C" {
namespace Scu
{
#endif

#ifdef CONFIG_SCU_USE_DDR3
   typedef DDR3_PAYLOAD_T SYSLOG_MEM_ITEM_T;
#endif

/*!
 * @ingroup LM32_LOG
 * @brief Type for administration of the FiFo.
 */
typedef struct PACKED_SIZE
{  /*!
    * @brief Administration of the offset and indexes.
    */
   RAM_RING_SHARED_INDEXES_T admin;

   /*!
    * @brief Necessary for it to be divisible by SYSLOG_MEM_ITEM_T.
    */
   RAM_RING_INDEX_T __padding__;
} SYSLOG_FIFO_ADMIN_T;

STATIC_ASSERT( sizeof(SYSLOG_FIFO_ADMIN_T) % sizeof(SYSLOG_MEM_ITEM_T) == 0 );

/*!
 * @brief Size of fifo administration object in smallest addressable memory
 *        items.
 */
STATIC const size_t SYSLOG_FIFO_ADMIN_SIZE = (sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(SYSLOG_MEM_ITEM_T));


/*!
 * @ingroup LM32_LOG
 * @brief Type of a single log item.
 */
typedef struct PACKED_SIZE
{  /*!
    * @brief White rabbit time stamp.
    */
   uint64_t  timestamp;

   /*!
    * @brief Priority
    */
   uint32_t  priority;

   /*!
    * @brief LM32 start address of control string.
    */
   uint32_t  format;

   /*!
    * @brief Field for optional parameter(s).
    */
   uint32_t  param[4];
} SYSLOG_FIFO_ITEM_T;

STATIC_ASSERT( sizeof(SYSLOG_FIFO_ITEM_T) % sizeof(SYSLOG_MEM_ITEM_T) == 0 );

/*!
 * @brief Size of a fifo item object in smallest addressable memory
 *        items.
 */
STATIC const size_t SYSLOG_FIFO_ITEM_SIZE = (sizeof(SYSLOG_FIFO_ITEM_T) / sizeof(SYSLOG_MEM_ITEM_T));


/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Returns the number of currently used memory items
 * @param pThis Pointer to the shared ring indexes object.
 * @return Actual number written items
 */
STATIC inline
RAM_RING_INDEX_T sysLogFifoGetSize( const SYSLOG_FIFO_ADMIN_T* pThis )
{
   return ramRingSharedGetSize( &pThis->admin ) / SYSLOG_FIFO_ITEM_SIZE;
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Returns the remaining free items of the currently used memory
 * @param pThis Pointer to the shared ring indexes object.
 * @return Number of free memory items.
 */
STATIC inline
RAM_RING_INDEX_T sysLogFifoGetRemainingCapacity( const SYSLOG_FIFO_ADMIN_T* pThis )
{
   return ramRingSharedGetRemainingCapacity( &pThis->admin ) / SYSLOG_FIFO_ITEM_SIZE;
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Adds a value to the write index.
 * @param pThis Pointer to the shared ring indexes object.
 * @param toAdd to add to the write index.
 */
STATIC inline
void sysLogFifoAddToWriteIndex( SYSLOG_FIFO_ADMIN_T* pThis,
                                const RAM_RING_INDEX_T toAdd )
{
   ramRingSharedAddToWriteIndex( &pThis->admin, toAdd * SYSLOG_FIFO_ITEM_SIZE );
}

/*!----------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Increments the write-index.
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline
void sysLogFifoIncWriteIndex( SYSLOG_FIFO_ADMIN_T* pThis )
{
   sysLogFifoAddToWriteIndex( pThis, 1 );
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Adds a value to the read index.
 * @param pThis Pointer to the shared ring indexes object.
 * @param toAdd to add to the read index.
 */
STATIC inline
void sysLogFifoAddToReadIndex( SYSLOG_FIFO_ADMIN_T* pThis,
                               const RAM_RING_INDEX_T toAdd )
{
   ramRingSharedAddToReadIndex( &pThis->admin, toAdd * SYSLOG_FIFO_ITEM_SIZE );
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
 * @brief Increments the read-index.
 * @param pThis Pointer to the shared ring indexes object.
 */
STATIC inline
void sysLogFifoIncReadIndex( SYSLOG_FIFO_ADMIN_T* pThis )
{
   sysLogFifoAddToReadIndex( pThis, 1 );
}

/*! ---------------------------------------------------------------------------
 * @ingroup LM32_LOG
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
void sysLogFifoSynchonizeReadIndex( SYSLOG_FIFO_ADMIN_T* pThis )
{
   ramRingSharedSynchonizeReadIndex( &pThis->admin );
}


#ifdef __cplusplus
} /* extern "C" */
} /* namespace Scu */
#endif
#endif /* ifndef _LM32_SYSLOG_COMMON_H */
/*================================== EOF ====================================*/

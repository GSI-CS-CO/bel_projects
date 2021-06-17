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
#else
 #include <daq_descriptor.h>
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

/*!
 * @brief Calculates the start offset of the payload data in the ring-buffer
 *        during the compile time, so that the offset is dividable by
 *        RAM_DAQ_PAYLOAD_T.
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_DATA_START_OFFSET                            \
(                                                            \
   (sizeof(DAQ_DESCRIPTOR_T) / sizeof(RAM_DAQ_PAYLOAD_T)) +  \
   !!(sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T))  \
)

/*!
 * @brief Calculates any missing DAQ data words of type DAQ_DATA_T to
 *        make the length of the DAQ device descriptor dividable
 *        by the length of RAM_DAQ_PAYLOAD_T.
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_DESCRIPTOR_COMPLETION                          \
(                                                              \
   ((sizeof(RAM_DAQ_PAYLOAD_T) -                               \
   (sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T))) *   \
    !!(sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T)))  \
    / sizeof(DAQ_DATA_T)                                       \
)

/*!
 * @brief Definition of return values of function ramRingGetTypeOfOldestBlock
 */
typedef enum
{
   RAM_DAQ_EMPTY,     //!<@brief No block present
   RAM_DAQ_UNDEFINED, //!<@brief No block recognized
   RAM_DAQ_SHORT,     //!<@brief Short block
   RAM_DAQ_LONG       //!<@brief Long block
} RAM_DAQ_BLOCK_T;

#ifdef CONFIG_SCU_USE_DDR3

/*!
 * @brief Smallest memory unit of the used memory type.
 */
typedef DDR3_PAYLOAD_T RAM_DAQ_PAYLOAD_T;

typedef DDR3_POLL_FT   RAM_DAQ_POLL_FT;

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

#ifndef RAM_DAQ_MIN_INDEX
   /*!
    * @brief Minimum value for ring buffer read and write index
    */
   #define RAM_DAQ_MIN_INDEX 0
#endif
#ifndef RAM_DAQ_MAX_INDEX
   /*!
    * @brief Maximum value for ring buffer read and write index
    */
   #define RAM_DAQ_MAX_INDEX DDR3_MAX_INDEX64
#endif


/*!
 * @brief Maximum capacity of ring buffer in RAM_DAQ_PAYLOAD_T
 */
#define RAM_DAQ_MAX_CAPACITY (RAM_DAQ_MAX_INDEX - RAM_DAQ_MIN_INDEX)

typedef uint32_t RAM_RING_INDEX_T;

#endif /* ifdef CONFIG_SCU_USE_DDR3 */

/*!
 * @brief Calculates the number of memory items from a given number
 *        of data words in DAQ_DATA_T.
 * @see DAQ_DATA_T
 * @see RAM_DAQ_PAYLOAD_T
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define __RAM_DAQ_GET_BLOCK_LEN( b )      \
(                                         \
   (b / sizeof(RAM_DAQ_PAYLOAD_T) +       \
   !!(b % sizeof(RAM_DAQ_PAYLOAD_T))) *   \
   sizeof(DAQ_DATA_T)                     \
)

/*!
 * @brief Calculates the rest in DAQ_DATA_T of a given block length to
 *        complete a full number of RAM_DAQ_PAYLOAD_T.
 */
#define __RAM_DAQ_GET_BLOCK_REST( b )                        \
(                                                            \
   ((b * sizeof(DAQ_DATA_T)) % sizeof(RAM_DAQ_PAYLOAD_T)) /  \
   sizeof(DAQ_DATA_T)                                        \
)

/*!
 * @brief Length of long blocks in RAM_DAQ_PAYLOAD_T for
 *        PostMortem and/or HiRes mode.
 */
#define RAM_DAQ_LONG_BLOCK_LEN  \
   __RAM_DAQ_GET_BLOCK_LEN( DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC )

/*!
 * @brief Rest of long blocks in DAQ_DATA_T to complete a full number of
 *        RAM_DAQ_PAYLOAD_T.
 */
#define RAM_DAQ_LONG_BLOCK_REST \
   __RAM_DAQ_GET_BLOCK_REST( DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC )

/*!
 * @brief Length of short blocks in RAM_DAQ_PAYLOAD_T for
 *        DAQ continuous mode.
 */
#define RAM_DAQ_SHORT_BLOCK_LEN \
   __RAM_DAQ_GET_BLOCK_LEN( DAQ_FIFO_DAQ_WORD_SIZE_CRC )

/*!
 * @brief Rest of short blocks in DAQ_DATA_T to complete a full number of
 *        RAM_DAQ_PAYLOAD_T.
 */
#define RAM_DAQ_SHORT_BLOCK_REST \
   __RAM_DAQ_GET_BLOCK_REST( DAQ_FIFO_DAQ_WORD_SIZE_CRC )

#define RAM_DAQ_DATA_WORDS_PER_RAM_INDEX \
   (sizeof(RAM_DAQ_PAYLOAD_T) / sizeof(DAQ_DATA_T))


#define RAM_DAQ_INDEX_OFFSET_OF_CHANNEL_CONTROL     \
(                                                   \
   offsetof( _DAQ_DISCRIPTOR_STRUCT_T, cControl ) / \
   sizeof(RAM_DAQ_PAYLOAD_T)                        \
)

#define RAM_DAQ_DAQ_WORD_OFFSET_OF_CHANNEL_CONTROL    \
(                                                     \
   (offsetof( _DAQ_DISCRIPTOR_STRUCT_T, cControl ) /  \
   sizeof(RAM_DAQ_PAYLOAD_T) )                        \
)


/*!
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_INDEX_LENGTH_OF_CHANNEL_CONTROL                 \
(                                                               \
   (sizeof(_DAQ_CHANNEL_CONTROL) / sizeof(RAM_DAQ_PAYLOAD_T)) + \
   !!(sizeof(_DAQ_CHANNEL_CONTROL) % sizeof(RAM_DAQ_PAYLOAD_T)) \
)
//#endif
/*!
 * @brief Initializer for object of data type RAM_RING_INDEXES_T
 */
#define RAM_RING_INDEXES_INITIALIZER  \
{                                     \
   .offset   = RAM_DAQ_MIN_INDEX,     \
   .capacity = RAM_DAQ_MAX_CAPACITY,  \
   .start    = 0,                     \
   .end      = 0                      \
}

/*! ---------------------------------------------------------------------------
 * @brief Data type for data residing in the shared memory for the
 *        communication between uC server and Linux client.
 * @note The implementation has to be within the shared memory!
 *       They must be visible in the LM32 and in the Linux side.
 */
typedef struct PACKED_SIZE
{
   /*
    * At the moment it's one member only.
    */
   RAM_RING_INDEXES_T ringIndexes;
} RAM_RING_SHARED_OBJECT_T;

/*!
 * @brief Initializer of the shared object for the communication between
 *        server and Linux client.
 */
#define RAM_RING_SHARED_OBJECT_INITIALIZER      \
{                                               \
   .ringIndexes = RAM_RING_INDEXES_INITIALIZER  \
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
   DDR3_T   ram;
#else
   //TODO maybe in the future will use a other memory type
#endif
   /*!
    * @brief Administration of fifo- indexes.
    * @note The memory space of this object has to be within the shared
    *       memory. \n
    *       Therefore its a pointer in this object.
    */
   RAM_RING_SHARED_OBJECT_T* volatile pSharedObj;
} RAM_SCU_T;

/*! --------------------------------------------------------------------------
 */
static inline
void ramSetPayload16( RAM_DAQ_PAYLOAD_T* pPl, const uint16_t d,
                      const unsigned int i )
{
#ifdef CONFIG_SCU_USE_DDR3
   ddr3SetPayload16( pPl, d, i );
#else
   //TODO
#endif
}

/*! --------------------------------------------------------------------------
 */
static inline
uint16_t ramGetPayload16( RAM_DAQ_PAYLOAD_T* pPl, const unsigned int i )
{
#ifdef CONFIG_SCU_USE_DDR3
   return ddr3GetPayload16( pPl, i );
#else
   //TODO
#endif
}


/*! ---------------------------------------------------------------------------
 * @brief Resets respectively clears the ring buffer
 * @param pThis Pointer to the ring index object
 */
static inline void ramRingReset( register RAM_RING_INDEXES_T* pThis )
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
RAM_RING_INDEX_T ramRingGetSize( const RAM_RING_INDEXES_T* pThis );

/*! ---------------------------------------------------------------------------
 * @brief Returns the remaining free items of the currently used memory
 * @param pThis Pointer to the ring index object
 * @return Number of free memory items.
 */
static inline
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
 * @param pSharedObj Pointer to the fifo administration in shared memory.
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_OBJECT_T* pSharedObj
           #ifdef __linux__
            , EB_HANDLE_T* pEbHandle
           #endif
           );

#if defined(__lm32__) || defined(__DOXYGEN__)

/*! ----------------------------------------------------------------------------
 * @brief Exchanges the order of devicedeskriptor and payload so that the
 *        devicedescriptor appears at first of the given DAQ channel during
 *        writing in the ring buffer. \n
 *        If not enough free space in the ring buffer, so the oldest
 *        DAQ data blocks becomes deleted until its enough space.
 * @param pThis Pointer to the RAM object object.
 * @param pDaqChannel Pointer of the concerning DAQ-channel-object.
 * @param isShort Decides between long and short DAQ-block.
 *                If true it trades is a short block (DAQ continuous)
 *                else (DAQ HiRes or PostMortem)
 * @return Number of deleted old data blocks.
 */
int ramPushDaqDataBlock( register RAM_SCU_T* pThis,
                         DAQ_CANNEL_T* pDaqChannel,
                         bool isShort
                       );

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */

#if defined(__linux__) || defined(__DOXYGEN__)

/*! ---------------------------------------------------------------------------
 */
int ramReadDaqDataBlock( register RAM_SCU_T* pThis, RAM_DAQ_PAYLOAD_T* pData,
                         unsigned int len, RAM_DAQ_POLL_FT poll );

#endif /* defined(__linux__) || defined(__DOXYGEN__) */

/*! @} */ //End of group SCU_RAM_BUFFER
#ifdef __cplusplus
}
#endif
#endif /* ifndef _SCU_RAMBUFFER_H */
/* ================================= EOF ====================================*/

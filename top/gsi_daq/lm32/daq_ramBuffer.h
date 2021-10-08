/*!
 *  @file daq_ramBuffer.h
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
#ifndef _DAQ_RAMBUFFER_H
#define _DAQ_RAMBUFFER_H

#include <daq_ring_admin.h>

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
 * @defgroup DAQ_RAM_BUFFER
 * @brief Abstraction layer for handling SCU RAM-Buffer
 *
 * @note At the moment its DDR3 only.
 * @see SCU_DDR3
 * @{
 */

#ifdef __cplusplus
extern "C" {
namespace Scu
{
namespace daq
{
#endif

/*! ---------------------------------------------------------------------------
 * @brief Calculates the start offset of the payload data in the ring-buffer
 *        during the compile time, so that the offset is dividable by
 *        RAM_DAQ_PAYLOAD_T.
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_DATA_START_OFFSET                                             \
(                                                                             \
   (sizeof(DAQ_DESCRIPTOR_T) / sizeof(RAM_DAQ_PAYLOAD_T)) +                   \
   !!(sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T))                   \
)

/*! ---------------------------------------------------------------------------
 * @brief Calculates any missing DAQ data words of type DAQ_DATA_T to
 *        make the length of the DAQ device descriptor dividable
 *        by the length of RAM_DAQ_PAYLOAD_T.
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_DESCRIPTOR_COMPLETION                                         \
(                                                                             \
   ((sizeof(RAM_DAQ_PAYLOAD_T) -                                              \
   (sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T))) *                  \
    !!(sizeof(DAQ_DESCRIPTOR_T) % sizeof(RAM_DAQ_PAYLOAD_T)))                 \
    / sizeof(DAQ_DATA_T)                                                      \
)

#if defined( CONFIG_SCU_USE_DDR3 ) || defined(__DOXYGEN__)

/*!
 * @brief Smallest memory unit of the used memory type.
 */
typedef DDR3_PAYLOAD_T RAM_DAQ_PAYLOAD_T;

#ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
/*!
 * @see DDR3_POLL_FT
 */
typedef DDR3_POLL_FT   RAM_DAQ_POLL_FT;
#endif
#endif /* ifdef CONFIG_SCU_USE_DDR3 */

/*! ---------------------------------------------------------------------------
 * @brief Calculates the number of memory items from a given number
 *        of data words in DAQ_DATA_T.
 * @see DAQ_DATA_T
 * @see RAM_DAQ_PAYLOAD_T
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define __RAM_DAQ_GET_BLOCK_LEN( b )                                          \
(                                                                             \
   (b / sizeof(RAM_DAQ_PAYLOAD_T) +                                           \
   !!(b % sizeof(RAM_DAQ_PAYLOAD_T))) * sizeof(DAQ_DATA_T)                    \
)

/*! ---------------------------------------------------------------------------
 * @brief Calculates the remainder in DAQ_DATA_T of a given block length to
 *        complete a full number of RAM_DAQ_PAYLOAD_T.
 */
#define __RAM_DAQ_GET_BLOCK_REMAINDER( b )                                    \
(                                                                             \
   ((b * sizeof(DAQ_DATA_T)) % sizeof(RAM_DAQ_PAYLOAD_T)) /                   \
   sizeof(DAQ_DATA_T)                                                         \
)

/*! ---------------------------------------------------------------------------
 * @brief Length of long blocks in RAM_DAQ_PAYLOAD_T for
 *        PostMortem and/or HiRes mode.
 */
#define RAM_DAQ_LONG_BLOCK_LEN                                                \
   __RAM_DAQ_GET_BLOCK_LEN( DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC )

/*! ---------------------------------------------------------------------------
 * @brief Remainder of long blocks in DAQ_DATA_T to complete a full number of
 *        RAM_DAQ_PAYLOAD_T.
 */
#define RAM_DAQ_LONG_BLOCK_REMAINDER                                          \
   __RAM_DAQ_GET_BLOCK_REMAINDER( DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC )

/*! ---------------------------------------------------------------------------
 * @brief Length of short blocks in RAM_DAQ_PAYLOAD_T for
 *        DAQ continuous mode.
 */
#define RAM_DAQ_SHORT_BLOCK_LEN                                               \
   __RAM_DAQ_GET_BLOCK_LEN( DAQ_FIFO_DAQ_WORD_SIZE_CRC )

/*! ---------------------------------------------------------------------------
 * @brief Remainder of short blocks in DAQ_DATA_T to complete a full number of
 *        RAM_DAQ_PAYLOAD_T.
 */
#define RAM_DAQ_SHORT_BLOCK_REMAINDER                                         \
   __RAM_DAQ_GET_BLOCK_REMAINDER( DAQ_FIFO_DAQ_WORD_SIZE_CRC )

/*! ---------------------------------------------------------------------------
 * @brief DAQ data words per RAM item
 */
#define RAM_DAQ_DATA_WORDS_PER_RAM_INDEX                                      \
   (sizeof(RAM_DAQ_PAYLOAD_T) / sizeof(DAQ_DATA_T))


/*! ---------------------------------------------------------------------------
 * @brief Calculates the offset in RAM-items to the channel control register
 *        in the device descriptor
 */
#define RAM_DAQ_INDEX_OFFSET_OF_CHANNEL_CONTROL                               \
(                                                                             \
   offsetof( _DAQ_DISCRIPTOR_STRUCT_T, cControl ) /                           \
   sizeof(RAM_DAQ_PAYLOAD_T)                                                  \
)

/*! ---------------------------------------------------------------------------
 * @brief Calculates the data length to read from the DAQ-RAM to obtain the
 *        channel control register in the device descriptor.
 * @note CAUTION: Don't remove the double exclamation mark (!!) because
 *       it will be used to convert a value that is not equal to zero to one!
 */
#define RAM_DAQ_INDEX_LENGTH_OF_CHANNEL_CONTROL                               \
(                                                                             \
   (sizeof(_DAQ_CHANNEL_CONTROL) / sizeof(RAM_DAQ_PAYLOAD_T)) +               \
   !!(sizeof(_DAQ_CHANNEL_CONTROL) % sizeof(RAM_DAQ_PAYLOAD_T))               \
)

/*! ---------------------------------------------------------------------------
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
   #error Unknown RAM-object!
   //TODO maybe in the future will use a other memory type
#endif
   /*!
    * @brief Administration of fifo- indexes.
    * @note The memory space of this object has to be within the shared
    *       memory. \n
    *       Therefore its a pointer in this object.
    */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   RAM_RING_SHARED_INDEXES_T* volatile pSharedObj;
#else
   RAM_RING_SHARED_OBJECT_T* volatile pSharedObj;
#endif
} RAM_SCU_T;

/*! --------------------------------------------------------------------------
 * @brief Returns the number of RAM items of the data block belonging to this
 *        descriptor.
 */
STATIC inline
size_t ramGetSizeByDescriptor( register DAQ_DESCRIPTOR_T* pDescriptor )
{
   return daqDescriptorIsShortBlock( pDescriptor )?
             RAM_DAQ_SHORT_BLOCK_LEN : RAM_DAQ_LONG_BLOCK_LEN;
}

/*! --------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
void ramSetPayload16( RAM_DAQ_PAYLOAD_T* pPl, const uint16_t d,
                      const unsigned int i )
{
#ifdef CONFIG_SCU_USE_DDR3
   ddr3SetPayload16( pPl, d, i );
#else
   #error Function ramSetPayload16() not implemented yet!
#endif
}

/*! --------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
uint16_t ramGetPayload16( RAM_DAQ_PAYLOAD_T* pPl, const unsigned int i )
{
#ifdef CONFIG_SCU_USE_DDR3
   return ddr3GetPayload16( pPl, i );
#else
   #error Function ramGetPayload16() not implemented yet!
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Initializing SCU RAM buffer ready to use.
 * @param pThis Pointer to the RAM object.
 * @param pSharedObj Pointer to the fifo administration in shared memory.
 * @retval 0 Initializing was successful
 * @retval <0 Error
 */
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_INDEXES_T* pSharedObj
           #ifdef __linux__
         //   , EB_HANDLE_T* pEbHandle
           #endif
           );
#else
int ramInit( register RAM_SCU_T* pThis, RAM_RING_SHARED_OBJECT_T* pSharedObj
           #ifdef __linux__
         //   , EB_HANDLE_T* pEbHandle
           #endif
           );
#endif

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
                         const bool isShort
                       );

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
void ramWriteItem( register RAM_SCU_T* pThis, const RAM_RING_INDEX_T index,
                   RAM_DAQ_PAYLOAD_T* pItem )
{
#if defined( CONFIG_SCU_USE_DDR3 ) || defined(__DOXYGEN__)
   ddr3write64( &pThis->ram, index, pItem );
#else
   #error Nothing implemented in function ramWriteItem()!
#endif
}

#endif /* if defined(__lm32__) || defined(__DOXYGEN__) */

#if (defined(__linux__) || defined(__DOXYGEN__))
/*! ---------------------------------------------------------------------------
 */
int ramReadDaqDataBlock( register RAM_SCU_T* pThis, RAM_DAQ_PAYLOAD_T* pData,
                         unsigned int len
                       #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                         , RAM_DAQ_POLL_FT poll
                       #endif
                       );

#endif /* defined(__linux__) || defined(__DOXYGEN__) */

/*! @} */ //End of group DAQ_RAM_BUFFER
#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C"    */
#endif
#endif /* ifndef _DAQ_RAMBUFFER_H */
/* ================================= EOF ====================================*/

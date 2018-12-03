/*!
 *  @file daq_descriptor.h
 *  @brief Definition of DAQ-Descriptor data-type
 *  @date 22.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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
#ifndef _DAQ_DESCRIPTOR_H
#define _DAQ_DESCRIPTOR_H


#include <stdbool.h>
#include "inttypes.h"
#include "helper_macros.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DAQ_DISCRIPTOR_WORD_SIZE 10 /*!< @brief byte size size of
                                     *  DAQ descriptor + crc */

typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   uint16_t diobId: 5;
   uint16_t unused: 7;
   uint16_t slot:   4;
#else
   uint16_t slot:   4;
   uint16_t unused: 7;
   uint16_t diobId: 5;
#endif
} _DAQ_BF_SLOT_DIOB ;
STATIC_ASSERT( sizeof( _DAQ_BF_SLOT_DIOB ) == sizeof(uint16_t));


typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   uint8_t daqMode:       1;
   uint8_t hiResMode:     1;
   uint8_t pmMode:        1;
   uint8_t channelNumber: 5;
#else
   uint8_t channelNumber: 5;
   uint8_t pmMode:        1;
   uint8_t hiResMode:     1;
   uint8_t daqMode:       1;
#endif
} _DAQ_BF_CANNEL_MODE;
STATIC_ASSERT( sizeof( _DAQ_BF_CANNEL_MODE ) == sizeof(uint8_t) );


typedef struct PACKED_SIZE
{
   uint8_t controlReg;
   _DAQ_BF_CANNEL_MODE channelMode;
} _DAQ_CHANNEL_CONTROL;
STATIC_ASSERT( offsetof(_DAQ_CHANNEL_CONTROL, controlReg ) == 0 );
STATIC_ASSERT( offsetof(_DAQ_CHANNEL_CONTROL, channelMode ) == offsetof(_DAQ_CHANNEL_CONTROL, controlReg ) + sizeof(_DAQ_BF_CANNEL_MODE ));
STATIC_ASSERT( sizeof( _DAQ_CHANNEL_CONTROL ) == sizeof(uint16_t) );

typedef struct PACKED_SIZE
{
   uint32_t nSec;
   uint32_t utSec;
} _DAQ_WR_NAME_T;
STATIC_ASSERT( sizeof(_DAQ_WR_NAME_T) == sizeof(uint64_t) );

typedef union PACKED_SIZE
{
   _DAQ_WR_NAME_T name;
   uint8_t  byteIndex[sizeof(uint64_t)];
   uint16_t wordIndex[sizeof(uint64_t)/sizeof(uint16_t)];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   uint64_t timeStamp;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
   uint64_t timeStampToEndianConvert;
#endif
} _DAQ_WR_T;
STATIC_ASSERT( sizeof( _DAQ_WR_T) == sizeof(uint64_t) );

typedef struct PACKED_SIZE
{
   uint16_t low;
   uint16_t high;
   uint16_t delay;
} _DAQ_TRIGGER;
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, low )   == 0 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, high )  == 1 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, delay ) == 2 * sizeof(uint16_t) );
STATIC_ASSERT( sizeof(_DAQ_TRIGGER)            == 3 * sizeof(uint16_t) );

typedef struct PACKED_SIZE
{
   _DAQ_BF_SLOT_DIOB    slotDiob;
   _DAQ_WR_T            wr;
   _DAQ_TRIGGER         trigger;
   _DAQ_CHANNEL_CONTROL cControl;
   uint8_t              __unused;
   uint8_t              crc;
} _DAQ_DISCRIPTOR_STRUCT_T;
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) == 0 );
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) + sizeof( _DAQ_BF_SLOT_DIOB ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) + sizeof( _DAQ_WR_T ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) + sizeof(_DAQ_TRIGGER ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, __unused ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) + sizeof(_DAQ_CHANNEL_CONTROL));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, crc ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, __unused ) + sizeof( uint8_t ));
STATIC_ASSERT( sizeof(_DAQ_DISCRIPTOR_STRUCT_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof(uint16_t) );

/*!
 * @brief Final type of DAQ- descriptor
 */
typedef union PACKED_SIZE
{
   uint16_t index[DAQ_DISCRIPTOR_WORD_SIZE]; //!< @brief WORD-access by index
   _DAQ_DISCRIPTOR_STRUCT_T name;            //!< @brief Access by name
} DAQ_DESCRIPTOR_T;
STATIC_ASSERT( sizeof( DAQ_DESCRIPTOR_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof( uint16_t ) );

/*! ---------------------------------------------------------------------------
 * @brief Tells the origin slot of the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Slot number in the range of [1..12]
 */
static inline int daqDescriptorGetSlot( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.slotDiob.slot;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the Digital IO Board ID (DIOB) from the record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return DIOB
 */
static inline int daqDescriptorGetDiobId( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.slotDiob.diobId;
}

/*! ---------------------------------------------------------------------------
 * @brief Tells the origin DAQ device channel number of the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Channel number in the range of [0..15]
 */
static inline int daqDescriptorGetChannel( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.channelMode.channelNumber - 1;
}

/*! ---------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "Post Mortem mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @retval ==true Post Mortem was active.
 */
static inline bool daqDescriptorWasPM( register DAQ_DESCRIPTOR_T* pThis )
{
   return (pThis->name.cControl.channelMode.pmMode != 0);
}

/*! ---------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "High Resolution
 *        mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @retval ==true High Resolution mode was active.
 */
static inline bool daqDescriptorWasHiRes( register DAQ_DESCRIPTOR_T* pThis )
{
   return (pThis->name.cControl.channelMode.hiResMode != 0);
}

/*! --------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "DAQ- mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @retval ==true DAQ- mode was active.
 */
static inline bool daqDescriptorWasDaq( register DAQ_DESCRIPTOR_T* pThis )
{
   return (pThis->name.cControl.channelMode.daqMode != 0);
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the least significant word of the bus tag event
 *        trigger condition from the last record.
 * @see daqChannelGetTriggerConditionLW in daq.h
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Least significant word of trigger condition.
 */
static inline uint16_t daqDescriptorGetTriggerConditionLW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.low;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the most significant word of the bus tag event
 *        trigger condition from the last record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Most significant word of trigger condition.
 */
static inline uint16_t daqDescriptorGetTriggerConditionHW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.high;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the trigger delay from the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Trigger delay
 */
static inline uint16_t daqDescriptorGetTriggerDelay( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.delay;
}

/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 */
static inline uint32_t daqDescriptorGetTimeStampNanoSec( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.name.nSec;
}

/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 */
static inline uint32_t daqDescriptorGetTimeStampSec( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.name.utSec;
}

/*! ---------------------------------------------------------------------------
 * @brief Get the CRC of this record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return 8 bit CRC
 */
static inline uint8_t daqDescriptorGetCRC( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.crc;
}

#ifdef __cplusplus
}
#endif

#endif
/*================================== EOF ====================================*/


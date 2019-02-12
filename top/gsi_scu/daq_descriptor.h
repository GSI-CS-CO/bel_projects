/*!
 *  @file daq_descriptor.h
 *  @brief Definition of DAQ-Descriptor data-type
 *
 *  This file is suitable for LM32-apps within the SCU environment and
 *  for Linux applications.
 *
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 *     Data Aquisition Macro fuer SCU Slave Baugruppen</a>
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
#include <stdint.h>
#include <helper_macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ingroup DAQ
 * @defgroup DAQ_DESCRIPTOR
 * @brief DAQ Descriptor
 * @{
 */

/*!
 * @brief byte size size of DAQ descriptor + crc
 */
#define DAQ_DISCRIPTOR_WORD_SIZE 10

/*!
 * @brief Maximum DAQ-FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor but without check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetDaqFifoWords
 */
#define DAQ_FIFO_DAQ_WORD_SIZE       509

//#define DAQ_FIFO_DAQ_WORD_SIZE (4 + DAQ_DISCRIPTOR_WORD_SIZE) //!! For test only!

#if (DAQ_FIFO_DAQ_WORD_SIZE < 0x1FD)
#warning DAQ module in test!
#endif

#if (DAQ_FIFO_DAQ_WORD_SIZE < DAQ_DISCRIPTOR_WORD_SIZE)
  #error Fatal: DAQ_FIFO_DAQ_WORD_SIZE shall not be smaler than DAQ_DISCRIPTOR_WORD_SIZE !
#endif

/*!
 * @brief Maximum PM_HIRES FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor but without check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetPmFifoWords
 */
#define DAQ_FIFO_PM_HIRES_WORD_SIZE  1023

#if (DAQ_FIFO_PM_HIRES_WORD_SIZE < DAQ_DISCRIPTOR_WORD_SIZE)
  #error Fatal: DAQ_FIFO_PM_HIRES_WORD_SIZE shall not be smaler than DAQ_DISCRIPTOR_WORD_SIZE !
#endif

#if (DAQ_FIFO_DAQ_WORD_SIZE > DAQ_FIFO_PM_HIRES_WORD_SIZE)
  #warning DAQ_FIFO_DAQ_WORD_SIZE is greater than DAQ_FIFO_PM_HIRES_WORD_SIZE ! Realy?
#endif

typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   unsigned int diobId: 5; /*!< @brief Diob Extntion ID */
   unsigned int unused: 7; /*!< @brief not used */
   unsigned int slot:   4; /*!< @brief Slot number of SCU-Bus [1..12] */
#else
   unsigned int slot:   4;
   unsigned int unused: 7;
   unsigned int diobId: 5;
#endif
} _DAQ_BF_SLOT_DIOB ;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( _DAQ_BF_SLOT_DIOB ) == sizeof(uint16_t));
#endif

typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   bool    daqMode:       1; /*!< @brief Data Acquisition continuous mode */
   bool    hiResMode:     1; /*!< @brief High Resolution mode */
   bool    pmMode:        1; /*!< @brief Post Mortem mode */
   uint8_t channelNumber: 5; /*!< @brief Channel number [1..16] */
#else
   uint8_t channelNumber: 5;
   bool    pmMode:        1;
   bool    hiResMode:     1;
   bool    daqMode:       1;
#endif
} _DAQ_BF_CANNEL_MODE;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( _DAQ_BF_CANNEL_MODE ) == sizeof(uint8_t) );
#endif

typedef struct PACKED_SIZE
{
   uint8_t controlReg;               /*!< @brief Bits [7:0] of control register */
   _DAQ_BF_CANNEL_MODE channelMode;  /*!< @see _DAQ_BF_CANNEL_MODE */
} _DAQ_CHANNEL_CONTROL;
#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof(_DAQ_CHANNEL_CONTROL, controlReg ) == 0 );
STATIC_ASSERT( offsetof(_DAQ_CHANNEL_CONTROL, channelMode ) == offsetof(_DAQ_CHANNEL_CONTROL, controlReg ) + sizeof(_DAQ_BF_CANNEL_MODE ));
STATIC_ASSERT( sizeof( _DAQ_CHANNEL_CONTROL ) == sizeof(uint16_t) );
#endif

#if 1
typedef struct PACKED_SIZE
{
   uint32_t nSec;
   uint32_t utSec;
} _DAQ_WR_NAME_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(_DAQ_WR_NAME_T) == sizeof(uint64_t) );
#endif
#endif
/*!
 * @brief White Rabbit time stamp
 */
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
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( _DAQ_WR_T) == sizeof(uint64_t) );
#endif

/*!
 * @brief Trigger data type
 */
typedef struct PACKED_SIZE
{
   uint16_t low;   /*!<@brief Trigger last significant word */
   uint16_t high;  /*!<@brief Trigger most significant word */
   uint16_t delay; /*!<@brief Trigger delay */
} _DAQ_TRIGGER;
#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, low )   == 0 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, high )  == 1 * sizeof(uint16_t) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, delay ) == 2 * sizeof(uint16_t) );
STATIC_ASSERT( sizeof(_DAQ_TRIGGER)            == 3 * sizeof(uint16_t) );
#endif

typedef struct PACKED_SIZE
{
   _DAQ_BF_SLOT_DIOB    slotDiob; /*!<@see _DAQ_BF_SLOT_DIOB */
   _DAQ_WR_T            wr;       /*!<@brief White Rabbit Timestamp @see _DAQ_WR_T */
   _DAQ_TRIGGER         trigger;  /*!<@brief Trigger @see _DAQ_TRIGGER */
   _DAQ_CHANNEL_CONTROL cControl; /*!<@see _DAQ_CHANNEL_CONTROL */
   uint8_t              __unused;
   uint8_t              crc;
} _DAQ_DISCRIPTOR_STRUCT_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) == 0 );
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) + sizeof( _DAQ_BF_SLOT_DIOB ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) + sizeof( _DAQ_WR_T ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) + sizeof(_DAQ_TRIGGER ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, __unused ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) + sizeof(_DAQ_CHANNEL_CONTROL));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, crc ) == offsetof(_DAQ_DISCRIPTOR_STRUCT_T, __unused ) + sizeof( uint8_t ));
STATIC_ASSERT( sizeof(_DAQ_DISCRIPTOR_STRUCT_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof(uint16_t) );
#endif

/*!
 * @brief Final type of DAQ- descriptor
 */
typedef union PACKED_SIZE
{
   uint16_t index[DAQ_DISCRIPTOR_WORD_SIZE]; //!< @brief WORD-access by index
   _DAQ_DISCRIPTOR_STRUCT_T name;            //!< @brief Access by name
} DAQ_DESCRIPTOR_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( DAQ_DESCRIPTOR_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof( uint16_t ) );
#endif

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
   return pThis->name.cControl.channelMode.pmMode;
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
   return pThis->name.cControl.channelMode.hiResMode;
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
   return pThis->name.cControl.channelMode.daqMode;
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

#if 1
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
#endif
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

/*! @} */ //End of group  DAQ_DESCRIPTOR
#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_DESCRIPTOR_H */
/*================================== EOF ====================================*/


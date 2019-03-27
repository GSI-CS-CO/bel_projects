/*!
 *  @file daq_descriptor.h
 *  @brief Definition of DAQ-Descriptor data-type
 *
 *  This file is suitable for LM32-apps within the SCU environment and
 *  for Linux applications.
 *  @note Different endianes conventions of bit fields becomes considered!
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 *     Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 *  @date 22.11.2018
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#ifndef _DAQ_DESCRIPTOR_H
#define _DAQ_DESCRIPTOR_H

#include <stdbool.h>
#include <stdint.h>
#include <helper_macros.h>
/*
 * For the reason LM32 RAM consume can be reduced here is the possibility
 * to overwrite DAQ_MAX by the Makefile.
 */
#ifndef DAQ_MAX
   /*! @ingroup DAQ_DEVICE
    * @brief Maximum number of DAQ's
    */
   #define DAQ_MAX MAX_SCU_SLAVES
#else
  #if DAQ_MAX > MAX_SCU_SLAVES
    #error Macro DAQ_MAX can not be greater than MAX_SCU_SLAVES !
  #endif
#endif

#ifndef DAQ_MAX_CHANNELS
   /*! @ingroup DAQ_CHANNEL
    *  @brief Maximum number of DAQ-channels
    */
  #define DAQ_MAX_CHANNELS 16
#else
  #if DAQ_MAX_CHANNELS > 16
    #error Not more than 16 channels per DAQ
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ingroup DAQ_CHANNEL
 * @brief Data type of DAQ FiFo containment
 */
typedef uint16_t DAQ_DATA_T;

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

/*!
 * @brief Maximum DAQ-FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor and check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetDaqFifoWords
 */
#define DAQ_FIFO_DAQ_WORD_SIZE_CRC   (DAQ_FIFO_DAQ_WORD_SIZE + 1)

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

#define DAQ_FIFO_PM_HIRES_WORD_SIZE     1023
/*!
 * @brief Maximum PM_HIRES FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor and check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetPmFifoWords
 */

#define DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC (DAQ_FIFO_PM_HIRES_WORD_SIZE + 1)

#if (DAQ_FIFO_PM_HIRES_WORD_SIZE < DAQ_DISCRIPTOR_WORD_SIZE)
  #error Fatal: DAQ_FIFO_PM_HIRES_WORD_SIZE shall not be smaler than DAQ_DISCRIPTOR_WORD_SIZE !
#endif

#if (DAQ_FIFO_DAQ_WORD_SIZE > DAQ_FIFO_PM_HIRES_WORD_SIZE)
  #warning DAQ_FIFO_DAQ_WORD_SIZE is greater than DAQ_FIFO_PM_HIRES_WORD_SIZE ! Realy?
#endif

/*! ---------------------------------------------------------------------------
 * @brief Bit field of DIOB extention ID ind slot number.
 */
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
STATIC_ASSERT( sizeof( _DAQ_BF_SLOT_DIOB ) == sizeof(DAQ_DATA_T));
#endif

/*! ---------------------------------------------------------------------------
 * @brief Bit field of acquisition mode and channel number
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   uint8_t controlReg:    8; /*!< @brief Bits [7:0] of control register */
   bool    daqMode:       1; /*!< @brief Data Acquisition continuous mode */
   bool    hiResMode:     1; /*!< @brief High Resolution mode */
   bool    pmMode:        1; /*!< @brief Post Mortem mode */
   uint8_t channelNumber: 5; /*!< @brief Channel number [1..16] */
#else
   uint8_t channelNumber: 5;
   bool    pmMode:        1;
   bool    hiResMode:     1;
   bool    daqMode:       1;
   uint8_t controlReg:    8;
#endif
} _DAQ_CHANNEL_CONTROL;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( _DAQ_CHANNEL_CONTROL ) == sizeof(DAQ_DATA_T) );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Summary of nano seconds and seconds
 */
typedef struct PACKED_SIZE
{
   uint32_t nSec;  /*!<@brief Nano seconds */
   uint32_t utSec; /*!<@brief Seconds */
} _DAQ_WR_NAME_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(_DAQ_WR_NAME_T) == sizeof(uint64_t) );
#endif

/*! ---------------------------------------------------------------------------
 * @brief White Rabbit time stamp
 */
typedef union PACKED_SIZE
{
   _DAQ_WR_NAME_T name;
   uint8_t  byteIndex[sizeof(uint64_t)];
   uint16_t wordIndex[sizeof(uint64_t)/sizeof(DAQ_DATA_T)];
   uint64_t timeStamp;
} _DAQ_WR_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( _DAQ_WR_T) == sizeof(uint64_t) );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Trigger data type
 */
typedef struct PACKED_SIZE
{
   DAQ_DATA_T low;   /*!<@brief Trigger last significant word */
   DAQ_DATA_T high;  /*!<@brief Trigger most significant word */
   DAQ_DATA_T delay; /*!<@brief Trigger delay */
} _DAQ_TRIGGER;
#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, low )   == 0 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, high )  == 1 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, delay ) == 2 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( sizeof(_DAQ_TRIGGER)            == 3 * sizeof(DAQ_DATA_T) );
#endif

/*! ---------------------------------------------------------------------------
 * @brief CRC and placeholder data type
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   DAQ_DATA_T __unused: 8;
   DAQ_DATA_T crc:      8;
#else
   DAQ_DATA_T crc:      8;
   DAQ_DATA_T __unused: 8;
#endif
} _DAQ_BF_CRC_REG;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(_DAQ_BF_CRC_REG) == sizeof(DAQ_DATA_T) );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Named type of DAQ- descriptor
 */
typedef struct PACKED_SIZE
{
   _DAQ_BF_SLOT_DIOB    slotDiob; /*!<@see _DAQ_BF_SLOT_DIOB */
   _DAQ_WR_T            wr;       /*!<@brief White Rabbit Timestamp
                                       @see _DAQ_WR_T */
   _DAQ_TRIGGER         trigger;  /*!<@brief Trigger @see _DAQ_TRIGGER */
   _DAQ_CHANNEL_CONTROL cControl; /*!<@see _DAQ_CHANNEL_CONTROL */
   _DAQ_BF_CRC_REG      crcReg;   /*!<@see _DAQ_BF_CRC_REG */
} _DAQ_DISCRIPTOR_STRUCT_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) == 0 );
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) +
               sizeof( _DAQ_BF_SLOT_DIOB ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) + sizeof( _DAQ_WR_T ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) +
               sizeof(_DAQ_TRIGGER ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, crcReg ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) +
               sizeof(_DAQ_BF_CRC_REG ));
STATIC_ASSERT( sizeof(_DAQ_DISCRIPTOR_STRUCT_T ) ==
               DAQ_DISCRIPTOR_WORD_SIZE * sizeof(DAQ_DATA_T) );
#endif /* ifndef __DOXYGEN__ */

/*! ---------------------------------------------------------------------------
 * @brief Final type of DAQ- descriptor
 */
typedef union PACKED_SIZE
{
   DAQ_DATA_T index[DAQ_DISCRIPTOR_WORD_SIZE]; //!< @brief WORD-access by index
   _DAQ_DISCRIPTOR_STRUCT_T name;            //!< @brief Access by name
} DAQ_DESCRIPTOR_T;
#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( DAQ_DESCRIPTOR_T ) ==
               DAQ_DISCRIPTOR_WORD_SIZE * sizeof( DAQ_DATA_T ) );
#endif

///////////////////////////////////////////////////////////////////////////////
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

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetSlot( register DAQ_DESCRIPTOR_T* pThis,
                                         unsigned int slot )
{
   pThis->name.slotDiob.slot = slot;
}
#endif

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

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetDiobId( register DAQ_DESCRIPTOR_T* pThis,
                                           unsigned int diobId )
{
   pThis->name.slotDiob.diobId = diobId;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Tells the origin DAQ device channel number of the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Channel number in the range of [0..15]
 */
static inline int daqDescriptorGetChannel( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.channelNumber - 1;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetChannel( register DAQ_DESCRIPTOR_T* pThis,
                                            unsigned int channel )
{
   pThis->name.cControl.channelNumber = channel + 1;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "Post Mortem mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @retval ==true Post Mortem was active.
 */
static inline bool daqDescriptorWasPM( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.pmMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetPM( register DAQ_DESCRIPTOR_T* pThis,
                                       bool pmMode )
{
   pThis->name.cControl.pmMode = pmMode;
}
#endif

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
  // return pThis->name.cControl.channelMode.hiResMode;
   return pThis->name.cControl.hiResMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetHiRes( register DAQ_DESCRIPTOR_T* pThis,
                                          bool hiResMode )
{
   pThis->name.cControl.hiResMode = hiResMode;
}
#endif

/*! --------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "DAQ- mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @retval true DAQ- mode was active.
 */
static inline bool daqDescriptorWasDaq( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.daqMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline void daqDescriptorSetDaq( register DAQ_DESCRIPTOR_T* pThis,
                                        bool daqMode )
{
   pThis->name.cControl.daqMode = daqMode;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Verifies the integrity of the set DAQ- mode in the device-
 *        descriptor.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 * @retval true Set mote is OK.
 * @retval false Error in descriptor.
 */
static inline bool daqDescriptorVerifyMode( register DAQ_DESCRIPTOR_T* pThis )
{
   return ( ((int)daqDescriptorWasPM( pThis ))    +
            ((int)daqDescriptorWasHiRes( pThis )) +
            ((int)daqDescriptorWasDaq( pThis )) == 1 );
}

/*! ----------------------------------------------------------------------------
 * @brief Returns true if it is a long block.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 * @retval true Long block   (has DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC words)
 * @retval false Short block (has DAQ_FIFO_DAQ_WORD_SIZE_CRC words)
 */
static inline bool daqDescriptorIsLongBlock( register DAQ_DESCRIPTOR_T* pThis )
{
   return !daqDescriptorWasDaq( pThis );
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
static inline
DAQ_DATA_T daqDescriptorGetTriggerConditionLW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.low;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline
void daqDescriptorSetTriggerConditionLW( register DAQ_DESCRIPTOR_T* pThis,
                                         DAQ_DATA_T trLow )
{
   pThis->name.trigger.low = trLow;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Gets the most significant word of the bus tag event
 *        trigger condition from the last record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Most significant word of trigger condition.
 */
static inline
DAQ_DATA_T daqDescriptorGetTriggerConditionHW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.high;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline
void daqDescriptorSetTriggerConditionHW( register DAQ_DESCRIPTOR_T* pThis,
                                         DAQ_DATA_T trHigh )
{
   pThis->name.trigger.high = trHigh;
}
#endif

/*! -------------------------------------------------------------------------
 */
static inline
uint32_t daqDescriptorGetTriggerCondition( register DAQ_DESCRIPTOR_T* pThis )
{
   uint32_t condition;
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   ((DAQ_DATA_T*)&condition)[0] = pThis->name.trigger.high;
   ((DAQ_DATA_T*)&condition)[1] = pThis->name.trigger.low;
#else
   ((DAQ_DATA_T*)&condition)[0] = pThis->name.trigger.low;
   ((DAQ_DATA_T*)&condition)[1] = pThis->name.trigger.high;
#endif
   return condition;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the trigger delay from the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 * @return Trigger delay
 */
static inline
DAQ_DATA_T daqDescriptorGetTriggerDelay( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.delay;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline
void daqDescriptorSetTriggerDelay( register DAQ_DESCRIPTOR_T* pThis,
                                   DAQ_DATA_T delay )
{
   pThis->name.trigger.delay = delay;
}
#endif

#if 1
/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 */
static inline 
uint32_t daqDescriptorGetTimeStampNanoSec( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.name.nSec;
}

/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DISCRIPTOR_WORD_SIZE
 */
static inline 
uint32_t daqDescriptorGetTimeStampSec( register DAQ_DESCRIPTOR_T* pThis )
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
static inline 
uint8_t daqDescriptorGetCRC( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.crcReg.crc;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
static inline
void daqDescriptorSetCRC( register DAQ_DESCRIPTOR_T* pThis, uint8_t crc )
{
   pThis->name.crcReg.crc = crc;
}
#endif

/*! @} */ //End of group  DAQ_DESCRIPTOR
#ifdef __cplusplus
}
#endif

#endif /* ifndef _DAQ_DESCRIPTOR_H */
/*================================== EOF ====================================*/


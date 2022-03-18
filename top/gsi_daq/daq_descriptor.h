/*!
 *  @file daq_descriptor.h
 *  @brief Definition of DAQ-Descriptor data-type
 *
 *  This file is suitable for LM32-apps within the SCU environment and
 *  for Linux applications.
 *
 *  @note Header only
 *  @note Different endianes conventions of bit fields becomes considered!
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/DataAquisitionMacrof%C3%BCrSCUSlaveBaugruppen">
 *     Data Aquisition Macro fuer SCU Slave Baugruppen</a>
 *  @date 22.11.2018
 *  Revision: 06.06.2019
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *  @todo Constants like e.g. DAQ_FIFO_DAQ_WORD_SIZE and
 *        DAQ_FIFO_PM_HIRES_WORD_SIZE has to be defined by a corresponding
 *        VHDL source file by a VHDL to C-header parser.
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
#include <daq_fg_allocator.h>
/*
 * For the reason LM32 RAM consume can be reduced here is the possibility
 * to overwrite DAQ_MAX by the Makefile.
 */
#ifndef DAQ_MAX
   /*! @ingroup DAQ_DEVICE
    * @brief Maximum number of DAQ's
    */
   #define DAQ_MAX ADD_NAMESPACE( Bus, MAX_SCU_SLAVES )
#else
  #if DAQ_MAX > DAQ_MAX ADD_NAMESPACE( Bus, MAX_SCU_SLAVES )
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
namespace Scu
{
namespace daq
{
#endif

#define __DAQ_DEFAULT_ECA_SYNC_TAG__ 0xDEADBEEF
#ifdef __cplusplus
   constexpr uint32_t DEFAULT_ECA_SYNC_TAG = __DAQ_DEFAULT_ECA_SYNC_TAG__;
#else
   #define DAQ_DEFAULT_ECA_SYNC_TAG ((uint32_t)__DAQ_DEFAULT_ECA_SYNC_TAG__)
#endif


#define __DAQ_DEFAULT_SYNC_TIMEOFFSET__  10000
#ifdef __cplusplus
   constexpr uint32_t DEFAULT_SYNC_TIMEOFFSET = __DAQ_DEFAULT_SYNC_TIMEOFFSET__;
#else
   #define DAQ_DEFAULT_SYNC_TIMEOFFSET ((uint32_t)__DAQ_DEFAULT_SYNC_TIMEOFFSET__)
#endif


/*!
 * @ingroup DAQ_CHANNEL
 * @brief Data type of DAQ FiFo containment
 */
typedef uint16_t DAQ_DATA_T;

/*!
 * @brief Register type of DAQ device
 */
typedef uint16_t DAQ_REGISTER_T;

/*!
 * @ingroup DAQ
 * @defgroup DAQ_DESCRIPTOR
 * @brief DAQ Descriptor
 * @{
 */

/*!
 * @brief byte size size of DAQ descriptor + crc
 */
#define DAQ_DESCRIPTOR_WORD_SIZE 10

/*!
 * @brief Maximum DAQ-FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor but without check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetDaqFifoWords
 */
#define DAQ_FIFO_DAQ_WORD_SIZE       511

/*!
 * @brief Maximum DAQ-FIFO capacity in 16 bit words
 *        inclusive the DAQ-Descriptor and check-sum.
 * @see DAQ_DESCRIPTOR_T
 * @see daqChannelGetDaqFifoWords
 */
#define DAQ_FIFO_DAQ_WORD_SIZE_CRC   (DAQ_FIFO_DAQ_WORD_SIZE + 1)

//#define DAQ_FIFO_DAQ_WORD_SIZE (4 + DAQ_DESCRIPTOR_WORD_SIZE) //!! For test only!

#if (DAQ_FIFO_DAQ_WORD_SIZE < 0x1FD)
#warning DAQ module in test!
#endif

#if (DAQ_FIFO_DAQ_WORD_SIZE < DAQ_DESCRIPTOR_WORD_SIZE)
  #error Fatal: DAQ_FIFO_DAQ_WORD_SIZE shall not be smaler than DAQ_DESCRIPTOR_WORD_SIZE !
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

#if (DAQ_FIFO_PM_HIRES_WORD_SIZE < DAQ_DESCRIPTOR_WORD_SIZE)
  #error Fatal: DAQ_FIFO_PM_HIRES_WORD_SIZE shall not be smaler than DAQ_DESCRIPTOR_WORD_SIZE !
#endif

#if (DAQ_FIFO_DAQ_WORD_SIZE > DAQ_FIFO_PM_HIRES_WORD_SIZE)
  #warning DAQ_FIFO_DAQ_WORD_SIZE is greater than DAQ_FIFO_PM_HIRES_WORD_SIZE ! Realy?
#endif

#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)  || defined(__DOXYGEN__)
/*! ---------------------------------------------------------------------------
 * @brief Definition of the control register bits.
 *
 * This bits are present in the DAQ control-register it self
 * and in the device descriptor. Therefore this macro will used avoiding
 * code redundancy.
 *
 * ExtTrig_nEvTrig_HiRes: Bit [7] trigger source in high resolution mode
 *                                1= external trigger, 0= event trigger. \n
 * Ena_HiRes:             Bit [6] high resolution sampling with 4 MHz.
 *                                <b>CAUTION:</b>
 *                                Ena_PM shall not be active at the same time! \n
 * ExtTrig_nEvTrig:       Bit [5] trigger source in DAQ mode:
 *                                1=ext trigger, 0= event trigger. \n
 * Ena_TrigMod:           Bit [4] prevents sampling till triggering. \n
 * Sample1ms:             Bit [3] use 1 ms sample. \n
 * Sample100us:           Bit [2] use 100 us sample. \n
 * Sample10us:            Bit [1] use 10 us sample. \n
 * Ena_PM:                Bit [0] starts PM sampling with 100 us. \n
 *
 * Big endian (LM32):
 */
   #define __DAQ_BF_CONTROL_REGISTER_BITS         \
      DAQ_REGISTER_T ExtTrig_nEvTrig_HiRes: 1;    \
      DAQ_REGISTER_T Ena_HiRes:             1;    \
      DAQ_REGISTER_T ExtTrig_nEvTrig:       1;    \
      DAQ_REGISTER_T Ena_TrigMod:           1;    \
      DAQ_REGISTER_T Sample1ms:             1;    \
      DAQ_REGISTER_T Sample100us:           1;    \
      DAQ_REGISTER_T Sample10us:            1;    \
      DAQ_REGISTER_T Ena_PM:                1;

#else
/*
 * Little endian (Linux):
 */
   #define __DAQ_BF_CONTROL_REGISTER_BITS         \
      DAQ_REGISTER_T Ena_PM:                1;    \
      DAQ_REGISTER_T Sample10us:            1;    \
      DAQ_REGISTER_T Sample100us:           1;    \
      DAQ_REGISTER_T Sample1ms:             1;    \
      DAQ_REGISTER_T Ena_TrigMod:           1;    \
      DAQ_REGISTER_T ExtTrig_nEvTrig:       1;    \
      DAQ_REGISTER_T Ena_HiRes:             1;    \
      DAQ_REGISTER_T ExtTrig_nEvTrig_HiRes: 1;
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
   __DAQ_BF_CONTROL_REGISTER_BITS /*!< @brief Bits [7:0] of control register */
   bool    daqMode:       1; /*!< @brief Data Acquisition continuous mode */
   bool    hiResMode:     1; /*!< @brief High Resolution mode */
   bool    pmMode:        1; /*!< @brief Post Mortem mode */
   uint8_t channelNumber: 5; /*!< @brief Channel number [1..16] */
#else
   uint8_t channelNumber: 5;
   bool    pmMode:        1;
   bool    hiResMode:     1;
   bool    daqMode:       1;
   __DAQ_BF_CONTROL_REGISTER_BITS
#endif
} _DAQ_CHANNEL_CONTROL;

STATIC_ASSERT( sizeof( _DAQ_CHANNEL_CONTROL ) == sizeof(DAQ_DATA_T) );

/*! ---------------------------------------------------------------------------
 * @brief Summary of nano seconds and seconds
 */
typedef struct PACKED_SIZE
{
   uint32_t nSec;  /*!<@brief Nano seconds */
   uint32_t utSec; /*!<@brief Seconds */
} _DAQ_WR_NAME_T;

STATIC_ASSERT( sizeof(_DAQ_WR_NAME_T) == sizeof(uint64_t) );

/*! ---------------------------------------------------------------------------
 * @brief White Rabbit time stamp
 */
typedef union PACKED_SIZE
{
   _DAQ_WR_NAME_T name;
   uint8_t  byteIndex[sizeof(uint64_t)];
   uint16_t wordIndex[sizeof(uint64_t)/sizeof(DAQ_REGISTER_T)];
   uint64_t timeStamp;
} _DAQ_WR_T;

STATIC_ASSERT( sizeof( _DAQ_WR_T) == sizeof(uint64_t) );

/*! ---------------------------------------------------------------------------
 * @brief Trigger data type
 */
typedef struct PACKED_SIZE
{
   DAQ_DATA_T low;   /*!<@brief Trigger last significant word */
   DAQ_DATA_T high;  /*!<@brief Trigger most significant word */
   DAQ_DATA_T delay; /*!<@brief Trigger delay */
} _DAQ_TRIGGER;

/*
 * We have to made a static check verifying whether the structure-format
 * is equal on both platforms: Linux and LM32.
 */
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, low )   == 0 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, high )  == 1 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( offsetof( _DAQ_TRIGGER, delay ) == 2 * sizeof(DAQ_DATA_T) );
STATIC_ASSERT( sizeof(_DAQ_TRIGGER)            == 3 * sizeof(DAQ_DATA_T) );

/*!
 * @brief Data type of ADDAC-DAQ- block sequence counter
 */
typedef uint8_t DAQ_SEQUENCE_T;

/*! ---------------------------------------------------------------------------
 * @brief CRC and placeholder data type
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   DAQ_SEQUENCE_T sequence: 8;
   DAQ_DATA_T     crc:      8;
#else
   DAQ_DATA_T     crc:      8;
   DAQ_SEQUENCE_T sequence: 8;
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

/*
 * Due to the implementation in different platforms - LM32 and linux -
 * we have a bit of paranoia, which can't hurt. ;-)
 */
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) == 0 );
#if 1
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) +
               sizeof( _DAQ_BF_SLOT_DIOB ));
#else
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) == GET_OFFSET_AFTER( _DAQ_DISCRIPTOR_STRUCT_T, slotDiob ) )
#endif
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, wr ) + sizeof( _DAQ_WR_T ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, trigger ) +
               sizeof(_DAQ_TRIGGER ));
STATIC_ASSERT( offsetof(_DAQ_DISCRIPTOR_STRUCT_T, crcReg ) ==
               offsetof(_DAQ_DISCRIPTOR_STRUCT_T, cControl ) +
               sizeof(_DAQ_BF_CRC_REG ));
STATIC_ASSERT( sizeof(_DAQ_DISCRIPTOR_STRUCT_T ) ==
               DAQ_DESCRIPTOR_WORD_SIZE * sizeof(DAQ_DATA_T) );

/*! ---------------------------------------------------------------------------
 * @brief Final type of DAQ- descriptor
 */
typedef union PACKED_SIZE
{
   DAQ_DATA_T index[DAQ_DESCRIPTOR_WORD_SIZE]; //!< @brief WORD-access by index
   _DAQ_DISCRIPTOR_STRUCT_T name;            //!< @brief Access by name
} DAQ_DESCRIPTOR_T;

STATIC_ASSERT( sizeof( DAQ_DESCRIPTOR_T ) ==
               DAQ_DESCRIPTOR_WORD_SIZE * sizeof( DAQ_DATA_T ) );

///////////////////////////////////////////////////////////////////////////////

/*! ---------------------------------------------------------------------------
 * @brief Constants of receive states.
 * @ingroup DAQ_CHANNEL
 */
typedef enum
{
   DAQ_RECEIVE_STATE_OK             = 0,
   DAQ_RECEIVE_STATE_DATA_LOST      = 1,
   DAQ_RECEIVE_STATE_CORRUPT_BLOCK  = 2
} DAQ_REC_STAT_T;

/*! ---------------------------------------------------------------------------
 * @brief Datatype for holding the last appeared error.
 * @ingroup DAQ_CHANNEL
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   DAQ_REC_STAT_T   status:   7;
   uint16_t         slot:     4;
   uint16_t         channel:  5;
#else
   uint16_t         channel:  5;
   uint16_t         slot:     4;
   DAQ_REC_STAT_T   status:   7;
#endif
} DAQ_LAST_STATUS_T;

STATIC_ASSERT( sizeof( DAQ_LAST_STATUS_T ) == sizeof( DAQ_REGISTER_T ) );

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Tells the origin slot of the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Slot number in the range of [1..12]
 */
STATIC inline int daqDescriptorGetSlot( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.slotDiob.slot;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetSlot( register DAQ_DESCRIPTOR_T* pThis,
                                         unsigned int slot )
{
   pThis->name.slotDiob.slot = slot;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Gets the Digital IO Board ID (DIOB) from the record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return DIOB
 */
STATIC inline int daqDescriptorGetDiobId( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.slotDiob.diobId;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetDiobId( register DAQ_DESCRIPTOR_T* pThis,
                                           unsigned int diobId )
{
   pThis->name.slotDiob.diobId = diobId;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Tells the origin DAQ device channel number of the last record
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Channel number in the range of [0..15]
 */
STATIC inline int daqDescriptorGetChannel( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.channelNumber - 1;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetChannel( register DAQ_DESCRIPTOR_T* pThis,
                                            unsigned int channel )
{
   pThis->name.cControl.channelNumber = channel + 1;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "Post Mortem mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @retval ==true Post Mortem was active.
 */
STATIC inline bool daqDescriptorWasPM( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.pmMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetPM( register DAQ_DESCRIPTOR_T* pThis,
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
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @retval ==true High Resolution mode was active.
 */
STATIC inline bool daqDescriptorWasHiRes( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.hiResMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetHiRes( register DAQ_DESCRIPTOR_T* pThis,
                                          bool hiResMode )
{
   pThis->name.cControl.hiResMode = hiResMode;
}
#endif

/*! --------------------------------------------------------------------------
 * @brief Indicates whether the last record was received in "DAQ- mode".
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @retval true DAQ- mode was active.
 */
STATIC inline bool daqDescriptorWasDaq( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.cControl.daqMode;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline void daqDescriptorSetDaq( register DAQ_DESCRIPTOR_T* pThis,
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
STATIC inline bool daqDescriptorVerifyMode( register DAQ_DESCRIPTOR_T* pThis )
{
   return ( ((int)daqDescriptorWasPM( pThis ))    +
            ((int)daqDescriptorWasHiRes( pThis )) +
            ((int)daqDescriptorWasDaq( pThis )) == 1 );
}

/*! ----------------------------------------------------------------------------
 * @brief Returns true if it is a long block.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 * @retval true Long block   (has DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC words)
 * @retval false Short block (has DAQ_FIFO_DAQ_WORD_SIZE_CRC words)
 */
STATIC inline bool daqDescriptorIsLongBlock( register DAQ_DESCRIPTOR_T* pThis )
{
   return !daqDescriptorWasDaq( pThis );
}

/*! ----------------------------------------------------------------------------
 * @brief Returns true if it is a short block.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 * @retval true Long block   (has DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC words)
 * @retval false Short block (has DAQ_FIFO_DAQ_WORD_SIZE_CRC words)
 */
STATIC inline bool daqDescriptorIsShortBlock( register DAQ_DESCRIPTOR_T* pThis )
{
   return daqDescriptorWasDaq( pThis );
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the data block size in words (type DAQ_DATA_T).
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 * @return Number of data words of the block belonging to this descriptor.
 */
STATIC inline
size_t daqDescriptorGetBlockLen( register DAQ_DESCRIPTOR_T* pThis )
{
   return daqDescriptorIsLongBlock( pThis )?
             DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC : DAQ_FIFO_DAQ_WORD_SIZE_CRC;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the number of data words (type DAQ_DATA_T) of the
 *        payload part belonging to the block of this descriptor.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 * @return Number of payload data words of the block belonging to this
 *         descriptor.
 */
STATIC inline
size_t daqDescriptorGetPayloadLen( register DAQ_DESCRIPTOR_T* pThis )
{
   return daqDescriptorGetBlockLen( pThis ) - DAQ_DESCRIPTOR_WORD_SIZE;
}

/*! ---------------------------------------------------------------------------
 * @brief Gets the least significant word of the bus tag event
 *        trigger condition from the last record.
 * @see daqChannelGetTriggerConditionLW in daq.h
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type DAQ_DATA_T) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Least significant word of trigger condition.
 */
STATIC inline
DAQ_DATA_T daqDescriptorGetTriggerConditionLW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.low;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline
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
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Most significant word of trigger condition.
 */
STATIC inline
DAQ_DATA_T daqDescriptorGetTriggerConditionHW( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.high;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline
void daqDescriptorSetTriggerConditionHW( register DAQ_DESCRIPTOR_T* pThis,
                                         DAQ_DATA_T trHigh )
{
   pThis->name.trigger.high = trHigh;
}
#endif

/*! -------------------------------------------------------------------------
 */
STATIC inline
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
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Trigger delay
 */
STATIC inline
DAQ_DATA_T daqDescriptorGetTriggerDelay( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.trigger.delay;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline
void daqDescriptorSetTriggerDelay( register DAQ_DESCRIPTOR_T* pThis,
                                   DAQ_DATA_T delay )
{
   pThis->name.trigger.delay = delay;
}
#endif

/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 */
STATIC inline
uint32_t daqDescriptorGetTimeStampNanoSec( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.name.nSec;
}

/*! ---------------------------------------------------------------------------
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 */
STATIC inline
uint32_t daqDescriptorGetTimeStampSec( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.name.utSec;
}

/*! --------------------------------------------------------------------------
 * @brief Returns the white rabbit time from the given descriptor.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 * @return 64-bit white rabbit time-stamp.
 */
STATIC inline
uint64_t daqDescriptorGetTimeStamp( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.wr.timeStamp;
}

/*! ---------------------------------------------------------------------------
 * @brief Get the CRC of this record.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return 8 bit CRC
 */
STATIC inline
uint8_t daqDescriptorGetCRC( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.crcReg.crc;
}

#ifdef CONFIG_DAQ_SIMULATE_CHANNEL
STATIC inline
void daqDescriptorSetCRC( register DAQ_DESCRIPTOR_T* pThis, uint8_t crc )
{
   pThis->name.crcReg.crc = crc;
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns the sequence number of the current block.
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Sequence number.
 */
STATIC inline
DAQ_SEQUENCE_T daqDescriptorGetSequence( register DAQ_DESCRIPTOR_T* pThis )
{
   return pThis->name.crcReg.sequence;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the time-base in nanoseconds belonging to this data block
 * @param pThis Pointer to the DAQ- descriptor object, that means to the last
 *              10 received words (type uint16_t) of the received record.
 *              @see DAQ_DESCRIPTOR_WORD_SIZE
 * @return Timebase in nanoseconds [ns]
 */
STATIC inline
unsigned int daqDescriptorGetTimeBase( register DAQ_DESCRIPTOR_T* pThis )
{
   if( daqDescriptorWasHiRes( pThis ) )
      return 250;     /* 4 mHz */
   if( daqDescriptorWasPM( pThis ) )
      return 100000;  /* 10 kHz */
   if( pThis->name.cControl.Sample10us )
      return 10000;   /* 100 kHz */
   if( pThis->name.cControl.Sample100us )
      return 100000;  /* 10 kHz */
   if( pThis->name.cControl.Sample1ms )
      return 1000000; /* 1 kHz */

   return 0;
}

/*! @} */ //End of group  DAQ_DESCRIPTOR
#ifdef __cplusplus
} /* namespace daq */
} /* namespace Scu */
} /* extern "C" */
#endif
// Revision: 06.06.2019
#endif /* ifndef _DAQ_DESCRIPTOR_H */
/*================================== EOF ====================================*/


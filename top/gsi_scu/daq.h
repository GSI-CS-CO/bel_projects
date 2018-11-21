/*!
 *  @file daq.h
 *  @brief Control module for Data Acquisition Unit (DAQ)
 *  @date 13.11.2018
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
#ifndef _DAQ_H
#define _DAQ_H

#include "scu_bus.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * For the reason LM32 RAM consume can be reduced here is the possibility
 * to overwrite DAQ_MAX by the Makefile.
 */
#ifndef DAQ_MAX
   #define DAQ_MAX MAX_SCU_SLAVES //!< @brief Maximum number of DAQ's
#else
  #if DAQ_MAX > MAX_SCU_SLAVES
    #error Macro DAQ_MAX can not be greater than MAX_SCU_SLAVES !
  #endif
#endif

#ifndef DAQ_MAX_CHANNELS
  #define DAQ_MAX_CHANNELS 16 //!< @brief Maximum number of DAQ-channels
#else
  #if DAQ_MAX_CHANNELS > 16
    #error Not more than 16 channels per DAQ
  #endif
#endif

/*!
 * @brief Access indexes for writing and reading the DAQ-registers
 */
typedef enum
{
   CTRLREG         = 0x00,
   TRIG_LW         = 0x10,
   TRIG_HW         = 0x20,
   TRIG_DLY        = 0x30,
   PM_DAT          = 0x40,
   DAQ_DAT         = 0x50,
   DAQU_FIFO_WORDS = 0x70,
   PM_FIFO_WORDS   = 0x80
} DAQ_REGISTER_INDEX;

struct _CTRL_REG_T
{
   uint16_t Ena_PM:          1; //TODO
   uint16_t Sample10us:      1;
   uint16_t Sample100us:     1;
   uint16_t Sample1ms:       1;
   uint16_t ExtTrig_nEvTrig: 1;
};

union CTRL_REG_T
{
   volatile uint16_t total;
   volatile struct _CTRL_REG_T bf;
};
STATIC_ASSERT( sizeof( union CTRL_REG_T ) == sizeof(uint16_t) );

/*!
 * @brief Relative start address of the SCU registers
 *
 * Accessing to a SCU register will made as followed:
 * Absolute-register-address = SCU-bus-slave base_address + DAQ_REGISTER_OFFSET
 *                           + canal_number * sizeof(uint16_t)
 */
#define DAQ_REGISTER_OFFSET 0x4000

union DAQ_REGISTER_T;

/*!
 * @brief Object represents a single channel of a DAQ.
 */
struct DAQ_CANNEL_T
{
   void* p; //TODO
};

/*!
 * @brief Object represents a single SCU-Bus slave including a DAQ
 */
struct DAQ_DEVICE_T
{
   unsigned int slot;     //!< @brief Slot number of this DAQ. Range: 1..MAX_SCU_SLAVES
   unsigned int maxChannels; //!< @brief Number of DAQ-channels
   struct DAQ_CANNEL_T aChannel[DAQ_MAX_CHANNELS];
   union DAQ_REGISTER_T* volatile pReg; //!< @brief Pointer to DAQ-registers (start of address space)
};

/*!
 * @brief Object represents all on the SCU-bus connected DAQs.
 */
struct DAQ_ALL_T
{
   unsigned int foundDevices;         //!< @brief Number of found DAQs
   struct DAQ_DEVICE_T aDaq[DAQ_MAX]; //!< @brief Array of all possible existing DAQs
};

struct DAQ_DATA_T
{
   //TODO
};

union DAQ_REGISTER_T
{
   volatile uint16_t i[SCUBUS_SLAVE_ADDR_SPACE/sizeof(uint16_t)];
   volatile struct DAQ_DATA_T s;
};
STATIC_ASSERT( sizeof( union DAQ_REGISTER_T ) == SCUBUS_SLAVE_ADDR_SPACE );

/* ***********************  Building of DAQ descriptor **********************/
#define DAQ_DISCRIPTOR_WORD_SIZE 10 /*!< @brief byte size size of
                                     *  DAQ descriptor + crc */

struct _DAQ_BF_SLOT_DIOB
{
   uint16_t slot:   4;
   uint16_t unused: 7;
   uint16_t diobId: 5;
} PACKED_SIZE;
STATIC_ASSERT( sizeof( struct _DAQ_BF_SLOT_DIOB ) == sizeof(uint16_t));

struct _DAQ_BF_CANNEL_MODE
{
   uint8_t channelNumber: 5;
   uint8_t pmMode:        1;
   uint8_t hiResMode:     1;
   uint8_t daqMode:       1;
} PACKED_SIZE;
STATIC_ASSERT( sizeof( struct _DAQ_BF_CANNEL_MODE ) == sizeof(uint8_t) );

struct _DAQ_CHANNEL_CONTROL
{
   struct _DAQ_BF_CANNEL_MODE channelMode;
   uint8_t controlReg;
} PACKED_SIZE;
STATIC_ASSERT( sizeof( struct _DAQ_CHANNEL_CONTROL ) == sizeof(uint16_t) );

union _DAQ_WR_T
{
   uint8_t  byteIndex[sizeof(uint64_t)];
   uint16_t wordIndex[sizeof(uint64_t)/sizeof(uint16_t)];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   uint64_t timeStamp;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
   uint64_t timeStampToEndianConvert;
#endif
} PACKED_SIZE;
STATIC_ASSERT( sizeof(union _DAQ_WR_T) == sizeof(uint64_t) );

union _DAQ_USED_TRIGGER
{
   uint8_t  byteIndex[sizeof(uint32_t)];
   uint16_t wordIndex[sizeof(uint32_t)/sizeof(uint16_t)];
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   uint32_t usedTrigger;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
   uint32_t usedTriggerToEndianConvert;
#endif
} PACKED_SIZE;
STATIC_ASSERT( sizeof(union _DAQ_USED_TRIGGER) == sizeof(uint32_t) );

struct _DAQ_TRIGGER
{
   union _DAQ_USED_TRIGGER trigger;
   uint16_t delay;
} PACKED_SIZE;
STATIC_ASSERT( sizeof(struct _DAQ_TRIGGER) == 3 * sizeof(uint16_t) );

struct _DAQ_DISCRIPTOR_STRUCT_T
{
   struct _DAQ_BF_SLOT_DIOB    slotDiob;
   union  _DAQ_WR_T            wr;
   struct _DAQ_TRIGGER         trigger;
   struct _DAQ_CHANNEL_CONTROL cControl;
   uint8_t                     unused;
   uint8_t                     crc;
} PACKED_SIZE;
STATIC_ASSERT( sizeof( struct _DAQ_DISCRIPTOR_STRUCT_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof(uint16_t) );

/*!
 * @brief Final type of DAQ- descriptor
 */
union DAQ_DESCRIPTOR_T
{
   uint16_t index[DAQ_DISCRIPTOR_WORD_SIZE]; //!< @brief WORD-access by index
   struct _DAQ_DISCRIPTOR_STRUCT_T name;     //!< @brief Access by name
} PACKED_SIZE;
STATIC_ASSERT( sizeof( union DAQ_DESCRIPTOR_T ) == DAQ_DISCRIPTOR_WORD_SIZE * sizeof( uint16_t ) );

/* ****************** End of device descriptor building **********************/

/*! ------------------------------------------------------------------------
 * @brief Preinitialized the DAQ_ALL_T by zero and try to find all
 *        existing DAQs connected to SCU-bus.
 *
 * For each found DAQ the a element of DAQ_ALL_T::DAQ_DEVICE_T becomes initialized.
 *
 * @param pAllDAQ Pointer object of DAQ_ALL_T including a list of all DAQ.
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @retval -1 Error occurred.
 * @retval  0 No DAQ found.
 * @retval >0 Number of connected DAQ in SCU-bus.
 */
int daqFindAndInitializeAll( struct DAQ_ALL_T* pAllDAQ, const void* pScuBusBase );

/*! ---------------------------------------------------------------------------
 * @brief Returns the total number of DAQ input channels from all DAQ-slaves
 *        of the whole SCU-bus.
 * @param pAllDAQ Pointer object of DAQ_ALL_T including a list of all DAQ.
 * @note Function daqFindAndInitializeAll has to be invoked before!
 *
 */
int daqGetNumberOfAllFoundChannels( struct DAQ_ALL_T* pAllDAQ );

/*!
 * @brief Writes the given value in addressed register
 */
static inline void daqSetChannelReg( union DAQ_REGISTER_T* volatile preg,
                                     const DAQ_REGISTER_INDEX index,
                                     const unsigned int channel,
                                     const uint16_t value )
{
   LM32_ASSERT( channel < DAQ_MAX_CHANNELS );
   LM32_ASSERT( (index & 0x0F) == 0x00 );
   preg->i[index+channel] = value;
}

/*!
 * @brief Reads a value from a addressed register
 */
static inline uint16_t daqGetChannelReg( union DAQ_REGISTER_T* volatile preg,
                                         const DAQ_REGISTER_INDEX index,
                                         const unsigned int channel )
{
   LM32_ASSERT( channel < DAQ_MAX_CHANNELS );
   LM32_ASSERT( (index & 0x0F) == 0x00 );
   return preg->i[index+channel];
}


#ifdef __cplusplus
}
#endif
#endif /* ifndef _DAQ_H */
/*================================== EOF ====================================*/

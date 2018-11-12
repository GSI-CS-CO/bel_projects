/** @file scu_bus.h
 *  
 *  Copyright (C) 2011-2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch <s.rauch@gsi.de>
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

#ifndef __SCU_H
#define __SCU_H

#include <stdbool.h>
#include "inttypes.h"
#include "helper_macros.h"
#include "lm32_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CID_SYS           0x4
#define CID_GROUP         0x5
#define SLAVE_VERSION     0x6
#define SLAVE_INT_ENA     0x21
#define SLAVE_INT_PEND    0x22
#define SLAVE_INT_ACT     0x24
#define SLAVE_EXT_CLK     0x30

#define DAC1_BASE         0x200
#define DAC2_BASE         0x210
#define DAC_CNTRL         0x0
#define DAC_DATA          0x1

#define IO4x8             0x220
#define ADC_BASE          0x230
#define ADC_CNTRL         0x0
#define ADC_CHN1          0x1   
#define ADC_CHN2          0x2   
#define ADC_CHN3          0x3   
#define ADC_CHN4          0x4   
#define ADC_CHN5          0x5   
#define ADC_CHN6          0x6   
#define ADC_CHN7          0x7   
#define ADC_CHN8          0x8   


#define FG1_BASE          0x300
#define FG2_BASE          0x340
#define FG_CNTRL          0x0
#define FG_A              0x1
#define FG_B              0x2
#define FG_BROAD          0x3
#define FG_SHIFT          0x4
#define FG_STARTH         0x5
#define FG_STARTL         0x6
#define FG_RAMP_CNT_LO    0x7
#define FG_RAMP_CNT_HI    0x8
#define FG_TAG_LOW        0x9
#define FG_TAG_HIGH       0xa
#define FG_VER            0xb

#define FG1_IRQ           (1<<15)
#define FG2_IRQ           (1<<14)
#define DREQ              (1<<4) 

#define WB_FG_CNTRL       0x0
#define WB_FG_A           0x1
#define WB_FG_B           0x2
#define WB_FG_BROAD       0x3
#define WB_FG_SHIFTA      0x4
#define WB_FG_SHIFTB      0x5
#define WB_FG_START       0x6
#define WB_RAMP_CNT       0x7
#define WB_FG_SW_DST      0x8

#define TMR_BASE          0x330
#define TMR_CNTRL         0x0
#define TMR_IRQ_CNT       0x1
#define TMR_VALUEL        0x2
#define TMR_VALUEH        0x3
#define TMR_REPEAT        0x4
 
#define GLOBAL_IRQ_ENA    0x2
#define SRQ_ENA           0x6
#define SRQ_ACT           0x8
#define MULTI_SLAVE_SEL   0xc
#define MULTICAST_ACC     0x8
#define MAX_SCU_SLAVES    12    /*!< @brief Maximum number of slots */

#define SYS_LOEP    3
#define SYS_CSCO    55
#define SYS_PBRF    42

#define GRP_ADDAC1  3
#define GRP_ADDAC2  38
#define GRP_DIOB    26
#define GRP_FIB_DDS 1
#define GRP_MFU     2
#define GRP_SIO3    69
#define GRP_SIO2    23 

#define SCUBUS_INVALID_VALUE     (uint16_t)0xdead
#define SCUBUS_SLAVE_ADDR_SPACE  (1 << 17)         /*!< @brief Address space in bytes for each SCU bus slave 128k */

typedef uint16_t SCU_BUS_SLAVE_FLAGS_T;
STATIC_ASSERT( sizeof( SCU_BUS_SLAVE_FLAGS_T ) * 8 >= MAX_SCU_SLAVES );

extern struct w1_bus wrpc_w1_bus;
void ReadTemperatureDevices(int bus, uint64_t *id, uint16_t *temp);
void probe_scu_bus(volatile unsigned short*, unsigned short, unsigned short, int*);
void ReadTempDevices(int bus, uint64_t *id, uint32_t *temp);

/*!
 * @brief Calculates the relative address offset in bytes of a SCU bus slave
 *        from the given slot number.
 * @see MAX_SCU_SLAVES
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Relative slot address
 */
static inline uint32_t getSlotOffset( const unsigned int slot )
{
   LM32_ASSERT( slot > 0 );
   LM32_ASSERT( slot <= MAX_SCU_SLAVES );
   return slot * SCUBUS_SLAVE_ADDR_SPACE;
}

/*!
 * @brief Calculates the absolute address of a SCU bus slave from the
 *        given slot number.
 * @see MAX_SCU_SLAVES
 * @see find_device_adr
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return Absolute SCU bus slave address
 */
static inline void* getAbsScuBusSlaveAddr( const void* pScuBusBase,
                                           const unsigned int slot )
{
   return &(((uint8_t*)pScuBusBase)[getSlotOffset(slot)]);
}

/*!
 * @brief Reads a 16 bit register value from a SCU bus slave
 * @see getAbsScuBusSlaveAddr
 * @see setScuBusSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by getAbsScuBusSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @return Content of the addressed register
 */
static inline
uint16_t getScuBusSlaveValue16( const void* pAbsSlaveAddr, const unsigned int index )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < SCUBUS_SLAVE_ADDR_SPACE );
   LM32_ASSERT( ((int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!
   return ((uint16_t*)pAbsSlaveAddr)[index];
}

/*!
 * @brief Writes a given 16 bit value in the addressed SCU bus slave register.
 * @see getAbsScuBusSlaveAddr
 * @see getScuBusSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by getAbsScuBusSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param value 16 bit value to write
 */
static inline
void setScuBusSlaveValue16( void* pAbsSlaveAddr, const unsigned int index, const uint16_t value )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < SCUBUS_SLAVE_ADDR_SPACE );
   LM32_ASSERT( ((int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!
   ((uint16_t*)pAbsSlaveAddr)[index] = value;
}

/*!
 * @brief Reads a 32 bit register value from a SCU bus slave
 * @see getAbsScuBusSlaveAddr
 * @see setScuBusSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by getAbsScuBusSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @return Content of the addressed register
 */
static inline
uint32_t getScuBusSlaveValue32( const void* pAbsSlaveAddr, const unsigned int index )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < SCUBUS_SLAVE_ADDR_SPACE );
   LM32_ASSERT( ((int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!
   return ((uint32_t*)pAbsSlaveAddr)[index];
}

/*!
 * @brief Writes a given 32 bit value in the addressed SCU bus slave register.
 * @see getAbsScuBusSlaveAddr
 * @see getScuBusSlaveValue16
 * @param pAbsSlaveAddr Absolute SCU bus slave address e.g. obtained by getAbsScuBusSlaveAddr
 * @param index Location of relevant register to read, that means offset to pAbsSlaveAddr
 * @param value 16 bit value to write
 */
static inline
void setScuBusSlaveValue32( void* pAbsSlaveAddr, const unsigned int index, const uint32_t value )
{
   LM32_ASSERT( index >= 0 );
   LM32_ASSERT( index < SCUBUS_SLAVE_ADDR_SPACE );
   LM32_ASSERT( ((int)pAbsSlaveAddr % sizeof(uint16_t)) == 0 ); // At least 2 bytes alignment assumed!
   ((uint32_t*)pAbsSlaveAddr)[index] = value;
}

/*!
 * @brief Extract a single slave-present-flag from the SCU-slave-flag-present field
 * @see scuBusFindSpecificSlaves
 * @see scuFindAllSlaves
 * @param flags packed slave present flags of all SCU bus slots
 * @param slot Slot number, valid range 1 .. MAX_SCU_SLAVES (12)
 * @return true: slave present
 * @return false: slave not present
 */
static inline bool scuBusIsSlavePresent( const SCU_BUS_SLAVE_FLAGS_T flags, const int slot )
{
   LM32_ASSERT( slot > 0 );
   LM32_ASSERT( slot <= MAX_SCU_SLAVES );
   return ((flags & (1 << (slot-1))) != 0);
}

/*!
 * @brief Scans the whole SCU bus and initialized a slave-flags present field if
 *        the given system address and group address match.
 * @see scuBusIsSlavePresent
 * @see scuBusFindAllSlaves
 * @see find_device_adr
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param systemAddr System address
 * @param groupAddr  group address
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0000 0010 1000: means: Slot 4 and 6 are used by devices where \n
 *         system address and group address match.
 */
SCU_BUS_SLAVE_FLAGS_T scuBusFindSpecificSlaves( const void* pScuBusBase,
                                                const uint16_t systemAddr,
                                                const uint16_t grupAddr );

/*!
 * @brief Scans the whole SCU bus for all slots and initialized a slave-flags
 *        present field for each found device.
 * @see scuBusIsSlavePresent
 * @see scuBusFindSpecificSlaves
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0001 0001 0000: means: Slot 5 and 9 are used all others are free.
 */
SCU_BUS_SLAVE_FLAGS_T scuBusFindAllSlaves( const void* pScuBusBase );


#ifdef __cplusplus
}
#endif

#endif

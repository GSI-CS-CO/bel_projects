/*!
 *  @file daq_command_interface.h
 *  @brief Definition of DAQ-commandos and data object for shared memory
 *
 *  @note This file is suitable for LM32-apps within the SCU environment and
 *        for Linux applications.
 *  @date 27.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _DAQ_COMMAND_INTERFACE_H
#define _DAQ_COMMAND_INTERFACE_H

#include <generated/shared_mmap.h>
#include <scu_bus_defines.h>
#include <daq_ramBuffer.h>
#include <stdint.h>

/*!
 * @ingroup DAQ
 * @defgroup DAQ_INTERFACE
 * @brief DAQ communication module between Linux and LM32
 * @{
 */

#define DAQ_MAGIC_NUMBER           ((uint32_t)0xCAFEAD04)

#define DAQ_RET_ERR_UNKNOWN_OPERATION        -1
#define DAQ_RET_ERR_SLAVE_NOT_PRESENT        -2
#define DAQ_RET_ERR_CHANNEL_NOT_PRESENT      -3
#define DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND -4
#define DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE     -5
#define DAQ_RET_ERR_SLAVE_OUT_OF_RANGE       -6
#define DAQ_RET_ERR_WRONG_SAMPLE_PARAMETER   -7


#define DAQ_RET_OK                        0
#define DAQ_RET_RESCAN                    1

#ifndef DAQ_OP_OFFSET
  #define DAQ_OP_OFFSET 0
#endif

/*!
 * @brief Operation code to controlling the DAQs from host.
 */
typedef enum
{
   DAQ_OP_IDLE                   = 0,
   DAQ_OP_LOCK                   = DAQ_OP_OFFSET +  1,
   DAQ_OP_UNLOCK                 = DAQ_OP_OFFSET +  2,
   DAQ_OP_RESET                  = DAQ_OP_OFFSET +  3,
   DAQ_OP_GET_MACRO_VERSION      = DAQ_OP_OFFSET +  4,
   DAQ_OP_GET_SLOTS              = DAQ_OP_OFFSET +  5,
   DAQ_OP_GET_CHANNELS           = DAQ_OP_OFFSET +  6,
   DAQ_OP_RESCAN                 = DAQ_OP_OFFSET +  7,
   DAQ_OP_PM_ON                  = DAQ_OP_OFFSET +  8,
   DAQ_OP_HIRES_ON               = DAQ_OP_OFFSET +  9,
   DAQ_OP_PM_HIRES_OFF           = DAQ_OP_OFFSET + 10,
   DAQ_OP_CONTINUE_ON            = DAQ_OP_OFFSET + 11,
   DAQ_OP_CONTINUE_OFF           = DAQ_OP_OFFSET + 12,
   DAQ_OP_SET_TRIGGER_CONDITION  = DAQ_OP_OFFSET + 13,
   DAQ_OP_GET_TRIGGER_CONDITION  = DAQ_OP_OFFSET + 14,
   DAQ_OP_SET_TRIGGER_DELAY      = DAQ_OP_OFFSET + 15,
   DAQ_OP_GET_TRIGGER_DELAY      = DAQ_OP_OFFSET + 16,
   DAQ_OP_SET_TRIGGER_MODE       = DAQ_OP_OFFSET + 17,
   DAQ_OP_GET_TRIGGER_MODE       = DAQ_OP_OFFSET + 18
} DAQ_OPERATION_CODE_T;
STATIC_ASSERT( sizeof( DAQ_OPERATION_CODE_T ) == sizeof(uint32_t) );

typedef enum
{
   DAQ_SAMPLE_1MS      = 1,
   DAQ_SAMPLE_100US    = 2,
   DAQ_SAMPLE_10US     = 3
} DAQ_SAMPLE_RATE_T;

typedef struct PACKED_SIZE
{
   uint16_t  deviceNumber;
   uint16_t  channel;
} DAQ_CHANNEL_LOCATION_T;
STATIC_ASSERT( sizeof( DAQ_CHANNEL_LOCATION_T ) == 2 * sizeof(uint16_t));

typedef struct PACKED_SIZE
{
   DAQ_CHANNEL_LOCATION_T location;
   uint16_t param1;
   uint16_t param2;
   uint16_t param3;
   uint16_t param4;
} DAQ_OPERATION_IO_T;
STATIC_ASSERT( sizeof(DAQ_OPERATION_IO_T) == (sizeof(DAQ_CHANNEL_LOCATION_T)
                                            + 4 * sizeof( uint16_t ) ));

typedef int32_t DAQ_RETURN_CODE_T;

typedef struct PACKED_SIZE
{
   DAQ_OPERATION_CODE_T code;
   DAQ_RETURN_CODE_T    retCode;
   DAQ_OPERATION_IO_T   ioData;
} DAQ_OPERATION_T;
STATIC_ASSERT( sizeof(DAQ_OPERATION_T) == (sizeof(DAQ_OPERATION_CODE_T)
                                         + sizeof(DAQ_RETURN_CODE_T)
                                         + sizeof(DAQ_OPERATION_IO_T) ));
typedef struct PACKED_SIZE
{
   uint32_t                 magicNumber;
   RAM_RING_SHARED_OBJECT_T ramIndexes;
   DAQ_OPERATION_T          operation;
} DAQ_SHARED_IO_T;
STATIC_ASSERT( sizeof( DAQ_SHARED_IO_T ) == (sizeof(uint32_t)
                                           + sizeof(RAM_RING_SHARED_OBJECT_T)
                                           + sizeof(DAQ_OPERATION_T) ));
STATIC_ASSERT( sizeof( DAQ_SHARED_IO_T ) <= SHARED_SIZE );


/*!@} */
#endif /* ifndef _DAQ_COMMAND_INTERFACE_H */
/*================================== EOF ====================================*/

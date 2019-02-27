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
#include <helper_macros.h>
#include <stdint.h>

/*!
 * @ingroup DAQ
 * @defgroup DAQ_INTERFACE
 * @brief DAQ communication module between Linux and LM32
 * @{
 */

#define DAQ_MAGIC_NUMBER ((unit32_t)0xCAFEAD04)

#ifndef DAQ_OP_OFFSET
  #define DAQ_OP_OFFSET 0
#endif

/*!
 * @brief Operation code to controlling the DAQs from host.
 */
typedef enum
{
   DAQ_OP_NO          = 0,
   DAQ_OP_RESET       = DAQ_OP_OFFSET + 1,
   DAQ_OP_GET_STATUS  = DAQ_OP_OFFSET + 2
} DAQ_OPERATION_CODE_T;
STATIC_ASSERT( sizeof( DAQ_OPERATION_CODE_T ) == sizeof(uint32_t) );

/*!@} */
#endif /* ifndef _DAQ_COMMAND_INTERFACE_H */
/*================================== EOF ====================================*/

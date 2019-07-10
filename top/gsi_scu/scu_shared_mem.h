/*!
 *  @file scu_shared_mem.h
 *  @brief Definition of shared memory for communication of
 *         function generator between LM32 and Linux host
 *
 *  @date 10.07.2019
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
#ifndef _SCU_SHARED_MEM_H
#define _SCU_SHARED_MEM_H

#include <fg.h>
#include <helper_macros.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include <daq_command_interface.h>
#endif

typedef struct PACKED_SIZE
{
   uint64_t board_id;       /*!< 1Wire ID of the pcb temp sensor */
   uint64_t ext_id;         /*!< 1Wire ID of the extension board temp sensor */
   uint64_t backplane_id;   /*!< 1Wire ID of the backplane temp sensor */
   uint32_t board_temp;     /*!< temperature value of the pcb sensor */
   uint32_t ext_temp;       /*!< temperature value of the extension board sensor */
   uint32_t backplane_temp; /*!< temperature value of the backplane sensor */
   uint32_t fg_magic_number;
   uint32_t fg_version;     /*!< 0x2 saftlib, 0x3 new msi system with mailbox */
   uint32_t fg_mb_slot;
   uint32_t fg_num_channels;
   uint32_t fg_buffer_size;
   uint32_t fg_macros[MAX_FG_MACROS]; // hi..lo bytes: slot, device, version, output-bits
   struct channel_regs fg_regs[MAX_FG_CHANNELS];
   struct channel_buffer fg_buffer[MAX_FG_CHANNELS];
   struct daq_buffer daq_buf;
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   DAQ_SHARED_IO_T daq;
#endif
} SCU_SHARED_DATA_T;

#define GET_SCU_SHM_OFFSET( m ) offsetof( SCU_SHARED_DATA_T, m )

#ifndef __DOXYGEN__

#endif


#define FG_MAGIC_NUMBER ((uint32_t)0xdeadbeef)

#define SCU_INVALID_VALUE -1
#define FG_VERSION         0x03

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #define __DAQ_SHARAD_MEM_INITIALIZER_ITEM \
             , .daq = DAQ_SHARAD_MEM_INITIALIZER
#else
  #define __DAQ_SHARAD_MEM_INITIALIZER_ITEM
#endif

#define SCU_SHARED_DATA_INITIALIZER        \
{                                          \
   .board_id         = SCU_INVALID_VALUE,  \
   .ext_id           = SCU_INVALID_VALUE,  \
   .backplane_id     = SCU_INVALID_VALUE,  \
   .board_temp       = SCU_INVALID_VALUE,  \
   .ext_temp         = SCU_INVALID_VALUE,  \
   .backplane_temp   = SCU_INVALID_VALUE,  \
   .fg_magic_number  = FG_MAGIC_NUMBER,    \
   .fg_version       = FG_VERSION,         \
   .fg_mb_slot       = SCU_INVALID_VALUE,  \
   .fg_num_channels  = MAX_FG_CHANNELS,    \
   .fg_buffer_size   = BUFFER_SIZE,        \
   .fg_macros        = {0},                \
   .daq_buf          = {0}                 \
   __DAQ_SHARAD_MEM_INITIALIZER_ITEM       \
}

#endif
/*================================== EOF ====================================*/

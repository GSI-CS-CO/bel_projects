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
#include <cb.h>
#include <helper_macros.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include <daq_command_interface.h>
  #ifdef __cplusplus
     #define __DAQ_SHARED_IO_T Scu::daq::DAQ_SHARED_IO_T
  #else
     #define __DAQ_SHARED_IO_T DAQ_SHARED_IO_T
  #endif
#endif

#ifdef __cplusplus
namespace Scu
{
namespace FG
{
#endif

/*! ---------------------------------------------------------------------------
 * @brief Definition of shared memory area for the communication between LM32
 *        and Linux.
 */
typedef struct PACKED_SIZE
{
   uint64_t board_id;       /*!<@brief 1Wire ID of the pcb temp sensor */
   uint64_t ext_id;         /*!<@brief 1Wire ID of the extension board temp sensor */
   uint64_t backplane_id;   /*!<@brief 1Wire ID of the backplane temp sensor */
   uint32_t board_temp;     /*!<@brief temperature value of the pcb sensor */
   uint32_t ext_temp;       /*!<@brief temperature value of the extension board sensor */
   uint32_t backplane_temp; /*!<@brief temperature value of the backplane sensor */
   uint32_t fg_magic_number;
   uint32_t fg_version;     /*!<@brief 0x2 saftlib, 0x3 new msi system with mailbox */
   uint32_t fg_mb_slot;
   uint32_t fg_num_channels;
   uint32_t fg_buffer_size;
   uint32_t fg_macros[MAX_FG_MACROS]; // hi..lo bytes: slot, device, version, output-bits
   struct channel_regs fg_regs[MAX_FG_CHANNELS];
   struct channel_buffer fg_buffer[MAX_FG_CHANNELS];
#ifndef CONFIG_MIL_DAQ_USE_RAM
   struct daq_buffer daq_buf;
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
   __DAQ_SHARED_IO_T daq;
#endif
} SCU_SHARED_DATA_T;


#define GET_SCU_SHM_OFFSET( m ) offsetof( SCU_SHARED_DATA_T, m )

#ifndef __DOXYGEN__
/*
 * Unfortunately necessary because in some elements of some sub-structures of
 * the main structure SCU_SHARED_DATA_T are still declared as "unsigned int",
 * and it's never guaranteed that the type "unsigned int" is a 32 bit type
 * on all platforms!
 */
STATIC_ASSERT( sizeof(uint32_t) == sizeof(unsigned int) );
STATIC_ASSERT( sizeof(int32_t) == sizeof(signed int) );
STATIC_ASSERT( sizeof(uint16_t) == sizeof(signed short) );

/*
 * We have to made a static check verifying whether the structure-format
 * is equal on both platforms: Linux and LM32.
 */
STATIC_ASSERT( offsetof( param_set, coeff_a ) == 0 );
STATIC_ASSERT( offsetof( param_set, coeff_b ) ==
               offsetof( param_set, coeff_a ) + sizeof( uint16_t ));
STATIC_ASSERT( offsetof( param_set, coeff_c ) ==
               offsetof( param_set, coeff_b ) + sizeof( uint16_t ));
STATIC_ASSERT( offsetof( param_set, control ) ==
               offsetof( param_set, coeff_c ) + sizeof(int32_t) );
STATIC_ASSERT( sizeof( param_set ) ==
               offsetof( param_set, control ) + sizeof(uint32_t) );

STATIC_ASSERT( sizeof( channel_buffer ) == BUFFER_SIZE * sizeof( param_set ) );

STATIC_ASSERT( offsetof( channel_regs, wr_ptr  ) == 0 );
STATIC_ASSERT( offsetof( channel_regs, rd_ptr  ) ==
               offsetof( channel_regs, wr_ptr  ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( channel_regs, mbx_slot ) ==
               offsetof( channel_regs, rd_ptr  ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( channel_regs, macro_number ) ==
               offsetof( channel_regs, mbx_slot ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( channel_regs, ramp_count ) ==
               offsetof( channel_regs, macro_number ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( channel_regs, tag ) ==
               offsetof( channel_regs, ramp_count ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( channel_regs, state ) ==
               offsetof( channel_regs, tag ) + sizeof( uint32_t ));
STATIC_ASSERT( sizeof( channel_regs ) ==
               offsetof( channel_regs, state ) + sizeof( uint32_t ));

STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, board_id ) == 0 );
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, ext_id ) ==
               offsetof( SCU_SHARED_DATA_T, board_id ) + sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, backplane_id ) ==
               offsetof( SCU_SHARED_DATA_T, ext_id ) + sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, board_temp ) ==
               offsetof( SCU_SHARED_DATA_T, backplane_id ) + sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, ext_temp ) ==
               offsetof( SCU_SHARED_DATA_T, board_temp ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, backplane_temp ) ==
               offsetof( SCU_SHARED_DATA_T, ext_temp ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_magic_number ) ==
               offsetof( SCU_SHARED_DATA_T, backplane_temp ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_version ) ==
               offsetof( SCU_SHARED_DATA_T, fg_magic_number ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_mb_slot ) ==
               offsetof( SCU_SHARED_DATA_T, fg_version ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_num_channels ) ==
               offsetof( SCU_SHARED_DATA_T, fg_mb_slot ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_buffer_size ) ==
               offsetof( SCU_SHARED_DATA_T, fg_num_channels ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_macros ) ==
               offsetof( SCU_SHARED_DATA_T, fg_buffer_size ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_regs ) ==
               offsetof( SCU_SHARED_DATA_T, fg_macros ) + MAX_FG_MACROS * sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_buffer ) ==
               offsetof( SCU_SHARED_DATA_T, fg_regs ) + MAX_FG_CHANNELS * sizeof( struct channel_regs ));
#ifndef CONFIG_MIL_DAQ_USE_RAM
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, daq_buf ) ==
               offsetof( SCU_SHARED_DATA_T, fg_buffer ) + MAX_FG_CHANNELS * sizeof( struct channel_buffer ));
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, daq ) ==
               offsetof( SCU_SHARED_DATA_T, daq_buf ) + sizeof( struct daq_buffer ));
STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
               offsetof( SCU_SHARED_DATA_T, daq ) + sizeof( __DAQ_SHARED_IO_T ));
#else
STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
               offsetof( SCU_SHARED_DATA_T, daq_buf ) + sizeof( struct daq_buffer ));
#endif
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

#ifndef CONFIG_MIL_DAQ_USE_RAM
   #define __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM , .daq_buf = {0}
#else
   #define __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM
#endif

/*! ---------------------------------------------------------------------------
 * @brief Initializer of the entire LM32 shared memory of application
 *        scu_control.
 */
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
   .fg_macros        = {0}                 \
   __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM   \
   __DAQ_SHARAD_MEM_INITIALIZER_ITEM       \
}

#ifdef __cplusplus
} /* namespace FG */

namespace MiLdaq
{
#endif

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqAdressByChannel( const unsigned int channel )
{
   return (channel >> 16) & 0xFF;
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqAddress( const register struct daq* pMilDaq )
{
   return getMilDaqAdressByChannel( pMilDaq->channel );
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqLocationByChannel( const unsigned int channel )
{
   return channel >> 24;
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqLocation( const register struct daq* pMilDaq )
{
   return getMilDaqLocationByChannel( pMilDaq->channel );
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getDaqMilScuBusSlotbyLocation( const unsigned int loc )
{
   return loc & 0x0F;
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getDaqMilExrentionByLocation( const unsigned int loc )
{
   return loc >> 4;
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqScuBusSlot( const register struct daq* pMilDaq )
{
   return getDaqMilScuBusSlotbyLocation( getMilDaqLocation( pMilDaq ));
}

/*! ---------------------------------------------------------------------------
 */
static inline
unsigned int getMilDaqScuMilExtention( const register struct daq* pMilDaq )
{
   return getDaqMilExrentionByLocation( getMilDaqLocation( pMilDaq ) );
}

#ifdef __cplusplus
} // namespace MiLdaq
} // namespace Scu
#endif

#endif /* ifndef _SCU_SHARED_MEM_H */
/*================================== EOF ====================================*/

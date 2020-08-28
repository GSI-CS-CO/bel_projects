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
 *  @todo Include this file in "saftlib/drivers/fg_regs.h" and
 *        replace or define the constants and offset-addresses defined
 *        in fg_regs.h by the constants and offest-addresses defined
 *        in this file, reducing dangerous redundancy.\n
 *        <b>That is highly recommended!</b>\n
 *        The dependencies building "SAFTLIB" has to be also depend on this
 *        file!\n
 *        Find a well defined place for this file, where the front end group
 *        AND the timing group can access.
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

/*!
 * @defgroup SHARED_MEMORY Sheard memory for communication between
 *                         LM32 and Linux.
 */

#include <helper_macros.h>
#include <scu_mailbox.h>
#include <scu_function_generator.h>
#include <scu_circular_buffer.h>
#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #include <daq_command_interface.h>
  #ifdef __cplusplus
     #define __DAQ_SHARED_IO_T Scu::daq::DAQ_SHARED_IO_T
  #else
     /*
      * @ingroup SHARED_MEMORY
      */
     #define __DAQ_SHARED_IO_T DAQ_SHARED_IO_T
  #endif
#else
  #ifdef CONFIG_MIL_DAQ_USE_RAM
    #include <daq_ramBuffer.h>
  #endif
#endif

#ifdef __cplusplus
extern "C"
{
namespace Scu
{
namespace FG
{
#endif

#ifdef CONFIG_MIL_DAQ_USE_RAM

typedef uint16_t MIL_DAQ_T;

typedef struct PACKED_SIZE
{
   uint64_t   timestamp;
   MIL_DAQ_T  setValue;
   MIL_DAQ_T  actValue;
   FG_MACRO_T fgMacro;
} MIL_DAQ_RAM_ITEM_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( MIL_DAQ_RAM_ITEM_T, timestamp ) == 0 );
STATIC_ASSERT( offsetof( MIL_DAQ_RAM_ITEM_T, setValue ) == sizeof(uint64_t) );
STATIC_ASSERT( offsetof( MIL_DAQ_RAM_ITEM_T, actValue ) ==
               offsetof( MIL_DAQ_RAM_ITEM_T, setValue ) + sizeof(MIL_DAQ_T) );
STATIC_ASSERT( offsetof( MIL_DAQ_RAM_ITEM_T, channel ) ==
               offsetof( MIL_DAQ_RAM_ITEM_T, actValue ) + sizeof(MIL_DAQ_T) );
STATIC_ASSERT( sizeof( MIL_DAQ_RAM_ITEM_T ) ==
                offsetof( MIL_DAQ_RAM_ITEM_T, channel ) + sizeof(uint32_t) );
#endif

/*!
 * @brief Number of required RAM-items per Mil-DAQ item
 */
#define RAM_ITEM_PER_MIL_DAQ_ITEM                                   \
   (sizeof( MIL_DAQ_RAM_ITEM_T ) / sizeof( RAM_DAQ_PAYLOAD_T ) +    \
    !!(sizeof( MIL_DAQ_RAM_ITEM_T ) % sizeof( RAM_DAQ_PAYLOAD_T )))

typedef union PACKED_SIZE
{
   RAM_DAQ_PAYLOAD_T  ramPayload[RAM_ITEM_PER_MIL_DAQ_ITEM];
   MIL_DAQ_RAM_ITEM_T item;
} MIL_DAQ_RAM_ITEM_PAYLOAD_T;

#else /* ifdef CONFIG_MIL_DAQ_USE_RAM */

 #ifdef __cplusplus
   #define _MIL_DAQ_BUFFER_T Scu::MiLdaq::MIL_DAQ_BUFFER_T
 #else
   #define _MIL_DAQ_BUFFER_T MIL_DAQ_BUFFER_T
 #endif

#endif /* else ifdef CONFIG_MIL_DAQ_USE_RAM */

#define CONFIG_USE_RESCAN_FLAG /* A very bad idea!!! :-( */

/*! ---------------------------------------------------------------------------
 * @ingroup SHARED_MEMORY
 * @brief Definition of shared memory area for the communication between LM32
 *        and Linux.
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc#Memory_map_of_the_LM32_ram
 * @see saftlib/drivers/fg_regs.h
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief 1Wire ID of the pcb temp sensor
    * @see BOARD_ID in saftlib/drivers/fg_regs.
    * @todo Check if this variable is really necessary in the future.
    */
   uint64_t            board_id;

   /*!
    * @brief 1Wire ID of the extension board temp sensor
    * @see EXT_ID in saftlib/drivers/fg_regs.h
    * @todo Check if this variable is really necessary in the future.
    */
   uint64_t            ext_id;

   /*!
    * @brief 1Wire ID of the backplane temp sensor
    * @see BACKPLANE_ID in saftlib/drivers/fg_regs.h
    * @todo Check if this variable is really necessary in the future.
    */
   uint64_t            backplane_id;

   /*!
    * @brief temperature value of the pcb sensor
    * @see BOARD_TEMP in saftlib/drivers/fg_regs.h
    * @todo Check if this variable is really necessary in the future.
    */
   uint32_t            board_temp;

   /*!
    * @brief temperature value of the extension board sensor
    * @see EXT_TEMP in saftlib/drivers/fg_regs.h
    * @todo Check if this variable is really necessary in the future.
    */
   uint32_t            ext_temp;

   /*!
    * @brief temperature value of the backplane sensor
    * @see BACKPLACE_TEMP in saftlib/drivers/fg_regs.h
    * @todo Check if this variable is really necessary in the future.
    */
   uint32_t            backplane_temp;

   /*!
    * @brief Magic number for recognizing the LM32 firmware.
    * @see FG_MAGIC_NUMBER in saftlib/drivers/fg_regs.h
    */
   uint32_t            fg_magic_number;

   /*!
    * @brief Version of this firmware
    *        0x2 saftlib, 0x3 new msi system with
    * @see FG_VERSION in saftlib/drivers/fg_regs.h
    */
   uint32_t            fg_version;

   /*!
    * @brief Mailbox-slot for host => LM32
    * @see FG_MB_SLOT saftlib/drivers/fg_regs.h
    * @see FunctionGeneratorFirmware::ScanFgChannels() in
    *      saftlib/drivers/FunctionGeneratorFirmware.cpp
    * @see FunctionGeneratorFirmware::ScanMasterFg() in
    *      saftlib/drivers/FunctionGeneratorFirmware.cpp
    */
   uint32_t            fg_mb_slot;

   /*!
    * @brief Maximum number of function generator channels which can
    *        support this SCU.
    * @see MAX_FG_CHANNELS
    * @see FG_NUM_CHANNELS in saftlib/drivers/fg_regs.h
    * @see FunctionGeneratorImpl::acquireChannel() in
    *      saftlib/drivers/FunctionGeneratorImpl.cpp
    * @todo Check if this variable is really necessary in the future,
    *       this information can be obtained by macro ARRAY_SIZE(fg_macros)
    *       or MAX_FG_CHANNELS. Once this file becomes included in the sources of
    *       SAFTLIB as well.
    */
   uint32_t            fg_num_channels;

   /*!
    * @brief Maximum size of the data buffer for a single function generator channel.
    * @see FG_BUFFER_SIZE saftlib/drivers/fg_regs.h
    * @see BUFFER_SIZE
    * @todo Check if this variable is really necessary in the future,
    *       this information can be obtained by macro BUFFER_SIZE
    *       Once this file becomes included in the sources of
    *       SAFTLIB as well.
    */
   uint32_t            fg_buffer_size;

   /*!
    * @brief  Array of found function generator channels of
    *         this SCU. \n Bytes: slot, device, version, output-bits
    *
    * This array becomes initialized by scan_all_fgs
    *
    * @see scan_all_fgs
    * @see FG_MACROS in saftlib/drivers/fg_regs.h
    */
   FG_MACRO_T          fg_macros[MAX_FG_MACROS];

   /*!
    * @see FG_REGS_BASE_ in saftlib/drivers/fg_regs.h
    * @see FunctionGeneratorImpl::acquireChannel() in
    *      saftlib/drivers/FunctionGeneratorImpl.cpp
    */
   FG_CHANNEL_REG_T    fg_regs[MAX_FG_CHANNELS];

   /*!
    * @see FunctionGeneratorImpl::refill()
    *      in saftlib/drivers/FunctionGeneratorImpl.cpp
    */
   FG_CHANNEL_BUFFER_T fg_buffer[MAX_FG_CHANNELS];

#ifdef CONFIG_MIL_DAQ_USE_RAM
   /*!
    * @brief MIL-DAQ ring-buffer administration indexes
    *        for DDR3-RAM.
    */
   RAM_RING_INDEXES_T  mdaqRing;
#else
   /*!
    * @brief MIL-DAQ-ring-buffer object in LM32 shared memory
    */
   _MIL_DAQ_BUFFER_T   daq_buf;
 #ifdef CONFIG_USE_RESCAN_FLAG
   /*!
    * @see FG_SCAN_DONE in saftlib/drivers/fg_regs.h
    * @see FunctionGeneratorFirmware::firmware_rescan in
    *      saftlib/drivers/FunctionGeneratorFirmware.cpp
    * @todo <b>Remove this f... flag ASAP!!!!</b>
    */
   uint32_t            fg_rescan_busy;
 #endif
#endif

#ifdef CONFIG_SCU_DAQ_INTEGRATION
   /*!
    * @brief Shared memory objects of non-MIL-DAQs
    */
   __DAQ_SHARED_IO_T sDaq;
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
STATIC_ASSERT( offsetof( FG_PARAM_SET_T, coeff_a ) == 0 );
STATIC_ASSERT( offsetof( FG_PARAM_SET_T, coeff_b ) ==
               offsetof( FG_PARAM_SET_T, coeff_a ) + sizeof( uint16_t ));
STATIC_ASSERT( offsetof( FG_PARAM_SET_T, coeff_c ) ==
               offsetof( FG_PARAM_SET_T, coeff_b ) + sizeof( uint16_t ));
STATIC_ASSERT( offsetof( FG_PARAM_SET_T, control ) ==
               offsetof( FG_PARAM_SET_T, coeff_c ) + sizeof(int32_t) );
STATIC_ASSERT( sizeof( FG_PARAM_SET_T ) ==
               offsetof( FG_PARAM_SET_T, control ) + sizeof(uint32_t) );

STATIC_ASSERT( sizeof( FG_CHANNEL_BUFFER_T ) ==
               BUFFER_SIZE * sizeof( FG_PARAM_SET_T ) );

STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, wr_ptr  ) == 0 );
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, rd_ptr  ) ==
               offsetof( FG_CHANNEL_REG_T, wr_ptr  ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, mbx_slot ) ==
               offsetof( FG_CHANNEL_REG_T, rd_ptr  ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, macro_number ) ==
               offsetof( FG_CHANNEL_REG_T, mbx_slot ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, ramp_count ) ==
               offsetof( FG_CHANNEL_REG_T, macro_number ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, tag ) ==
               offsetof( FG_CHANNEL_REG_T, ramp_count ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( FG_CHANNEL_REG_T, state ) ==
               offsetof( FG_CHANNEL_REG_T, tag ) + sizeof( uint32_t ));
STATIC_ASSERT( sizeof( FG_CHANNEL_REG_T ) ==
               offsetof( FG_CHANNEL_REG_T, state ) + sizeof( uint32_t ));

STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, board_id ) == 0 );
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, ext_id ) ==
               offsetof( SCU_SHARED_DATA_T, board_id ) + sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, backplane_id ) ==
               offsetof( SCU_SHARED_DATA_T, ext_id ) + sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, board_temp ) ==
               offsetof( SCU_SHARED_DATA_T, backplane_id ) +
               sizeof( uint64_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, ext_temp ) ==
               offsetof( SCU_SHARED_DATA_T, board_temp ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, backplane_temp ) ==
               offsetof( SCU_SHARED_DATA_T, ext_temp ) + sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_magic_number ) ==
               offsetof( SCU_SHARED_DATA_T, backplane_temp ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_version ) ==
               offsetof( SCU_SHARED_DATA_T, fg_magic_number ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_mb_slot ) ==
               offsetof( SCU_SHARED_DATA_T, fg_version ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_num_channels ) ==
               offsetof( SCU_SHARED_DATA_T, fg_mb_slot ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_buffer_size ) ==
               offsetof( SCU_SHARED_DATA_T, fg_num_channels ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_macros ) ==
               offsetof( SCU_SHARED_DATA_T, fg_buffer_size ) +
               sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_regs ) ==
               offsetof( SCU_SHARED_DATA_T, fg_macros ) +
               MAX_FG_MACROS * sizeof( uint32_t ));
STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_buffer ) ==
               offsetof( SCU_SHARED_DATA_T, fg_regs ) +
               MAX_FG_CHANNELS * sizeof( FG_CHANNEL_REG_T ));
#ifdef CONFIG_MIL_DAQ_USE_RAM
 STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, mdaqRing ) ==
                offsetof( SCU_SHARED_DATA_T, fg_buffer ) +
                MAX_FG_CHANNELS * sizeof( FG_CHANNEL_BUFFER_T ));
#else
 STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, daq_buf ) ==
                offsetof( SCU_SHARED_DATA_T, fg_buffer ) +
                MAX_FG_CHANNELS * sizeof( FG_CHANNEL_BUFFER_T ));
#endif
#ifdef CONFIG_SCU_DAQ_INTEGRATION
 #ifdef CONFIG_MIL_DAQ_USE_RAM
   STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, sDaq ) ==
                  offsetof( SCU_SHARED_DATA_T, mdaqRing ) +
                  sizeof( RAM_RING_INDEXES_T ));
 #else
  #ifdef  CONFIG_USE_RESCAN_FLAG
   STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, fg_rescan_busy ) ==
                  offsetof( SCU_SHARED_DATA_T, daq_buf ) +
                  sizeof( _MIL_DAQ_BUFFER_T ));

   STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, sDaq ) ==
                  offsetof( SCU_SHARED_DATA_T, fg_rescan_busy ) +
                  sizeof( uint32_t ));
  #else
   STATIC_ASSERT( offsetof( SCU_SHARED_DATA_T, sDaq ) ==
                  offsetof( SCU_SHARED_DATA_T, daq_buf ) +
                  sizeof( _MIL_DAQ_BUFFER_T ));
  #endif
 #endif
 STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
                offsetof( SCU_SHARED_DATA_T, sDaq ) +
                sizeof( __DAQ_SHARED_IO_T ));
#else /* ifdef CONFIG_SCU_DAQ_INTEGRATION */
 #ifdef CONFIG_MIL_DAQ_USE_RAM
  STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
                 offsetof( SCU_SHARED_DATA_T, mdaqRing ) +
                 sizeof( RAM_RING_INDEXES_T ));
 #else
  #ifdef  CONFIG_USE_RESCAN_FLAG
    STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
                   offsetof( SCU_SHARED_DATA_T, fg_rescan_busy ) +
                   sizeof( uint32_t ));
  #else
    STATIC_ASSERT( sizeof( SCU_SHARED_DATA_T ) ==
                   offsetof( SCU_SHARED_DATA_T, daq_buf ) +
                   sizeof( _MIL_DAQ_BUFFER_T ));
  #endif
 #endif
#endif /* / ifdef CONFIG_SCU_DAQ_INTEGRATION */
#endif /* ifndef __DOXYGEN__ */


#define FG_MAGIC_NUMBER ((uint32_t)0xdeadbeef)

#define SCU_INVALID_VALUE -1

#ifndef FG_VERSION
  #define FG_VERSION         3
#endif

#if (FG_VERSION != 3)
  #warning "Could be incompatiple to SAFTLIB!"
#endif

#ifdef CONFIG_SCU_DAQ_INTEGRATION
  #define __DAQ_SHARAD_MEM_INITIALIZER_ITEM \
             , .sDaq = DAQ_SHARAD_MEM_INITIALIZER
#else
  #define __DAQ_SHARAD_MEM_INITIALIZER_ITEM
#endif

#ifdef CONFIG_USE_RESCAN_FLAG
  #define __RESCAN_BUSY_INITIALIZER , .fg_rescan_busy = 0
#else
  #define __RESCAN_BUSY_INITIALIZER
#endif

#ifdef CONFIG_MIL_DAQ_USE_RAM
  #define __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM \
     , .mdaqRing = RAM_RING_INDEXES_MDAQ_INITIALIZER
#else
  #define __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM \
     , .daq_buf = {0}                           \
     __RESCAN_BUSY_INITIALIZER
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
   .fg_macros        = {{0,0,0,0}}         \
   __MIL_DAQ_SHARAD_MEM_INITIALIZER_ITEM   \
   __DAQ_SHARAD_MEM_INITIALIZER_ITEM       \
}

typedef enum
{
   FG_OP_INITIALIZE          = 0,
   FG_OP_RFU                 = 1,
   FG_OP_CONFIGURE           = 2, // SWI_ENABLE
   FG_OP_DISABLE_CHANNEL     = 3, // SWI_DISABLE
   FG_OP_RESCAN              = 4, // SWI_SCAN
   FG_OP_CLEAR_HANDLER_STATE = 5,
   FG_OP_PRINT_HISTORY       = 6
} FG_OP_CODE_T;

/*!
 * @brief Helper function for debug purposes only.
 */
static inline const char* fgCommand2String( const FG_OP_CODE_T op )
{
   #define __FG_COMMAND_CASE( cmd ) case cmd: return #cmd
   switch( op )
   {
      __FG_COMMAND_CASE( FG_OP_INITIALIZE );
      __FG_COMMAND_CASE( FG_OP_RFU );
      __FG_COMMAND_CASE( FG_OP_CONFIGURE );
      __FG_COMMAND_CASE( FG_OP_DISABLE_CHANNEL );
      __FG_COMMAND_CASE( FG_OP_RESCAN );
      __FG_COMMAND_CASE( FG_OP_CLEAR_HANDLER_STATE );
      __FG_COMMAND_CASE( FG_OP_PRINT_HISTORY );
   }
   return "unknown";
   #undef __FG_COMMAND_CASE
}

/*!
 * @ingroup MAILBOX
 * @brief Definition of signals to send from server (LM32) to client
 * @see sendSignal
 * @see saftlib/drivers/fg_regs.h
 * @see FunctionGeneratorImpl::irq_handler in saftlib/drivers/FunctionGeneratorImpl.cpp
 */
typedef enum
{
   IRQ_DAT_REFILL         = 0,
   IRQ_DAT_START          = 1,
   IRQ_DAT_STOP_EMPTY     = 2, /*!<@brief normal stop */
   IRQ_DAT_STOP_NOT_EMPTY = 3, /*!<@brief something went wrong */
   IRQ_DAT_ARMED          = 4,
   IRQ_DAT_DISARMED       = 5
} SIGNAL_T;

/*!
 * @brief Helper function for debug purposes only.
 */
static inline const char* signal2String( const SIGNAL_T sig )
{
   #define __SIGNAL_CASE( sig ) case sig: return #sig
   switch( sig )
   {
      __SIGNAL_CASE( IRQ_DAT_REFILL );
      __SIGNAL_CASE( IRQ_DAT_START );
      __SIGNAL_CASE( IRQ_DAT_STOP_EMPTY );
      __SIGNAL_CASE( IRQ_DAT_STOP_NOT_EMPTY );
      __SIGNAL_CASE( IRQ_DAT_ARMED );
      __SIGNAL_CASE( IRQ_DAT_DISARMED );
   }
   return "unknown";
   #undef  __SIGNAL_CASE
}

#ifdef __cplusplus
} /* namespace FG */

namespace MiLdaq
{
#endif

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getSocketByFgMacro( const FG_MACRO_T fgMacro )
{
   return fgMacro.socket;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getDeviceByFgMacro( const FG_MACRO_T fgMacro )
{
   return fgMacro.device;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getFgMacroVersion( const FG_MACRO_T fgMacro )
{
   return fgMacro.version;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getFgOutputBits( const FG_MACRO_T fgMacro )
{
   return fgMacro.outputBits;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getMilDaqDevice( const register MIL_DAQ_OBJ_T* pMilDaq )
{
   return getDeviceByFgMacro( pMilDaq->fgMacro );
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getMilDaqSocket( const register MIL_DAQ_OBJ_T* pMilDaq )
{
   return getSocketByFgMacro( pMilDaq->fgMacro );
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getDaqMilScuBusSlotbySocket( const unsigned int socket )
{
   return socket & SCU_BUS_SLOT_MASK;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getDaqMilExtentionBySocket( const unsigned int socket )
{
   return socket >> 4;
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getMilDaqScuBusSlot( const register MIL_DAQ_OBJ_T* pMilDaq )
{
   return getDaqMilScuBusSlotbySocket( getMilDaqSocket( pMilDaq ));
}

/*! ---------------------------------------------------------------------------
 */
STATIC inline ALWAYS_INLINE
unsigned int getMilDaqScuMilExtention( const register MIL_DAQ_OBJ_T* pMilDaq )
{
   return getDaqMilExtentionBySocket( getMilDaqSocket( pMilDaq ) );
}
#ifdef __cplusplus
} // namespace MiLdaq
} // namespace Scu
} // extern "C"
#endif

#endif /* ifndef _SCU_SHARED_MEM_H */
/*================================== EOF ====================================*/

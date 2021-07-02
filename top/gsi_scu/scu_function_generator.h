/*!
 * @file scu_function_generator.h
 * @brief SCU-Function generator module for LM32.
 * @note Header only
 * @date 21.10.2019
 * @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @see https://www-acc.gsi.de/wiki/bin/viewauth/Hardware/Intern/ScuFgDoc
 *
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @note This file is suitable for LM32 and Linux.
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
#ifndef _SCU_FUNCTION_GENERATOR_H
#define _SCU_FUNCTION_GENERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <helper_macros.h>
#include <scu_bus_defines.h>

#ifdef __lm32__
  #define LINUX_CONST
  #define LM32_CONST const
#else
  #define LINUX_CONST const
  #define LM32_CONST
#endif


#ifdef __cplusplus
extern "C"
{
namespace Scu
{
#endif



/*!
 * @brief Bit masks of the function generators control register.
 * @see https://www-acc.gsi.de/wiki/Hardware/Intern/FunctionGeneratorQuadratic#cntrl_reg
 * @see FG_CTRL_RG_T_BV
 */
typedef enum
{
   /*!
    * @brief Function generator reset mask.
    */
   FG_RESET   = (1 << 0),

   /*!
    * @brief Function generator enable mask.
    */
   FG_ENABLED = (1 << 1),

   /*!
    * @brief Function generator running indicator mask.
    */
   FG_RUNNING = (1 << 2),

   /*!
    * @brief Function generator data request indicator mask.
    */
   FG_DREQ    = (1 << 3),

   /*!
    * @brief Mask for function generator number.
    */
   FG_NUMBER  = 0x03F0,

   /*!
    * @brief Mask for the function generators polynomial step.
    */
   FG_STEP    = 0x1C00,

   /*!
    * @brief Mask for the function generators frequency select.
    */
   FG_FREQU   = 0xE000
} FG_MASK_T;



/*!
 * @brief Definition of flag-masks and constants for ADDAC/ACU- and MIL-
 *        function generators.
 */
typedef enum
{
  /*!
   * @ingroup SHARED_MEMORY
   * @brief Maximum of function generator macros
   */
   MAX_FG_MACROS =    256,

  /*!
   * @ingroup SHARED_MEMORY
   * @brief Maximum of supported function generator channels
   *
   * Its the size of the array of FG_CHANNEL_BUFFER_T and FG_CHANNEL_REG_T
   *
   * @see FG_CHANNEL_BUFFER_T
   * @see FG_CHANNEL_REG_T
   */
   MAX_FG_CHANNELS =  16,

  /*!
   * @ingroup SHARED_MEMORY
   * @brief Maximum number of function generator channels per SCU-bus slave
   */
   MAX_FG_PER_SLAVE =  2,

  /*!
   * @ingroup SHARED_MEMORY
   * @brief Maximum number of polynomial (of type FG_PARAM_SET_T)
   *        per function generator channel.
   *
   * Its the number of polynomials containing in FG_CHANNEL_BUFFER_T
   *
   * @see FG_PARAM_SET_T
   * @see FG_CHANNEL_BUFFER_T
   */
   BUFFER_SIZE         =    121,

   FG_REFILL_THRESHOLD = BUFFER_SIZE * 40 / 100,

   OUTPUT_BITS         = 24,
   MIL_EXT             = 1,
   MAX_SIO3            = ADD_NAMESPACE( Bus, MAX_SCU_SLAVES ),
   IFK_MAX_ADR         = 254,
   GRP_IFA8            = 24,
   IFA_ID              = 0xCC,
   IFA_VERS            = 0xCD,
   DRQ_BIT             = (1 << 10),
   DEV_DRQ             = (1 << 0),
   DEV_STATE_IRQ       = (1 << 1),
   MIL_EXT_SLOT        = ADD_NAMESPACE( Bus, MAX_SCU_SLAVES ) + 1,
   DEV_SIO             = 0x20,
   DEV_MIL_EXT         = 0x10,
   FC_CNTRL_WR         = (0x14 << 8),
   FC_COEFF_A_WR       = (0x15 << 8),
   FC_COEFF_B_WR       = (0x16 << 8),
   FC_SHIFT_WR         = (0x17 << 8),
   FC_START_L_WR       = (0x18 << 8),
   FC_START_H_WR       = (0x19 << 8),
   FC_CNTRL_RD         = (0xA0 << 8),
   FC_COEFF_A_RD       = (0xA1 << 8),
   FC_COEFF_B_RD       = (0xA2 << 8),
   FC_IRQ_STAT         = (0xC9 << 8),
   FC_IRQ_MSK          = (0x12 << 8),
   FC_IRQ_ACT_RD       = (0xA7 << 8),
   FC_IRQ_ACT_WR       = (0x21 << 8),
   FC_IFAMODE_WR       = (0x60 << 8),
   FC_BLK_WR           = (0x6B << 8),
   FC_ACT_RD           = (0x81 << 8),

   /*!
    * @brief Mask for extracting the SCU-bus slot- respectively slave-number
    *        from the socket number.
    */
   SCU_BUS_SLOT_MASK   = 0x000F,

   /*!
    * @brief Bit mask for "set-value not valid flag" which is integrated
    *        in the element outputBits of FG_MACRO_T.
    * @note This flag is in the ring-buffer data present only!
    * @see FG_MACRO_T::outputBits
    */
   SET_VALUE_NOT_VALID_MASK = (1 << (BIT_SIZEOF(uint8_t)-1)),

   /*!
    * @brief Mask for obtaining the number of output bits of
    *        the element  outputBits of FG_MACRO_T.
    * @note This mask it in the ring-buffer necessary only.
    * @see FG_MACRO_T::outputBits
    */
   OUTPUT_BIT_MASK           = ~SET_VALUE_NOT_VALID_MASK

} FG_CONSTANT_T;

/*!
 * @ingroup SHARED_MEMORY
 * @brief Info type of a discovered function generator.
 * @see SCU_SHARED_DATA_T::fg_macros
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Contains the slot number and the MIL connection
    * @see SCU_BUS_SLOT_MASK
    * @see DEV_SIO
    * @see DEV_MIL_EXT
    * @see isNonMilFg
    * @see isMilScuBusFg
    * @see isMilExtentionFg
    * @see getFgSlotNumber
    */
   uint8_t socket;

   /*!
    * @brief Mil device address in the case of MIL FG or
    *        device number in the case of non MIL FG.
    */
   uint8_t device;

   /*!
    * @brief Version of function generator.
    */
   uint8_t version;

   /*!
    * @brief Number of output bits.
    * @see SET_VALUE_NOT_VALID_MASK
    * @see OUTPUT_BIT_MASK
    */
   uint8_t outputBits;
} FG_MACRO_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( FG_MACRO_T, socket ) == 0 );
STATIC_ASSERT( offsetof( FG_MACRO_T, device ) == offsetof( FG_MACRO_T, socket ) + sizeof( uint8_t ) );
STATIC_ASSERT( offsetof( FG_MACRO_T, outputBits ) == offsetof( FG_MACRO_T, version ) + sizeof( uint8_t ) );
STATIC_ASSERT( sizeof( FG_MACRO_T ) == sizeof( uint32_t ) );
#endif

/*!
 * @brief Constants for mask out the bit values of the polynomials
 *        control register.
 * @see FG_CONTROL_REG_T
 * @see FG_PARAM_SET_T
 */
typedef enum
{
   PSET_STEP     = 0x00000007,
   PSET_FREQU    = 0x00000038,
   PSET_SHIFT_B  = 0x00000FC0,
   PSET_SHIFT_A  = 0x0003F000
} PSET_CONTROLREG_MASK_T;

/*!
 * @see https://github.com/GSI-CS-CO/bel_projects/blob/proposed_master/modules/function_generators/fg_quad/fg_quad_scu_bus.vhd
 * @see FG_PARAM_SET_T
 */
typedef struct PACKED_SIZE
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__DOXYGEN__)
   unsigned int __not_used__: 14;
   unsigned int shift_a:       6;
   unsigned int shift_b:       6;
   unsigned int frequency:     3;
   unsigned int step:          3;
#else
   unsigned int step:          3;
   unsigned int frequency:     3;
   unsigned int shift_b:       6;
   unsigned int shift_a:       6;
   unsigned int __not_used__: 14;
#endif
} FG_CONTROL_REG_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof(FG_CONTROL_REG_T) == sizeof(uint32_t) );
#endif

/*!
 * @brief Helper type for bit or integer access
 * @see FG_CONTROL_REG_T
 * @see FG_PARAM_SET_T
 */
typedef union PACKED_SIZE
{
   uint32_t          i32;
   FG_CONTROL_REG_T  bv;
} __FG_CONTROL_REG_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( __FG_CONTROL_REG_T ) == sizeof(uint32_t) );
#endif

/*!
 * @ingroup SHARED_MEMORY
 * @brief Polynomial type for function generator.
 * @see send_fg_param
 * @see configure_fg_macro
 *
 * Meaning of the polynomial coefficients:\n
 * @f$ f(x) = coeff\_a \times 2^{shift\_a - 1} \times x^2
 *          + coeff\_b \times 2^{shift\_b} \times x
 *          + coeff\_c \times 2^{32} @f$
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Polynomial coefficient 'a'
    */
   int16_t coeff_a;

   /*!
    * @brief Polynomial coefficient 'b'
    */
   int16_t coeff_b;

   /*!
    * @brief Polynomial coefficient 'c'
    * @note Its also the set-value for MIL-DAQs
    */
   int32_t coeff_c;

   /*!
    * @brief Control register \n
    * Bit [0:2]   Step \n
    * Bit [3:5]   Frequency \n
    * Bit [6:11]  Shift 'b' \n
    * Bit [12:17] Shift 'a'
    * @todo Use a bit field structure in attention of the
    *       endianes convention rather than uint32_t.
    */
   __FG_CONTROL_REG_T control;
} FG_PARAM_SET_T;


#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_PARAM_SET_T ) == 12 );
#endif

/*!
 * @see FG_CHANNEL_REG_T::state
 */
typedef enum
{
   STATE_STOPPED = 0,
   STATE_ACTIVE  = 1,
   STATE_ARMED   = 2
} FG_REG_STATE_T;

/*!
 * @ingroup SHARED_MEMORY
 * @see SCU_SHARED_DATA_T::fg_regs
 * @see FG_REGS_BASE_ in saftlib/drivers/fg_regs.h
 * @see FunctionGeneratorImpl::acquireChannel in
 *      saftlib/drivers/FunctionGeneratorImpl.cpp
 */
typedef struct PACKED_SIZE
{
   /*!
    * @brief Write index
    * @see FG_WPTR in saftlib/drivers/fg_regs.h
    */
   uint32_t       wr_ptr;

   /*!
    * @brief Read index
    * @see FG_RPTR in saftlib/drivers/fg_regs.h
    */
   uint32_t       rd_ptr;

   /*!
    * @brief mbx slot
    * @see FG_MBX_SLOT in saftlib/drivers/fg_regs.
    * @see FunctionGeneratorImpl::acquireChannel in
    *      FunctionGeneratorImpl.cpp
    */
   LM32_CONST uint32_t mbx_slot;

   /*!
    * @brief Link-index of found FG list
    * @see FG_MACRO_T
    * @see FG_MACRO_NUM in saftlib/drivers/fg_regs.h
    */
   uint32_t       macro_number;

   /*!
    * @brief Ramp counter
    * @see FG_RAMP_COUNT in saftlib/drivers/fg_regs.h
    */
   LINUX_CONST uint32_t ramp_count;

   /*!
    * @brief Tag for non-MIL- function generators
    * @see configure_fg_macro
    * @see FG_TAG in saftlib/drivers/fg_regs.h
    */
   LM32_CONST uint32_t  tag;

   /*!
    * @brief Current function generator state
    *        meaning private to LM32
    * @see FG_STATE in saftlib/drivers/fg_regs.h
    */
   LINUX_CONST FG_REG_STATE_T state;
} FG_CHANNEL_REG_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_REG_STATE_T ) == sizeof( uint32_t ) );
STATIC_ASSERT( sizeof( FG_CHANNEL_REG_T ) == sizeof( uint32_t ) * 7 );
#endif

/*!
 * @ingroup SHARED_MEMORY
 * @brief Buffer (vector) of polynomials for a single function generator channel.
 * @see FG_PARAM_SET_T
 */
typedef struct PACKED_SIZE
{
  FG_PARAM_SET_T pset[BUFFER_SIZE];
} FG_CHANNEL_BUFFER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_CHANNEL_BUFFER_T ) == sizeof( FG_PARAM_SET_T ) * BUFFER_SIZE );
#endif

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" in the case the function generator belonging to the
 *        given socket is a "non MIL function generator".
 */
GSI_DEPRECATED
ALWAYS_INLINE STATIC inline bool isNonMilFg( const unsigned int socket )
{
   return (socket & (DEV_MIL_EXT | DEV_SIO)) == 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" in the case the function generator belonging to the
 *        given socket is a "non MIL function generator".
 */
ALWAYS_INLINE STATIC inline
bool isAddacFg( const unsigned int socket )
{
   return (socket >= ADD_NAMESPACE( Bus, SCUBUS_START_SLOT )) &&
          (socket <= ADD_NAMESPACE( Bus, MAX_SCU_SLAVES ));
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" in the case the function generator belonging to the
 *        given socket is a MIL function generator connected via SCU-bus slave.
 */
ALWAYS_INLINE STATIC inline
bool isMilScuBusFg( const unsigned int socket )
{
   return (socket & DEV_SIO) != 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" in the case the function generator belonging to the
 *        given socket is connected via MIL extension.
 */
ALWAYS_INLINE STATIC inline
bool isMilExtentionFg( const unsigned int socket )
{
   return (socket & DEV_MIL_EXT) != 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns "true" in the case the function generator is a MIL device.
 */
ALWAYS_INLINE STATIC inline
bool isMilFg( const unsigned int socket )
{
   return (socket & (DEV_MIL_EXT | DEV_SIO)) != 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the SCU bus slot number from the given socket.
 */
ALWAYS_INLINE STATIC inline
unsigned int getFgSlotNumber( const unsigned int socket )
{
   return socket & SCU_BUS_SLOT_MASK;
}


#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SCU_FUNCTION_GENERATOR_H */
/*================================== EOF ====================================*/

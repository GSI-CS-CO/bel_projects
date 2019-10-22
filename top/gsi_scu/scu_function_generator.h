/*!
 *  @file scu_function_generator.h
 *  @brief SCU-Function generator module for LM32.
 *
 *  @date 21.10.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch perhaps...
 *  @revision Ulrich Becker <u.becker@gsi.de>
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

#include <helper_macros.h>
// 12 SIOs with dev busses and 1 mil extension
#ifdef __cplusplus
extern "C"
{
namespace Scu
{
#endif

#define   MAX_FG_MACROS     256
#define   MAX_FG_CHANNELS   16
#define   MAX_FG_PER_SLAVE  2
#define   BUFFER_SIZE       121
#define   THRESHOLD         BUFFER_SIZE * 40 / 100
#define   OUTPUT_BITS       24
#define   MIL_EXT           1
#define   MAX_SIO3          MAX_SCU_SLAVES 
#define   IFK_MAX_ADR       254
#define   GRP_IFA8          24
#define   IFA_ID            0xcc 
#define   IFA_VERS          0xcd 

#define FG_RUNNING    0x4
#define FG_ENABLED    0x2
#define FG_DREQ       0x8
#define DRQ_BIT       (1 << 10)
#define DEV_DRQ       (1 << 0)
#define DEV_STATE_IRQ (1 << 1)
#define MIL_EXT_SLOT  13
#define DEV_SIO       0x20
#define DEV_MIL_EXT   0x10
#define FC_CNTRL_WR   (0x14 << 8)
#define FC_COEFF_A_WR (0x15 << 8)
#define FC_COEFF_B_WR (0x16 << 8)
#define FC_SHIFT_WR   (0x17 << 8)
#define FC_START_L_WR (0x18 << 8)
#define FC_START_H_WR (0x19 << 8)
#define FC_CNTRL_RD   (0xa0 << 8)
#define FC_COEFF_A_RD (0xa1 << 8)
#define FC_COEFF_B_RD (0xa2 << 8)
#define FC_IRQ_STAT   (0xc9 << 8)
#define FC_IRQ_MSK    (0x12 << 8)
#define FC_IRQ_ACT_RD (0xa7 << 8)
#define FC_IRQ_ACT_WR (0x21 << 8)
#define FC_IFAMODE_WR (0x60 << 8)
#define FC_BLK_WR     (0x6b << 8)
#define FC_ACT_RD     (0x81 << 8)

typedef struct PACKED_SIZE
{
   uint8_t socket;
   uint8_t device;
   uint8_t version;
   uint8_t outputBits;
} FG_MACRO_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( offsetof( FG_MACRO_T, socket ) == 0 );
STATIC_ASSERT( offsetof( FG_MACRO_T, device ) == offsetof( FG_MACRO_T, socket ) + sizeof( uint8_t ) );
STATIC_ASSERT( offsetof( FG_MACRO_T, outputBits ) == offsetof( FG_MACRO_T, version ) + sizeof( uint8_t ) );
STATIC_ASSERT( sizeof( FG_MACRO_T ) == sizeof( uint32_t ) );
#endif

typedef struct PACKED_SIZE
{
  int16_t coeff_a;
  int16_t coeff_b;
  int32_t coeff_c;
  uint32_t control; /* Bit 2..0   step
                               5..3   freq
                              11..6   shift_b
                              17..12  shift_a */
} FG_PARAM_SET_T;


#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_PARAM_SET_T ) == 12 );
#endif

typedef enum
{
   STATE_STOPPED = 0,
   STATE_ACTIVE  = 1,
   STATE_ARMED   = 2
} FG_REG_STATE_T;

typedef struct PACKED_SIZE
{
  uint32_t       wr_ptr;
  uint32_t       rd_ptr;
  uint32_t       mbx_slot;
  uint32_t       macro_number;
  uint32_t       ramp_count;
  uint32_t       tag;
  FG_REG_STATE_T state; // meaning private to LM32
} FG_CHANNEL_REG_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_REG_STATE_T ) == sizeof( uint32_t ) );
STATIC_ASSERT( sizeof( FG_CHANNEL_REG_T ) == sizeof( uint32_t ) * 7 );
#endif


typedef struct PACKED_SIZE
{
  FG_PARAM_SET_T pset[BUFFER_SIZE];
} FG_CHANNEL_BUFFER_T;

#ifndef __DOXYGEN__
STATIC_ASSERT( sizeof( FG_CHANNEL_BUFFER_T ) == sizeof( FG_PARAM_SET_T ) * BUFFER_SIZE );
#endif

//#pragma pack(pop)

#ifdef __lm32__
void scan_all_fgs( volatile uint16_t *base_adr,
                   volatile unsigned int* mil_base,
                   FG_MACRO_T* fglist,
                   uint64_t *ext_id );

void init_buffers( FG_CHANNEL_REG_T* cr,
                   const unsigned int channel,
                   FG_MACRO_T* macro,
                   volatile uint16_t* scub_base,
                   volatile unsigned int* devb_base) ;
#endif /* ifdef __lm32__ */

#ifdef __cplusplus
} /* namespace Scu */
} /* extern "C" */
#endif

#endif /* ifndef _SCU_FUNCTION_GENERATOR_H */
/*================================== EOF ====================================*/

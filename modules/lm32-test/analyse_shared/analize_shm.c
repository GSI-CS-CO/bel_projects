/*!
 *
 * @brief     Testprogram for using shared memory in LM32.
 *
 * @file      shared.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      26.11.2018
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
#include <string.h>
#include <stdbool.h>
#include "eb_console_helper.h"
#include "scu_lm32_macros.h"
#include "lm32_assert.h"
#include "shared_memory_helper.h"
#include "generated/shared_mmap.h"

//#define PAKED_SIZE_TEST PACKED_SIZE
#define PAKED_SIZE_TEST
#define SHARED_TEST SHARED
//#define SHARED_TEST
// 32952
// 32440

#define SHARED_BASE (INT_BASE_ADR + SHARED_OFFS)

#define REL_ADDR( x ) ( ((int)&(x)) - SHARED_BASE )

#if 1
#define   MAX_FG_MACROS     256
#define   MAX_FG_CHANNELS   16
#define HISTSIZE 	512

typedef struct // defined in history.h
{
   uint64_t timeStamp;
   char *message;
   char padding;
   char padding1;
   unsigned char associatedData;
   /* add more fields here  */
} PAKED_SIZE_TEST HistItem;

struct channel_regs // defined in fg.h
{
  unsigned int wr_ptr;
  unsigned int rd_ptr;
  unsigned int mbx_slot;
  unsigned int macro_number;
  unsigned int ramp_count;
  unsigned int tag;
  unsigned int state; // meaning private to LM32
} PAKED_SIZE_TEST;

struct param_set // defined in fg.h
{
  signed short coeff_a;
  signed short coeff_b;
  signed int coeff_c;
  unsigned int control; /*!< @brief Bit 2..0   step
                               5..3   freq
                              11..6   shift_b
                              17..12  shift_a */
} PAKED_SIZE_TEST; //12 Bytes
#define BUFFER_SIZE       121
struct channel_buffer // defined in fg.h
{
  struct param_set pset[BUFFER_SIZE];
} PAKED_SIZE_TEST;

// #define CONFIG_NO_STRUCT

#ifdef CONFIG_NO_STRUCT

#define _S(cont) cont

uint64_t SHARED_TEST board_id           = -1;
uint64_t SHARED_TEST ext_id             = -1;
uint64_t SHARED_TEST backplane_id       = -1;
uint32_t SHARED_TEST board_temp         = -1;
uint32_t SHARED_TEST ext_temp           = -1;
uint32_t SHARED_TEST backplane_temp     = -1;
uint32_t SHARED_TEST fg_magic_number    = 0xdeadbeef;
uint32_t SHARED_TEST fg_version         = 0x3; //!< @brief 0x2 saftlib,
                                          // 0x3 new msi system with mailbox
uint32_t SHARED_TEST fg_mb_slot               = -1;
uint32_t SHARED_TEST fg_num_channels          = MAX_FG_CHANNELS;
uint32_t SHARED_TEST fg_buffer_size           = BUFFER_SIZE;
uint32_t SHARED_TEST fg_macros[MAX_FG_MACROS] = {0}; //!< @brief hi..lo bytes: slot, device, version, output-bits 256
struct channel_regs  SHARED_TEST fg_regs[MAX_FG_CHANNELS]; // 16
struct channel_buffer  SHARED_TEST fg_buffer[MAX_FG_CHANNELS];
HistItem SHARED_TEST histbuf[HISTSIZE];

#else
typedef struct PAKED_SIZE_TEST
{
   uint64_t board_id;
   uint64_t ext_id;
   uint64_t backplane_id;
   uint32_t board_temp;
   uint32_t ext_temp;
   uint32_t backplane_temp;
   const uint32_t fg_magic_number;
   const uint32_t fg_version; //!< @brief 0x2 saftlib, 0x3 new msi system with mailbox
   uint32_t fg_mb_slot;
   const uint32_t fg_num_channels;
   const uint32_t fg_buffer_size;
   uint32_t fg_macros[MAX_FG_MACROS]; //!< @brief hi..lo bytes: slot, device, version, output-bits 256
   struct channel_regs fg_regs[MAX_FG_CHANNELS]; // 16
   struct channel_buffer fg_buffer[MAX_FG_CHANNELS];
   HistItem histbuf[HISTSIZE];
} volatile SHM_T;


#if 1
STATIC_ASSERT( offsetof( SHM_T, board_id ) == 0 );
STATIC_ASSERT( offsetof( SHM_T, ext_id ) == 8 );
STATIC_ASSERT( offsetof( SHM_T, backplane_id ) == 16 );
STATIC_ASSERT( offsetof( SHM_T, board_temp) == 24 );
STATIC_ASSERT( offsetof( SHM_T, ext_temp) == 28 );
STATIC_ASSERT( offsetof( SHM_T, backplane_temp) == 32 );
STATIC_ASSERT( offsetof( SHM_T, fg_magic_number) == 36 );
STATIC_ASSERT( offsetof( SHM_T, fg_version) == 40 );
STATIC_ASSERT( offsetof( SHM_T, fg_mb_slot) == 44 );
STATIC_ASSERT( offsetof( SHM_T, fg_num_channels ) == 48 );
STATIC_ASSERT( offsetof( SHM_T, fg_buffer_size ) == 52 );
STATIC_ASSERT( offsetof( SHM_T, fg_macros ) == 56 );
STATIC_ASSERT( offsetof( SHM_T, fg_regs )   == 1080 );
STATIC_ASSERT( offsetof( SHM_T, fg_buffer ) == 1528 );
STATIC_ASSERT( offsetof( SHM_T, histbuf ) == 24760 );
STATIC_ASSERT( sizeof( SHM_T ) == 32952 );
STATIC_ASSERT( sizeof( SHM_T ) <= SHARED_SIZE );
#endif

SHM_T SHARED_TEST g_shared =
{
   .board_id        = -1,
   .ext_id          = -1,
   .backplane_id    = -1,
   .board_temp      = -1,
   .ext_temp        = -1,
   .backplane_temp  = -1,
   .fg_magic_number = 0xdeadbeef,
   .fg_version      = 0x3, //!< @brief 0x2 saftlib,
   .fg_mb_slot      = -1,
   .fg_num_channels = MAX_FG_CHANNELS,
   .fg_buffer_size  = BUFFER_SIZE,
   .fg_macros       = {0} //!< @brief hi..lo bytes: slot, device, version, output-bits 256
};

#define _S(cont) g_shared.cont

#endif

#endif

void main( void )
{
   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Analyzing shared memory...\n");
   mprintf( "Address of board_id:        0x%08x, %d\n", &_S(board_id),        REL_ADDR(_S(board_id)) );
   mprintf( "Address of ext_id:          0x%08x, %d\n", &_S(ext_id),          REL_ADDR(_S(ext_id)));
   mprintf( "Address of backplane_id:    0x%08x, %d\n", &_S(backplane_id),    REL_ADDR(_S(backplane_id)));
   mprintf( "Address of board_temp:      0x%08x, %d\n", &_S(board_temp),      REL_ADDR(_S(board_temp)));
   mprintf( "Address of ext_temp:        0x%08x, %d\n", &_S(ext_temp),        REL_ADDR(_S(ext_temp)));
   mprintf( "Address of backplane_temp:  0x%08x, %d\n", &_S(backplane_temp),  REL_ADDR(_S(backplane_temp)));
   mprintf( "Address of fg_magic_number: 0x%08x, %d\n", &_S(fg_magic_number), REL_ADDR(_S(fg_magic_number)));
   mprintf( "Address of fg_version:      0x%08x, %d\n", &_S(fg_version),      REL_ADDR(_S(fg_version)));
   mprintf( "Address of fg_mb_slot:      0x%08x, %d\n", &_S(fg_mb_slot),      REL_ADDR(_S(fg_mb_slot)));
   mprintf( "Address of fg_num_channels: 0x%08x, %d\n", &_S(fg_num_channels), REL_ADDR(_S(fg_num_channels)));
   mprintf( "Address of fg_buffer_size:  0x%08x, %d\n", &_S(fg_buffer_size),  REL_ADDR(_S(fg_buffer_size)));
   mprintf( "Address of fg_macros:       0x%08x, %d\n", _S(fg_macros),        REL_ADDR(*_S(fg_macros)));
   mprintf( "Address of fg_regs:         0x%08x, %d\n", _S(fg_regs),          REL_ADDR(*_S(fg_regs)));
   mprintf( "Address of fg_buffer:       0x%08x, %d\n", _S(fg_buffer),        REL_ADDR(*_S(fg_buffer)));
   mprintf( "Address of histbuf:         0x%08x, %d\n", _S(histbuf),          REL_ADDR(*_S(histbuf)) );
   mprintf( "Address max:                0x%08x\n", (int)_S(histbuf) + sizeof(_S(histbuf)) );
   mprintf( "Size:                       %d\n\n", (int)_S(histbuf) + sizeof(_S(histbuf)) - (int)&_S(board_id) );
#ifndef CONFIG_NO_STRUCT
   mprintf( "Struct-Size:                %d\n\n", sizeof(g_shared) );
#endif

   mprintf( "Size of HistItem       %d\n", sizeof(HistItem) );
   mprintf( "Size of histbuf        %d\n\n", sizeof( _S(histbuf) ));

   mprintf( "Size of channel_regs   %d\n", sizeof( struct channel_regs));
   mprintf( "Size of fg_regs        %d\n\n", sizeof(_S(fg_regs)));

   mprintf( "Size of param_set      %d\n", sizeof(struct param_set ));
   mprintf( "Size of channel_buffer %d\n", sizeof(struct channel_buffer ));
   mprintf( "Size of fg_buffer      %d\n", sizeof( _S(fg_buffer) ));
   mprintf( "Size of fg_macros      %d\n", sizeof( _S(fg_macros) ));
}

/*================================== EOF ====================================*/

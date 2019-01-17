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

#if 1
#define   MAX_FG_MACROS     256
#define   MAX_FG_CHANNELS   16
#define HISTSIZE 	512

typedef struct
{
   uint64_t timeStamp;
   char *message;
   char padding;
   char padding1;
   unsigned char associatedData;
   /* add more fields here  */
} PAKED_SIZE_TEST HistItem;

struct channel_regs
{
  unsigned int wr_ptr;
  unsigned int rd_ptr;
  unsigned int mbx_slot;
  unsigned int macro_number;
  unsigned int ramp_count;
  unsigned int tag;
  unsigned int state; // meaning private to LM32
} PAKED_SIZE_TEST;

struct param_set
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
struct channel_buffer
{
  struct param_set pset[BUFFER_SIZE];
} PAKED_SIZE_TEST;


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
#endif

void main( void )
{
   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Analyzing shared memory...\n");
   mprintf( "Address of board_id:   0x%08x\n", &board_id );
   mprintf( "Address of histbuf:    0x%08x\n", histbuf );
   mprintf( "Address max:           0x%08x\n", (int)histbuf + sizeof(histbuf) );
   mprintf( "Size:                  %d\n\n", (int)histbuf + sizeof(histbuf) - (int)&board_id );

   mprintf( "Size of HistItem       %d\n", sizeof(HistItem) );
   mprintf( "Size of histbuf        %d\n\n", sizeof( histbuf ));



   mprintf( "Size of channel_regs   %d\n", sizeof(struct channel_regs ));
   mprintf( "Size of fg_regs        %d\n\n", sizeof(fg_regs));

   mprintf( "Size of param_set      %d\n", sizeof(struct param_set ));
   mprintf( "Size of channel_buffer %d\n", sizeof(struct channel_buffer ));
   mprintf( "Size of fg_buffer      %d\n", sizeof( fg_buffer ));
}

/*================================== EOF ====================================*/

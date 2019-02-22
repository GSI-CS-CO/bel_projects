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
#include "scu_assert.h"
#include "shared_memory_helper.h"
#include "shared.h"

volatile IO_T SHARED io =
{
   .a = 1,
   .b = 42,
   .c = 4711,
   .bf =
   {
      .a = 1,
      .b = 2,
      .c = 3
   },
   .sb =
   {
      .a = 0xAA,
      .b = 0x55,
   }
};

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Shared memory test %d\n", COMPILER_VERSION_NUMBER );

   uint32_t* pCpuRamExternal = shmGetRelatedEtherBoneAddress(SHARED_OFFS);
   if( pCpuRamExternal == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Could not find external WB address of my own RAM !\n" ESC_NORMAL);
      return;
   }

   mprintf( "intern: &io.a:  0x%x extern: 0x%x\n", &io.a,  ((uint32_t)pCpuRamExternal) + offsetof( IO_T, a ) );
   mprintf( "intern: &io.b:  0x%x extern: 0x%x\n", &io.b,  ((uint32_t)pCpuRamExternal) + offsetof( IO_T, b ) );
   mprintf( "intern: &io.c:  0x%x extern: 0x%x\n", &io.c,  ((uint32_t)pCpuRamExternal) + offsetof( IO_T, c ) );
   mprintf( "intern: &io.bf: 0x%x extern: 0x%x\n", &io.bf, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, bf ) );

   while( true )
   {
      while( io.a == 0 );
      printIO( &io );
      io.a = 0;
   }
}

/*================================== EOF ====================================*/

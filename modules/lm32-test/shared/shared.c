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

typedef struct
{
   uint32_t a;
   uint32_t b;
   uint32_t c;
} IO_T;
STATIC_ASSERT( sizeof(IO_T) <= SHARED_SIZE );

//extern uint32_t*       _startshared[];
volatile IO_T SHARED io;


void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Shared memory test\n");

   uint32_t* pCpuRamExternal = shmGetRelatedEtherBoneAddress(SHARED_OFFS);
   if( pCpuRamExternal == NULL )
   {
      mprintf( ESC_FG_RED "ERROR: Could not find external WB address of my own RAM !\n" ESC_NORMAL);
      return;
   }

   mprintf( "intern: &io.a: 0x%x extern: 0x%x\n", &io.a, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, a ) );
   mprintf( "intern: &io.b: 0x%x extern: 0x%x\n", &io.b, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, b ) );
   mprintf( "intern: &io.c: 0x%x extern: 0x%x\n", &io.c, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, c ) );

   while( true )
   {
      io.a = 0;
      while( io.a == 0 );
      mprintf( "io.a: 0x%x\n", io.a );
      mprintf( "io.b: 0x%x\n", io.b );
      mprintf( "io.c: 0x%x\n", io.c );
   }

}

/*================================== EOF ====================================*/

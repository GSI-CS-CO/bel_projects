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
#include "generated/shared_mmap.h"

typedef struct
{
   uint32_t      a;
   uint32_t      b;
   uint32_t      c;
   unsigned int* p;
} IO_T;
STATIC_ASSERT( sizeof(IO_T) <= SHARED_SIZE );
#if (COMPILER_VERSION_NUMBER >= 40600 )
  //_Static_assert( sizeof(IO_T) > SHARED_SIZE, "Uli" );
#endif

volatile IO_T SHARED io = { 1, 0, 4711 };

void main( void )
{
   unsigned int x = 0xabcdef;
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

   mprintf( "intern: &io.a: 0x%x extern: 0x%x\n", &io.a, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, a ) );
   mprintf( "intern: &io.b: 0x%x extern: 0x%x\n", &io.b, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, b ) );
   mprintf( "intern: &io.c: 0x%x extern: 0x%x\n", &io.c, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, c ) );
   mprintf( "intern: &io.p: 0x%x extern: 0x%x\n", &io.p, ((uint32_t)pCpuRamExternal) + offsetof( IO_T, p ) );

   io.p = &x;

   mprintf( "\nintern: io.p: 0x%x extern: 0x%x\n\n", io.p, ((uint32_t)pCpuRamExternal) + ((uint32_t)io.p ) );


   while( true )
   {
      while( io.a == 0 );
      mprintf( "io.a: 0x%08x %d\n", io.a, io.a );
      mprintf( "io.b: 0x%08x %d\n", io.b, io.b );
      mprintf( "io.c: 0x%08x %d\n", io.c, io.c );
      mprintf( "io.p: 0x%08x %d\n", io.p, io.p );
      io.a = 0;
   }

}

/*================================== EOF ====================================*/

/*!
 *
 * @brief     Testprogram for using shared memory in LM32.
 *
 * @file      static_test.c
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

#include <stdbool.h>
#include "mini_sdb.h"
#include "eb_console_helper.h"
#include "helper_macros.h"

#define SHARED __attribute__((section(".shared")))

extern uint32_t*       _startshared[];
uint32_t SHARED dummy0 = 0;
uint32_t SHARED dummy1 = 0;
uint32_t SHARED dummy2 = 0;

void main( void )
{
   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Shared memory test 0x%x\n", _startshared );
   mprintf( "Addr 0 = 0x%x\n", &dummy0 );
   mprintf( "Addr 1 = 0x%x\n", &dummy1 );
   mprintf( "Addr 2 = 0x%x\n", &dummy2 );
}

/*================================== EOF ====================================*/

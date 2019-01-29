/*!
 *
 * @brief     Testprogram for the macro SCU_ASSERT similar to ANSI-macro "assert"
 *
 * @file      assert_test.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      05.11.2018
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
#include "mprintf.h"
#include "mini_sdb.h"

//#define NDEBUG



#include "scu_assert.h"

void init( void )
{
   discoverPeriphery();   // mini-sdb: get info on important Wishbone infrastructure
   uart_init_hw();        // init UART, required for printf...
}

void main( void )
{
   init();
   mprintf("\nFarewell, you cruel world!\n");
   SCU_ASSERT( true );
   SCU_ASSERT( true );
   SCU_ASSERT( true );
   SCU_ASSERT( false );
   mprintf("\nI survived!\n");
   while( true );
}

/*================================== EOF ====================================*/

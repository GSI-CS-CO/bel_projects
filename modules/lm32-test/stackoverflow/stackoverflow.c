/*!
 *  @file stackoverflow.c
 *  @brief Stack overflow test
 *  @date 28.01.2019
 *  @copyright (C) 2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT AqNY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <string.h>
#include "mini_sdb.h"
#include "eb_console_helper.h"
#include "scu_lm32_macros.h"
#include "lm32_assert.h"

#ifndef CONFIG_STACK_PROTECTOR_CODE
//#error -fstack-protector or -fstack-protector-all or -fstack-protector has to be defined!
#endif

volatile int g_flag = 0;


void __stack_chk_fail( void )
{
   mprintf( ESC_FG_RED ESC_BOLD"PANIC: Stack overflow!!!\n"ESC_NORMAL );
   g_flag = 1;
}

void test( size_t faultyOffset )
{
   uint8_t buffer[16];
   mprintf( "Faulty offset: %d\n", faultyOffset );
   memset( buffer, 'U', sizeof( buffer ) + faultyOffset );
   mprintf( "return\n" );
}

void recursive( void )
{
   static int n;
   n++;
   mprintf( "n=%d\n", n );
   if( g_flag == 0 )
      recursive();
   else
      mprintf( "Return rec\n" );
}


void main( void )
{

   discoverPeriphery();
   uart_init_hw();

   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Stack overflow test "COMPILER_VERSION_STRING"\n" );
   test( 0 );
   test( 1 );
   g_flag = 0;
   //recursive();
   mprintf( "End...\n" );
}

/*================================== EOF ====================================*/
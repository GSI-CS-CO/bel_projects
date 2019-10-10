/*!
 *
 * @brief     Testprogram for check whether local static variables
 *            within functions operates properly by the currently used
 *            compiler and bit field structures.
 *
 * @file      static_test.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      20.11.2018
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
#include "daq.h"

#define STATIC_LOCAL __attribute__((section(".BSS"))) static

int foo( void )
{
   static int bar;
   bar++;
   return bar;
}

typedef struct
{
  uint16_t   a4:   4;
  uint16_t   b1:   1;
  uint16_t   c3:   3;
  uint16_t   d1:   1;
  uint16_t   e1:   1;
} BF_T;
STATIC_ASSERT( sizeof( BF_T ) == sizeof( uint16_t ) );

BF_T g_bf =
{
   .a4 = 3,
   .b1 = 0,
   .c3 = 7,
   .d1 = 0,
   .e1 = 1
};

void printBits16( uint16_t bits )
{
   for( uint16_t m = 1 << (BIT_SIZEOF(uint16_t) - 1); m != 0; m >>= 1 )
      mprintf( "%c", (m & bits)? '1' : '0' );
   mprintf( "\n" );
}

void printBF( BF_T* pThis )
{
   mprintf( "\na4 = %d\n", pThis->a4 );
   mprintf( "b1 = %d\n", pThis->b1 );
   mprintf( "c3 = %d\n", pThis->c3 );
   mprintf( "d1 = %d\n", pThis->d1 );
   mprintf( "e1 = %d\n", pThis->e1 );
   printBits16( *(uint16_t*)pThis );
}

typedef struct
{
   int itemMember;
} CONTENT_T;

typedef struct
{
   int contMember;
   CONTENT_T item;
} CONTAINER_T;

CONTAINER_T container =
{
   .contMember = 42,
   .item =
   {
     .itemMember = 4711
   }
};

void printContainerFromCintent( CONTENT_T* pContent )
{
   mprintf( "Content = %d\n", CONTAINER_OF( pContent, CONTAINER_T, item )->contMember );
}


int main( void )
{
   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf("Static-test compiler-version: " COMPILER_VERSION_STRING "\n" );
   
   mprintf( "Foo: %d\n", foo() );
   mprintf( "Foo: %d\n", foo() );
   mprintf( "Foo: %d\n\n", foo() );
   
   printBF( &g_bf );
   
   BF_T bf = {0};
   bf.a4 = 10;
   bf.b1 = 1;
   bf.c3 = 6;
   bf.d1 = 0;
   bf.e1 = 1;
   printBF( &bf );
   
   bf.a4 = 15;
   bf.b1 = 0;
   bf.c3 = 5;
   bf.d1 = 1;
   bf.e1 = 0;
   printBF( &bf );

   printContainerFromCintent( &container.item );

   DAQ_CTRL_REG_T creg = {0};

   creg.Ena_PM = 1;
   creg.Sample1ms = 1;
   creg.slot = 7;

   const char* str = "register";

   mprintf( "\nControl %s:\n", str  );
   printBits16( *((uint16_t*) &creg) );

   uint16_t v = 0x01;
   v |= 1 << 3;
   v |= 7 << 12;
   mprintf( "\nvalue:\n" );
   printBits16( v );


//   _DAQ_CHANNEL_CONTROL ccontrol = {0};

 //  ccontrol.controlReg = 0xFF;
 //  ccontrol.channelMode.channelNumber = 3;

//   mprintf( "_DAQ_CHANNEL_CONTROL:\n" );

 //  printBits16( *((uint16_t*) &ccontrol) );
   return 0;
}

//================================== EOF ======================================

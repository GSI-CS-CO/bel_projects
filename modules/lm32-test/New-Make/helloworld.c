/*!
 * @brief     Very simple example program (Hello world!) for LM32
 *            testing new build system that means new makefiles.
 * @file      helloworld.c
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      19.12.2018
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
 ******************************************************************************/
#ifdef __lm32__
 #include <mini_sdb.h>
#endif
#include <stdint.h>
#include <eb_console_helper.h>
#include <helper_macros.h>

/*!
 * @see https://gcc.gnu.org/onlinedocs/gcc/Structure-Layout-Pragmas.html#Structure-Layout-Pragmas
 */

#pragma scalar_storage_order big-endian
//#pragma scalar_storage_order little-endian
typedef struct PACKED_SIZE //__attribute__ ((scalar_storage_order("big-endian")))
{
   uint32_t n1: 4;
   uint32_t n2: 4;
   uint32_t n3: 4;
   uint32_t n4: 4;
   uint32_t n5: 4;
   uint32_t n6: 4;
   uint32_t n7: 4;
   uint32_t n8: 4;
} BV_T;


STATIC_ASSERT( sizeof( BV_T ) == sizeof( uint32_t ) );
//#pragma scalar_storage_order default
typedef union PACKED_SIZE
{
   uint32_t  n;
   BV_T      bv;
} A_T;
//#pragma scalar_storage_order default
STATIC_ASSERT( sizeof( A_T ) == sizeof( uint32_t ) );

void printBin( const uint32_t n )
{
   char str[BIT_SIZEOF( uint32_t ) + 1 + BIT_SIZEOF( uint32_t ) / 4];
   unsigned int i = 0;
   for( uint32_t m = 1; m != 0; m <<= 1 )
   {
      if( ((i+1) % 5) == 0 )
         str[i++] = ' ';
      str[i++] = ((m & n) != 0)? '1' : '0';
   }
   str[i] = '\0';
   mprintf( "%s", str );
}

void foo64( uint64_t ts )
{
    mprintf( "ts: 0x%04X%04X%04X%04X\n", ((uint16_t*)&ts)[0], ((uint16_t*)&ts)[1], ((uint16_t*)&ts)[2], ((uint16_t*)&ts)[3] );
    for( unsigned int i = 0; i < sizeof( uint64_t ) / sizeof( uint16_t ); i++ )
    {
        mprintf( "ts[%u]: 0x%04X\n", i, ((uint16_t*)&ts)[i] );
    }
}

void foo32( uint32_t ts )
{
    mprintf( "ts: 0x%08X\n", ts);
    for( unsigned int i = 0; i < sizeof( uint32_t ) / sizeof( uint16_t ); i++ )
    {
        mprintf( "ts[%u]: 0x%04X\n", i, ((uint16_t*)&ts)[i] );
    }
}



int main( void )
{
   A_T a = {0};
#ifdef __lm32__
   discoverPeriphery();
   uart_init_hw();
#endif
   gotoxy( 0, 0 );
   clrscr();
   mprintf( "Hello world!\n" );

   a.bv.n5 = 1;
   a.bv.n1 = 0xF;

   mprintf( "Bitvector: " );
   printBin( a.n );
   mprintf( "\n" );


   foo64( 0x1122334455667788L );
   mprintf( "\n" );
   foo32( 0x11223344 );
#ifdef __lm32__
   while( 1 );
#endif
   return 0;
}
//#pragma scalar_storage_order default
/*================================== EOF ====================================*/

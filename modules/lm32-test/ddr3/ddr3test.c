/*!
 *  @file scu3test.h
 *  @brief Testprogram for DDR3 within the SCU3.
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
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

#include <mini_sdb.h>
#include <scu_ddr3.h>
#include <eb_console_helper.h>

void ddrPrint16( DDR3_T* pthis, unsigned int index )
{
   DDR3_PAYLOAD_T toRead;
   ddr3read64( pthis, &toRead, index );
   for( int i = 0; i < ARRAY_SIZE( toRead.ad16 ); i++ )
      mprintf( "DDR-Index: %d, offset: %d, 0x%04x\n", index, i, toRead.ad16[i] );
}

void main( void )
{
   DDR3_T oDdr3;
   DDR3_PAYLOAD_T toWrite;
   DDR3_PAYLOAD_T toRead;

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf( ESC_FG_MAGNETA"DDR3 Test\n"ESC_NORMAL );

   if( ddr3init( &oDdr3 ) < 0  )
   {
      mprintf( "ERROR: Could not find DDR3 base address!\n" );
      return;
   }

   toWrite.ad16[0] = 0x4711;
   toWrite.ad16[1] = 0x4712;
   toWrite.ad16[2] = 0x4713;
   toWrite.ad16[3] = 0x4714;

   ddr3write64( &oDdr3, 0, &toWrite );

   toWrite.ad16[0] = 0xAAAA;
   toWrite.ad16[1] = 0xBBBB;
   toWrite.ad16[2] = 0xCCCC;
   toWrite.ad16[3] = 0xDDDD;

   ddr3write64( &oDdr3, 1, &toWrite );

   ddrPrint16( &oDdr3, 0 );
   ddrPrint16( &oDdr3, 1 );
}

/* ================================= EOF ====================================*/

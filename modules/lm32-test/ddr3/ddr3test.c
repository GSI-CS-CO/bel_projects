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

void main( void )
{
   DDR3_T oDdr3;

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

   ddr3write32( &oDdr3, 0, 0x11223346 );
   ddr3write32( &oDdr3, 4, 0xAABBCCDD );

   mprintf( "Index 1: 0x%08x\n", ddr3read32( &oDdr3, 0 ));
   mprintf( "Index 4: 0x%08x\n", ddr3read32( &oDdr3, 16 ));

}

/* ================================= EOF ====================================*/

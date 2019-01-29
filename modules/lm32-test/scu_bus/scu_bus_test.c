/*!
 *  @brief Test-program for functions in module scu_bus.h
 *  @file scu_bus_test.c
 *
 *
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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#include "mprintf.h"
#include "mini_sdb.h"
#include "scu_bus.h"
#include "eb_console_helper.h"

void _segfault( int sig )
{
   mprintf( ESC_FG_RED ESC_BOLD "Segmentation fault: %d\n" ESC_NORMAL, sig );
   while( 1 );
}


void main( void )
{
   SCUBUS_SLAVE_FLAGS_T slavePersentFlags, oldSlavePresentFlags;

   discoverPeriphery();
   uart_init_hw();
   clrscr();
   mprintf("\nTest...\n");

   void* pScuBusBase = find_device_adr(GSI, SCU_BUS_MASTER);
   SCU_ASSERT( pScuBusBase != (void*)ERROR_NOT_FOUND );
   mprintf( "SCU base address: 0x%x\n", pScuBusBase );

   slavePersentFlags = ~0;
   clrscr();
   while( true )
   {
      oldSlavePresentFlags = slavePersentFlags;
      slavePersentFlags = scuBusFindAllSlaves( pScuBusBase );
      if( oldSlavePresentFlags == slavePersentFlags )
         continue;

      if( slavePersentFlags != 0 )
      {
         for( int i = 1; i <= MAX_SCU_SLAVES; i++ )
         {
            gotoxy( 1, i );
            clrline();
            mprintf( "Slot %02d: ", i );
            if( scuBusIsSlavePresent( slavePersentFlags, i ) )
            {
               mprintf( ESC_BOLD ESC_FG_RED "used" ESC_NORMAL " Address: 0x%08x ",
                        scuBusGetAbsSlaveAddr( pScuBusBase, i ));
               continue;
            }
            mprintf( "free" );
         }
      }
      else
      {
         clrscr();
         gotoxy( 1, 1 );
         mprintf( "No slaves found!\n" );
      }
   }
}

/*================================== EOF ====================================*/

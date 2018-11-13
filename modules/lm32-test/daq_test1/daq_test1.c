/*!
 *  @file daq_test1.c
 *  @brief Testprogram for control module for Data Acquisition Unit (DAQ)
 *  @date 13.11.2018
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
#include "mini_sdb.h"
#include "../../top/gsi_scu/daq.h"
#include "eb_console_helper.h"


void main( void )
{
   struct ALL_DAQ_T allDaq;

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 1, 1 );
   clrscr();
   mprintf("\nTest...\n");
   if( daqFindAndInitializeAll( &allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
      return;
   mprintf( "%d DAQ found\n", allDaq.foundDevices );
#if 1
   for( int i = 0; i < allDaq.foundDevices; i++ )
   {
      mprintf( "DAQ in slot: %02d, address: 0x%08x, version: %d\n",
               allDaq.aDaq[i].slot, allDaq.aDaq[i].pReg,
               allDaq.aDaq[i].pReg->i[SLAVE_VERSION] );
   }
#endif
}

/*================================== EOF ====================================*/

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
#include "helper_macros.h"

IMPLEMENT_CONVERT_BYTE_ENDIAN( uint32_t )

void main( void )
{
   DAQ_ALL_T allDaq;

   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   mprintf("Test...\n");

   uint32_t x = 0xAABBCCDD;

   mprintf( "Convert 0x%x -> 0x%x\n", x,  convertByteEndian_uint32_t( x ) );

   mprintf( "SCU_IRQ_CTRL: 0x%08x\n",  find_device_adr(GSI, SCU_IRQ_CTRL) );

   if( daqFindAndInitializeAll( &allDaq, find_device_adr(GSI, SCU_BUS_MASTER) ) <= 0 )
   {
      mprintf( "Nothing found!\n" );
      return;
   }
   mprintf( "%d DAQ found\n", allDaq.foundDevices );

   for( int i = 0; i < allDaq.foundDevices; i++ )
   {
      mprintf( "DAQ in slot: %02d, DAQ macro start address: 0x%08x, channels %d\n",
               daqDeviceGetSlot( &allDaq.aDaq[i] ), allDaq.aDaq[i].pReg,
               allDaq.aDaq[i].maxChannels );
   }

   mprintf( "\nTotal-number of channels: %d\n", daqGetNumberOfAllFoundChannels( &allDaq ) );

}

/*================================== EOF ====================================*/

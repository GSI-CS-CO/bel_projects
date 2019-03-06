/*!
 *  @file daq_main.c
 *  @brief Main module for daq_control (including main())
 *
 *  @date 27.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 ******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
#include <daq_main.h>

#ifdef DEBUGLEVEL
  #include <mini_sdb.h>
  #include <eb_console_helper.h>
  #include <dbg.h>
#endif

static DAQ_ADMIN_T g_DaqAdmin;

/*! ---------------------------------------------------------------------------
 */
int scanScuBus( DAQ_BUS_T* pDaqDevices )
{
   void* pScuBusBase = find_device_adr( GSI, SCU_BUS_MASTER );

   /* That's not fine, but it's not my idea. */
   if( pScuBusBase == (void*)ERROR_NOT_FOUND )
   {
      DBPRINT1( "ERROR: find_device_adr() didn't find it!\n" );
      return DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND;
   }
   int ret = daqBusFindAndInitializeAll( pDaqDevices, pScuBusBase );
   if( ret < 0 )
   {
      DBPRINT1( "ERROR: in daqBusFindAndInitializeAll()\n" );
      return DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND;
   }
#ifdef DEBUGLEVEL
   if( ret == 0 )
      DBPRINT1( "WARNING: No DAQ devices present!\n" );
   else
   {
      DBPRINT1( "%d DAQ devices found.\n", ret );
      DBPRINT1( "Total number of all used channels: %d\n",
                daqBusGetUsedChannels( pDaqDevices ) );
   }
#endif
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static inline bool concernsChannel( DAQ_CANNEL_T* pChannel )
{
   if( daqChannelTestAndClearDaqIntPending( pChannel ) )
   {
      ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, true );
      return true;
   }
   if( daqChannelTestAndClearHiResIntPending( pChannel ) )
   {
      ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, false );
      return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
static inline bool forEachCahnnel( DAQ_DEVICE_T* pDevice )
{
   for( unsigned int channelNr = 0;
        channelNr < daqDeviceGetMaxChannels( pDevice ); channelNr++ )
   {
      if( executeIfRequested( &g_DaqAdmin ) )
         return true;
      concernsChannel( daqDeviceGetChannelObject( pDevice, channelNr ));
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
static inline void forEachDevice( void )
{
   for( unsigned int deviceNr = 0;
       deviceNr < daqBusGetFoundDevices( &g_DaqAdmin.oDaqDevs ); deviceNr++ )
   {
      if( forEachCahnnel( daqBusGetDeviceObject( &g_DaqAdmin.oDaqDevs, deviceNr ) ))
      {
         DBPRINT1( "DBG: Leaving loop\n" );
         break;
      }
   }
}

/*================================= main ====================================*/
/*! ---------------------------------------------------------------------------
 */
void main( void )
{
#ifdef DEBUGLEVEL
   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   DBPRINT1( "DAQ control started\n" );
#endif

   scanScuBus( &g_DaqAdmin.oDaqDevs );
   initBuffer( &g_DaqAdmin.oRam );

   while( true )
   {
      forEachDevice();
   }
}

/*================================== EOF ====================================*/

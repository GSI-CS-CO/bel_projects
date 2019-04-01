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
#include <daq_command_interface_uc.h>
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
      DBPRINT1( "DBG: %d DAQ devices found.\n", ret );
      DBPRINT1( "DBG: Total number of all used channels: %d\n",
                daqBusGetUsedChannels( pDaqDevices ) );
   }
#endif
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static inline void handleContinuousMode( DAQ_CANNEL_T* pChannel )
{
   if( !daqChannelTestAndClearDaqIntPending( pChannel ) )
      return;

   ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, true );
   daqChannelDecrementBlockCounter( pChannel );
}

/*! ---------------------------------------------------------------------------
 */
static inline void handleHiresMode( DAQ_CANNEL_T* pChannel )
{
   if( !daqChannelTestAndClearHiResIntPending( pChannel ) )
      return;

   ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, false );
}

/*! ---------------------------------------------------------------------------
 */
static inline bool forEachContinuousCahnnel( DAQ_DEVICE_T* pDevice )
{
   for( unsigned int channelNr = 0;
        channelNr < daqDeviceGetMaxChannels( pDevice ); channelNr++ )
   {
      if( executeIfRequested( &g_DaqAdmin ) )
         return true;
      DAQ_CANNEL_T* pChannel = daqDeviceGetChannelObject( pDevice, channelNr );
      handleContinuousMode( pChannel );
   }

 //  daqDeviceClearDaqChannelInterrupts( pDevice );
   return false;
}

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_DAQ_SINGLE_MODULE
static inline
#endif
void forEachScuDevice( void )
{
   for( unsigned int deviceNr = 0;
       deviceNr < daqBusGetFoundDevices( &g_DaqAdmin.oDaqDevs ); deviceNr++ )
   {
      DAQ_DEVICE_T* pDevice = daqBusGetDeviceObject( &g_DaqAdmin.oDaqDevs, deviceNr );
      if( daqDeviceTestAndClearDaqInt( pDevice ) )
      {
         if( forEachContinuousCahnnel( pDevice ))
         {
            DBPRINT1( "DBG: Leaving loop\n" );
            return;
         }
      }
      if( daqDeviceTestAndClearHiResInt( pDevice ) )
      {
         //TODO
      }
   }
   executeIfRequested( &g_DaqAdmin );
}

/*================================= main ====================================*/
#ifdef CONFIG_DAQ_SINGLE_MODULE
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

   daqInitialize( &g_DaqAdmin );

   while( true )
   {
      forEachScuDevice();
   }
}
#endif /* CONFIG_DAQ_SINGLE_MODULE */
/*================================== EOF ====================================*/

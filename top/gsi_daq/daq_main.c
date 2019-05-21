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

static inline uint32_t getInterruptPending( void )
{
   uint32_t ip;
   asm volatile ("rcsr %0, ip": "=r"(ip));
   return ip;
}


/*! ---------------------------------------------------------------------------
 */
int scanScuBus( DAQ_BUS_T* pDaqDevices )
{
   void* pScuBusBase = find_device_adr( GSI, SCU_BUS_MASTER );

   /* That's not fine, but it's not my idea. */
   if( pScuBusBase == (void*)ERROR_NOT_FOUND )
   {
      DBPRINT1( ESC_BOLD ESC_FG_RED
               "ERROR: find_device_adr() didn't find it!\n"
               ESC_NORMAL );
      return DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND;
   }
   int ret = daqBusFindAndInitializeAll( pDaqDevices, pScuBusBase );
   if( ret < 0 )
   {
      DBPRINT1(  ESC_BOLD ESC_FG_RED
                 "ERROR: in daqBusFindAndInitializeAll()\n"
                 ESC_NORMAL );
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
   if( daqChannelGetDaqFifoWords( pChannel ) == 0 )
   {
      DBPRINT1( ESC_BOLD ESC_FG_YELLOW
                "DBG WARNING: Discarding continuous block!\n"
                ESC_NORMAL );
      return;
   }

   ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, true );
   daqChannelDecrementBlockCounter( pChannel );
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
      handleContinuousMode( daqDeviceGetChannelObject( pDevice, channelNr ) );
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
static inline void handleHiresMode( DAQ_CANNEL_T* pChannel )
{
   if( !daqChannelTestAndClearHiResIntPending( pChannel ) )
      return;

   daqChannelDisableHighResolution( pChannel );
   ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, false );
}

/*! ---------------------------------------------------------------------------
 */
static inline bool forEachHiresChannel( DAQ_DEVICE_T* pDevice )
{
   for( unsigned int channelNr = 0;
        channelNr < daqDeviceGetMaxChannels( pDevice ); channelNr++ )
   {
      //if( executeIfRequested( &g_DaqAdmin ) )
      //   return true;
      handleHiresMode( daqDeviceGetChannelObject( pDevice, channelNr ) );
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
static inline void handlePostMortemMode( DAQ_CANNEL_T* pChannel )
{
   if( !pChannel->properties.postMortemEvent )
      return;
   if( !daqChannelIsPmHiResFiFoFull( pChannel ) )
      return;

   pChannel->properties.postMortemEvent = false;
   daqChannelDisablePostMortem( pChannel ); //!!
   ramPushDaqDataBlock( &g_DaqAdmin.oRam, pChannel, false );
}

/*! ---------------------------------------------------------------------------
 */
static inline bool forEachPostMortemChennel( DAQ_DEVICE_T* pDevice )
{
   for( unsigned int channelNr = 0;
        channelNr < daqDeviceGetMaxChannels( pDevice ); channelNr++ )
   {
      if( executeIfRequested( &g_DaqAdmin ) )
         return true;
      handlePostMortemMode( daqDeviceGetChannelObject( pDevice, channelNr ) );
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_DAQ_SINGLE_APP
static inline
#endif
void forEachScuDevice( void )
{
   bool isIrq;

   // TODO disable irq
 //!!  isIrq = g_DaqAdmin.isIrq;
 //!!  g_DaqAdmin.isIrq = false;
   // TODO enable irq

   isIrq = true; //!!

   uint32_t pending = getInterruptPending();
   if( pending != 0 )
      DBPRINT1( "DBG: pending: 0x%08x\n", pending );

   for( unsigned int deviceNr = 0;
       deviceNr < daqBusGetFoundDevices( &g_DaqAdmin.oDaqDevs ); deviceNr++ )
   {
      DAQ_DEVICE_T* pDevice = daqBusGetDeviceObject( &g_DaqAdmin.oDaqDevs,
                                                                    deviceNr );
      if( isIrq )
      {
         DAQ_REGISTER_T* volatile pIntFlags =
                                         daqDeviceGetInterruptFlags( pDevice );
         if( _daqDeviceTestAndClearDaqInt( pIntFlags ) )
         {
            if( forEachContinuousCahnnel( pDevice ))
            {
               DBPRINT1( "DBG: Leaving loop 1\n" );
               return;
            }
         }
         if( _daqDeviceTestAndClearHiResInt( pIntFlags ) )
         {
            if( forEachHiresChannel( pDevice ) )
            {
               DBPRINT1( "DBG: Leaving loop 2\n" );
               return;
            }
         }
      }

      if( forEachPostMortemChennel( pDevice ) )
      {
         DBPRINT1( "DBG: Leaving loop 3\n" );
         return;
      }
   }
   executeIfRequested( &g_DaqAdmin );
}

/*================================= main ====================================*/
#ifdef CONFIG_DAQ_SINGLE_APP
/*! ---------------------------------------------------------------------------
 */
void main( void )
{
#ifdef DEBUGLEVEL
   discoverPeriphery();
   uart_init_hw();
   gotoxy( 0, 0 );
   clrscr();
   DBPRINT1( "DAQ control started; Compiler: "COMPILER_VERSION_STRING"\n" );
#endif

   daqInitialize( &g_DaqAdmin );

   while( true )
   {
      forEachScuDevice();
   }
}
#endif /* CONFIG_DAQ_SINGLE_APP */
/*================================== EOF ====================================*/

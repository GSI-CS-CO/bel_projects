/*!
 *  @file daq_command_interface_uc.c
 *  @brief Definition of DAQ-commandos and data object for shared memory
 *         LM32 part
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
#include <scu_lm32_macros.h>
#include <daq_ramBuffer.h>
#include <dbg.h>
#ifdef DEBUGLEVEL
  #include <eb_console_helper.h>
#endif

#ifdef CONFIG_DAQ_SINGLE_APP
/*!!!!!!!!!!!!!!!!!!!!!! Begin of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!*/
volatile DAQ_SHARED_IO_T SHARED g_shared = DAQ_SHARAD_MEM_INITIALIZER;
/*!!!!!!!!!!!!!!!!!!!!!!! End of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!!*/
#endif /* CONFIG_DAQ_SINGLE_APP */

/*!
 * @brief Definition of the function type for all functions which becomes
 *        invoked by the Linux host beginning with the prefix "op".
 */
typedef DAQ_RETURN_CODE_T (*DAQ_OPERATION_FT)( DAQ_ADMIN_T* pDaqAdmin,
                                               volatile DAQ_OPERATION_IO_T* );

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Definition of the item for the operation match list.
 */
typedef struct
{  /*!
    * @brief Operation code
    * @see DAQ_OPERATION_CODE_T
    */
   DAQ_OPERATION_CODE_T code;
   /*!
    * @brief Pointer of the related function
    */
   DAQ_OPERATION_FT     operation;
} DAQ_OPERATION_TAB_ITEM_T;

#ifdef DEBUGLEVEL
/*! ---------------------------------------------------------------------------
 */
STATIC void printFunctionName( const char* str )
{
   DBPRINT1( ESC_FG_CYAN ESC_BOLD
             "DBG: executing %s(),\tDevice: %d, Channel: %d\n"
             ESC_NORMAL,
             str,
             GET_SHARED().operation.ioData.location.deviceNumber,
             GET_SHARED().operation.ioData.location.channel
           );
}
  #define DBG_FUNCTION_INFO() printFunctionName( __func__ )
#else
  #define DBG_FUNCTION_INFO()
#endif

/*! ---------------------------------------------------------------------------
 * @brief Initializing of the ring buffer.
 */
int initBuffer( RAM_SCU_T* poRam )
{
#ifdef  _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   return ramInit( poRam, &GET_SHARED().ringAdmin );
#else
   return ramInit( poRam, (RAM_RING_SHARED_OBJECT_T*)&GET_SHARED().ramIndexes );
#endif
}

/*! ---------------------------------------------------------------------------
 * @brief Checking whether the selected DAQ device is present.
 */
STATIC DAQ_RETURN_CODE_T
verifyDeviceAccess( DAQ_BUS_T* pDaqBus,
                    volatile DAQ_CHANNEL_LOCATION_T* pLocation )
{
   if( (pLocation->deviceNumber == 0) || (pLocation->deviceNumber > DAQ_MAX) )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_SLAVE_OUT_OF_RANGE\n" );
      return DAQ_RET_ERR_SLAVE_OUT_OF_RANGE;
   }

   if( pDaqBus->aDaq[pLocation->deviceNumber-1].maxChannels == 0 )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_NO_VHDL_MACRO_FOUND\n" );
      return DAQ_RET_ERR_NO_VHDL_MACRO_FOUND;
   }
   
   if( pLocation->deviceNumber > pDaqBus->foundDevices )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_SLAVE_NOT_PRESENT\n" );
      return DAQ_RET_ERR_SLAVE_NOT_PRESENT;
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Checking whether the selected DAQ device and channel is present.
 */
STATIC DAQ_RETURN_CODE_T
verifyChannelAccess( DAQ_BUS_T* pDaqBus,
                     volatile DAQ_CHANNEL_LOCATION_T* pLocation )
{
   const DAQ_RETURN_CODE_T ret = verifyDeviceAccess( pDaqBus, pLocation );
   if( ret != DAQ_RET_OK )
      return ret;

   if( (pLocation->channel == 0) || (pLocation->channel > DAQ_MAX_CHANNELS))
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE\n" );
      return DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE;
   }

   if( pLocation->channel >
                      pDaqBus->aDaq[pLocation->deviceNumber-1].maxChannels )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_CHANNEL_NOT_PRESENT\n" );
      return DAQ_RET_ERR_CHANNEL_NOT_PRESENT;
   }

   return DAQ_RET_OK;
}

#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
/*! ---------------------------------------------------------------------------
 * @brief Locks the access of the DAQ ring buffer access.
 * @note After the this command the command function table is not any more
 *       a reachable from the Linux host! \n
 *       Therefore the opposite action (unlock) will made from the
 *       Linux host directly in the shared memory.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opLock( DAQ_ADMIN_T* pDaqAdmin,
                                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   GET_SHARED().ramIndexes.ramAccessLock = true;
   return DAQ_RET_OK;
}
#endif
/*! ---------------------------------------------------------------------------
 */
STATIC
DAQ_RETURN_CODE_T opReadErrorStatus( DAQ_ADMIN_T* pDaqAdmin,
                                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   pData->param1 = *((DAQ_REGISTER_T*)&pDaqAdmin->oDaqDevs.lastErrorState);
   pDaqAdmin->oDaqDevs.lastErrorState.status = DAQ_RECEIVE_STATE_OK;
   pDaqAdmin->oDaqDevs.lastErrorState.slot = 0;
   pDaqAdmin->oDaqDevs.lastErrorState.channel = 0;
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Performs a reset of all DAQ devices residing in the SCU bus.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opReset( DAQ_ADMIN_T* pDaqAdmin,
                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   daqBusReset( &pDaqAdmin->oDaqDevs );
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   ramRingSharedReset( pDaqAdmin->oRam.pSharedObj );
#else
   ramRingReset( &pDaqAdmin->oRam.pSharedObj->ringIndexes );
   GET_SHARED().ramIndexes.ramAccessLock = false;
#endif
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Sending of the macro version of the selected DAQ device back to the
 *        Linux host.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetMacroVersion( DAQ_ADMIN_T* pDaqAdmin,
                                     volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyDeviceAccess( &pDaqAdmin->oDaqDevs,
                                                     &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 = daqDeviceGetMacroVersion(
                   &pDaqAdmin->oDaqDevs.aDaq[pData->location.deviceNumber-1] );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Sending of the SCU bus slot flag field back to the Linux host.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetSlots( DAQ_ADMIN_T* pDaqAdmin,
                              volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   STATIC_ASSERT( sizeof( pData->param1 ) >=
                            sizeof(pDaqAdmin->oDaqDevs.slotDaqUsedFlags));

   pData->param1 = pDaqAdmin->oDaqDevs.slotDaqUsedFlags;

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Sends the number channels of a selected DAQ device back to the
 *        Linux host.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetChannels( DAQ_ADMIN_T* pDaqAdmin,
                                 volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyDeviceAccess( &pDaqAdmin->oDaqDevs,
                                                     &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 =
      pDaqAdmin->oDaqDevs.aDaq[pData->location.deviceNumber-1].maxChannels;

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Performs a rescan of the whole SCU bus for SCU devices
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opRescan( DAQ_ADMIN_T* pDaqAdmin,
                                   volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
#ifdef CONFIG_DAQ_SINGLE_APP
   daqScanScuBus( &pDaqAdmin->oDaqDevs );
#else
   daqScanScuBus( &pDaqAdmin->oDaqDevs, NULL );
#endif
   return DAQ_RET_RESCAN;
}

/*! ---------------------------------------------------------------------------
 * @brief Returns the pointer of the requested channel object
 */
STATIC inline
DAQ_CANNEL_T* getChannel( DAQ_ADMIN_T* pDaqAdmin,
                          volatile DAQ_OPERATION_IO_T* pData )
{
   return &pDaqAdmin->oDaqDevs.aDaq
          [pData->location.deviceNumber-1].aChannel[pData->location.channel-1];
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Switching post mortem mode on.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opPostMortemOn( DAQ_ADMIN_T* pDaqAdmin,
                                  volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   pChannel->properties.restart = (pData->param1 != 0);
#ifdef CONFIG_DAQ_SW_SEQUENCE
   pChannel->sequencePmHires = 0;
#endif
   daqChannelDisableHighResolution( pChannel );
   daqChannelEnablePostMortem( pChannel );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief switching high resolution mode on.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opHighResolutionOn( DAQ_ADMIN_T* pDaqAdmin,
                                          volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );
   pChannel->properties.restart = (pData->param1 != 0);
#ifdef CONFIG_DAQ_SW_SEQUENCE
   pChannel->sequencePmHires = 0;
#endif
   daqChannelDisablePostMortem( pChannel );
   daqChannelEnableHighResolution( pChannel );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @brief Switching post-mortem and high-resolution mode off.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opPmHighResOff( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );
   pChannel->properties.restart = (pData->param1 != 0);
   daqChannelDisableHighResolution( pChannel );
   if( daqChannelIsPostMortemActive( pChannel ) )
   {
      pChannel->properties.postMortemEvent = true;
      daqChannelDisablePostMortem( pChannel );
   }
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Switching continue mode on.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opContinueOn( DAQ_ADMIN_T* pDaqAdmin,
                                       volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );
#ifdef CONFIG_DAQ_SW_SEQUENCE
   pChannel->sequenceContinuous = 0;
#endif
   pChannel->blockDownCounter = pData->param2;
   DBPRINT1( "DBG: blockDownCounter = %d\n", pChannel->blockDownCounter );

   switch( (DAQ_SAMPLE_RATE_T)pData->param1 )
   {
      case DAQ_SAMPLE_1MS:
      {
         DBPRINT1( "DBG: 1 ms sample ON\n" );
         daqChannelSample1msOn( pChannel );
         break;
      }
      case DAQ_SAMPLE_100US:
      {
         DBPRINT1( "DBG: 100 us sample ON\n" );
         daqChannelSample100usOn( pChannel );
         break;
      }
      case DAQ_SAMPLE_10US:
      {
         DBPRINT1( "DBG: 10 us sample ON\n" );
         daqChannelSample10usOn( pChannel );
         break;
      }
      default:
      {
         return DAQ_RET_ERR_WRONG_SAMPLE_PARAMETER;
      }
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Switching continuous mode off.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opContinueOff( DAQ_ADMIN_T* pDaqAdmin,
                                        volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   daqChannelSample10usOff( pChannel );
   daqChannelSample100usOff( pChannel );
   daqChannelSample1msOff( pChannel );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Setting trigger condition.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opSetTriggerCondition( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );
   daqChannelSetTriggerConditionLW( pChannel, pData->param1 );
   daqChannelSetTriggerConditionHW( pChannel, pData->param2 );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Send actual trigger condition back to Linux host.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opGetTriggerCondition( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );
   pData->param1 = daqChannelGetTriggerConditionLW( pChannel );
   pData->param2 = daqChannelGetTriggerConditionHW( pChannel );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Setting trigger delay.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opSetTriggerDelay( DAQ_ADMIN_T* pDaqAdmin,
                                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   daqChannelSetTriggerDelay( getChannel( pDaqAdmin, pData ), pData->param1 );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Send actual trigger delay back to the Linux host.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opGetTriggerDelay( DAQ_ADMIN_T* pDaqAdmin,
                                          volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 = daqChannelGetTriggerDelay( getChannel( pDaqAdmin, pData ) );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Enabling or disabling the trigger mode.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opSetTriggerMode( DAQ_ADMIN_T* pDaqAdmin,
                                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   if( pData->param1 != 0 )
   {
      DBPRINT1( "DBG: Enable\n" );
      daqChannelEnableTriggerMode( pChannel );
   }
   else
   {
      DBPRINT1( "DBG: Disable\n" );
      daqChannelDisableTriggerMode( pChannel );
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Send the actual state of the trigger mode back to the Linux host.
 * @see executeIfRequested
 */
STATIC DAQ_RETURN_CODE_T opGetTriggerMode( DAQ_ADMIN_T* pDaqAdmin,
                                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;
   pData->param1 = daqChannelIsTriggerModeEnabled(
                                             getChannel( pDaqAdmin, pData ) );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Setting of the trigger source for continuous mode.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opSetTriggerSourceCon( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   if( pData->param1 != 0 )
   {
      DBPRINT1( "DBG: Extern\n" );
      daqChannelEnableExtrenTrigger( pChannel );
   }
   else
   {
      DBPRINT1( "DBG: Event\n" );
      daqChannelEnableEventTrigger( pChannel );
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Sending of the actual trigger source for continuous mode back to
 *        the Linux host.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetTriggerSourceCon( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 =
             daqChannelGetTriggerSource( getChannel( pDaqAdmin, pData ) );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Setting of the trigger source for the high resolution mode.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opSetTriggerSourceHir( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   if( pData->param1 != 0 )
   {
      DBPRINT1( "DBG: Extern\n" );
      daqChannelEnableExternTriggerHighRes( pChannel );
   }
   else
   {
      DBPRINT1( "DBG: Event\n" );
      daqChannelEnableEventTriggerHighRes( pChannel );
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Sending of the actual trigger source for high resolution mode
 *        back to the Linux host.
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetTriggerSourceHir( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs,
                                                      &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 = daqChannelGetTriggerSourceHighRes(
                                            getChannel( pDaqAdmin, pData ) );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Sending the device type of the given channel
 * @see DAQ_DEVICE_TYP_T
 * @see executeIfRequested
 */
STATIC
DAQ_RETURN_CODE_T opGetDeviceType( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();
   const DAQ_RETURN_CODE_T ret = verifyDeviceAccess( &pDaqAdmin->oDaqDevs,
                                                     &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 = pDaqAdmin->oDaqDevs.aDaq[pData->location.deviceNumber-1].type;

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Sending the device type of the given channel
 */
STATIC
DAQ_RETURN_CODE_T opSyncTimeStamp( DAQ_ADMIN_T* pDaqAdmin,
                                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBG_FUNCTION_INFO();

   ATOMIC_SECTION()
   {
      daqBusPresetAllTimeStampCounters( &pDaqAdmin->oDaqDevs,
                                        pData->param1 |
                                        (pData->param2 << BIT_SIZEOF(DAQ_REGISTER_T)) );

      daqBusSetAllTimeStampCounterEcaTags( &pDaqAdmin->oDaqDevs,
                                           pData->param3 |
                                           (pData->param4 << BIT_SIZEOF(DAQ_REGISTER_T)) );
   }
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Helper macro for making a item in the command function table.
 * @see DAQ_OPERATION_TAB_ITEM_T
 */
#define OPERATION_ITEM( opcode, function )                                    \
{                                                                             \
   .code      = opcode,                                                       \
   .operation = function                                                      \
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Last item of the operation match list.
 * @note CAUTION: Don't forget it!
 * @see DAQ_OPERATION_TAB_ITEM_T
 */
#define OPERATION_ITEM_TERMINATOR OPERATION_ITEM( DAQ_OP_IDLE, NULL )

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Operation match list respectively command function table.
 * @see executeIfRequested
 */
STATIC const DAQ_OPERATION_TAB_ITEM_T g_operationTab[] =
{
#ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
   OPERATION_ITEM( DAQ_OP_LOCK,                   opLock                ),
#endif
   OPERATION_ITEM( DAQ_OP_GET_ERROR_STATUS,       opReadErrorStatus     ),
   OPERATION_ITEM( DAQ_OP_RESET,                  opReset               ),
   OPERATION_ITEM( DAQ_OP_GET_MACRO_VERSION,      opGetMacroVersion     ),
   OPERATION_ITEM( DAQ_OP_GET_SLOTS,              opGetSlots            ),
   OPERATION_ITEM( DAQ_OP_GET_CHANNELS,           opGetChannels         ),
   OPERATION_ITEM( DAQ_OP_RESCAN,                 opRescan              ),
   OPERATION_ITEM( DAQ_OP_PM_ON,                  opPostMortemOn        ),
   OPERATION_ITEM( DAQ_OP_HIRES_ON,               opHighResolutionOn    ),
   OPERATION_ITEM( DAQ_OP_PM_HIRES_OFF,           opPmHighResOff        ),
   OPERATION_ITEM( DAQ_OP_CONTINUE_ON,            opContinueOn          ),
   OPERATION_ITEM( DAQ_OP_CONTINUE_OFF,           opContinueOff         ),
   OPERATION_ITEM( DAQ_OP_SET_TRIGGER_CONDITION,  opSetTriggerCondition ),
   OPERATION_ITEM( DAQ_OP_GET_TRIGGER_CONDITION,  opGetTriggerCondition ),
   OPERATION_ITEM( DAQ_OP_SET_TRIGGER_DELAY,      opSetTriggerDelay     ),
   OPERATION_ITEM( DAQ_OP_GET_TRIGGER_DELAY,      opGetTriggerDelay     ),
   OPERATION_ITEM( DAQ_OP_SET_TRIGGER_MODE,       opSetTriggerMode      ),
   OPERATION_ITEM( DAQ_OP_GET_TRIGGER_MODE,       opGetTriggerMode      ),
   OPERATION_ITEM( DAQ_OP_SET_TRIGGER_SOURCE_CON, opSetTriggerSourceCon ),
   OPERATION_ITEM( DAQ_OP_GET_TRIGGER_SOURCE_CON, opGetTriggerSourceCon ),
   OPERATION_ITEM( DAQ_OP_SET_TRIGGER_SOURCE_HIR, opSetTriggerSourceHir ),
   OPERATION_ITEM( DAQ_OP_GET_TRIGGER_SOURCE_HIR, opGetTriggerSourceHir ),
   OPERATION_ITEM( DAQ_OP_GET_DEVICE_TYPE,        opGetDeviceType       ),
   OPERATION_ITEM( DAQ_OP_SYNC_TIMESTAMP,         opSyncTimeStamp       ),

   OPERATION_ITEM_TERMINATOR
};

#ifdef CONFIG_USE_LM32LOG
char* logDaqCmd2String( DAQ_OPERATION_CODE_T opcode )
{
#define CASE_RETURN( c ) case c: return #c
   switch( opcode )
   {
      CASE_RETURN( DAQ_OP_IDLE );
    #ifndef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
      CASE_RETURN( DAQ_OP_LOCK );
   #endif
      CASE_RETURN( DAQ_OP_GET_ERROR_STATUS );
      CASE_RETURN( DAQ_OP_RESET );
      CASE_RETURN( DAQ_OP_GET_MACRO_VERSION );
      CASE_RETURN( DAQ_OP_GET_SLOTS );
      CASE_RETURN( DAQ_OP_GET_CHANNELS );
      CASE_RETURN( DAQ_OP_RESCAN );
      CASE_RETURN( DAQ_OP_PM_ON );
      CASE_RETURN( DAQ_OP_HIRES_ON );
      CASE_RETURN( DAQ_OP_PM_HIRES_OFF );
      CASE_RETURN( DAQ_OP_CONTINUE_ON );
      CASE_RETURN( DAQ_OP_CONTINUE_OFF );
      CASE_RETURN( DAQ_OP_SET_TRIGGER_CONDITION );
      CASE_RETURN( DAQ_OP_GET_TRIGGER_CONDITION );
      CASE_RETURN( DAQ_OP_SET_TRIGGER_DELAY );
      CASE_RETURN( DAQ_OP_GET_TRIGGER_DELAY );
      CASE_RETURN( DAQ_OP_SET_TRIGGER_MODE );
      CASE_RETURN( DAQ_OP_GET_TRIGGER_MODE );
      CASE_RETURN( DAQ_OP_SET_TRIGGER_SOURCE_CON );
      CASE_RETURN( DAQ_OP_GET_TRIGGER_SOURCE_CON );
      CASE_RETURN( DAQ_OP_SET_TRIGGER_SOURCE_HIR );
      CASE_RETURN( DAQ_OP_GET_TRIGGER_SOURCE_HIR );
      CASE_RETURN( DAQ_OP_GET_DEVICE_TYPE );
      CASE_RETURN( DAQ_OP_SYNC_TIMESTAMP );
   }
   return "unknown";
}
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Performs the list and executes a function in the table if it is
 *        present
 * @see DAQ_OPERATION_TAB_ITEM_T
 */
bool executeIfRequested( DAQ_ADMIN_T* pDaqAdmin )
{
   /*
    * Requests the Linux host an operation?
    */
   if( GET_SHARED().operation.code == DAQ_OP_IDLE )
   { /*
      * No, there is nothing to do...
      */
      return false;
   }

   /*
    * Yes, executing the requested operation if present
    * in the operation table.
    */
   unsigned int i = 0;
   while( g_operationTab[i].operation != NULL )
   {
      if( g_operationTab[i].code == GET_SHARED().operation.code )
      {
         lm32Log( LM32_LOG_CMD, "DAQ command: %s( %u, %u )\n",
                  logDaqCmd2String( g_operationTab[i].code ),
                  GET_SHARED().operation.ioData.location.deviceNumber,
                  GET_SHARED().operation.ioData.location.channel
                );

         GET_SHARED().operation.retCode =
            g_operationTab[i].operation( pDaqAdmin,
                                         &GET_SHARED().operation.ioData );
         break;
      }
      i++;
   }

   /*
    * Was the requested operation known?
    */
   if( g_operationTab[i].operation == NULL )
   { /*
      * No, making known this for the Linux host.
      */
      lm32Log( LM32_LOG_ERROR, "Unknown DAQ command!\n" );
      DBPRINT1( "DBG: DAQ_RET_ERR_UNKNOWN_OPERATION\n" );
      GET_SHARED().operation.retCode = DAQ_RET_ERR_UNKNOWN_OPERATION;
   }

   bool ret;
   if( GET_SHARED().operation.retCode == DAQ_RET_RESCAN )
   {
      GET_SHARED().operation.retCode = DAQ_RET_OK;
      ret = true;
   }
   else
      ret = false;

   /*
    * Making known for the Linux host that this application is ready
    * for the next operation.
    */
   GET_SHARED().operation.code = DAQ_OP_IDLE;
   return ret;
}

/*================================== EOF ====================================*/

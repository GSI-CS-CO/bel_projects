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
#include <daq_main.h>
#include <scu_ramBuffer.h>
#include <dbg.h>

/*!!!!!!!!!!!!!!!!!!!!!! Begin of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!*/
volatile DAQ_SHARED_IO_T SHARED g_shared =
{
   .magicNumber = DAQ_MAGIC_NUMBER,
   .ramIndexes  = RAM_RING_SHARED_OBJECT_INITIALIZER,
   .operation =
   {
      .code    = DAQ_OP_IDLE,
      .retCode = 0
   }
};
/*!!!!!!!!!!!!!!!!!!!!!!! End of shared memory area !!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef int32_t (*DAQ_OPERATION_FT)( DAQ_ADMIN_T* pDaqAdmin,
                                     volatile DAQ_OPERATION_IO_T* );

/*!
 * @ingroup DAQ_INTERFACE
 * @brief Definition of the item for the operation match list.
 */
typedef struct
{
   /*!
    * @brief Operation code
    * @see DAQ_OPERATION_CODE_T
    */
   DAQ_OPERATION_CODE_T code;
   /*!
    * @brief Pointer of the related function
    */
   DAQ_OPERATION_FT     operation;
} DAQ_OPERATION_TAB_ITEM_T;

/*!
 * @ingroup DAQ_INTERFACE
 * @brief Last item of the operation match list.
 */
#define DAQ_OPERATION_ITEM_TERMINATOR { .code = DAQ_OP_IDLE, .operation = NULL }

/*! ---------------------------------------------------------------------------
 */
int initBuffer( RAM_SCU_T* poRam )
{
   return ramInit( poRam, (RAM_RING_SHARED_OBJECT_T*)&g_shared.ramIndexes );
}

/*! ---------------------------------------------------------------------------
 */
static int
verifyDeviceAccess( DAQ_BUS_T* pDaqBus,
                    volatile DAQ_CHANNEL_LOCATION_T* pLocation )
{
   if( (pLocation->deviceNumber == 0) || (pLocation->deviceNumber > DAQ_MAX) )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_SLAVE_OUT_OF_RANGE\n" );
      return DAQ_RET_ERR_SLAVE_OUT_OF_RANGE;
   }

   if( pLocation->deviceNumber > pDaqBus->foundDevices )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_SLAVE_NOT_PRESENT\n" );
      return DAQ_RET_ERR_SLAVE_NOT_PRESENT;
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int
verifyChannelAccess( DAQ_BUS_T* pDaqBus,
                     volatile DAQ_CHANNEL_LOCATION_T* pLocation )
{
   int ret = verifyDeviceAccess( pDaqBus, pLocation );
   if( ret != DAQ_RET_OK )
      return ret;

   if( (pLocation->channel == 0) || (pLocation->channel > DAQ_MAX_CHANNELS))
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE\n" );
      return DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE;
   }

   if( pLocation->channel > pDaqBus->aDaq[pLocation->deviceNumber-1].maxChannels )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_CHANNEL_NOT_PRESENT\n" );
      return DAQ_RET_ERR_CHANNEL_NOT_PRESENT;
   }

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static
int32_t opLock( DAQ_ADMIN_T* pDaqAdmin, volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static
int32_t opUnlock( DAQ_ADMIN_T* pDaqAdmin, volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}


/*! ---------------------------------------------------------------------------
 */
static
int32_t opReset( DAQ_ADMIN_T* pDaqAdmin, volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opGetSlots( DAQ_ADMIN_T* pDaqAdmin,
                           volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   STATIC_ASSERT( sizeof( pData->param1 ) >= sizeof(pDaqAdmin->oDaqDevs.slotDaqUsedFlags));

   pData->param1 = pDaqAdmin->oDaqDevs.slotDaqUsedFlags;

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opGetChannels( DAQ_ADMIN_T* pDaqAdmin,
                              volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   int ret = verifyDeviceAccess( &pDaqAdmin->oDaqDevs, &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   pData->param1 = pDaqAdmin->oDaqDevs.aDaq[pData->location.deviceNumber-1].maxChannels;

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opRescan( DAQ_ADMIN_T* pDaqAdmin,
                         volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   scanScuBus( &pDaqAdmin->oDaqDevs );
   return DAQ_RET_RESCAN;
}

/*! ---------------------------------------------------------------------------
 */
static inline
DAQ_CANNEL_T* getChannel( DAQ_ADMIN_T* pDaqAdmin,
                          volatile DAQ_OPERATION_IO_T* pData )
{
   return &pDaqAdmin->oDaqDevs.aDaq
            [pData->location.deviceNumber-1].aChannel[pData->location.channel-1];
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opPostMortemOn( DAQ_ADMIN_T* pDaqAdmin,
                               volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );

   int ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs, &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   daqChannelEnablePostMortem( getChannel( pDaqAdmin, pData ) );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opHighResolutionOn( DAQ_ADMIN_T* pDaqAdmin,
                                   volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );

   int ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs, &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   daqChannelEnableHighResolution( getChannel( pDaqAdmin, pData ) );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opContinueOn( DAQ_ADMIN_T* pDaqAdmin,
                             volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );

   int ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs, &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   switch( pData->param1 )
   {
      case DAQ_SAMPLE_1MS:
      {
         daqChannelSample1msOn( pChannel );
         break;
      }
      case DAQ_SAMPLE_100US:
      {
         daqChannelSample100usOn( pChannel );
         break;
      }
      case DAQ_SAMPLE_10US:
      {
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
 */
static int32_t opOff( DAQ_ADMIN_T* pDaqAdmin,
                             volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );

   int ret = verifyChannelAccess( &pDaqAdmin->oDaqDevs, &pData->location );
   if( ret != DAQ_RET_OK )
      return ret;

   DAQ_CANNEL_T* pChannel = getChannel( pDaqAdmin, pData );

   daqChannelSample10usOff( pChannel );
   daqChannelSample100usOff( pChannel );
   daqChannelSample1msOff( pChannel );
   daqChannelDisablePostMortem( pChannel );
   daqChannelDisableHighResolution( pChannel );

   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ_INTERFACE
 * @brief Operation match list
 */
static const DAQ_OPERATION_TAB_ITEM_T g_operationTab[] =
{
   { .code = DAQ_OP_LOCK,         .operation = opLock             },
   { .code = DAQ_OP_UNLOCK,       .operation = opUnlock           },
   { .code = DAQ_OP_RESET,        .operation = opReset            },
   { .code = DAQ_OP_GET_SLOTS,    .operation = opGetSlots         },
   { .code = DAQ_OP_GET_CHANNELS, .operation = opGetChannels      },
   { .code = DAQ_OP_RESCAN,       .operation = opRescan           },
   { .code = DAQ_OP_PM_ON,        .operation = opPostMortemOn     },
   { .code = DAQ_OP_HIRES_ON,     .operation = opHighResolutionOn },
   { .code = DAQ_OP_CONTINUE_ON,  .operation = opContinueOn       },
   { .code = DAQ_OP_OFF,          .operation = opOff              },
   DAQ_OPERATION_ITEM_TERMINATOR
};

/*! ---------------------------------------------------------------------------
 */
int executeIfRequested( DAQ_ADMIN_T* pDaqAdmin )
{
   if( g_shared.operation.code == DAQ_OP_IDLE )
      return DAQ_RET_OK;

   unsigned int i = 0;
   while( g_operationTab[i].operation != NULL )
   {
      if( g_operationTab[i].code == g_shared.operation.code )
      {
         g_shared.operation.retCode =
            g_operationTab[i].operation( pDaqAdmin,
                                         &g_shared.operation.ioData );
         break;
      }
      i++;
   }
   if( g_operationTab[i].operation == NULL )
   {
      DBPRINT1( "DBG: DAQ_RET_ERR_UNKNOWN_OPERATION\n" );
      g_shared.operation.retCode = DAQ_RET_ERR_UNKNOWN_OPERATION;
   }

   g_shared.operation.code = DAQ_OP_IDLE;
   return g_shared.operation.retCode;
}

/*================================== EOF ====================================*/

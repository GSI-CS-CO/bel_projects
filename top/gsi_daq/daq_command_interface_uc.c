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

typedef int32_t (*DAQ_OPERATION_FT)( volatile DAQ_OPERATION_IO_T* );

typedef struct
{
   DAQ_OPERATION_CODE_T code;
   DAQ_OPERATION_FT     operation;
} DAQ_OPERATION_TAB_ITEM_T;

#define DAQ_OPERATION_ITEM_TERMINATOR { .code = DAQ_OP_IDLE, .operation = NULL }


/*! ---------------------------------------------------------------------------
 */
static
int32_t opReset( volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opGetSlots( volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
static int32_t opRescan( volatile DAQ_OPERATION_IO_T* pData )
{
   DBPRINT1( "DBG: executing %s\n", __func__ );
   return DAQ_RET_OK;
}

/*! ---------------------------------------------------------------------------
 */
const DAQ_OPERATION_TAB_ITEM_T g_operationTab[] =
{
   { .code = DAQ_OP_RESET,           .operation = opReset     },
   { .code = DAQ_OP_GET_SLOTS,       .operation = opGetSlots  },
   { .code = DAQ_OP_RESCAN,          .operation = opRescan    },
   DAQ_OPERATION_ITEM_TERMINATOR
};

/*! ---------------------------------------------------------------------------
 */
void executeIfRequested( void )
{
   if( g_shared.operation.code == DAQ_OP_IDLE )
      return;

   unsigned int i = 0;
   while( g_operationTab[i].operation != NULL )
   {
      if( g_operationTab[i].code == g_shared.operation.code )
      {
         g_shared.operation.retCode =
            g_operationTab[i].operation( &g_shared.operation.ioData );
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
}

/*================================== EOF ====================================*/

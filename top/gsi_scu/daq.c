/*!
 *  @file daq.c
 *  @brief Control module for Data Acquisition Unit (DAQ)
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
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <string.h>   // necessary for memset()
#include "mini_sdb.h" // necessary for ERROR_NOT_FOUND
#include "dbg.h"
#include "daq.h"

/*! ----------------------------------------------------------------------------
 * @see daq.h
 */
int daqFindAndInitializeAll( struct ALL_DAQ_T* pAllDAQ, const void* pScuBusBase )
{
   SCU_BUS_SLAVE_FLAGS_T daqPersentFlags;

   // Paranoia...
   LM32_ASSERT( pScuBusBase != (void*)ERROR_NOT_FOUND );
   LM32_ASSERT( pAllDAQ != NULL );

   // Pre-initializing
   memset( pAllDAQ, 0, sizeof( struct ALL_DAQ_T ));

   daqPersentFlags = scuBusFindSpecificSlaves( pScuBusBase, 0x37, 0x26 );
   if( daqPersentFlags == 0 )
   {
      DBPRINT( "DBG: No DAQ slaves found!\n" );
      return 0;
   }

   for( int slot = 1; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( daqPersentFlags, slot ) )
         continue;
      pAllDAQ->aDaq[pAllDAQ->foundDevices].slot = slot;
      pAllDAQ->aDaq[pAllDAQ->foundDevices].pReg = getAbsScuBusSlaveAddr( pScuBusBase, slot );
      DBPRINT2( "DBG: DAQ found in slot: %02d, address: 0x%08x\n",
                pAllDAQ->aDaq[pAllDAQ->foundDevices].slot,
                pAllDAQ->aDaq[pAllDAQ->foundDevices].pReg );
      pAllDAQ->foundDevices++;
      if( pAllDAQ->foundDevices == ARRAY_SIZE( pAllDAQ->aDaq ) )
         break;
   }

   return pAllDAQ->foundDevices;
}


/*================================== EOF ====================================*/

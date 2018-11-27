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
 * @brief Scans all potential existing input-channels of the given
 *        DAQ-Device ant initialize each found channel with
 *        the slot number.
 * @param pDaqDev Start-address of DAQ-registers
 * @return Number of real existing channels
 */
inline static int daqFindChannels( DAQ_DEVICE_T* pDaqDev, int slot )
{
   LM32_ASSERT( pDaqDev != NULL );
   LM32_ASSERT( pDaqDev->pReg != NULL );

   for( int channel = 0; channel < ARRAY_SIZE(pDaqDev->aChannel); channel++ )
   {
      DBPRINT2( "DBG: Slot: %02d, Channel: %02d, ctrlReg: 0x%04x\n",
                slot, channel, daqChannelGetReg( pDaqDev->pReg, CtrlReg, channel ));
      pDaqDev->aChannel[channel].n = channel;
      /*
       * The next three lines probes the channel by writing and read back
       * the slot number. At the first look this algorithm seems not meaningful
       * (writing and immediately reading a value from the same memory place)
       * but we have to keep in mind that is a memory mapped IO areal and
       * its attribute was declared as "volatile".
       *
       * If no (further) channel present the value of the control-register is
       * 0xDEAD. That means the slot number has the hex number 0xD (13).
       * Fortunately the highest slot number is 0xC (12). Therefore no further
       * probing is necessary.
       */
      daqChannelGetCtrlRegPtr( &pDaqDev->aChannel[channel] )->slot = slot;
      if( daqChannelGetSlot( &pDaqDev->aChannel[channel] ) != slot )
         break; /* Supposing this channel isn't present. */

      pDaqDev->maxChannels++;
   }
   return pDaqDev->maxChannels;
}

/*! ----------------------------------------------------------------------------
 * @see daq.h
 */
int daqFindAndInitializeAll( DAQ_ALL_T* pAllDAQ, const void* pScuBusBase )
{
   SCUBUS_SLAVE_FLAGS_T daqPersentFlags;

   // Paranoia...
   LM32_ASSERT( pScuBusBase != (void*)ERROR_NOT_FOUND );
   LM32_ASSERT( pAllDAQ != NULL );

   // Pre-initializing
   memset( pAllDAQ, 0, sizeof( DAQ_ALL_T ));

   daqPersentFlags = scuBusFindSpecificSlaves( pScuBusBase, 0x37, 0x26 );
   if( daqPersentFlags == 0 )
   {
      DBPRINT( "DBG: No DAQ slaves found!\n" );
      return 0;
   }

   for( int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( !scuBusIsSlavePresent( daqPersentFlags, slot ) )
         continue;

      pAllDAQ->aDaq[pAllDAQ->foundDevices].pReg =
          getAbsScuBusSlaveAddr( pScuBusBase, slot ) + DAQ_REGISTER_OFFSET;
      DBPRINT2( "DBG: DAQ found in slot: %02d, address: 0x%08x\n", slot,
                pAllDAQ->aDaq[pAllDAQ->foundDevices].pReg );
      if( daqFindChannels( &pAllDAQ->aDaq[pAllDAQ->foundDevices], slot ) == 0 )
      {
         DBPRINT2( "DBG: DAQ in slot %d has no input channels - skipping\n", slot );
         continue;
      }
#ifdef CONFIG_DAQ_PEDANTIC_CHECK
      LM32_ASSERT( pAllDAQ->aDaq[pAllDAQ->foundDevices].maxChannels ==
         daqDeviceGetMaxChannels( &pAllDAQ->aDaq[pAllDAQ->foundDevices] ) );
#endif
      pAllDAQ->foundDevices++; // At least one channel was found.
#if DAQ_MAX < MAX_SCU_SLAVES
      if( pAllDAQ->foundDevices == ARRAY_SIZE( pAllDAQ->aDaq ) )
         break;
#endif
   }

   return pAllDAQ->foundDevices;
}

/*! ---------------------------------------------------------------------------
 * @see daq.h
 */
int daqGetNumberOfAllFoundChannels( DAQ_ALL_T* pAllDAQ )
{
   int ret = 0;
   for( int i = 0; i < pAllDAQ->foundDevices; i++ )
      ret += pAllDAQ->aDaq[i].maxChannels;
   return ret;
}

/*================================== EOF ====================================*/

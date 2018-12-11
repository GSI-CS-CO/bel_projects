/** @file scu_bus.c
 *
 *  Copyright (C) 2011-2018 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Stefan Rauch <s.rauch@gsi.de> perhaps
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
#include "scu_bus.h"
#include "display.h"
#include "w1.h"
#include "inttypes.h"
#include "mprintf.h"
#include "dow_crc.h"
#include "dbg.h"

#define DEBUG


/*!  for every found slave the slotnumber is added to the slave array
     e.g. [2,3] means slaves in slot 2 and 3
*/
void probe_scu_bus(volatile unsigned short* bus_addr, unsigned short system_addr,
                   unsigned short group_addr, int* slaves)
{
  int slot;
  unsigned short cid_sys, cid_group;
  for (slot = 1; slot <= MAX_SCU_SLAVES; slot++)  {
    cid_sys = bus_addr[(slot<<16) + CID_SYS];     //CID system addr from slave
    cid_group = bus_addr[(slot<<16) + CID_GROUP]; //CID group addr from slave
    if (cid_sys == system_addr && cid_group == group_addr) 
      *(slaves++) = slot;  
  }
  *slaves = 0; // end of device list 
}

void ReadTempDevices(int bus, uint64_t *id, uint32_t *temp)
{
  struct w1_dev *d;
  int i;
  int tvalue;
  wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
  if (w1_scan_bus(&wrpc_w1_bus) > 0)  {
    for (i = 0; i < W1_MAX_DEVICES; i++) {
      d = wrpc_w1_bus.devs + i;
      if (d->rom) {
        if ((calc_crc((int)(d->rom >> 32), (int)d->rom)) != 0)
          continue;
        #ifdef DEBUG
        mprintf("bus,device (%d,%d): 0x%08x%08x ", wrpc_w1_bus.detail, i, (int)(d->rom >> 32), (int)d->rom);
        #endif
        if ((char)d->rom == 0x42) {
          *id = d->rom;
          tvalue = w1_read_temp(d, 0);
          *temp = (tvalue >> 12); //full precision with 1/16 degree C
          #ifdef DEBUG
          mprintf("temp: %dC", tvalue >> 16); //show only integer part for debug
          #endif
        }
        #ifdef DEBUG
        mprintf("\n");
        #endif
      }
    }
  } else {
    #ifdef DEBUG
    mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail);
    #endif
  }
} 

/*! ---------------------------------------------------------------------------
 * @brief Finds all scu-bus slaves which match by one or all items of the
 *        given match-list depending on mode.
 * @see SCU_BUS_MATCH_ITEM16
 * @see scuBusIsSlavePresent
 * @see scuBusFindAllSlaves
 * @see find_device_adr
 * @see SCUBUS_FIND_MODE_T
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param pMatchList  Match-list with SCU_BUS_MATCH_LIST16_TERMINATOR as last element.
 * @note The last item of pMatchList has always to be the terminator:
 *       SCU_BUS_MATCH_LIST16_TERMINATOR
 * @param mode Determines how the match-list becomes handled.
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0000 0010 1000: means: Slot 4 and 6 are used by devices where \n
 *         all or one item of the given match-list match depending on parameter mode.
 */
SCUBUS_SLAVE_FLAGS_T
  scuBusFindSlavesByMatchList16( const void* pScuBusBase,
                                 const struct SCU_BUS_MATCH_ITEM16 pMatchList[],
                                 const enum SCUBUS_FIND_MODE_T mode )
{
   bool _or( bool a, bool b )  { return (a || b); }
   bool _and( bool a, bool b ) { return (a && b); }

   LM32_ASSERT( pMatchList[0].index < SCUBUS_INVALID_INDEX16 );
   bool (*op)( bool, bool ) = (mode == ALL)? _and : _or;
   SCUBUS_SLAVE_FLAGS_T slaveFlags = 0;
   for( int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      const void* pSlaveAddr = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
      int i = 0;
      bool match = (mode == ALL);
      while( pMatchList[i].index < SCUBUS_INVALID_INDEX16 )
      {
         match = op( match,
                     scuBusGetSlaveValue16( pSlaveAddr, pMatchList[i].index ) ==
                        pMatchList[i].value );
         i++;
      }
      if( match )
         slaveFlags |= (1 << (slot-SCUBUS_START_SLOT));
   }
   return slaveFlags;
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU bus and initialized a slave-flags present field if
 *        the given system address and group address match.
 * @see scuBusIsSlavePresent
 * @see scuBusFindAllSlaves
 * @see find_device_adr
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @param systemAddr System address
 * @param groupAddr  group address
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0000 0010 1000: means: Slot 4 and 6 are used by devices where \n
 *         system address and group address match.
 */
SCUBUS_SLAVE_FLAGS_T scuBusFindSpecificSlaves( const void* pScuBusBase,
                                               const uint16_t systemAddr,
                                               const uint16_t grupAddr )
{
   const struct SCU_BUS_MATCH_ITEM16 matchList[] =
   {
      { .index = CID_SYSTEM,.value = systemAddr },
      { .index = CID_GROUP, .value = grupAddr },
      SCUBUS_MATCH_LIST16_TERMINATOR
   };
   return scuBusFindSlavesByMatchList16( pScuBusBase, matchList, ALL );
}

/*! ---------------------------------------------------------------------------
 * @brief Scans the whole SCU bus for all slots and initialized a slave-flags
 *        present field for each found device.
 * @see scuBusIsSlavePresent
 * @see scuBusFindSpecificSlaves
 * @param pScuBusBase Base address of SCU bus.
 *                    Obtained by find_device_adr(GSI, SCU_BUS_MASTER);
 * @return Flag field for SCU present bits e.g.: \n
 *         0000 0001 0001 0000: means: Slot 5 and 9 are used all others are free.
 */
SCUBUS_SLAVE_FLAGS_T scuBusFindAllSlaves( const void* pScuBusBase )
{
   SCUBUS_SLAVE_FLAGS_T slaveFlags = 0;

   for( int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      const void* pSlaveAddr = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
      if( scuBusGetSlaveValue16( pSlaveAddr, CID_SYS )   != SCUBUS_INVALID_VALUE ||
          scuBusGetSlaveValue16( pSlaveAddr, CID_GROUP ) != SCUBUS_INVALID_VALUE )
         slaveFlags |= (1 << (slot-SCUBUS_START_SLOT));
   }

   return slaveFlags;
}

/*! ---------------------------------------------------------------------------
 * @see scu_bus.h
 */
unsigned int getNumberOfSlaves( const SCUBUS_SLAVE_FLAGS_T slaveFlags )
{
   unsigned int ret = 0;
   for( int i = 0; i <= (MAX_SCU_SLAVES-SCUBUS_START_SLOT); i++ )
      if( (slaveFlags & (1 << i)) != 0 )
         ret++;
   return ret;
}




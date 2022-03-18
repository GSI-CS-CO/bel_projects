/*!
 * @file scu_bus.c
 * @brief Administration of SCU-Bus for LM32 applications.
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 *
 * @see
 * <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/StdRegScuBusSlave">
 *    Registersatz SCU-Bus-Slaves</a>
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

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Function will need in function scuBusFindSlavesByMatchList16
 * @see scuBusFindSlavesByMatchList16
 */
STATIC bool _or( const bool a, const bool b )
{
   return (a || b);
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Function will need in function scuBusFindSlavesByMatchList16
 * @see scuBusFindSlavesByMatchList16
 */
STATIC bool _and( const bool a, const bool b )
{
   return (a && b);
}

/*! ---------------------------------------------------------------------------
 * @ingroup SCU_BUS
 * @brief Finds all scu-bus slaves which match by one or all items of the
 *        given match-list depending on mode.
 * @see SCU_BUS_MATCH_ITEM16_T
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
                                 const SCU_BUS_MATCH_ITEM16_T pMatchList[],
                                 const SCUBUS_FIND_MODE_T mode )
{
   SCUBUS_ASSERT( pMatchList[0].index < SCUBUS_INVALID_INDEX16 );
   bool (*op)( const bool, const bool ) = (mode == ALL)? _and : _or;
   SCUBUS_SLAVE_FLAGS_T slaveFlags = 0;
   SCU_BUS_FOR_EACH_SLOT( slot )
   {
      const void* pSlaveAddr = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
      unsigned int i = 0;
      bool match = (mode == ALL);
      while( pMatchList[i].index < SCUBUS_INVALID_INDEX16 )
      {
         match = op( match,
                     scuBusGetSlaveValue16( pSlaveAddr, pMatchList[i].index ) ==
                        pMatchList[i].value );
         i++;
      }
      if( match )
         slaveFlags |= scuBusGetSlaveFlag( slot );
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
                                               const SLAVE_SYSTEM_T systemAddr,
                                               const SLAVE_GROUP_T grupAddr )
{
   const SCU_BUS_MATCH_ITEM16_T matchList[] =
   {
      { .index = CID_SYSTEM, .value = systemAddr },
      { .index = CID_GROUP,  .value = grupAddr },
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

   SCU_BUS_FOR_EACH_SLOT( slot )
   {
      const void* pSlaveAddr = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
      if( scuBusGetSlaveValue16( pSlaveAddr, CID_SYSTEM ) != SCUBUS_INVALID_VALUE ||
          scuBusGetSlaveValue16( pSlaveAddr, CID_GROUP ) != SCUBUS_INVALID_VALUE )
         slaveFlags |= scuBusGetSlaveFlag( slot );
   }

   return slaveFlags;
}

/*! ---------------------------------------------------------------------------
 * @see scu_bus.h
 */
unsigned int scuBusGetNumberOfSlaves( const SCUBUS_SLAVE_FLAGS_T slaveFlags )
{
   unsigned int ret = 0;
   for( int i = 0; i <= (MAX_SCU_SLAVES-SCUBUS_START_SLOT); i++ )
      if( (slaveFlags & (1 << i)) != 0 )
         ret++;
   return ret;
}

/*================================== EOF ====================================*/



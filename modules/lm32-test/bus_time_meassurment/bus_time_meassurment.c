/*!
 * @file      bus_time_measurement.c
 * @brief     Testprogram to measure SCU-bus acces time to a slave register
 *
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      26.11.2020
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
#include <stdbool.h>
#include "eb_console_helper.h"
#include "scu_wr_time.h"
#include "scu_bus.h"
#include "scu_ddr3.h"

#include "helper_macros.h"

static inline uint64_t getTimeOffset( void )
{
   volatile uint64_t t1 = getWrSysTime();
   volatile uint64_t t2 = getWrSysTime();
   return  (t2 - t1);
}

void main( void )
{
   uint64_t  toff = getTimeOffset();
   mprintf( ESC_CLR_SCR ESC_XY( "0", "0" ) "Start measurement."
            "Compiler: " COMPILER_VERSION_STRING
            "\ntime-offset = %u\n", (unsigned int)toff );
#if 1
   void* pScuBusBase = find_device_adr( GSI, SCU_BUS_MASTER );
   if( pScuBusBase == (void*)ERROR_NOT_FOUND )
   {
      mprintf( ESC_ERROR "Can't find SCU bus master\n" ESC_NORMAL );
      while( true );
   }
   mprintf( "Scanning SCU bus...\n" );
   const SCUBUS_SLAVE_FLAGS_T slavePersentFlags = scuBusFindAllSlaves( pScuBusBase );

   /*
    * Using the first found slave on the SCU- bus.
    */
   void* pAddress = NULL;
   for( unsigned int slot = SCUBUS_START_SLOT; slot <= MAX_SCU_SLAVES; slot++ )
   {
      if( scuBusIsSlavePresent( slavePersentFlags, slot ) )
      {
         pAddress = scuBusGetAbsSlaveAddr( pScuBusBase, slot );
         mprintf( "Using slave on slot %u, address: 0x%p\n\n\n", slot, pAddress );
         break;
      }
   }

   if( pAddress == NULL )
   {
      mprintf( ESC_ERROR
               "No slave(s) found on SCU-bus so this test isn't meaningful!\n"
               ESC_NORMAL );
      while( true );
   }

#endif
   volatile uint64_t t1, t2;
   t1 = getWrSysTime();
  // NOP();
  // NOP();
  // NOP();
  // NOP();
   scuBusSetSlaveValue16( pAddress, Echo_Register, 0xAAAA );
 //  scuBusSetSlaveValue16( pAddress, Echo_Register, 0x5555 );
   t2 = getWrSysTime();

   volatile uint64_t tr = t2 - t1 - toff;
   mprintf( "Writing on echo register: Time: 0x%08X%08X -> %u ns\n",
            ((uint32_t*)&tr)[0], ((uint32_t*)&tr)[1],
            (unsigned int)tr );

   uint16_t ret;
   t1 = getWrSysTime();
   ret = scuBusGetSlaveValue16( pAddress, Echo_Register );
   t2 = getWrSysTime();
   tr = t2 - t1 - toff;
   mprintf( "Reading on echo register: Time: 0x%08X%08X -> %u ns\n",
            ((uint32_t*)&tr)[0], ((uint32_t*)&tr)[1],
            (unsigned int)tr );
   mprintf( "Value: 0x%04X\n\n\n", ret );

   DDR3_T oDdr3;

   if( ddr3init( &oDdr3 ) < 0 )
   {
      mprintf( ESC_ERROR "Unable to initialize DDR3 RAM!\n" ESC_NORMAL );
      while( true );
   }

   const DDR3_PAYLOAD_T toWrite =
   {
      .ad32[0] = 0x11223344,
      .ad32[1] = 0x55667788
   };

   t1 = getWrSysTime();
   ddr3write64( &oDdr3, 4711, &toWrite );
   t2 = getWrSysTime();

   tr = t2 - t1 - toff;
   mprintf( "Writing on DDR3-RAM: Time: 0x%08X%08X -> %u ns\n",
            ((uint32_t*)&tr)[0], ((uint32_t*)&tr)[1],
            (unsigned int)tr );

   DDR3_PAYLOAD_T toRead;

   t1 = getWrSysTime();
   ddr3read64( &oDdr3, &toRead, 4711 );
   t2 = getWrSysTime();

   tr = t2 - t1 - toff;
   mprintf( "Reading on DDR3-RAM: Time: 0x%08X%08X -> %u ns\n",
            ((uint32_t*)&tr)[0], ((uint32_t*)&tr)[1],
            (unsigned int)tr );

   mprintf( "Value: 0x%08X%08X\n", toRead.ad32[0], toRead.ad32[1] );

   while( true );
}

/*================================== EOF ====================================*/

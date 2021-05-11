/*!
 * @file scu_temperature.c
 * @brief Updates the temperature information in the shared section
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */

#include "scu_temperature.h"
#include "dow_crc.h"
#include "w1.h"
#include "dbg.h"

/*!
 * @brief Object contains the base pointer of one wire connection
 *        necessary for using updateTemperature();
 */
ONE_WIRE_T g_oneWireBase = { NULL, NULL };

/*! ---------------------------------------------------------------------------
 * @see scu_temperature.h
 */
bool initOneWire( void )
{
   g_oneWireBase.pWr =   (uint8_t*)find_device_adr( CERN, WR_1Wire );
   g_oneWireBase.pUser = (uint8_t*)find_device_adr( GSI, User_1Wire );
   return false;
}

#define DEBUG

/*! ---------------------------------------------------------------------------
 * @todo for-loop is due to parameter pTemperature suspicious!
 */
void readTemperatureFromDevices( const int bus, uint64_t* pId, uint32_t* pTemperature )
{
   wrpc_w1_bus.detail = bus; // set the portnumber of the onewire controller
   if( w1_scan_bus( &wrpc_w1_bus ) <= 0 )
   {
    #ifdef DEBUG
      mprintf("no devices found on bus %d\n", wrpc_w1_bus.detail );
    #endif
      return;
   }

   for( unsigned int i = 0; i < W1_MAX_DEVICES; i++ )
   {
      struct w1_dev* pData = wrpc_w1_bus.devs + i;

      if( pData->rom == 0 )
         continue;
      if(( calc_crc( (int)(pData->rom >> BIT_SIZEOF(uint32_t) ),
                     (int)pData->rom)) != 0 )
         continue;
      #ifdef DEBUG
      mprintf( "bus,device (%d,%d): 0x%08X%08X ",
                wrpc_w1_bus.detail,
                i, (int)(pData->rom >> BIT_SIZEOF(uint32_t)),
                (int)pData->rom );
      #endif
      if( (char)pData->rom == 0x42 )
      {
         *pId = pData->rom;
         int tvalue = w1_read_temp(pData, 0);
         *pTemperature = (tvalue >> 12); //full precision with 1/16 degree C
       #ifdef DEBUG
         mprintf("temperature: %d°C", tvalue >> 16); //show only integer part for debug
       #endif
      }
      #ifdef DEBUG
      mprintf("\n");
      #endif
   }
}

/*! ---------------------------------------------------------------------------
 * @see scu_temperature.h
 */
void updateTemperature( void )
{
   BASE_ONEWIRE = g_oneWireBase.pWr;
   wrpc_w1_init();
#if __GNUC__ >= 9
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#endif
   readTemperatureFromDevices( 0, &g_shared.oTemperatures.board_id,
                                  &g_shared.oTemperatures.board_temp );
   BASE_ONEWIRE = g_oneWireBase.pUser;
   wrpc_w1_init();
   readTemperatureFromDevices( 0, &g_shared.oTemperatures.ext_id,
                                  &g_shared.oTemperatures.ext_temp );
   readTemperatureFromDevices( 1, &g_shared.oTemperatures.backplane_id,
                                  &g_shared.oTemperatures.backplane_temp );
#if __GNUC__ >= 9
   #pragma GCC diagnostic pop
#endif
   BASE_ONEWIRE = g_oneWireBase.pWr; // important for PTP deamon
   wrpc_w1_init();
}

/*================================== EOF ====================================*/

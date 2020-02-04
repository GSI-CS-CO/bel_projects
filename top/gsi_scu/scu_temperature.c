/*!
 * @file scu_temperature.c
 * @brief Updates the temperature information in the shared section
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author Ulrich Becker <u.becker@gsi.de>
 * @date 03.02.2020
 * Outsourced from scu_main.c
 */

#include "scu_temperature.h"

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
   ReadTempDevices( 0, &g_shared.board_id, &g_shared.board_temp );
   BASE_ONEWIRE = g_oneWireBase.pUser;
   wrpc_w1_init();
   ReadTempDevices( 0, &g_shared.ext_id,       &g_shared.ext_temp );
   ReadTempDevices( 1, &g_shared.backplane_id, &g_shared.backplane_temp );
#if __GNUC__ >= 9
   #pragma GCC diagnostic pop
#endif
   BASE_ONEWIRE = g_oneWireBase.pWr; // important for PTP deamon
   wrpc_w1_init();
}

/*================================== EOF ====================================*/

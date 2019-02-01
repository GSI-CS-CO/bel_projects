/*!
 *  @file scu_ddr3.c
 *  @brief Interface routines for DDR3 RAM in SCU3
 *
 *  @see scu_ddr3.h
 *  @see
 *  <a href="https://www-acc.gsi.de/wiki/Hardware/Intern/MacroF%C3%BCr1GbitDDR3MT41J64M16LADesSCUCarrierboards">
 *     DDR3 VHDL Macro der SCU3 </a>
 *  @date 01.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
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
#include <scu_ddr3.h>
#include <mini_sdb.h>
#include <dbg.h>

/*! ---------------------------------------------------------------------------
 * @see scu_ddr3.h
 */
int ddr3init( register DDR3_T* pThis )
{
   DDR_ASSERT( pThis != NULL );
   pThis->pBurstModeBase = NULL;

   pThis->pTrModeBase = find_device_adr( GSI, WB_DDR3_if1 );
   if( pThis->pTrModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pTrModeBase = NULL;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if1 !\n" );
      return -1;
   }

   pThis->pBurstModeBase = find_device_adr( GSI, WB_DDR3_if2 );
   if( pThis->pBurstModeBase == (uint32_t*)ERROR_NOT_FOUND )
   {
      pThis->pBurstModeBase = NULL;
      pThis->pTrModeBase    = NULL;
      DBPRINT1( "DBG: ERROR: DDR3: Can't find address of WB_DDR3_if2 !\n" );
      return -1;
   }
   
   return 0;
}

/*================================== EOF ====================================*/

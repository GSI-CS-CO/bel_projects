/*!
 *  @file mdaq_administration.cpp
 *  @brief MIL-DAQ administration
 *
 *  @date 15.08.2019
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
#include <mdaq_administration.hpp>

using namespace Scu::MiLdaq;

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqEb::EtherboneConnection* poEtherbone )
  :DaqInterface( poEtherbone )
{
}

DaqAdministration::DaqAdministration( daq::EbRamAccess* poEbAccess )
  :DaqInterface( poEbAccess )
{
}

/*-----------------------------------------------------------------------------
 */
DaqAdministration::~DaqAdministration( void )
{
}

/*-----------------------------------------------------------------------------
 */
int DaqAdministration::distributeData( void )
{
   if( !readRingPosition() )
      return 0;

   RingItem oRingItem;
   readRingItem( oRingItem );
// TODO
   incrementRingTail();
   updateRingTail();
   return 0;
}

//================================== EOF ======================================

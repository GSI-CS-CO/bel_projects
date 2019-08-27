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
DaqCompare::DaqCompare( const uint iterfaceAddress )
   :m_iterfaceAddress( iterfaceAddress )
   ,m_pParent( nullptr )
{
}

/*-----------------------------------------------------------------------------
 */
DaqCompare::~DaqCompare( void )
{
}

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( uint location )
   :m_location( location )
   ,m_pParent( nullptr )
{
}

/*-----------------------------------------------------------------------------
 */
DaqDevice::~DaqDevice( void )
{
}

/*-----------------------------------------------------------------------------
 */
bool DaqDevice::registerDaqCompare( DaqCompare* poCompare )
{
   assert( poCompare != nullptr );
   for( auto& i: m_channelPtrList )
   {
      if( poCompare->getAddress() == i->getAddress() )
         return true;
   }
   poCompare->m_pParent = this;
   m_channelPtrList.push_back( poCompare );
   if( m_pParent != nullptr )
      poCompare->onInit();
   return false;
}

/*-----------------------------------------------------------------------------
 */
DaqCompare* DaqDevice::getDaqCompare( const uint address )
{
   for( auto& i: m_channelPtrList )
   {
      if( i->getAddress() == address )
         return i;
   }

   return nullptr;
}

/*-----------------------------------------------------------------------------
 */
void DaqDevice::initAll( void )
{
   for( auto& i: m_channelPtrList )
      i->onInit();
}

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
bool DaqAdministration::registerDevice( DaqDevice* pDevice )
{
   assert( pDevice != nullptr );

   for( auto& i: m_devicePtrList )
   {
      if( i->getLocation() == pDevice->getLocation() )
         return true;
   }
   pDevice->m_pParent = this;
   m_devicePtrList.push_back( pDevice );
   pDevice->initAll();
   return false;
}

/*-----------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDevice( const uint location )
{
   for( auto& i: m_devicePtrList )
   {
      if( i->getLocation() == location )
         return i;
   }
   return nullptr;
}

/*-----------------------------------------------------------------------------
 */
inline
DaqCompare* DaqAdministration::findDaqCompare( const uint channel )
{
   DaqDevice* pDaqDevice = getDevice( getMilDaqLocationByChannel( channel ) );
   if( pDaqDevice == nullptr )
      return nullptr;

   return pDaqDevice->getDaqCompare( getMilDaqAdressByChannel( channel ) );
}

/*-----------------------------------------------------------------------------
 */
int DaqAdministration::distributeData( void )
{
   if( !readRingPosition() )
      return 0;

   RingItem oRingItem;
   readRingItem( oRingItem );

   DaqCompare* pCurrent = findDaqCompare( oRingItem.getChannel() );
   if( pCurrent != nullptr )
      pCurrent->onData( oRingItem.getTimestamp(),
                        oRingItem.getActValue32(),
                        oRingItem.getSetValue32() );
   else
      onUnregistered( &oRingItem );


   incrementRingTail();
   updateRingTail();
   return 0;
}

//================================== EOF ======================================

/*!
 *  @file daq_administration.cpp
 *  @brief DAQ administration
 *
 *  @date 04.03.2019
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
#include <daq_administration.hpp>

using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqChannel::DaqChannel( unsigned int number )
   :m_number( number )
   ,m_pParent(nullptr)
{
   SCU_ASSERT( m_number > 0 );
   SCU_ASSERT( m_number<= DaqInterface::c_maxChannels );
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( unsigned int number )
   :m_deviceNumber( number )
   ,m_slot( 0 )
   ,m_pParent(nullptr)
{
   SCU_ASSERT( m_deviceNumber > 0 );
   SCU_ASSERT( m_deviceNumber <= DaqInterface::c_maxDevices );
}

/* ----------------------------------------------------------------------------
 */
bool DaqDevice::registerChannel( DaqChannel* pChannel )
{
   SCU_ASSERT( dynamic_cast<DaqChannel*>(pChannel) != nullptr );
   SCU_ASSERT( m_channelPtrList.size() <= DaqInterface::c_maxChannels );
   for( auto& i: m_channelPtrList )
   {
      if( pChannel->getNumber() == i->getNumber() )
         return true;
   }
   pChannel->m_pParent = this;
   m_channelPtrList.push_back( pChannel );
   return false;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqAdmin::DaqAdmin( const std::string wbDevice ): DaqInterface( wbDevice )
{
}

/*! ---------------------------------------------------------------------------
 */
DaqAdmin::~DaqAdmin( void )
{
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdmin::registerDevice( DaqDevice* pDevice )
{
   SCU_ASSERT( dynamic_cast<DaqDevice*>(pDevice) != nullptr );
   SCU_ASSERT( m_devicePtrList.size() <= DaqInterface::c_maxDevices );
   for( auto& i: m_devicePtrList )
   {
      if( pDevice->getDeviceNumber() == i->getDeviceNumber() )
         return true;
   }
   pDevice->m_pParent = this;
   m_devicePtrList.push_back( pDevice );
   return false;
}

bool DaqAdmin::unregisterDevice( DaqDevice* pDevice )
{
}


//================================== EOF ======================================

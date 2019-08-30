/*!
 *  @file daq_channel_container.hpp
 *  @brief Helper template to simplify the DAQ-channel administration
 *
 *  Nice to have.
 *
 *  @date 15.03.2019
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
#ifndef _DAQ_CHANNEL_CONTAINER_HPP
#define _DAQ_CHANNEL_CONTAINER_HPP

#include <daq_administration.hpp>

namespace Scu
{
namespace daq
{

/*!
 * @ingroup DAQ
 */
///////////////////////////////////////////////////////////////////////////////
/*!
 */
template <typename CHANNEL_T, typename DEVICE_T = DaqDevice>
class DaqChannelContainer: public DaqAdministration
{
   static_assert( std::is_class<CHANNEL_T>::value,
                  "CHANNEL_T type is not a class!" );
   static_assert( std::is_base_of<DaqChannel, CHANNEL_T>::value,
                  "CHANNEL_T has not inherited from class daq::DaqChannel!" );

   static_assert( std::is_class<DEVICE_T>::value,
                  "DEVICE_T type is not a class!" );
   static_assert( std::is_base_of<DaqDevice, DEVICE_T>::value,
                  "DEVICE_T has not inherited from class daq::DaqDevice!" );

   void init( void )
   {
      for( uint i = 1; i <= getMaxFoundDevices(); i++ )
      {
         DEVICE_T* pDevice = new DEVICE_T;
         registerDevice( pDevice );
         for( uint j = 1; j <= pDevice->getMaxChannels(); j++ )
            pDevice->registerChannel( new CHANNEL_T );
      }
   }

public:
   DaqChannelContainer( DaqEb::EtherboneConnection* poEtherbone,
                                                        bool doReset = true ):
      DaqAdministration( poEtherbone, doReset )
   {
      init();
   }

   DaqChannelContainer(  EbRamAccess* poEbAccess, bool doReset = true ):
      DaqAdministration( poEbAccess, doReset )
   {
      init();
   }

   virtual ~DaqChannelContainer( void )
   {
      for( auto& itDev: *this )
      {
         for( auto& itChannel: *itDev )
            delete static_cast<CHANNEL_T*>(itChannel);
         delete static_cast<DEVICE_T*>(itDev);
      }
   }

   DEVICE_T* getDeviceByNumber( const uint number )
   {
      return static_cast<DEVICE_T*>
      (
         DaqAdministration::getDeviceByNumber( number )
      );
   }

   DEVICE_T* getDeviceBySlot( const uint slot )
   {
      return static_cast<DEVICE_T*>
      (
         DaqAdministration::getDeviceBySlot( slot )
      );
   }

   CHANNEL_T* getChannelByAbsoluteNumber( uint absChannelNumber )
   {
      return static_cast<CHANNEL_T*>
      (
         DaqAdministration::getChannelByAbsoluteNumber( absChannelNumber )
      );
   }

   CHANNEL_T* getChannelByDeviceNumber( const uint deviceNumber,
                                        const uint channelNumber )
   {
      return static_cast<CHANNEL_T*>
      (
         DaqAdministration::getChannelByDeviceNumber( deviceNumber,
                                                      channelNumber )
      );
   }

   CHANNEL_T* getChannelBySlotNumber( const uint slotNumber,
                                      const uint channelNumber )
   {
      return static_cast<CHANNEL_T*>
      (
         DaqAdministration::getChannelBySlotNumber( slotNumber,
                                                    channelNumber )
      );
   }
}; // class DaqChannelContainer

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQ_CHANNEL_CONTAINER_HPP
//================================== EOF ======================================

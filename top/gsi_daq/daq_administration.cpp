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
#include <algorithm>
#include <iostream>
#include <boost/circular_buffer.hpp>

using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqChannel::DaqChannel( unsigned int number )
   :m_number( number )
   ,m_pParent(nullptr)
{
   SCU_ASSERT( m_number <= DaqInterface::c_maxChannels );
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel::~DaqChannel( void )
{
}


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( unsigned int number )
   :m_deviceNumber( number )
   ,m_slot( 0 )
   ,m_maxChannels( 0 )
   ,m_pParent(nullptr)
{
   SCU_ASSERT( m_deviceNumber <= DaqInterface::c_maxDevices );
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice::~DaqDevice( void )
{
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
   if( pChannel->m_number == 0 )
      pChannel->m_number = m_channelPtrList.size() + 1;
   pChannel->m_pParent = this;
   m_channelPtrList.push_back( pChannel );
   return false;
}

/* ----------------------------------------------------------------------------
 */
DaqChannel* DaqDevice::getChannel( const unsigned int number )
{
   SCU_ASSERT( number > 0 );
   SCU_ASSERT( number <= DaqInterface::c_maxChannels );
   for( auto& i: m_channelPtrList )
   {
      if( i->getNumber() == number )
         return i;
   }
   return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( const std::string wbDevice )
  :DaqInterface( wbDevice )
  ,m_maxChannels( 0 )
{
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::~DaqAdministration( void )
{
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdministration::registerDevice( DaqDevice* pDevice )
{
   SCU_ASSERT( dynamic_cast<DaqDevice*>(pDevice) != nullptr );
   SCU_ASSERT( m_devicePtrList.size() <= DaqInterface::c_maxDevices );

   for( auto& i: m_devicePtrList )
   {
      if( pDevice->getDeviceNumber() == i->getDeviceNumber() )
         return true;
   }

   // Is device number forced?
   if( pDevice->m_deviceNumber == 0 )
   { // No, allocation automatically.
      pDevice->m_deviceNumber = m_devicePtrList.size() + 1;
   }

   pDevice->m_maxChannels = readMaxChannels( pDevice->m_deviceNumber );
   m_maxChannels          += pDevice->m_maxChannels;
   pDevice->m_slot        = getSlotNumber( pDevice->m_deviceNumber );
   pDevice->m_pParent     = this;
   m_devicePtrList.push_back( pDevice );

   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdministration::unregisterDevice( DaqDevice* pDevice )
{
}

/*! ---------------------------------------------------------------------------
 */
int DaqAdministration::redistributeSlotNumbers( void )
{
   if( readSlotStatus() != DAQ_RET_OK )
   {
      for( auto& i: m_devicePtrList )
      {
         i->m_slot = 0; // Invalidate slot number
      }
      return getLastReturnCode();
   }

   for( auto& i: m_devicePtrList )
   {
      i->m_slot = getSlotNumber( i->m_deviceNumber );
   }

   return getLastReturnCode();
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDeviceByNumber( const unsigned int number )
{
   SCU_ASSERT( number > 0 );
   SCU_ASSERT( number <= c_maxDevices );

   for( auto& i: m_devicePtrList )
   {
      if( i->getDeviceNumber() == number )
         return i;
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDeviceBySlot( const unsigned int slot )
{
   SCU_ASSERT( slot > 0 );
   SCU_ASSERT( slot <= c_maxSlots );

   for( auto& i: m_devicePtrList )
   {
      if( i->getSlot() == slot )
         return i;
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelByAbsoluteNumber( unsigned int absChannelNumber )
{
   SCU_ASSERT( absChannelNumber > 0 );
   SCU_ASSERT( absChannelNumber <= (c_maxChannels * c_maxDevices) );

   for( auto& i: m_devicePtrList )
   {
      if( absChannelNumber > i->getMaxChannels() )
      {
         absChannelNumber -= i->getMaxChannels();
         continue;
      }
      return i->getChannel( absChannelNumber );
   }

   return nullptr;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelByDeviceNumber( const unsigned int deviceNumber,
                                             const unsigned int channelNumber )
{
   DAQ_ASSERT_CHANNEL_ACCESS( deviceNumber, channelNumber );

   DaqDevice* poDevice = getDeviceByNumber( deviceNumber );
   if( poDevice == nullptr )
      return nullptr;

   return poDevice->getChannel( channelNumber );
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel*
DaqAdministration::getChannelBySlotNumber( const unsigned int slotNumber,
                                           const unsigned int channelNumber )
{
   SCU_ASSERT( slotNumber > 0 );
   SCU_ASSERT( slotNumber <= c_maxSlots );
   SCU_ASSERT( channelNumber > 0 );
   SCU_ASSERT( channelNumber <= c_maxChannels );

   DaqDevice* poDevice = getDeviceBySlot( slotNumber );
   if( poDevice == nullptr )
      return nullptr;

   return poDevice->getChannel( channelNumber );
}

#ifdef CONFIG_SCU_USE_DDR3
extern "C" {

static int ddr3Poll( const DDR3_T* pThis UNUSED, unsigned int count )
{
   if( count >= 10 )
   {
      return -1;
   }
   usleep( 10 );
   return 0;
}

} // extern "C"
#endif // ifdef CONFIG_SCU_USE_DDR3

/*! ---------------------------------------------------------------------------
 */
int DaqAdministration::distributeData( void )
{
   union PROBE_BUFFER_T
   {
      DAQ_DATA_T        buffer[sizeof(DAQ_DESCRIPTOR_T)/sizeof(DAQ_DATA_T) +
                                      RAM_DAQ_DESCRIPTOR_COMPLETION];
      RAM_DAQ_PAYLOAD_T ramItems[sizeof(PROBE_BUFFER_T::buffer) /
                                 sizeof(RAM_DAQ_PAYLOAD_T)];
      DAQ_DESCRIPTOR_T  descriptor;
   } PACKED_SIZE;

   static_assert( sizeof(PROBE_BUFFER_T) % sizeof(RAM_DAQ_PAYLOAD_T) == 0,
                  "sizeof(PROBE_BUFFER_T) has to be dividable by "
                  "sizeof(RAM_DAQ_PAYLOAD_T) !" );

   PROBE_BUFFER_T probeBuffer;

#ifdef CONFIG_SCU_USE_DDR3
   if( ::ddr3FlushFiFo( &m_oScuRam.ram, 0, ARRAY_SIZE(probeBuffer.ramItems),
                    probeBuffer.ramItems, ddr3Poll ) != EB_OK )
      throw EbException( "Unable to read SCU-Ram buffer" );
#else
   #error At hthe moment DDR3 is supported only please define CONFIG_SCU_USE_DDR3
#endif
   std::cout << "Slot:    " << daqDescriptorGetSlot( &probeBuffer.descriptor ) << std::endl;
   std::cout << "Channel: " << daqDescriptorGetChannel( &probeBuffer.descriptor ) + 1 << std::endl;

   std::cout << "trigger: " << std::hex << daqDescriptorGetTriggerCondition( &probeBuffer.descriptor )
             << std::dec << std::endl;
   std::cout << "delay: " << std::hex << daqDescriptorGetTriggerDelay( &probeBuffer.descriptor )
             << std::dec << std::endl;

   if( daqDescriptorWasDaq( &probeBuffer.descriptor )   +
       daqDescriptorWasHiRes( &probeBuffer.descriptor ) +
       daqDescriptorWasPM( &probeBuffer.descriptor )    != 1 )
   {
      throw( DaqException( "Erroneous descriptor" ) );
   }

   DaqChannel* pChannel = getChannelBySlotNumber(
                            daqDescriptorGetSlot( &probeBuffer.descriptor ),
                            daqDescriptorGetChannel( &probeBuffer.descriptor ) + 1
                          );
   if( pChannel != nullptr )
   {
      std::cout << "Channel found!" << std::endl;
      for( unsigned int i = 0; i < sizeof( probeBuffer.buffer ); i++ )
      {
         pChannel->onDataInput( probeBuffer.buffer[i],
                     i > sizeof( DAQ_DESCRIPTOR_T ) / sizeof( DAQ_DATA_T ));
      }

   }

   return 0;
}

//================================== EOF ======================================

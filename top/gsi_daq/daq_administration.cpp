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
#include <cstddef>
#include <unistd.h>
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
   :m_deviceNumber( 0 )
   ,m_slot( number )
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
bool DaqDevice::unregisterChannel( DaqChannel* pChannel )
{
   SCU_ASSERT( dynamic_cast<DaqChannel*>(pChannel) != nullptr );

   if( pChannel->m_pParent != this )
      return true;

   for( auto& i: m_channelPtrList )
   {
      if( i != pChannel )
         continue;
      //m_channelPtrList.erase( pChannel );
   }

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
std::exception_ptr DaqAdministration::c_exceptionPtr = nullptr;

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( const std::string wbDevice )
  :DaqInterface( wbDevice )
  ,m_maxChannels( 0 )
  ,m_poCurrentDescriptor( nullptr )
//  ,m_pThread( nullptr )
//  ,m_finalizeThread( false )
{
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::~DaqAdministration( void )
{
  // stop();
}

/*! ---------------------------------------------------------------------------
 */
#if 0
void DaqAdministration::start( unsigned int toSleep )
{
   SCU_ASSERT( m_pThread == nullptr );
   m_finalizeThread = false;
   m_pThread = new boost::thread( boost::bind( &DaqAdministration::thread,
                                               this,
                                               toSleep
                                             ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqAdministration::stop( void )
{
   if( m_pThread == nullptr )
      return;

   m_finalizeThread = true;
   m_pThread->join();
   delete m_pThread;
   m_pThread = nullptr;
   if( c_exceptionPtr == nullptr )
      return;
  // std::rethrow_exception( c_exceptionPtr );
   throw( c_exceptionPtr );
}

/*! ---------------------------------------------------------------------------
 */
void DaqAdministration::thread( unsigned int toSleep )
{
   try {
   while( !m_finalizeThread )
   {
      //distributeData();
      //throw( DaqException( "Test" ) );
      throw( std::exception(  ));
      ::usleep( toSleep );

   }
   }
   catch( ... )
   {
      std::cerr << "Exception in thread" << std::endl;
      c_exceptionPtr = std::current_exception();
   }
   std::cout << "Thread left" << std::endl;
}
#endif

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

   if( pDevice->m_slot != 0 )
   {
      if( pDevice->m_slot != getSlotNumber( pDevice->m_deviceNumber ) )
         return true;
   }
   else
      pDevice->m_slot = getSlotNumber( pDevice->m_deviceNumber );

   pDevice->m_maxChannels = readMaxChannels( pDevice->m_deviceNumber );
   m_maxChannels          += pDevice->m_maxChannels;
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

static int ramReadPoll( const DDR3_T* pThis UNUSED, unsigned int count )
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
      DAQ_DATA_T        buffer[c_hiresPmDataLen];
      RAM_DAQ_PAYLOAD_T ramItems[sizeof(PROBE_BUFFER_T::buffer) /
                                 sizeof(RAM_DAQ_PAYLOAD_T)];
      DAQ_DESCRIPTOR_T  descriptor;
   } PACKED_SIZE;

   static_assert( sizeof(PROBE_BUFFER_T)
                   == c_hiresPmDataLen * sizeof(DAQ_DATA_T),
                  "sizeof(PROBE_BUFFER_T) has to be equal"
                  "c_hiresPmDataLen * sizeof(DAQ_DATA_T) !" );
   static_assert( sizeof(PROBE_BUFFER_T) % sizeof(RAM_DAQ_PAYLOAD_T) == 0,
                  "sizeof(PROBE_BUFFER_T) has to be dividable by "
                  "sizeof(RAM_DAQ_PAYLOAD_T) !" );

   std::size_t size = getCurrentRamSize( true );
   if( size == 0 )
   { /*
      * Nothing to do...
      */
      return 0;
   }

   if( size % c_ramBlockShortLen != 0 )
   {
      //TODO data perhaps corrupt!
      clearBuffer();
      throw( DaqException( "Memory size not dividable by block size" ) );
      //return -1;
   }

   PROBE_BUFFER_T probe;
#ifdef CONFIG_DAQ_DEBUG
   ::memset( &probe, 0, sizeof( probe ) );
#endif

   sendLockRamAccess();

   /*
    * At first a short block is supposed. It's necessary to read this data
    * obtaining the device-descriptor.
    */
   if( ::ramReadDaqDataBlock( &m_oScuRam, probe.ramItems,
                              c_ramBlockShortLen, ramReadPoll ) != EB_OK )
   {
      sendUnlockRamAccess();
      throw EbException( "Unable to read SCU-Ram buffer first part" );
   }

   /*
    * Rough check of the device descriptors integrity.
    */
   if( !::daqDescriptorVerifyMode( &probe.descriptor ) )
   {
      //TODO Maybe clearing the entire buffer?
      clearBuffer();
      sendUnlockRamAccess();
      throw( DaqException( "Erroneous descriptor" ) );
   }

   std::size_t wordLen;
   if( ::daqDescriptorIsLongBlock( &probe.descriptor ) )
   { /*
      * Long block has been detected, in this case the rest of the data
      * has still to be read from the DAQ-Ram-buffer.
      */
      if( ::ramReadDaqDataBlock( &m_oScuRam,
                                 &probe.ramItems[c_ramBlockShortLen],
                                 c_ramBlockLongLen - c_ramBlockShortLen,
                                 ramReadPoll ) != EB_OK )
      {
         sendUnlockRamAccess();
         throw EbException( "Unable to read SCU-Ram buffer second part" );
      }
      wordLen = c_hiresPmDataLen - c_discriptorWordSize;
   }
   else
   { /*
      * Short block has been detected.
      */
      wordLen = c_contineousDataLen - c_discriptorWordSize;
   }
   writeRamIndexesAndUnlock();

   //TODO Make CRC check here!

   DaqChannel* pChannel = getChannelByDescriptor( probe.descriptor );

   if( pChannel != nullptr )
   {
      m_poCurrentDescriptor = &probe.descriptor;
      pChannel->onDataBlock( &probe.buffer[c_discriptorWordSize], wordLen );
      m_poCurrentDescriptor = nullptr;
   }

   return getCurrentRamSize( false );
}

//================================== EOF ======================================

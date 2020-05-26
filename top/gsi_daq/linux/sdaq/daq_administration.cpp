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
#include <dbg.h>
#include <eb_console_helper.h>
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
#include <sys/time.h>
#endif

using namespace Scu;
using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
bool DaqChannel::SequenceNumber::compare( uint8_t sequence )
{
   m_blockLost = (m_sequence != sequence) && m_continued;
   if( m_blockLost )
   {
      m_lostCount++;
      DBPRINT1( ESC_FG_RED
                "DBG: ERROR: Sequence is: %d, expected: %d\n"
                ESC_NORMAL,
                sequence, m_sequence
              );
   }
   m_continued = true;
   m_sequence = sequence;
   m_sequence++;
   return m_blockLost;
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel::DaqChannel( uint number )
   :m_number( number )
   ,m_pParent(nullptr)
   ,m_poSequence(nullptr)
{
   SCU_ASSERT( m_number <= DaqInterface::c_maxChannels );
}

/*! ---------------------------------------------------------------------------
 */
DaqChannel::~DaqChannel( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void DaqChannel::verifySequence( void )
{
   if( descriptorWasContinuous() )
      m_poSequence = &m_oSequenceContinueMode;
   else
      m_poSequence = &m_oSequencePmHighResMode;

   if( m_poSequence->compare( descriptorGetSequence() ) )
   {
      DaqAdministration* pAdmin = getParent()->getParent();
      SCU_ASSERT( dynamic_cast<DaqAdministration*>(pAdmin) != nullptr );
      pAdmin->readLastStatus();
      pAdmin->onBlockReceiveError();
   }
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( uint number )
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
DaqChannel* DaqDevice::getChannel( const uint number )
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
//std::exception_ptr DaqAdministration::c_exceptionPtr = nullptr;

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqEb::EtherboneConnection* poEtherbone,
                                      const bool doReset,
                                      const bool doSendCommand
                                    )
   :DaqInterface( poEtherbone, doReset, doSendCommand )
   ,m_maxChannels( 0 )
   ,m_poCurrentDescriptor( nullptr )
   ,m_receiveCount( 0 )
   ,m_maxEbCycleDataLen( c_defaultMaxEbCycleDataLen )
   ,m_blockReadEbCycleGapTimeUs( c_defaultBlockReadEbCycleGapTimeUs )
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   ,m_elapsedTime( 0 )
#endif
{
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( EbRamAccess* poEbAccess,
                                      const bool doReset,
                                      const bool doSendCommand
                                    )
   :DaqInterface( poEbAccess, doReset, doSendCommand )
   ,m_maxChannels( 0 )
   ,m_poCurrentDescriptor( nullptr )
   ,m_receiveCount( 0 )
   ,m_maxEbCycleDataLen( c_defaultMaxEbCycleDataLen )
   ,m_blockReadEbCycleGapTimeUs( c_defaultBlockReadEbCycleGapTimeUs )
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   ,m_elapsedTime( 0 )
#endif
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
      if( pDevice->m_slot == 0  || !isLM32CommandEnabled() )
         pDevice->m_deviceNumber = m_devicePtrList.size() + 1;
      else
         pDevice->m_deviceNumber = getDeviceNumber( pDevice->m_slot );
   }

   if( isLM32CommandEnabled() )
   {
      if( pDevice->m_slot != 0 )
      {
         if( pDevice->m_slot != getSlotNumber( pDevice->m_deviceNumber ) )
            return true;
      }
      else
         pDevice->m_slot = getSlotNumber( pDevice->m_deviceNumber );

      pDevice->m_maxChannels = readMaxChannels( pDevice->m_deviceNumber );
   }
   else
      pDevice->m_maxChannels= 4; //TODO

   m_maxChannels          += pDevice->m_maxChannels;
   pDevice->m_pParent     = this;
   m_devicePtrList.push_back( pDevice );

   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool DaqAdministration::unregisterDevice( DaqDevice* pDevice )
{
   return false;
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
DaqDevice* DaqAdministration::getDeviceByNumber( const uint number )
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
DaqDevice* DaqAdministration::getDeviceBySlot( const uint slot )
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
DaqAdministration::getChannelByAbsoluteNumber( uint absChannelNumber )
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
DaqAdministration::getChannelByDeviceNumber( const uint deviceNumber,
                                             const uint channelNumber )
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
DaqAdministration::getChannelBySlotNumber( const uint slotNumber,
                                           const uint channelNumber )
{
   if( slotNumber == 0 )
      return nullptr;

   if( slotNumber > c_maxSlots )
      return nullptr;

   if( channelNumber == 0 )
      return nullptr;

   if( channelNumber > c_maxChannels )
      return nullptr;

   DaqDevice* poDevice = getDeviceBySlot( slotNumber );
   if( poDevice == nullptr )
      return nullptr;

   return poDevice->getChannel( channelNumber );
}

#if defined( CONFIG_SCU_USE_DDR3 ) && !defined( CONFIG_DDR3_NO_BURST_FUNCTIONS )
/*! ---------------------------------------------------------------------------
 */
extern "C" {

static int ramReadPoll( const DDR3_T* pThis UNUSED, uint count )
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
inline bool DaqAdministration::dataBlocksPresent( void )
{
   std::size_t size = getCurrentRamSize( true );
   return ((size != 0) && ((size % c_ramBlockShortLen) == 0));
}

/*! ---------------------------------------------------------------------------
 * @brief Function performs a block reading divided in smaller sub-blocks to
 *        reduce the maximum EB-cycle open time.
 *
 * That makes time gaps for making occasions for other EB-access devices
 * e.g.: SAFTLIB
 */
int DaqAdministration::readDaqDataBlock( RAM_DAQ_PAYLOAD_T* pData,
                                         std::size_t len )
{
   int ret = EB_OK;
   uint offset = 0;

   assert( m_maxEbCycleDataLen > 0 );

   while( len > 0 )
   {
      std::size_t partLen = std::min( len, m_maxEbCycleDataLen );
      ret = getEbAccess()->readDaqDataBlock( &pData[offset], partLen
                                        #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                                           , ::ramReadPoll
                                        #endif
                                          );
      if( ret != EB_OK )
         return ret;

      len    -= partLen;
      offset += partLen;

      if( m_blockReadEbCycleGapTimeUs != 0 )
         ::usleep( m_blockReadEbCycleGapTimeUs );
   }

   return ret;
}

/*! ---------------------------------------------------------------------------
 */
uint DaqAdministration::distributeData( void )
{
   union PROBE_BUFFER_T
   {
      DAQ_DATA_T        buffer[c_hiresPmDataLen];
      RAM_DAQ_PAYLOAD_T ramItems[sizeof(PROBE_BUFFER_T::buffer) /
                                 sizeof(RAM_DAQ_PAYLOAD_T)];
      DAQ_DESCRIPTOR_T  descriptor;
   };

   static_assert( sizeof(PROBE_BUFFER_T)
                   == c_hiresPmDataLen * sizeof(DAQ_DATA_T),
                  "sizeof(PROBE_BUFFER_T) has to be equal"
                  "c_hiresPmDataLen * sizeof(DAQ_DATA_T) !" );
   static_assert( sizeof(PROBE_BUFFER_T) % sizeof(RAM_DAQ_PAYLOAD_T) == 0,
                  "sizeof(PROBE_BUFFER_T) has to be dividable by "
                  "sizeof(RAM_DAQ_PAYLOAD_T) !" );
   /*
    * For performance reasons the RAM-Size will at first read in the
    * unlocked state.
    * At least one block in RAM?
    */
   if( !dataBlocksPresent() )
   { /*
      * No, nothing to do.
      */
      return getCurrentRamSize( false );
   }

   m_receiveCount++;

   /*
    * At least one date block in RAM. For further actions
    * the LM32 has to be locked, otherwise it could crash in the
    * wishbone-bus. >:-O
    */
  //!! sendLockRamAccess();

   /*
    * After the access locking for the LM32 the RAM size has to be read again.
    * Because it could be that in the meanwhile the LM32 has been received
    * further data- blocks.
    */
   std::size_t size = getCurrentRamSize( true );
   if( (size == 0) || (size % c_ramBlockShortLen != 0) )
   { /*
      * Nothing to do?
      * Or does the LM32 has made bullshit?!?
      * Very unlikely but not excluded.
      * Data in RAM could be corrupt,
      * therefore the entire RAM becomes cleared.
      */
      clearBuffer();
      sendUnlockRamAccess();
      return size;
   }

   PROBE_BUFFER_T probe;
#ifdef CONFIG_DAQ_DEBUG
   ::memset( &probe, 0x7f, sizeof( probe ) );
#endif

#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   USEC_T startTime = getSysMicrosecs();
#endif

   /*
    * At first a short block is supposed. It's necessary to read this data
    * obtaining the device-descriptor.
    */
   if( readDaqDataBlock( &probe.ramItems[0], c_ramBlockShortLen ) != EB_OK )
   {
      sendUnlockRamAccess();
      throw EbException( "Unable to read SCU-Ram buffer first part" );
   }
#ifdef CONFIG_DAQ_TIME_MEASUREMENT
   m_elapsedTime = std::max( getSysMicrosecs() - startTime, m_elapsedTime );
#endif
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
   #ifdef CONFIG_DAQ_TIME_MEASUREMENT
      startTime = getSysMicrosecs();
   #endif
      if( readDaqDataBlock( &probe.ramItems[c_ramBlockShortLen],
                            c_ramBlockLongLen - c_ramBlockShortLen
                          ) != EB_OK )
      {
         sendUnlockRamAccess();
         throw EbException( "Unable to read SCU-Ram buffer second part" );
      }
   #ifdef CONFIG_DAQ_TIME_MEASUREMENT
       m_elapsedTime = std::max( getSysMicrosecs() - startTime, m_elapsedTime );
   #endif
       wordLen = c_hiresPmDataLen - c_discriptorWordSize;
   }
   else
   { /*
      * Short block has been detected.
      */
      wordLen = c_contineousDataLen - c_discriptorWordSize;
   }

   /*
    * Unlock the LM32!
    */
   writeRamIndexesAndUnlock();

   //TODO Make CRC check here!

   DaqChannel* pChannel = getChannelByDescriptor( probe.descriptor );

   if( pChannel != nullptr )
   {
      m_poCurrentDescriptor = &probe.descriptor;
      pChannel->verifySequence();
      pChannel->onDataBlock( &probe.buffer[c_discriptorWordSize], wordLen );
      m_poCurrentDescriptor = nullptr;
   }
   else
   {
      readLastStatus();
      onBlockReceiveError();
   }


   return getCurrentRamSize( false );
}

//================================== EOF ======================================

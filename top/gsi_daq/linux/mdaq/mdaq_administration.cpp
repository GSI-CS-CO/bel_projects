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
#include <string.h>
#include <assert.h>
using namespace Scu::MiLdaq;


///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
DaqCompare::DaqCompare( const uint iterfaceAddress )
   :m_iterfaceAddress( iterfaceAddress )
   ,m_pParent( nullptr )
   ,m_setValueInvalid( true )
{
}

/*-----------------------------------------------------------------------------
 */
DaqCompare::~DaqCompare( void )
{
   if( m_pParent != nullptr )
   {
      m_pParent->unregisterDaqCompare( this );
   }
}

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
DaqDevice::DaqDevice( uint location )
   :DaqBaseDevice( location )
   ,m_location( location )
   ,m_pParent( nullptr )
{
}

/*-----------------------------------------------------------------------------
 */
DaqDevice::~DaqDevice( void )
{
   for( const auto& i: m_channelPtrList )
      i->m_pParent = nullptr;

   if( m_pParent != nullptr )
   {
      m_pParent->unregisterDevice( this );
   }
}

/*-----------------------------------------------------------------------------
 */
bool DaqDevice::registerDaqCompare( DaqCompare* poCompare )
{
   assert( poCompare != nullptr );
   for( const auto& i: m_channelPtrList )
   {
      if( poCompare->getAddress() == i->getAddress() )
      {
         assert( poCompare->m_pParent == this );
         return true;
      }
   }
   poCompare->m_pParent = this;
   m_channelPtrList.push_back( poCompare );
   if( m_pParent != nullptr )
      poCompare->onInit();
   return false;
}

/*-----------------------------------------------------------------------------
 */
bool DaqDevice::unregisterDaqCompare( DaqCompare* poCompare )
{
   for( const auto& i: m_channelPtrList )
   {
      if( i == poCompare )
      {
         assert( i->m_pParent == this );
         m_channelPtrList.remove( i );
         i->m_pParent = nullptr;
         return false;
      }
   }
   return true;
}

/*-----------------------------------------------------------------------------
 */
DaqCompare* DaqDevice::getDaqCompare( const uint address )
{
   for( const auto& i: m_channelPtrList )
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
   for( const auto& i: m_channelPtrList )
      i->onInit();
}

/*-----------------------------------------------------------------------------
 */
void DaqDevice::onReset( void )
{
   for( const auto& i: m_channelPtrList )
      i->onReset();
}

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqEb::EtherboneConnection* poEtherbone )
  :DaqInterface( poEtherbone )
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
  ,m_pfPollDaqData( nullptr )
#endif
  ,m_pMiddleBufferMem( nullptr )
  ,m_middleBufferSize( 0 )
  ,m_nextReadOutTime( 0 )
{
   initPtr();
}

/*-----------------------------------------------------------------------------
 */
DaqAdministration::DaqAdministration( DaqAccess* poEbAccess )
  :DaqInterface( poEbAccess )
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
  ,m_pfPollDaqData( nullptr )
#endif
  ,m_pMiddleBufferMem( nullptr )
  ,m_middleBufferSize( 0 )
  ,m_nextReadOutTime( 0 )
{
   initPtr();
}

/*-----------------------------------------------------------------------------
 */
DaqAdministration::~DaqAdministration( void )
{
   for( const auto& i: m_devicePtrList )
      i->m_pParent = nullptr;

   if( m_pMiddleBufferMem != nullptr )
      delete [] m_pMiddleBufferMem;
}

/*-----------------------------------------------------------------------------
 */
void DaqAdministration::initPtr( void )
{
   if( (m_middleBufferSize == 0)
 #ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
      && !isMilDataInLm32Mem()
 #endif
     )
      m_middleBufferSize = std::min( getRamCapacity() / RAM_ITEM_PER_MIL_DAQ_ITEM, 400UL );

   assert( (m_middleBufferSize % RAM_ITEM_PER_MIL_DAQ_ITEM) == 0 );

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   if( m_pfPollDaqData != nullptr )
      return;

   if( isMilDataInLm32Mem() )
      m_pfPollDaqData = &DaqAdministration::distributeDataOld;
   else
      m_pfPollDaqData = &DaqAdministration::distributeDataNew;
#endif
}

/*-----------------------------------------------------------------------------
 */
bool DaqAdministration::registerDevice( DaqDevice* pDevice )
{
   assert( pDevice != nullptr );

   for( const auto& i: m_devicePtrList )
   {
      if( i->getLocation() == pDevice->getLocation() )
      {
         assert( pDevice->m_pParent == this );
         return true;
      }
   }
   pDevice->m_pParent = this;
   pDevice->m_deviceTyp = daq::MIL;
   m_devicePtrList.push_back( pDevice );
   pDevice->initAll();
   return false;
}

/*-----------------------------------------------------------------------------
 */
bool DaqAdministration::unregisterDevice( DaqDevice* pDevice )
{
   assert( pDevice != nullptr );

   for( const auto& i: m_devicePtrList )
   {
      if( i == pDevice )
      {
         assert( i->m_pParent == this );
         m_devicePtrList.remove( i );
         i->m_pParent = nullptr;
         i->m_deviceTyp = daq::UNKNOWN;
         return false;
      }
   }
   return true;
}

/*-----------------------------------------------------------------------------
 */
void DaqAdministration::reset( void )
{
   for( const auto& i: m_devicePtrList )
      i->onReset();
}

/*-----------------------------------------------------------------------------
 */
DaqDevice* DaqAdministration::getDevice( const uint location )
{
   for( const auto& i: m_devicePtrList )
   {
      if( i->getLocation() == location )
         return i;
   }
   return nullptr;
}

/*-----------------------------------------------------------------------------
 */
inline
DaqCompare* DaqAdministration::findDaqCompare( FG_MACRO_T macro )
{
   DaqDevice* pDaqDevice = getDevice( getSocketByFgMacro( macro ) );
   if( pDaqDevice == nullptr )
      return nullptr;

   return pDaqDevice->getDaqCompare( getDeviceByFgMacro( macro ) );
}

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
/*-----------------------------------------------------------------------------
 */
uint DaqAdministration::distributeData( void )
{
#ifdef __DOXYGEN__
   distributeDataOld();
   distributeDataNew();
#endif
   assert( m_pfPollDaqData != nullptr );
   /*
    * Invoking of distributeDataOld() or distributeDataNew() depending on
    * the detected LM32-firmware.
    */
   return (this->*m_pfPollDaqData)();
}

/*!----------------------------------------------------------------------------
 * @brief Access function for MIL-DAQ data when a old LM32-firmware is running.
 */
uint DaqAdministration::distributeDataOld( void )
{
   if( !readRingPosition() ) // WB-access
      return 0;
   uint size = getBufferSize();
   if( size <= 0 )
      return 0;

   size = std::min( size, static_cast<uint>(8) );
   RingItem sDaqData[size];

   size = readRingItems( sDaqData, size ); // WB-access

   for( uint i = 0; i < size; i++ )
   {
      RingItem* pItem = &sDaqData[i];
      DaqCompare* pCurrent = findDaqCompare( pItem->getChannel() );

      if( pCurrent == nullptr )
      {
         onUnregistered( pItem->getChannel() );
         continue;
      }

      pCurrent->m_setValueInvalid =
            (pItem->getChannel().outputBits & SET_VALUE_NOT_VALID_MASK) != 0;

      pCurrent->onData( pItem->getTimestamp(),
                        pItem->getActValue32(),
                        pItem->getSetValue32() );
   }
   return size;
}
#endif // ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE

/*-----------------------------------------------------------------------------
 */
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
uint DaqAdministration::distributeDataNew( void )
#else
uint DaqAdministration::distributeData( void )
#endif
{
//   if( m_nextReadOutTime > daq::getSysMicrosecs() )
//      return 0;
   /*
    * Getting the number of DDR3 memory items which has to be copied
    * in the middle buffer.
    */
   const uint toRead = std::min( getNumberOfNewData(), m_middleBufferSize );

   if( toRead == 0 )
      return toRead;

   if( (toRead % RAM_ITEM_PER_MIL_DAQ_ITEM) != 0 )
   {
      DEBUG_MESSAGE( toRead << " items in MIL buffer not dividable by RAM_ITEM_PER_MIL_DAQ_ITEM" );
      onDataError();
      return toRead;
   }

   /*
    * Middle buffer not created yet?
    */
   if( m_pMiddleBufferMem == nullptr )
   { /*
      * Why not using std::vector?
      * Well a fast data transfer is requested in this case std::vector
      * could be a bit expensive. So the classical C- array will preferred.
      */
      m_pMiddleBufferMem = new MIDDLE_BUFFER_T[m_middleBufferSize];
   }

   DEBUG_MESSAGE( "Before\ntoRead: " << toRead <<
                "\nWrite-index: " << getWriteIndex() <<
                "\nRead-index:  " << getReadIndex() );

   /*
    * Copying via wishbone/etherbone the DDR3-RAM data in the middle buffer.
    * This occupies the wishbone/etherbone bus!
    */
   readDaqData( m_pMiddleBufferMem->aPayload, toRead );
   sendWasRead( toRead );

   DEBUG_MESSAGE( "After\ntoRead: " << toRead <<
                "\nWrite-index: " << getWriteIndex() <<
                "\nRead-index:  " << getReadIndex() );

   /*
    * Splitting the received data containing in the middle buffer in
    * the registered channels.
    */
   const uint itemsToHandling = toRead / RAM_ITEM_PER_MIL_DAQ_ITEM;
   for( uint i = 0; i < itemsToHandling; i++ )
   {
      const BufferItem* pCurrentItem = &m_pMiddleBufferMem[i].oData;

      DaqCompare* pCurrent = findDaqCompare( pCurrentItem->getChannel() );
      if( pCurrent == nullptr )
      {
         onUnregistered( pCurrentItem->getChannel() );
         continue;
      }

      pCurrent->m_setValueInvalid =
            (pCurrentItem->getChannel().outputBits & SET_VALUE_NOT_VALID_MASK) != 0;

      pCurrent->onData( pCurrentItem->getTimestamp(),
                        pCurrentItem->getActValue32(),
                        pCurrentItem->getSetValue32() );

      pCurrent->m_setValueInvalid = true;
   }

   m_nextReadOutTime = daq::getSysMicrosecs() + daq::MICROSECS_PER_SEC / 8;

   return getCurrentNumberOfData();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqAdministrationFgList::DaqAdministrationFgList( DaqEb::EtherboneConnection* poEtherbone )
   :DaqAdministration( poEtherbone )
{
   /*
    * Synchronizing the FG -list from lm32 shared memory only no active
    * scanning!
    */
   m_oFgList.sync( getEbAccess() );
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministrationFgList::DaqAdministrationFgList( DaqAccess* poEbAccess )
   :DaqAdministration( poEbAccess )
{
  /*
   * Synchronizing the FG -lit from lm32 shared memory only no active
   * scanning!
   */
   m_oFgList.sync( getEbAccess() );
}

/*! ---------------------------------------------------------------------------
 */
DaqAdministrationFgList::~DaqAdministrationFgList( void )
{
}

//================================== EOF ======================================

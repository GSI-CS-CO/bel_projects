/*!
 * @file daq_base_interface.cpp
 * @brief DAQ common base interface for ADDAC-DAQ and MIL-DAQ.
 *
 * @date 26.05.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#include <string>
#include <daq_base_interface.hpp>
#include <unistd.h>

using namespace Scu;

///////////////////////////////////////////////////////////////////////////////
/*! --------------------------------------------------------------------------
 */
DaqBaseInterface::DaqBaseInterface( DaqEb::EtherboneConnection* poEtherbone,
                                    const uint64_t dataTimeout )
   :m_poEbAccess( new DaqAccess( poEtherbone ) )
   ,m_ebAccessSelfCreated( true )
   ,m_poRingAdmin( nullptr )
   ,m_lastReadIndex( 0 )
   ,m_daqBaseOffset( 0 )
   ,m_oWatchdog( dataTimeout )
   ,m_maxEbCycleDataLen( c_defaultMaxEbCycleDataLen )
   ,m_blockReadEbCycleGapTimeUs( c_defaultBlockReadEbCycleGapTimeUs )
{

}

/*! --------------------------------------------------------------------------
 */
DaqBaseInterface::DaqBaseInterface( DaqAccess* poEbAccess,
                                    const uint64_t dataTimeout )
   :m_poEbAccess( poEbAccess )
   ,m_ebAccessSelfCreated( false )
   ,m_poRingAdmin( nullptr )
   ,m_lastReadIndex( 0 )
   ,m_daqBaseOffset( 0 )
   ,m_oWatchdog( dataTimeout )
   ,m_maxEbCycleDataLen( c_defaultMaxEbCycleDataLen )
   ,m_blockReadEbCycleGapTimeUs( c_defaultBlockReadEbCycleGapTimeUs )
{

}

/*! --------------------------------------------------------------------------
 */
DaqBaseInterface::~DaqBaseInterface( void )
{
   if( m_ebAccessSelfCreated )
      delete m_poEbAccess;
}

#define BYTE_SWAP( pTarget, origin, member ) \
  pTarget->member = gsi::convertByteEndian( origin.member )

/*! ---------------------------------------------------------------------------
 */
void DaqBaseInterface::checkIntegrity( void )
{
   if( m_poRingAdmin->indexes.start >= m_poRingAdmin->indexes.capacity )
      throw daq::Exception( "Read index of DAQ-buffer is corrupt!" );

   if( m_poRingAdmin->indexes.end > m_poRingAdmin->indexes.capacity )
      throw daq::Exception( "Write index of DAQ-buffer is corrupt!" );

   if( m_poRingAdmin->wasRead > m_poRingAdmin->indexes.capacity )
      throw daq::Exception(  "Value of wasRead of DAQ-buffer is corrupt!" );
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::initRingAdmin( RAM_RING_SHARED_INDEXES_T* pAdmin,
                                      const std::size_t daqBaseOffset  )
{
   assert( m_poRingAdmin == nullptr );
   assert( m_daqBaseOffset == 0 );

   m_poRingAdmin   = pAdmin;
   m_daqBaseOffset = daqBaseOffset;
   assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );

   RAM_RING_SHARED_INDEXES_T lm32Order;


   readLM32( &lm32Order, sizeof( RAM_RING_SHARED_INDEXES_T ) );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.offset );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.capacity );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.start );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.end );
   BYTE_SWAP( m_poRingAdmin, lm32Order, wasRead );

   checkIntegrity();
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::updateMemAdmin( void )
{
   RAM_RING_SHARED_INDEXES_T lm32Order;

   assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );

   static_assert( offsetof( RAM_RING_SHARED_INDEXES_T, indexes.end ) ==
                  offsetof( RAM_RING_SHARED_INDEXES_T, indexes.start ) +
                  sizeof( lm32Order.indexes.start ), "" );

   static_assert( offsetof( RAM_RING_SHARED_INDEXES_T, wasRead ) ==
                  offsetof( RAM_RING_SHARED_INDEXES_T, indexes.end ) +
                  sizeof( lm32Order.indexes.end ), "" );

   readLM32( &lm32Order.indexes.start,
             sizeof( lm32Order.indexes.start )
             + sizeof( lm32Order.indexes.end )
             + sizeof( lm32Order.wasRead ),
             offsetof( RAM_RING_SHARED_INDEXES_T, indexes.start ) );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.start );
   BYTE_SWAP( m_poRingAdmin, lm32Order, indexes.end );
   BYTE_SWAP( m_poRingAdmin, lm32Order, wasRead );

   checkIntegrity();
}

/*! --------------------------------------------------------------------------
 */
uint DaqBaseInterface::getNumberOfNewData( void )
{
   assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );

   if( m_oWatchdog.isBarking() )
      onDataTimeout();

   const uint lastWasToRead = getWasRead();

   /*
    * Synchronize the ring administrator data with the LM32 shared memory.
    */
   updateMemAdmin();

   if( getWasRead() != 0 )
   { /*
      * Server respectively the LM32 has not synchronized the read index yet.
      * Therefore no data are present as well.
      */
      return 0;
   }

   if( (m_lastReadIndex == getReadIndex()) && (lastWasToRead != 0) )
   {
      sendWasRead( lastWasToRead );
      DEBUG_MESSAGE( "Second sendWasRead( " << lastWasToRead << " );" );
      return 0;
   }
   m_lastReadIndex = getReadIndex();

   const uint ret = getCurrentNumberOfData();

   if( ret != 0 )
      m_oWatchdog.start();

   return ret;
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::sendWasRead( const uint wasRead )
{
   assert( dynamic_cast<RAM_RING_SHARED_INDEXES_T*>(m_poRingAdmin) != nullptr );

   ramRingSharedSetWasRead( m_poRingAdmin, wasRead );
   RAM_RING_INDEX_T wasReadBe = gsi::convertByteEndian( m_poRingAdmin->wasRead );
   writeLM32( &wasReadBe, sizeof( wasReadBe ), offsetof( RAM_RING_SHARED_INDEXES_T, wasRead ));
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::onDataReadingPause( void )
{
   if( m_blockReadEbCycleGapTimeUs != 0 )
     ::usleep( m_blockReadEbCycleGapTimeUs );
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::readDaqData( daq::RAM_DAQ_PAYLOAD_T* pData,
                                    std::size_t len )
{
   const std::size_t maxLen = ( m_maxEbCycleDataLen == 0 )? len : m_maxEbCycleDataLen;
   while( true )
   {
      const std::size_t partLen = std::min( len, maxLen );
      /*
       * The next function occupies the wishbone/etherbone bus!
       */
      readRam( pData, partLen );
      len -= partLen;
      if( len == 0 )
         break;
      pData += partLen;
      onDataReadingPause();
   }
}

/*! --------------------------------------------------------------------------
 */
void DaqBaseInterface::onDataError( void )
{
   //TODO
}

//================================== EOF ======================================

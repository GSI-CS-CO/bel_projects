/*!
 *  @file mdaq_interface.cpp
 *  @brief MIL-DAQ Interface Library for Linux
 *
 *  @date 14.08.2019
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
#include <mdaq_interface.hpp>
#include <byteswap.h>
#include <unistd.h>

using namespace Scu::MiLdaq;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqInterface::DaqInterface( DaqEb::EtherboneConnection* poEtherbone )
   :DaqBaseInterface( poEtherbone )
{
   init();
}

DaqInterface::DaqInterface( DaqAccess* poEbAccess )
   :DaqBaseInterface( poEbAccess )
{
   init();
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::~DaqInterface( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::init( void )
{
   uint32_t tmpMagicNumber;
   getEbAccess()->readLM32( &tmpMagicNumber, sizeof( tmpMagicNumber ),
                           offsetof( FG::SCU_SHARED_DATA_T, oSaftLib.oFg.magicNumber ) );

   if( tmpMagicNumber != __bswap_constant_32( FG_MAGIC_NUMBER ) )
      throw Exception( "Wrong magic number respectively wrong LM32 app!" );

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   if( isMilDataInLm32Mem() )
      readRingPosition();
   else
#endif
   {
      readBufferAdmin(); // deprecated!
#ifdef __NEW__
      initRingAdmin( &m_oBufferAdmin.memAdmin,
                     getEbAccess()->getMilDaqOffset() + offsetof( MIL_DAQ_ADMIN_T, memAdmin ) );
#endif
   }
}

#define BYTE_SWAP( target, origin, member ) \
  target.member = gsi::convertByteEndian( origin.member )

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::clearBuffer( bool update )
{
#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   if( isMilDataInLm32Mem() )
   {
      m_oRing.m_tail = m_oRing.m_head = 0;
      if( !update )
         return;

      DAQ_RING_T tmp;
      BYTE_SWAP( tmp, m_oRing, m_head );
      BYTE_SWAP( tmp, m_oRing, m_tail );
      getEbAccess()->writeLM32( &tmp, sizeof( tmp ),
                                  offsetof( FG::SCU_SHARED_DATA_T, daq_buf ) );
      return;
   }
#endif
   ramRingSharedReset( &m_oBufferAdmin.memAdmin );
   if( update )
      writeIndexes();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::checkIntegrity( void )
{
   if( m_oBufferAdmin.memAdmin.indexes.start >= m_oBufferAdmin.memAdmin.indexes.capacity )
      throw Exception( "Read index of MIL-buffer is corrupt!" );

   if( m_oBufferAdmin.memAdmin.indexes.end > m_oBufferAdmin.memAdmin.indexes.capacity )
      throw Exception( "Write index of MIL-buffer is corrupt!" );
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readBufferAdmin( void )
{
   SCU_ASSERT( !isMilDataInLm32Mem() );
   MIL_DAQ_ADMIN_T temp;
   readLM32( &temp, sizeof( temp ) );
   BYTE_SWAP( m_oBufferAdmin, temp, magicNumber );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.offset );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.capacity );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.start );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.end );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.wasRead );
   checkIntegrity();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::updateMemAdmin( void )
{
   SCU_ASSERT( m_oBufferAdmin.magicNumber == MIL_DAQ_MAGIC_NUMBER );
   MIL_DAQ_ADMIN_T temp;

   static_assert( offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.end ) ==
                  offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.start ) +
                  sizeof( temp.memAdmin.indexes.start ), "" );


   static_assert( offsetof( MIL_DAQ_ADMIN_T, memAdmin.wasRead ) ==
                  offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.end ) +
                  sizeof( temp.memAdmin.indexes.end ), "" );

   readLM32( &temp.memAdmin.indexes.start,
             sizeof( temp.memAdmin.indexes.start )
             + sizeof( temp.memAdmin.indexes.end )
             + sizeof( temp.memAdmin.wasRead ),
             offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.start ) );

   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.start );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.indexes.end );
   BYTE_SWAP( m_oBufferAdmin, temp, memAdmin.wasRead );
   checkIntegrity();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::sendWasRead( const RAM_RING_INDEX_T wasRead )
{
   SCU_ASSERT( m_oBufferAdmin.magicNumber == MIL_DAQ_MAGIC_NUMBER );

   ramRingSharedSetWasRead( &m_oBufferAdmin.memAdmin, wasRead );
   RAM_RING_INDEX_T wasReadBe = gsi::convertByteEndian( m_oBufferAdmin.memAdmin.wasRead );
   writeLM32( &wasReadBe, sizeof( wasReadBe ), offsetof( MIL_DAQ_ADMIN_T, memAdmin.wasRead ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeIndexes( void )
{
   SCU_ASSERT( m_oBufferAdmin.magicNumber == MIL_DAQ_MAGIC_NUMBER );
   MIL_DAQ_ADMIN_T temp;

   static_assert( offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.end ) ==
                  offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.start ) +
                  sizeof( temp.memAdmin.indexes.start ), "" );

   BYTE_SWAP( temp, m_oBufferAdmin, memAdmin.indexes.start );
   BYTE_SWAP( temp, m_oBufferAdmin, memAdmin.indexes.end );
   writeLM32( &temp.memAdmin.indexes.start,
              sizeof( temp.memAdmin.indexes.start ), //!! + sizeof( temp.indexes.end ),
              offsetof( MIL_DAQ_ADMIN_T, memAdmin.indexes.start ) );
}

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::readRingPosition( void )
{
   DAQ_RING_T tmp;

   getEbAccess()->readLM32( &tmp, sizeof( tmp ),
                                 offsetof( FG::SCU_SHARED_DATA_T, daq_buf ) );

   BYTE_SWAP( m_oRing, tmp, m_head );
   if( m_oRing.m_head >= c_ringBufferCapacity )
      throw Exception( "Head-index of ring buffer is corrupt!" );

   BYTE_SWAP( m_oRing, tmp, m_tail );
   if( m_oRing.m_tail >= c_ringBufferCapacity )
      throw Exception( "Tail-index of ring buffer is corrupt!" );

   return areDataToRead();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::updateRingTail( void )
{
   RING_INDEX_T convTail = gsi::convertByteEndian( getTailRingIndex() );
   getEbAccess()->writeLM32( &convTail, sizeof( convTail ),
                            offsetof( FG::SCU_SHARED_DATA_T, daq_buf.ring_tail ) );
}

/*! ---------------------------------------------------------------------------
 */
uint DaqInterface::getBufferSize( void ) //TODO Renaming in getCurrentRamSize
{
#ifndef CONFIG_MIL_DAQ_USE_RAM
   int size = getHeadRingIndex() - getTailRingIndex();
   if( size >= 0 )
   {
     // std::cout << ">0" << std::endl;
      return static_cast<uint>(size);
   }
  // std::cout << "<0" << std::endl;
   return getHeadRingIndex() + c_ringBufferCapacity - getTailRingIndex();
#else
   #error TODO: DDR3-Application requiered!
#endif
}

/*! ---------------------------------------------------------------------------
 */
inline
void DaqInterface::readRingData( RING_ITEM_T* ptr, uint len, uint offset )
{
#if 1
   const uint maxLen = ( m_maxEbCycleDataLen == 0 )? len : m_maxEbCycleDataLen;
   while( true )
   {
      const uint partLen = std::min( len, maxLen );
      getEbAccess()->readLM32( ptr,
                               partLen * sizeof( RING_ITEM_T ),
                               offsetof( FG::SCU_SHARED_DATA_T, daq_buf.ring_data ) +
                                        offset * sizeof( RING_ITEM_T )
                             );
      len -= partLen;
      if( len == 0 )
         break;
      ptr    += partLen;
      offset += partLen;
      if( m_blockReadEbCycleGapTimeUs != 0 )
         ::usleep( m_blockReadEbCycleGapTimeUs );
   }
#else
   getEbAccess()->readLM32( ptr,
                           len * sizeof( RING_ITEM_T ),
                           offsetof( FG::SCU_SHARED_DATA_T,
                                     daq_buf.ring_data ) +
                           offset * sizeof( RING_ITEM_T )
                         );
#endif
}

/*! ---------------------------------------------------------------------------
 */
uint DaqInterface::readRingItems( RingItem* pItems, uint size )
{
   uint toRead  = std::min( size, getBufferSize() );
   uint toRead1 = std::min( toRead, c_ringBufferCapacity - getTailRingIndex() );
   uint toRead2 = toRead - toRead1;

   RING_ITEM_T beItems[toRead];

   /*
    * Overlaps the end the data set to be read the end of the linear
    * data buffer, so the data set has to be read in two parts.
    * Because it's a ring buffer.
    * First part:
    */
   if( toRead1 > 0 )
   {
      readRingData( &beItems[0], toRead1, getTailRingIndex() ); // WB-access
   }

   /*
    * Second part (if necessary):
    */
   if( toRead2 > 0 )
   {
      readRingData( &beItems[toRead1], toRead2 ); // WB-access
   }

   #define __BYTE_SWAP_ITEM( member ) \
      pItems[i].member = gsi::convertByteEndian( beItems[i].member )
   for( uint i = 0; i < toRead; i++ )
   {
      __BYTE_SWAP_ITEM( setvalue );
      __BYTE_SWAP_ITEM( actvalue );
      __BYTE_SWAP_ITEM( tmstmp_l );
      __BYTE_SWAP_ITEM( tmstmp_h );
      /*
       * All members of object fgMacro being of type uint8_t with a
       * total size of uint32_t, therefore it will handle
       * as single object without byte swapping!
       */
      pItems[i].fgMacro = beItems[i].fgMacro;
      incrementRingTail();
   }
   #undef __BYTE_SWAP_ITEM
   updateRingTail(); // WB-access
   return toRead;
}
#endif // #ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
//================================== EOF ======================================

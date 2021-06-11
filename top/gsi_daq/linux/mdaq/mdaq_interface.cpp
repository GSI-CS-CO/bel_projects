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
      readBufferAdmin();
}

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
      tmp.m_head = gsi::convertByteEndian( m_oRing.m_head );
      tmp.m_tail = gsi::convertByteEndian( m_oRing.m_tail );
      getEbAccess()->writeLM32( &tmp, sizeof( tmp ),
                                  offsetof( FG::SCU_SHARED_DATA_T, daq_buf ) );
      return;
   }
#endif
   //TODO
#ifndef CONFIG_MIL_DAQ_USE_RAM
//#error CONFIG_MIL_DAQ_USE_RAM not defined
#endif
#warning clearBuffer for DDR3 not implemented yet
   std::cout << "DaqInterface::clearBuffer" << std::endl;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readBufferAdmin( void )
{
   SCU_ASSERT( !isMilDataInLm32Mem() );
   MIL_DAQ_ADMIN_T temp;
   readLM32( &temp, sizeof( temp ) );
   m_oBufferAdmin.magicNumber      = gsi::convertByteEndian( temp.magicNumber );
   m_oBufferAdmin.indexes.offset   = gsi::convertByteEndian( temp.indexes.offset );
   m_oBufferAdmin.indexes.capacity = gsi::convertByteEndian( temp.indexes.capacity );
   m_oBufferAdmin.indexes.start    = gsi::convertByteEndian( temp.indexes.start );
   m_oBufferAdmin.indexes.end      = gsi::convertByteEndian( temp.indexes.end );
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readIndexes( void )
{
   SCU_ASSERT( m_oBufferAdmin.magicNumber == MIL_DAQ_MAGIC_NUMBER );
   MIL_DAQ_ADMIN_T temp;

   static_assert( offsetof( MIL_DAQ_ADMIN_T, indexes.end ) ==
                  offsetof( MIL_DAQ_ADMIN_T, indexes.start ) +
                  sizeof( temp.indexes.start ), "" );


   readLM32( &temp.indexes.start,
             sizeof( temp.indexes.start ) + sizeof( temp.indexes.end ),
             offsetof( MIL_DAQ_ADMIN_T, indexes.start ) );

   m_oBufferAdmin.indexes.start = gsi::convertByteEndian( temp.indexes.start );
   m_oBufferAdmin.indexes.end   = gsi::convertByteEndian( temp.indexes.end );
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeIndexes( void )
{
   SCU_ASSERT( m_oBufferAdmin.magicNumber == MIL_DAQ_MAGIC_NUMBER );
   MIL_DAQ_ADMIN_T temp;

   static_assert( offsetof( MIL_DAQ_ADMIN_T, indexes.end ) ==
                  offsetof( MIL_DAQ_ADMIN_T, indexes.start ) +
                  sizeof( temp.indexes.start ), "" );

   temp.indexes.start = gsi::convertByteEndian( m_oBufferAdmin.indexes.start );
   temp.indexes.end   = gsi::convertByteEndian( m_oBufferAdmin.indexes.end );
   writeLM32( &temp.indexes.start,
              sizeof( temp.indexes.start ) + sizeof( temp.indexes.end ),
              offsetof( MIL_DAQ_ADMIN_T, indexes.start ) );
}

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::readRingPosition( void )
{
   DAQ_RING_T tmp;

   getEbAccess()->readLM32( &tmp, sizeof( tmp ),
                                 offsetof( FG::SCU_SHARED_DATA_T, daq_buf ) );

   m_oRing.m_head = gsi::convertByteEndian( tmp.m_head );
   if( m_oRing.m_head >= c_ringBufferCapacity )
      throw Exception( "Head-index of ring buffer is corrupt!" );

   m_oRing.m_tail = gsi::convertByteEndian( tmp.m_tail );
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

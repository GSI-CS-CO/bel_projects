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

using namespace Scu::MiLdaq;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqInterface::DaqInterface( DaqEb::EtherboneConnection* poEtherbone )
   :m_ebAccessSelfCreated( true )
   ,m_poEbAccess( new daq::EbRamAccess( poEtherbone ) )
{
   init();
}

DaqInterface::DaqInterface( daq::EbRamAccess* poEbAccess )
   :m_ebAccessSelfCreated( false )
   ,m_poEbAccess( poEbAccess )
{
   init();
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::~DaqInterface( void )
{
   if( m_ebAccessSelfCreated )
      delete m_poEbAccess;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::init( void )
{
   uint32_t tmpMagicNumber;

   m_poEbAccess->readLM32( &tmpMagicNumber, sizeof( tmpMagicNumber ),
                           offsetof( FG::SCU_SHARED_DATA_T, fg_magic_number ) );

   if( tmpMagicNumber != __bswap_constant_32( FG_MAGIC_NUMBER ) )
      throw Exception( "Wrong magic number respectively wrong LM32 app!" );
   readRingPosition();
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::readRingPosition( void )
{
   DAQ_RING_T tmp;

   m_poEbAccess->readLM32( &tmp, sizeof( tmp ),
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
   m_poEbAccess->writeLM32( &convTail, sizeof( convTail ),
                            offsetof( FG::SCU_SHARED_DATA_T, daq_buf.ring_tail ) );
}

/*! ---------------------------------------------------------------------------
 */
uint DaqInterface::getBufferSize( void )
{
#ifndef CONFIG_MIL_DAQ_USE_RAM
   int size = getHeadRingIndex() - getTailRingIndex();
   if( size >= 0 )
      return static_cast<uint>(size);
   return getHeadRingIndex() + c_ringBufferCapacity - getTailRingIndex();
#else
   #error TODO: DDR3-Application requiered!
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readRingItem( RingItem& rRingItem )
{
   #define __CONV_MEMBER( m ) \
       rRingItem.m = gsi::convertByteEndian( unconvItem.m )

   RING_ITEM_T unconvItem;

   m_poEbAccess->readLM32( &unconvItem, sizeof( unconvItem ),
                           offsetof( FG::SCU_SHARED_DATA_T, daq_buf.ring_data ) +
                           getTailRingIndex() * sizeof( unconvItem ) );

   __CONV_MEMBER( setvalue );
   __CONV_MEMBER( actvalue );
   __CONV_MEMBER( tmstmp_l );
   __CONV_MEMBER( tmstmp_h );
   __CONV_MEMBER( channel );

   #undef __CONV_MEMBER
}

//================================== EOF ======================================
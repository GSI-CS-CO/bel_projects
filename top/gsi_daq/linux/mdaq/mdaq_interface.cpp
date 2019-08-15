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
#include <generated/shared_mmap.h> //!!
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
                           offsetof( SCU_SHARED_DATA_T, fg_magic_number ) );
   tmpMagicNumber = gsi::convertByteEndian( tmpMagicNumber );
   if( tmpMagicNumber != FG_MAGIC_NUMBER )
   {
      throw Exception( "Wrong magic number!" );
   }
   readRingPosition();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readRingPosition( void )
{
   DAQ_RING_T tmp;
   m_poEbAccess->readLM32( &tmp, sizeof( tmp ),
                                 offsetof( SCU_SHARED_DATA_T, daq_buf ) );
   m_oRing.m_head = gsi::convertByteEndian( tmp.m_head );
   m_oRing.m_tail = gsi::convertByteEndian( tmp.m_tail );
}

//================================== EOF ======================================
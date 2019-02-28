/*!
 *  @file daq_interface.cpp
 *  @brief DAQ Interface Library for Linux
 *
 *  @date 28.02.2019
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
#include <daq_interface.hpp>

using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Constructor of class daq::Daq
 */
Daq::Daq( const std::string wbDevice )
   :m_wbDevice( wbDevice )
   ,m_poEbHandle( nullptr )
{
   if( ::ebOpen( &m_oEbHandle, m_wbDevice.c_str() ) != EB_OK )
      throw Exception( ::ebGetStatusString( &m_oEbHandle ) );

   m_poEbHandle = &m_oEbHandle;

   if( ::ramInit( &m_oScuRam, &m_oSharedData.ramIndexes, m_poEbHandle ) < 0 )
   {
      ebClose();
      throw( Exception( "Could not find RAM-device!" ) );
   }

   readSharedTotal();
}

/*! ---------------------------------------------------------------------------
 * @brief Destructor of class daq::Daq
 */
Daq::~Daq( void )
{
   ebClose();
}

/*! ---------------------------------------------------------------------------
 */
void Daq::ebClose( void )
{
   if( m_poEbHandle == nullptr )
      return;

   if( ::ebClose( m_poEbHandle ) != EB_OK )
     throw Exception( ::ebGetStatusString( m_poEbHandle ) );

   m_poEbHandle = nullptr;
}

/*! ---------------------------------------------------------------------------
 */
void Daq::readSharedTotal( void )
{
   EB_MEMBER_INFO_T info[1];
   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.magicNumber );


   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ::ebObjectReadCycleOpen( m_poEbHandle, &cArg ) != EB_OK )
      throw Exception( ::ebGetStatusString( m_poEbHandle ) );

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, magicNumber );

   ::ebCycleClose( m_poEbHandle );

   while( !cArg.exit )
      ::ebSocketRun( m_poEbHandle );

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      throw Exception( ::ebGetStatusString( m_poEbHandle ) );

   if( m_oSharedData.magicNumber != DAQ_MAGIC_NUMBER )
   {
      throw Exception( "Wrong DAQ magic number" );
   }
}

/*! ---------------------------------------------------------------------------
 */
void Daq::setCommand( DAQ_OPERATION_CODE_T cmd )
{
}

/*! ---------------------------------------------------------------------------
 */
DAQ_OPERATION_CODE_T Daq::getCommand( void )
{
}


//================================== EOF ======================================

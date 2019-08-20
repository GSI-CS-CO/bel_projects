/*!
 *  @file mdaqt.cpp
 *  @brief Main module of MIL-Data Acquisition Tool
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

#ifndef __DOCFSM__
 #include <stdlib.h>
 #include <iostream>
 #include "daqt_read_stdin.hpp"
 #include "daqt_messages.hpp"
 #include "daqt_read_stdin.hpp"
 #include "mdaq_plot.hpp"
#endif
#include "mdaqt.hpp"

using namespace std;
using namespace Scu;
using namespace MiLdaq;
using namespace MiLdaqt;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqMilCompare::DaqMilCompare( uint iterfaceAddress )
   :DaqCompare( iterfaceAddress )
   ,m_pPlot( nullptr )
{
   reset();
}

/*! ---------------------------------------------------------------------------
 */
DaqMilCompare::~DaqMilCompare( void )
{
   if( m_pPlot != nullptr )
      delete m_pPlot;
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::reset( void )
{
   FSM_INIT_FSM( wait );
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onInit( void )
{
   if( m_pPlot != nullptr )
      return;
   m_pPlot = new Plot( this );
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onData( uint64_t wrTimeStamp, MIL_DAQ_T actValue,
                                                          MIL_DAQ_T setValue )
{
   if( (m_lastSetRawValue == setValue) )//&& (m_lastActRawValue == actlValue) )
      return;

   std::cout << "timestamp: " << wrTimeStamp
             << ", rel time: "
             << (static_cast<double>(wrTimeStamp - m_lastTime) /  1000000000.0)
             << ", act: " << (actValue >> 16)
             << ", set: " << (setValue >> 16) << std::endl;

   m_lastSetRawValue = setValue;
   m_lastActRawValue = actValue;
   m_lastTime = wrTimeStamp;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
MilDaqAdministration::MilDaqAdministration( std::string ebAddress )
   :DaqAdministration( new DaqEb::EtherboneConnection( ebAddress ) )
{
   m_showUnregistered = false;
}

/*! ---------------------------------------------------------------------------
 */
MilDaqAdministration::~MilDaqAdministration( void )
{

}

/*! ---------------------------------------------------------------------------
 */
void MilDaqAdministration::onUnregistered( RingItem* pUnknownItem )
{
   if( !m_showUnregistered )
      return;

   std::cout << pUnknownItem->getTimestamp() << ' '
             << static_cast<int>(pUnknownItem->getActValue()) << ' '
             << static_cast<int>(pUnknownItem->getSetValue())
             << " fg-" << pUnknownItem->getMilDaqLocation() << '-'
             <<  pUnknownItem->getMilDaqAddress() << std::endl;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
int mdaqtMain( int argc, char** ppArgv )
{
   MilDaqAdministration milDaqAdmin( ppArgv[1] );

   DaqMilCompare daqCompare( 130 );
   DaqDevice daqDevice( 39 );

   daqDevice.registerDaqCompare( &daqCompare );
   milDaqAdmin.registerDevice( &daqDevice );

   int key;
   Terminal oTerminal;
   while( (key = Terminal::readKey()) != '\e' )
   {
      milDaqAdmin.distributeData();
  //    DEBUG_MESSAGE( "Head: " << milDaqAdmin.getHeadRingIndex() );
  //    DEBUG_MESSAGE( "Tail: " << milDaqAdmin.getTailRingIndex() );
   }
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   try
   {
      return mdaqtMain( argc, ppArgv );
   }
#if 1
   catch( MiLdaq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: " << e.what() );
   }
#endif
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "std::exception occurred: " << e.what() );
   }
   catch( ... )
   {
      ERROR_MESSAGE( "Undefined exception occurred!" );
   }

   return EXIT_FAILURE;
}

//================================== EOF ======================================

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
 #include "mdaqt_command_line.hpp"
 #include "mdaq_plot.hpp"
 #include <daq_calculations.hpp>
#endif
#include "mdaqt.hpp"

using namespace std;
using namespace Scu;
using namespace Scu::daq;
using namespace MiLdaq;
using namespace MiLdaqt;

#define FSM_INIT_FSM( startState, attr... ) m_state = startState
#define FSM_TRANSITION( nextState, attr... ) m_state = nextState

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqMilCompare::DaqMilCompare( uint iterfaceAddress )
   :DaqCompare( iterfaceAddress )
   ,m_pPlot( nullptr )
   ,m_startTime( 0 )
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
   FSM_INIT_FSM( START, label='Start' );
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onInit( void )
{
   if( m_pPlot != nullptr )
      return;
   m_pPlot = new Plot( this );

   m_pPlot->plot();
}

/*! ---------------------------------------------------------------------------
 */
void
DaqMilCompare::addItem( uint64_t time, MIL_DAQ_T actValue, MIL_DAQ_T setValue )
{
   m_aPlotList.push_back(
   {
     .m_time = static_cast<double>(time) /
               static_cast<double>(daq::NANOSECS_PER_SEC),
     .m_set  = daq::rawToVoltage( static_cast<uint16_t>(setValue >> 16) ),
     .m_act  = daq::rawToVoltage( static_cast<uint16_t>(actValue >> 16) )
   } );
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onData( uint64_t wrTimeStamp, MIL_DAQ_T actValue,
                                                          MIL_DAQ_T setValue )
{
//   if( (m_lastSetRawValue == setValue) )//&& (m_lastActRawValue == actlValue) )
//      return;

   bool next;
   do
   {
      next = false;
      switch( m_state )
      {
         case START:
         {
            m_aPlotList.clear();
            m_startTime = wrTimeStamp;
            addItem( 0, actValue, setValue );
            FSM_TRANSITION( COLLECT );
            break;
         }
         case COLLECT:
         {
            uint64_t plotTime = wrTimeStamp - m_startTime;
            if( plotTime > getTimeLimitNanoSec() ||
                m_aPlotList.size() >= getItemLimit() )
            {
               next = true;
               FSM_TRANSITION( PLOT );
               break;
            }
            addItem( plotTime, actValue, setValue );
            //if( plotTime > NANOSECS_PER_SEC / 50 )
            //   m_pPlot->plot();
        #ifdef __DOCFSM__
            FSM_TRANSITION( COLLECT );
        #endif
            break;
         }
         case PLOT:
         {
            m_pPlot->plot();
            next = true;
            FSM_TRANSITION( START );
            break;
         }
         default: assert( false ); break;
      }
   }
   while( next );

   m_lastSetRawValue = setValue;
   m_lastActRawValue = actValue;
   m_lastTime = wrTimeStamp;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
MilDaqAdministration::MilDaqAdministration( CommandLine* m_poCommandLine,
                                            std::string ebAddress )
   :DaqAdministration( new DaqEb::EtherboneConnection( ebAddress ) )
   ,m_poCommandLine( m_poCommandLine )
{
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
   if( !m_poCommandLine->isVerbose() )
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

   CommandLine cmdLine( argc, ppArgv );

   MilDaqAdministration* pDaqAdmin = cmdLine();

   if( pDaqAdmin == nullptr )
      return EXIT_FAILURE;

   DEBUG_MESSAGE( "SCU: " << pDaqAdmin->getWbDevice() );
   int key;
   Terminal oTerminal;
   DEBUG_MESSAGE( "Entering loop" );
   while( (key = Terminal::readKey()) != '\e' )
   {
      pDaqAdmin->distributeData();
  //    DEBUG_MESSAGE( "Head: " << milDaqAdmin.getHeadRingIndex() );
  //    DEBUG_MESSAGE( "Tail: " << milDaqAdmin.getTailRingIndex() );
   }
   DEBUG_MESSAGE( "Loop left" );
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
   catch( MiLdaq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: " << e.what() );
   }
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

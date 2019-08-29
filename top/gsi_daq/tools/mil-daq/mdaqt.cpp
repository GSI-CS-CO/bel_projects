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

/*
folgende Kommandos versorgen die FGs in der scuxl0107 mit Daten


fesa-fg-load --dev ~kain/fgtest/aeg/gs11mu2.rmp xgs11mu2 -v

fesa-fg-load --dev ~kain/fgtest/aeg/gs11mu2.rmp xgs11mu3 -v
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
   ,m_lastTime( 0 )
   ,m_timeToPlot( 0 )
   //,m_aPlotList( 1000, {0.0, 0.0, 0.0 } )
   ,m_iterator(m_aPlotList.begin())
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
uint64_t DaqMilCompare::getTimeLimitNanoSec( void )
{
   return getCommandLine()->getXAxisLen() * daq::NANOSECS_PER_SEC;
}

std::size_t DaqMilCompare::getItemLimit( void )
{
   return getCommandLine()->getXAxisLen() * MAX_ITEMS_PER_SECOND;
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onInit( void )
{
   if( m_pPlot != nullptr )
      return;
   m_pPlot = new Plot( this );

  // m_pPlot->plot();
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onReset( void )
{
   if( m_pPlot == nullptr )
      return;

   m_pPlot->init();
   m_pPlot->plot();
}

/*! ---------------------------------------------------------------------------
 */
void
DaqMilCompare::addItem( uint64_t time, MIL_DAQ_T actValue, MIL_DAQ_T setValue )
{
#if 1
   m_aPlotList.push_back(
   {
     .m_time = static_cast<double>(time) /
               static_cast<double>(daq::NANOSECS_PER_SEC),
     .m_set  = daq::rawToVoltage( static_cast<uint16_t>(setValue >> 16) ),
     .m_act  = daq::rawToVoltage( static_cast<uint16_t>(actValue >> 16) )
   } );
#else
   if( m_iterator == m_aPlotList.end() )
      return;
   *m_iterator =
   {
     .m_time = static_cast<double>(time) /
               static_cast<double>(daq::NANOSECS_PER_SEC),
     .m_set  = daq::rawToVoltage( static_cast<uint16_t>(setValue >> 16) ),
     .m_act  = daq::rawToVoltage( static_cast<uint16_t>(actValue >> 16) )
   };
   m_iterator++;
#endif
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
            m_iterator = m_aPlotList.begin();
            m_startTime = wrTimeStamp;
            if( getCommandLine()->isContinuePlottingEnabled() )
               m_timeToPlot = wrTimeStamp + c_minimumPlotInterval;
            addItem( 0, actValue, setValue );
            FSM_TRANSITION( COLLECT );
            break;
         }
         case COLLECT:
         {
            uint64_t plotTime = wrTimeStamp - m_startTime;
            if( plotTime > getTimeLimitNanoSec()
               || m_aPlotList.size() >= getItemLimit()
            )
            {
               next = true;
               FSM_TRANSITION( PLOT );
               break;
            }
            addItem( plotTime, actValue, setValue );
            if( getCommandLine()->isContinuePlottingEnabled() &&
                (wrTimeStamp >= m_timeToPlot) )
            {
               m_pPlot->plot();
               m_timeToPlot = wrTimeStamp + c_minimumPlotInterval;
            }
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
            //cout << m_aPlotList.size() << endl;
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
   if( m_poCommandLine->isVerbose() )
   {
      std::cout << pUnknownItem->getTimestamp() << ' '
                << static_cast<int>(pUnknownItem->getActValue()) << ' '
                << static_cast<int>(pUnknownItem->getSetValue())
                << " fg-" << pUnknownItem->getMilDaqLocation() << '-'
                <<  pUnknownItem->getMilDaqAddress() << std::endl;
   }

   if( !m_poCommandLine->isAutoBuilding() )
      return;

   Device* pDevice = getDevice( pUnknownItem->getMilDaqLocation() );
   if( pDevice == nullptr )
   {
      pDevice = new Device( pUnknownItem->getMilDaqLocation() );
      registerDevice( pDevice );
   }
   pDevice->registerDaqCompare(
                        new DaqMilCompare( pUnknownItem->getMilDaqAddress()));
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
   bool doReceive = true;
   while( (key = Terminal::readKey()) != '\e' )
   {
      switch( key )
      {
         case HOT_KEY_RECEIVE:
         {
            doReceive = !doReceive;
            if( cmdLine.isVerbose() )
               cout << "Plot " << (doReceive? "enable" : "disable" ) << endl;
            break;
         }
         case HOT_KEY_RESET:
         {
            pDaqAdmin->reset();
            if( cmdLine.isVerbose() )
               cout << "Reset" << endl;
            break;
         }
      }
      if( doReceive )
         pDaqAdmin->distributeData();
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

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

#define FSM_TRANSITION( nextState, attr... ) \
{                                            \
   m_state = nextState;                      \
   break;                                    \
}

#define FSM_TRANSITION_NEXT( nextState, attr... ) \
{                                                 \
   next = true;                                   \
   m_state = nextState;                           \
   break;                                         \
}

#define FSM_TRANSITION_SELF( attr... ) break

/*! ---------------------------------------------------------------------------
 */
void onUexpectedException( void )
{
  ERROR_MESSAGE( "Unexpected exception occurred!" );
  throw 0;     // throws int (in exception-specification)
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
DaqMilCompare::DaqMilCompare( uint iterfaceAddress )
   :DaqCompare( iterfaceAddress )
   ,m_pPlot( nullptr )
   ,m_startTime( 0 )
   ,m_lastTime( 0 )
   ,m_minTime( static_cast<uint64_t>(~0) )
   ,m_maxTime( 0 )
   ,m_timeToPlot( 0 )
   ,m_plotIntervalTime( c_minimumPlotInterval )
   //,m_aPlotList( 1000, {0.0, 0.0, 0.0 } )
   ,m_iterator(m_aPlotList.begin())
   ,m_singleShoot( false )
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

/*! ---------------------------------------------------------------------------
 */
std::size_t DaqMilCompare::getItemLimit( void )
{
   return getCommandLine()->getXAxisLen() * MAX_ITEMS_PER_SECOND;
}

/*! ---------------------------------------------------------------------------
 */
uint64_t DaqMilCompare::getPlotIntervalTime( void )
{
   return std::max( m_plotIntervalTime,
                    getCommandLine()->getPoltTime() / 5 * m_plotIntervalTime );

}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onInit( void )
{
   if( m_pPlot != nullptr )
      return;
   m_pPlot = new Plot( this );

   //m_aPlotList.reserve( 1000000 );

  // m_pPlot->plot();
}

/*! ---------------------------------------------------------------------------
 */
void DaqMilCompare::onReset( void )
{
   m_minTime = static_cast<uint64_t>(~0);
   m_maxTime = 0;
   if( m_pPlot == nullptr )
      return;

   m_pPlot->init();
   m_pPlot->plot();
}

/*! ---------------------------------------------------------------------------
 */
void
DaqMilCompare::addItem( uint64_t time, MIL_DAQ_T actValue, MIL_DAQ_T setValue,
                        bool setValueValid )
{
   m_aPlotList.push_back(
   {
     .m_time = static_cast<double>(time) /
               static_cast<double>(daq::NANOSECS_PER_SEC),
     .m_set  = daq::rawToVoltage( setValue ),
     .m_act  = daq::rawToVoltage( actValue ),
     .m_setValid = setValueValid || getCommandLine()->isPlotAlwaysSetValueEnabled()
   } );
}

/*! ---------------------------------------------------------------------------
 * @dotfile mdaqt.gv
 */
void DaqMilCompare::onData( uint64_t wrTimeStamp, MIL_DAQ_T actValue,
                                                          MIL_DAQ_T setValue )
{
//   if( (m_lastSetRawValue == setValue) )//&& (m_lastActRawValue == actlValue) )
//      return;
   m_currentTime = wrTimeStamp;
   if( m_state != START )
   {
      uint64_t timeinterval = m_currentTime - m_lastTime;
      m_minTime = std::min( m_minTime, timeinterval );
      m_maxTime = std::max( m_maxTime, timeinterval );
   }

   /*!
    * @brief Repeat-flag becomes set to true in macro FSM_TRANSITION_NEXT
    */
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
            m_startTime = m_currentTime;
            if( getCommandLine()->isContinuePlottingEnabled() )
               m_timeToPlot = m_currentTime + getPlotIntervalTime();
            addItem( 0, actValue, setValue, !isSetValueInvalid() );
            m_minTime = static_cast<uint64_t>(~0);
            m_maxTime = 0;
            FSM_TRANSITION( COLLECT );
         }
         case COLLECT:
         {
            uint64_t plotTime = m_currentTime - m_startTime;
            if( plotTime > getTimeLimitNanoSec()
               || m_aPlotList.size() >= getItemLimit() )
            {
               FSM_TRANSITION_NEXT( PLOT, color = green );
            }
            addItem( plotTime, actValue, setValue, !isSetValueInvalid() );
            if( getCommandLine()->isContinuePlottingEnabled() &&
                (m_currentTime >= m_timeToPlot) )
            {
               m_pPlot->plot();
               m_timeToPlot = m_currentTime + getPlotIntervalTime();
            }
            FSM_TRANSITION_SELF( color = blue );
         }
         case PLOT:
         {
            m_pPlot->plot();
            if( isSingleShoot() )
               FSM_TRANSITION_NEXT( WAIT, label='Single shoot enabled' );
            FSM_TRANSITION_NEXT( START );
         }
         case WAIT:
         {
            if( isSingleShoot() )
               FSM_TRANSITION_SELF();
            FSM_TRANSITION_NEXT( START, label='Single shoot\ndisabled', color=green );
         }
         default: assert( false ); break;
      }
   }
   while( next );

   m_lastSetRawValue = setValue;
   m_lastActRawValue = actValue;
   m_lastTime = m_currentTime;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
MilDaqAdministration::MilDaqAdministration( CommandLine* m_poCommandLine,
                                            std::string ebAddress )
   :DaqAdministrationFgList( new DaqEb::EtherboneConnection( ebAddress ) )
   ,m_oSwi( getEbAccess() )
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

/*! ---------------------------------------------------------------------------
 */
void MilDaqAdministration::setSingleShoot( bool enable )
{
   for( const auto& i: *this )
   {
      for( const auto& j : *i )
      {
         static_cast<DaqMilCompare*>(j)->setSingleShoot( enable );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline
int mdaqtMain( int argc, char** ppArgv )
{
   DEBUG_MESSAGE( "Start" );
#ifdef DEBUGLEVEL
   for( int i = 0; i < argc; i++ )
      DEBUG_MESSAGE( "Arg " << i << ": " << ppArgv[i] );
#endif

   CommandLine cmdLine( argc, ppArgv );

   MilDaqAdministration* pDaqAdmin = cmdLine();

   if( pDaqAdmin == nullptr )
   {
      DEBUG_MESSAGE( "EXIT_FAILURE" );
      return EXIT_FAILURE;
   }

   DEBUG_MESSAGE( "SCU: " << pDaqAdmin->getWbDevice() );
   int key;
   Terminal oTerminal;
   DEBUG_MESSAGE( "Entering loop" );
   bool doReceive = true;
   bool singleShoot = false;
   uint gapReadInterval = 0;
   constexpr uint gapReadTime = 10;
   while( (key = Terminal::readKey()) != '\e' )
   {
      switch( key )
      {
         case HOT_KEY_RECEIVE:
         {
            doReceive = !doReceive;
            if( cmdLine.isVerbose() )
               cout << "Plot "
                    << (doReceive? "enable" : "disable" ) << endl;
            break;
         }
         case HOT_KEY_RESET:
         {
            pDaqAdmin->reset();
            if( cmdLine.isVerbose() )
               cout << "Reset" << endl;
            break;
         }
         case HOT_KEY_TOGGLE_SINGLE_SHOOT:
         {
            singleShoot = !singleShoot;
            pDaqAdmin->setSingleShoot( singleShoot );
            if( cmdLine.isVerbose() )
               cout << "Single shoot is: "
                    << (singleShoot? "enabled":"disabled") << endl;
            break;
         }
         case HOT_KEY_TOGGLE_GAP_READING:
         {
            if( gapReadInterval == 0 )
               gapReadInterval = gapReadTime;
            else
               gapReadInterval = 0;
            pDaqAdmin->sendSwi( FG::FG_OP_MIL_GAP_INTERVAL, gapReadInterval );
            if( cmdLine.isVerbose() )
               cout << "Gap reading " << ((gapReadInterval != 0)? "enabled" : "disabled") << endl;
            break;
         }
         case HOT_KEY_PRINT_HISTORY:
         {
            if( cmdLine.isVerbose() )
               cout << "Printing history..." << endl;
            pDaqAdmin->sendSwi( FG::FG_OP_PRINT_HISTORY );
         }
      }
      if( doReceive )
         pDaqAdmin->distributeData();
      ::usleep( 100 );
   }
   DEBUG_MESSAGE( "Loop left" );
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUexpectedException );
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
   DEBUG_MESSAGE( "EXIT_FAILURE" );
   return EXIT_FAILURE;
}

//================================== EOF ======================================

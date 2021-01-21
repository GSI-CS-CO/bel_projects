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
 #include "fb_command_line.hpp"
 #include "fb_plot.hpp"
 #include <daq_calculations.hpp>
#endif
#include "fg-feedback.hpp"

using namespace std;
using namespace Scu;

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

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
FbChannel::FbChannel( uint iterfaceAddress )
   :FgFeedbackChannel( iterfaceAddress )
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
   ,m_callCount( 0 )
{
   reset();

}

/*! ---------------------------------------------------------------------------
 */
FbChannel::~FbChannel( void )
{
   if( m_pPlot != nullptr )
      delete m_pPlot;
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::reset( void )
{
   FSM_INIT_FSM( START, label='Start' );
}

/*! ---------------------------------------------------------------------------
 */
uint64_t FbChannel::getTimeLimitNanoSec( void )
{
   return getCommandLine()->getXAxisLen() * daq::NANOSECS_PER_SEC;
}

/*! ---------------------------------------------------------------------------
 */
std::size_t FbChannel::getItemLimit( void )
{
   return getCommandLine()->getXAxisLen() * MAX_ITEMS_PER_SECOND;
}

/*! ---------------------------------------------------------------------------
 */
uint64_t FbChannel::getPlotIntervalTime( void )
{
   return std::max( m_plotIntervalTime,
                    getCommandLine()->getPoltTime() / 5 * m_plotIntervalTime );

}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onInit( void )
{
   if( m_pPlot != nullptr )
      return;

   try
   {
      m_pPlot = new Plot( this );
   }
   catch( gpstr::Exception& e )
   {
      WARNING_MESSAGE( "Unable to plot: \"" << e.what() << "\"" );
      m_pPlot = nullptr;
   }
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onReset( void )
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
void FbChannel::onAddacDataBlock( const bool isSetData,
                                  const uint64_t timestamp,
                                  daq::DAQ_DATA_T* pData,
                                  std::size_t wordLen )
{
   if( !getCommandLine()->isVerbose() )
      return;

   cout << (isSetData? "   set":"actual") << "-values: fg-";
   cout << getSocket() << '-' << getFgNumber() << " received: " << wordLen,
   cout << " words, timestamp: " << daq::wrToTimeDateString( timestamp );
   cout << ", " << timestamp << endl;
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onMilData( const uint64_t timestamp,
                           MiLdaq::MIL_DAQ_T actlValue,
                           MiLdaq::MIL_DAQ_T setValue )
{
   if( !getCommandLine()->isVerbose() )
      return;

   cout << "MIL value: fg-" << getSocket() << '-' << getFgNumber();
   cout << "timestamp: " << daq::wrToTimeDateString( timestamp );
   cout << ", " << timestamp << endl;
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::addItem( const uint64_t time,
                         const MiLdaq::MIL_DAQ_T actValue,
                         const MiLdaq::MIL_DAQ_T setValue,
                         const bool setValueValid )
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
void FbChannel::onData( uint64_t wrTimeStamp, MiLdaq::MIL_DAQ_T actValue,
                                              MiLdaq::MIL_DAQ_T setValue )
{
   if( m_pPlot == nullptr )
      return;

   m_callCount++;
   m_currentTime = wrTimeStamp;
   if( m_state != START )
   {
      uint64_t timeinterval = m_currentTime - m_lastTime;
      m_minTime = std::min( m_minTime, timeinterval );
      m_maxTime = std::max( m_maxTime, timeinterval );
   }

   if( !isMil() &&
      getCommandLine()->isContinuePlottingEnabled() &&
      ((m_callCount % getCommandLine()->getPlotInterval()) != 0)
     )
      return;
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
            addItem( 0, actValue, setValue, true ); //!!!isSetValueInvalid() );
            m_minTime = static_cast<uint64_t>(~0);
            m_maxTime = 0;
            m_callCount++;
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
            addItem( plotTime, actValue, setValue, true ); //!!!isSetValueInvalid() );
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

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onActSetBlockDeviation( const uint setSequ, const uint actSequ )
{
   if( getCommandLine()->isExitOnError() )
      FgFeedbackChannel::onActSetBlockDeviation( setSequ, actSequ );

   WARNING_MESSAGE( "Deviation of sequence numbers from set value input stream: "
                    << setSequ << ", and actual value input stream: "
                    << actSequ << "  are greater than one!" );
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
AllDaqAdministration::AllDaqAdministration( CommandLine* m_poCommandLine,
                                            std::string ebAddress )
   :FgFeedbackAdministration( new DaqEb::EtherboneConnection( ebAddress ) )
   ,m_poCommandLine( m_poCommandLine )
{
}

/*! ---------------------------------------------------------------------------
 */
AllDaqAdministration::~AllDaqAdministration( void )
{

}

/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::setSingleShoot( bool enable )
{
   for( const auto& i: *this )
   {
      for( const auto& j : *i )
      {
         static_cast<FbChannel*>(j)->setSingleShoot( enable );
      }
   }
}

uint AllDaqAdministration::getPlotInterval( void )
{
   return m_poCommandLine->getPlotInterval();
}


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline
int fbMain( int argc, char** ppArgv )
{
   DEBUG_MESSAGE( "Start" );
#ifdef DEBUGLEVEL
   for( int i = 0; i < argc; i++ )
      DEBUG_MESSAGE( "Arg " << i << ": " << ppArgv[i] );
#endif

   CommandLine cmdLine( argc, ppArgv );

   AllDaqAdministration* pDaqAdmin = cmdLine();

   if( pDaqAdmin == nullptr )
   {
      DEBUG_MESSAGE( "EXIT_FAILURE" );
      return EXIT_FAILURE;
   }

   DEBUG_MESSAGE( "SCU: " << pDaqAdmin->getScuDomainName() );
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
            pDaqAdmin->sendGapReadingInterval( gapReadInterval );
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
   try
   {
      return fbMain( argc, ppArgv );
   }
   catch( MiLdaq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: \"" << e.what() << '"' );
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "std::exception occurred: \"" << e.what() << '"' );
   }
   catch( ... )
   {
      ERROR_MESSAGE( "Undefined exception occurred!" );
   }
   DEBUG_MESSAGE( "EXIT_FAILURE" );
   return EXIT_FAILURE;
}

//================================== EOF ======================================

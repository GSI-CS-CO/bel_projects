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
   if( getCommandLine()->isVerbose() )
      cout << "Initializing channel for: fg-" << getSocket() << '-' << getFgNumber() << endl;

   if( getCommandLine()->doNotPlot() )
      return;

   if( getParent()->getParent()->isRunningOnScu() )
      return;

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
                           DAQ_T actlValue,
                           DAQ_T setValue )
{
   if( !getCommandLine()->isVerbose() )
      return;

   cout << "MIL value: fg-" << getSocket() << '-' << getFgNumber();
   cout << ", timestamp: " << daq::wrToTimeDateString( timestamp );
   cout << ", " << timestamp << ", set value: " << setValue
        << ", act value: " << actlValue << endl;
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onTimestampError( const uint64_t timestamp,
                                  DAQ_T actlValue,
                                  DAQ_T setValue )
{
   WARNING_MESSAGE( "Timestamp error of fg-" << getSocket() << '-' << getFgNumber()
        << ": timestamp: " << daq::wrToTimeDateString( timestamp ) << " -> "
        << timestamp
        << "; set-value: " << setValue
        << ", act-value: " << actlValue );
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::addItem( const uint64_t time,
                         const DAQ_T actValue,
                         const DAQ_T setValue,
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
 * @dotfile fg-feedback.gv
 */
void FbChannel::onData( uint64_t wrTimeStamp, DAQ_T actValue,
                                              DAQ_T setValue )
{
   m_callCount++;

   if( m_pPlot == nullptr )
   {
      cout << "fg-" << getSocket() << '-' << getFgNumber() << ":  ";
      if( getCommandLine()->isVerbose() )
         cout     << daq::wrToTimeDateString( wrTimeStamp );
      else
         cout     << wrTimeStamp;
      cout << ",   set: " << setValue
           << ",   act: " << actValue
           << ",   count: " << m_callCount
           << endl;
      return;
   }

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
         case  START:
         {
            m_aPlotList.clear();
            m_iterator = m_aPlotList.begin();
            m_startTime = m_currentTime;
            if( getCommandLine()->isContinuePlottingEnabled() )
               m_timeToPlot = m_currentTime + getPlotIntervalTime();
            addItem( 0, actValue, setValue, !isSetValueInvalid() );
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

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onActSetTimestampDeviation( const uint64_t setTimeStamp,
                                            const uint64_t actTimestamp )
{
   if( getCommandLine()->isExitOnError() )
      FgFeedbackChannel::onActSetTimestampDeviation( setTimeStamp, actTimestamp );

   WARNING_MESSAGE( "Timestamp deviation of " << getFgName() << " is: "
                    << static_cast<int>( actTimestamp - setTimeStamp ) );
}

/*! ---------------------------------------------------------------------------
 */
void FbChannel::onActSetBlockDeviation( const uint setSequ, const uint actSequ )
{
   if( getCommandLine()->isExitOnError() )
      FgFeedbackChannel::onActSetBlockDeviation( setSequ, actSequ );

   WARNING_MESSAGE( "Deviation of sequence numbers of " << getFgName()
                    << " from set value input stream: "
                    << setSequ << ", and actual value input stream: "
                    << actSequ << "  are greater than one!" );
}

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
Device::~Device( void )
{
   vector<FbChannel*> vChannels;

   for( const auto& i: *this )
   {
      vChannels.push_back( static_cast<FbChannel*>(i) );
   }

   for( const auto& channel: vChannels )
   {
      delete channel;
   }
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
   vector<Device*>    vDevices;

   for( const auto& i: *this )
   {
      vDevices.push_back( static_cast<Device*>(i) );
   }

   for( const auto& device: vDevices )
   {
      delete device;
   }
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

/*! ---------------------------------------------------------------------------
 */
uint AllDaqAdministration::getPlotInterval( void )
{
   return m_poCommandLine->getPlotInterval();
}


#ifdef CONFIG_MIL_FG
/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::onUnregisteredMilDevice( FG_MACRO_T fg )
{
   WARNING_MESSAGE( "MIL device: fg-" << static_cast<uint>(fg.socket) << '-' <<
                    static_cast<uint>(fg.device) << "\tnot registered" );
}
#endif

/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::onUnregisteredAddacDaq( uint slot, uint daqNumber )
{
   WARNING_MESSAGE( "ADDAC DAQ " << daqNumber << " in slot " << slot <<
                    "\tnot registered" ); 
}

/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::onDataError( const bool isMil )
{
   WARNING_MESSAGE( "Data length error of " << (isMil? "MIL":"ADDAC") << " data!" );
   FgFeedbackAdministration::onDataError( isMil );
}

/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::onAddacBlockError( uint slot, uint daqNumber )
{
   WARNING_MESSAGE( "Possible loss of data blocks of ADDAC DAQ "
                     << daqNumber << " in slot " << slot );
}

/*! ---------------------------------------------------------------------------
 */
void AllDaqAdministration::onDataTimeout( const bool isMil )
{
   WARNING_MESSAGE( "Timeout of " << (isMil? "MIL":"ADDAC") << " data stream detected!" );
}

/*! ---------------------------------------------------------------------------
 */
bool AllDaqAdministration::isRunningOnScu( void ) const
{
   return m_poCommandLine->isRunningOnScu();
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


   bool repeat;
   do
   {
      repeat = false;
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
      uint intervalTime = 0;
      uint remainingData = 0;
      while( ((key = Terminal::readKey()) != '\e') && !repeat )
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
               //pDaqAdmin = nullptr; //!!!!!!
               //*static_cast<int*>(0) = 4711;
               pDaqAdmin->reset();
               if( cmdLine.isVerbose() )
                  cout << "Reset" << endl;
               break;
            }
            case HOT_KEY_CLEAR_BUFFER:
            {
               pDaqAdmin->clearBuffer();
               if( cmdLine.isVerbose() )
                  cout << "clearing buffer" << endl;
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
               break;
            }
            case HOT_KEY_BUILD_NEW:
            {
               repeat = true;
               if( cmdLine.isVerbose() )
                  cout << "Restart..." << endl;
               break;
            }
         #ifdef CONFIG_EB_TIME_MEASSUREMENT
            case HOT_KEY_SHOW_TIMING:
            {
               daq::USEC_T timestamp;
               daq::USEC_T duration;
               size_t      dataSize;
               FgFeedbackAdministration::WB_ACCESS_T access;
               if( (access = pDaqAdmin->getWbMeasurementMaxTime( timestamp, duration, dataSize )) != FgFeedbackAdministration::UNKNOWN )
               {
                  cout << "Maximum access time in mode: "
                       << FgFeedbackAdministration::accessConstantToString( access )
                       << ": " << dataSize << " bytes; duration: "
                       << duration << " us; time: "
                       << daq::wrToTimeDateString( timestamp * 1000 )
                       << " + " << (timestamp % 1000000) << " us"
                       << endl;
               }
               if( (access = pDaqAdmin->getWbMeasurementMinTime( timestamp, duration, dataSize )) != FgFeedbackAdministration::UNKNOWN )
               {
                  cout << "Minimum access time in mode: "
                       << FgFeedbackAdministration::accessConstantToString( access )
                       << ": " << dataSize << " bytes; duration: "
                       << duration << " us; time: "
                       << daq::wrToTimeDateString( timestamp * 1000 )
                       << " + " << (timestamp % 1000000) << " us"
                       << endl;
               }
               break;
            }
         #endif
         }

         const uint it = daq::getSysMicrosecs();
         if( it >= intervalTime || remainingData != 0 )
         {
            if( it >= intervalTime )
               intervalTime = it + cmdLine.getPollInterwalTime() * 1000;
            if( doReceive )
               remainingData = pDaqAdmin->distributeData();
         }
         ::usleep( 100 );
      }
      DEBUG_MESSAGE( "Loop left" );
   }
   while( repeat );
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUexpectedException );
   try
   {
      return fbMain( argc, ppArgv );
   }
   catch( daq::DaqException& e )
   {
      ERROR_MESSAGE( "ADDAC/ACU-DAQ Exception occurred: \"" << e.what() << "\"\n"
                      << e.getStatusString() );
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

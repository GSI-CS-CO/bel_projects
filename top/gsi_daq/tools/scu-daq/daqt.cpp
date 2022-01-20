/*!
 *  @file daqt.cpp
 *  @brief Main module of Data Acquisition Tool
 *
 *  @date 11.04.2019
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
#include "daqt.hpp"
#include "daqt_messages.hpp"
#include "daqt_command_line.hpp"
#include "daqt_attributes.hpp"
#include "daqt_read_stdin.hpp"

using namespace std;
using namespace Scu;
using namespace daq;
using namespace daqt;

/*!
 * @brief Establishing a upper and lower margin of Y axis, so that the
 *        maximum voltages can be plot as well.
 *
 * The dimension is voltage.
 */
constexpr float Y_PADDING = 0.5;

/*! ---------------------------------------------------------------------------
 */
void onUexpectedException( void )
{
  ERROR_MESSAGE( "Unexpected exception occurred!" );
  throw 0;     // throws int (in exception-specification)
}


/*-----------------------------------------------------------------------------
 */
const char* getSampleRateText( ::DAQ_SAMPLE_RATE_T rate )
{
   switch( rate )
   {
      case ::DAQ_SAMPLE_1MS:   return "1 ms";
      case ::DAQ_SAMPLE_100US: return "100 us";
      case ::DAQ_SAMPLE_10US:  return "10 us";
   }
   return "unknown";
}

///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
Channel::Mode::Mode( Channel* pParent, std::size_t size, std::string text )
   :m_pParent( pParent )
   ,m_size( size )
   ,m_ramLevel( 0 )
   ,m_text( text )
   ,m_notFirst( false )
   ,m_blockCount( 0 )
   ,m_sequence( 0 )
   ,m_sampleTime( 0 )
   ,m_timeStamp( 0 )
   ,m_frequency( 0.0 )
{
   m_pY = new double[m_size];
   if( m_pParent->m_oAttributes.m_postMortem.m_value )
      m_sampleTime = 100000;
   else if( m_pParent->m_oAttributes.m_highResolution.m_value )
      m_sampleTime = 250;
}

/*-----------------------------------------------------------------------------
 */
Channel::Mode::~Mode( void )
{
   delete [] m_pY;
}

/*-----------------------------------------------------------------------------
 */
void Channel::Mode::write( DAQ_DATA_T* pData, std::size_t wordLen )
{
   std::size_t len = std::min( wordLen, m_size );

   m_blockCount++;
   m_sequence   = m_pParent->descriptorGetSequence();
   m_sampleTime = m_pParent->descriptorGetTimeBase();
   m_timeStamp  = m_pParent->descriptorGetTimeStamp() - m_sampleTime * wordLen;
#ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
  m_pParent->getParent()->getParent()->updateMemAdmin();
  m_ramLevel = m_pParent->getParent()->getParent()->getCurrentNumberOfData();
#else
   m_ramLevel   = m_pParent->getParent()->getParent()->getCurrentRamSize( false );
#endif
   m_pParent->calcFrequency( m_frequency, pData, wordLen );
   for( std::size_t i = 0; i < len; i++ )
      m_pY[i] = rawToVoltage( pData[i] );
}

/*-----------------------------------------------------------------------------
 */

inline uint64_t trunc100( uint64_t nsec )
{
   return nsec - (nsec % 100);
}

void Channel::Mode::plot( void )
{
   double visibleTime = nanoSecsToSecs( trunc100(m_size) * m_sampleTime );
   m_pParent->m_oPlot << "set xrange [0:"
                      << nanoSecsToSecs( m_size * m_sampleTime ) << "]" << endl;
   m_pParent->m_oPlot << "set xtics 0," << visibleTime / 10.0 << ","
                                        << visibleTime << endl;
   m_pParent->m_oPlot << "set title \"";
   if( !m_pParent->isMultiplot() )
   {
      m_pParent->m_oPlot << "Slot: " << m_pParent->getSlot()
                         << ", Channel: " << m_pParent->getNumber()
                         << "; ";
   }
   m_pParent->m_oPlot << "Mode: " << m_text << ", Block: " << m_blockCount
                      << ", Sequence: " <<  m_sequence
                      << ", Sample time: " << nanoSecsToSecs( m_sampleTime )
                      << " s, Lost: " << m_pParent->getLostCount()
                      << "\" font \",14\"" << endl;

   m_pParent->m_oPlot << "set xlabel \"Time: " << wrToTimeDateString(m_timeStamp)
                      << " WR: 0x" <<
                      hex << m_timeStamp << dec << ", " << m_timeStamp << " nsec"
                      ", RAM-level: " << m_ramLevel <<
                      " items -> " << std::fixed << setprecision( 2 )
                      << static_cast<double>(m_ramLevel * 100.0
                                   / RAM_SDAQ_MAX_CAPACITY)
                      << "%; Frequency: " << m_frequency << "Hz\"" << endl;

   m_pParent->m_oPlot << "plot '-' title \"\" with lines lc rgb 'green'"
                      << setprecision( 8 ) << endl;
   m_notFirst = true;

   for( std::size_t i = 0; i < m_size; i++ )
      m_pParent->m_oPlot << nanoSecsToSecs(i * m_sampleTime) << ' '
                         << m_pY[i] << endl;
   m_pParent->m_oPlot << 'e' << endl;
}

/*-----------------------------------------------------------------------------
 *  For detailed information about Gnuplot look in to the PDF documentation of
 *  Gnuplot.
 */
void Channel::Mode::reset( void )
{
   m_notFirst   = false;
   m_blockCount = 0;
}

/*-----------------------------------------------------------------------------
 */
Channel::Channel( unsigned int number, const string& rGnuplot )
   :DaqChannel( number )
   ,m_oPlot( "-noraise", rGnuplot )
   ,m_poModeContinuous( nullptr )
   ,m_poModePmHires( nullptr )
{
}

/*-----------------------------------------------------------------------------
 */
Channel::~Channel( void )
{
   if( m_poModeContinuous != nullptr )
      delete m_poModeContinuous;
   if( m_poModePmHires != nullptr )
      delete m_poModePmHires;
}

/*-----------------------------------------------------------------------------
 */
void Channel::sendAttributes( void )
{
   if( m_oAttributes.m_continueTriggerSouce.m_valid )
      sendTriggerSourceContinue( m_oAttributes.m_continueTriggerSouce.m_value );

   if( m_oAttributes.m_highResTriggerSource.m_valid )
      sendTriggerSourceHiRes( m_oAttributes.m_highResTriggerSource.m_value );

   if( m_oAttributes.m_triggerDelay.m_valid )
      sendTriggerDelay( m_oAttributes.m_triggerDelay.m_value );

   if( m_oAttributes.m_triggerCondition.m_valid )
      sendTriggerCondition( m_oAttributes.m_triggerCondition.m_value );

   if( m_oAttributes.m_triggerEnable.m_valid )
      sendTriggerMode( m_oAttributes.m_triggerEnable.m_value );
}

/*-----------------------------------------------------------------------------
 */
void Channel::start( void )
{
   SCU_ASSERT( dynamic_cast<DaqContainer*>(getParent()->getParent()) != nullptr );
   CommandLine* poCommandLine =
     static_cast<DaqContainer*>(getParent()->getParent())->getCommandLinePtr();

   m_oPlot << "set terminal " << poCommandLine->getTerminal();
   if( poCommandLine->isOutputFileDefined() )
   {
      m_oPlot << endl;
      m_oOutputFileName = poCommandLine->getOutputName();
      string inserter = "_";
      for( auto& it: getScuDomainName() )
      {
         if( it == '.' )
            inserter += '_';
         else
            inserter += it;
      }
      inserter += '_' + to_string( getSlot() )
                + '_' + to_string( getNumber() );
      m_oOutputFileName.insert( m_oOutputFileName.find_last_of( '.' ),
                                inserter );
   }
   else
   {
      m_oPlot << " title \"SCU: " << getScuDomainName() << ", LM32: ";
      if( isFgIntegrated() )
         m_oPlot << "DAQ+FG";
      else
         m_oPlot << "DAQ only";
      m_oPlot << '"' << endl;
   }
   m_oPlot << "set grid" << endl;
   m_oPlot << "set ylabel \"Voltage\"" << endl;

   if( !m_oAttributes.m_zoomGnuPlot.m_value )
       m_oPlot << "set yrange ["
               << -(DAQ_VPP_MAX/2 + Y_PADDING) << ':'
               << (DAQ_VPP_MAX/2 + Y_PADDING) << ']' << endl;

   if( m_oAttributes.m_continueMode.m_valid )
   {
      string sRate = ::getSampleRateText(m_oAttributes.m_continueMode.m_value);
      m_poModeContinuous =
         new Mode( this,
                   DaqInterface::c_contineousPayloadLen,
                   "continuous, sample rate: " + sRate );
      sendEnableContineous( m_oAttributes.m_continueMode.m_value,
                            m_oAttributes.m_blockLimit.m_value );
   }

   if( m_oAttributes.m_highResolution.m_valid )
   {
      m_poModePmHires =
         new Mode( this,
                   DaqInterface::c_pmHiresPayloadLen,
                   "high resolution" );
      sendEnableHighResolution( m_oAttributes.m_restart.m_value );
   }
   else if( m_oAttributes.m_postMortem.m_valid )
   {
      m_poModePmHires =
         new Mode( this,
                   DaqInterface::c_pmHiresPayloadLen,
                   "post mortem" );
      sendEnablePostMortem( m_oAttributes.m_restart.m_value );
   }
}

/*! ---------------------------------------------------------------------------
 */
inline
DAQ_DATA_T Channel::buildAverage( const DAQ_DATA_T* pData,
                                                   const std::size_t wordLen )
{
   SCU_ASSERT( wordLen > 0 );

   unsigned int summe = 0;
   for( std::size_t i = 0; i < wordLen; i++ )
      summe += pData[i];

   return summe / wordLen;
}

/*! ---------------------------------------------------------------------------
 */
bool Channel::calcFrequency( double& rFrequency, const DAQ_DATA_T* pData,
                                                    const std::size_t wordLen )
{
   DAQ_DATA_T average = buildAverage( pData, wordLen );

   enum EDGE_STATE
   {
      NON,
      RISING,
      FALLING
   };

   rFrequency = 0.0;
   EDGE_STATE edge = NON;
   unsigned int sampleCount = 0;
   unsigned int compleatedPeriods = 0;
   unsigned int samplesPerPeriod = 0;
   for( std::size_t i = 1; i < wordLen; i++ )
   {
      if( pData[i-1] < average && pData[i] >= average )
      {
         if( edge == NON )
         {
            sampleCount = 0;
            edge = RISING;
         }
         else if( edge == RISING )
         {
            samplesPerPeriod += sampleCount;
            sampleCount = 0;
            compleatedPeriods++;
         }
      }
      else if( pData[i-1] >= average && pData[i] < average )
      {
         if( edge == NON )
         {
            sampleCount = 0;
            edge = FALLING;
         }
         else if( edge == FALLING )
         {
            samplesPerPeriod += sampleCount;
            sampleCount = 0;
            compleatedPeriods++;
         }
      }
      sampleCount++;
   }

   if( compleatedPeriods == 0 )
      return false;

   double divisor = samplesPerPeriod * descriptorGetTimeBase() / compleatedPeriods;
   if( divisor == 0.0 )
      return false;

   rFrequency = NANOSECS_PER_SEC / divisor;
   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool Channel::onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen )
{
   if( descriptorWasContinuous() )
   {
      if( m_poModeContinuous != nullptr )
         m_poModeContinuous->write( pData, wordLen );
   }
   else
   {
      if( m_poModePmHires != nullptr )
         m_poModePmHires->write( pData, wordLen );
   }

   try
   {
      if( !m_oOutputFileName.empty() )
      {
         string currentName = m_oOutputFileName;
         string inserter = '_' + to_string( descriptorGetTimeStamp() );
         currentName.insert( currentName.find_last_of( '.' ), inserter );
         m_oPlot << "set output '" << currentName << '\'' << endl;
         if( static_cast<DaqContainer*>(getParent()->getParent())->
                                            getCommandLinePtr()->isVerbose() )
         {
            cout << "Generating output file: \"" << currentName << '"' << endl;
         }
      }

      if( isMultiplot() )
         m_oPlot << "set multiplot layout 2, 1 title \"Slot: " << getSlot() <<
                    " Channel: " << getNumber() << "\" font \",14\"" << endl;

      if( m_poModeContinuous != nullptr )
         m_poModeContinuous->plot();

      if( m_poModePmHires != nullptr )
         m_poModePmHires->plot();

      if( isMultiplot() )
         m_oPlot << "unset multiplot" << endl;
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( e.what() );
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
void Channel::showRunState( void )
{
   cout << "\tChannel " << getNumber() << ':' << endl;
   if( !m_oOutputFileName.empty() )
   {
      string outputFileName = m_oOutputFileName;
      outputFileName.insert( outputFileName.find_last_of( '.' ),
                                                          "_<wr-time-stamp>" );
      cout << "\t\tOutput: \"" << outputFileName << '"'  << endl;
   }
   if( m_oAttributes.m_continueMode.m_valid )
   {
      cout << "\t\tcontinuous: "
           << ::getSampleRateText(m_oAttributes.m_continueMode.m_value )
           << "; ";
      if( m_oAttributes.m_blockLimit.m_valid )
         cout << " limit: " << m_oAttributes.m_blockLimit.m_value
              << " blocks; ";
      if( m_oAttributes.m_triggerEnable.m_value )
      {
         cout << "Trigger: ";
         if( m_oAttributes.m_continueTriggerSouce.m_value )
            cout << " extern,";
         else
         {
            cout << " event: ";
            if( m_oAttributes.m_triggerCondition.m_valid )
               cout << m_oAttributes.m_triggerCondition.m_value;
            cout << ", ";
         }
         cout << " delay: ";
         if( m_oAttributes.m_triggerDelay.m_valid )
            cout << m_oAttributes.m_triggerDelay.m_value
                 << " samples;";
      }
      cout << endl;
   }
   if( m_oAttributes.m_postMortem.m_valid ||
       m_oAttributes.m_highResolution.m_valid )
   {
      if( m_oAttributes.m_postMortem.m_valid )
         cout << "\t\tpost mortem";
      else
      {
         cout << "\t\thigh resolution";
         if( m_oAttributes.m_triggerEnable.m_value )
         {
            cout << ", trigger: ";
            if( m_oAttributes.m_highResTriggerSource.m_value )
               cout << " extern,";
            else
            {
               cout << " event: ";
               if( m_oAttributes.m_triggerCondition.m_valid )
                  cout << m_oAttributes.m_triggerCondition.m_value;
            }
         }
      }
      if( m_oAttributes.m_restart.m_valid )
         cout << ", restart";
      cout << endl;
   }
}

/*! ---------------------------------------------------------------------------
 */
void Channel::doPostMortem( void )
{
   if( !m_oAttributes.m_postMortem.m_valid )
      return;

   if( static_cast<DaqContainer*>(getParent()->getParent())->
       getCommandLinePtr()->isVerbose() )
      cout << "\tSlot: " << getSlot() << ", Channel: " << getNumber() << endl;

   sendDisablePmHires( m_oAttributes.m_restart.m_value );
}

/*! ---------------------------------------------------------------------------
 */
void Channel::doHighRes( void )
{
   if( !m_oAttributes.m_highResolution.m_valid )
      return;

   if( static_cast<DaqContainer*>(getParent()->getParent())->
       getCommandLinePtr()->isVerbose() )
      cout << "\tSlot: " << getSlot() << ", Channel: " << getNumber() << endl;

   sendDisablePmHires( m_oAttributes.m_restart.m_value );
}

/*! ---------------------------------------------------------------------------
 */
void Channel::reset( void )
{
   if( m_poModeContinuous != nullptr )
      m_poModeContinuous->reset();

   if( m_poModePmHires != nullptr )
      m_poModePmHires->reset();
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
DaqContainer::DaqContainer( const std::string ebName,
                            CommandLine* poCommandLine )
      :DaqAdministration( ebName,
                          !poCommandLine->isNoReset(),
                          poCommandLine->isLM32CommandsEnabled() )
      ,m_poCommandLine( poCommandLine )
      {}
#else
DaqContainer::DaqContainer( DaqEb::EtherboneConnection* poEtherbone,
                            CommandLine* poCommandLine )
      :DaqAdministration( poEtherbone,
                          !poCommandLine->isNoReset(),
                          poCommandLine->isLM32CommandsEnabled() )
      ,m_poCommandLine( poCommandLine )
      {}
#endif

/*! ---------------------------------------------------------------------------
 */
DaqContainer::~DaqContainer( void )
{
   if( m_poCommandLine->isVerbose() )
      cout << "End " << getScuDomainName() << endl;

   if( isDoReset() )
      sendReset();
}

/*! ---------------------------------------------------------------------------
 */
bool DaqContainer::checkCommandLineParameter( void )
{
   if( empty() )
   {
      ERROR_MESSAGE( "No DAQ-device specified!" );
      return true;
   }
   for( const auto& iDev: *this )
   {
      if( iDev->empty() )
      {
         ERROR_MESSAGE( "No channel for DAQ-device in slot: " <<
                        iDev->getSlot() << " specified!" );
         return true;
      }
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
void DaqContainer::prioritizeAttributes( void )
{
   for( const auto& iDev: *this )
   {
      static_cast<Device*>(iDev)->m_oAttributes.set( m_oAttributes );
      for( const auto& iCha: *iDev )
      {
         static_cast<Channel*>(iCha)->
            m_oAttributes.set( static_cast<Device*>(iDev)->m_oAttributes );
      }
   }
}

/*! ---------------------------------------------------------------------------
 */
bool DaqContainer::checkForAttributeConflicts( void )
{
   bool ret = false;
   for( const auto& iDev: *this )
   {
      for( const auto& iCha: *iDev )
      {
         Channel* pCha = static_cast<Channel*>(iCha);
         if( pCha->m_oAttributes.m_postMortem.m_valid &&
             pCha->m_oAttributes.m_highResolution.m_valid )
         {
            ERROR_MESSAGE( "PostMorten-HighRes conflict on slot: "
                           << pCha->getSlot()  <<
                           " channel: " << pCha->getNumber() );
            ret = true;
         }
      }
   }
   return ret;
}

/*! ---------------------------------------------------------------------------
 */
bool DaqContainer::checkWhetherChannelsBecomesOperating( void )
{
   bool ret = true;
   for( const auto& iDev: *this )
   {
      for( const auto& iCha: *iDev )
      {
         Channel* pCha = static_cast<Channel*>(iCha);
         if( !pCha->m_oAttributes.m_postMortem.m_valid &&
             !pCha->m_oAttributes.m_highResolution.m_valid &&
             !pCha->m_oAttributes.m_continueMode.m_valid )
         {
            WARNING_MESSAGE( "Channel: " << pCha->getNumber() << " in slot: "
                             << pCha->getSlot() << " has nothing to do!" );
         }
         else
            ret = false;
      }
   }
   if( ret )
      ERROR_MESSAGE( "Nothing to do for all channels!" );
   return ret;
}

/*! ---------------------------------------------------------------------------
 */
void DaqContainer::sendAttributes( void )
{
   for( const auto& iDev: *this )
   {
      for( const auto& iCha: *iDev )
         static_cast<Channel*>(iCha)->sendAttributes();
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::start( void )
{
   for( const auto& iDev: *this )
   {
      for( const auto& iCha: *iDev )
         static_cast<Channel*>(iCha)->start();
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::showRunState( void )
{
   if( m_poCommandLine->isVerbose() )
      cout << "Status of SCU: " << getScuDomainName() << endl;

   cout << "Using Gnuplot binary: \"" << m_poCommandLine->getGnuplotBinary()
        << '"' << endl;
   cout << "Using terminal: \"" << m_poCommandLine->getTerminal() << '"' <<
       endl;

   for( const auto& iDev: *this )
   {
      cout << "Slot " << iDev->getSlot() << ':' << endl;
      for( const  auto& iCha: *iDev )
      {
         static_cast<Channel*>(iCha)->showRunState();
      }
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::doPostMortem( void )
{
   if( m_poCommandLine->isVerbose() )
      cout << "Post mortem of SCU: " << getScuDomainName() << ", ";

   try
   {
      for( const auto& iDev: *this )
      {
         for( const auto& iCha: *iDev )
         {
            static_cast<Channel*>(iCha)->doPostMortem();
         }
      }
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( __func__ << " " << e.what() );
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::doHighRes( void )
{
   if( m_poCommandLine->isVerbose() )
      cout << "High resolution of SCU: " << getScuDomainName() << endl;

   try
   {
      for( const auto& iDev: *this )
      {
         for( const auto& iCha: *iDev )
         {
            static_cast<Channel*>(iCha)->doHighRes();
         }
      }
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( __func__ << " " << e.what() );
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::doReset( void )
{
   if( m_poCommandLine->isVerbose() )
      cout << "Reset of SCU: " << getScuDomainName() << endl;

   try
   {
      sendReset();
      for( const auto& iDev: *this )
      {
         for( const auto& iCha: *iDev )
         {
            static_cast<Channel*>(iCha)->reset();
         }
      }
      sendAttributes();
      start();
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( __func__ << " " << e.what() );
   }

}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::onBlockReceiveError( void )
{
   ERROR_MESSAGE( "LM32 Blockreceiving: " << getLastStatusString() );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline int daqtMain( const int argc, char** ppArgv )
{
   DEBUG_MESSAGE( "Start" );
#ifdef DEBUGLEVEL
   for( int i = 0; i < argc; i++ )
      DEBUG_MESSAGE( "Arg " << i << ": " << ppArgv[i] );
#endif

   CommandLine cmdLine( argc, ppArgv );
   DaqContainer* pDaqContainer = cmdLine();
   if( pDaqContainer == nullptr )
   {
      DEBUG_MESSAGE( "Commandline parser returns nullptr!" );
      return EXIT_FAILURE;
   }

   int key;
   Terminal oTerminal;
   pDaqContainer->start();
   bool doRead = true;
   DEBUG_MESSAGE( "Enter main loop..." );
   while( (key = Terminal::readKey()) != '\e' )
   {
      switch( key )
      {
         case HOT_KEY_SHOW_STATE:
         {
            pDaqContainer->showRunState();
            break;
         }
         case HOT_KEY_POST_MORTEM:
         {
            pDaqContainer->doPostMortem();
            break;
         }
         case HOT_KEY_HIGH_RES:
         {
            pDaqContainer->doHighRes();
            break;
         }
         case HOT_KEY_RESET:
         {
            pDaqContainer->doReset();
            break;
         }
         case HOT_KEY_CLEAR_BUFFER:
         {
         #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
            pDaqContainer->clearBufferRequest();
         #else
            pDaqContainer->clearBuffer();
         #endif
            break;
         }
         case HOT_KEY_RECEIVE:
         {
            doRead = !doRead;
            if( cmdLine.isVerbose() )
               cout << "Receiving: " << (doRead? "enabled" : "disabled")
                    << endl;
            break;
         }
         case HOT_KEY_SHOW_RAM_LEVEL:
         {
         #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
            pDaqContainer->updateMemAdmin();
            const std::size_t level = pDaqContainer->getCurrentNumberOfData();
         #else
            const std::size_t level = pDaqContainer->getCurrentRamSize( !doRead );
         #endif
            cout << "RAM-level: " << level << " items -> "
                 << std::fixed << setprecision( 2 )
                 << static_cast<double>(level * 100.0 /
                                         RAM_SDAQ_MAX_CAPACITY)
                 << "%\"" << endl;
            break;
         }
      }

      if( !doRead )
         continue;

      try
      {
         pDaqContainer->distributeData();
         ::usleep( 100 );
      }
      catch( std::exception& e )
      {
         doRead = false;
         ERROR_MESSAGE( "Exception on function \"distributeData()\" occurred: "
                        "\"" << e.what() << '"' );
      }
   }
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUexpectedException );
   try
   {
      return daqtMain( argc, ppArgv );
   }
   catch( daq::EbException& e )
   {
      ERROR_MESSAGE( "daq::EbException occurred: " << e.what() );
   }
   catch( daq::DaqException& e )
   {
      ERROR_MESSAGE( "daq::DaqException occurred: " << e.what()
                     << "\nStatus: " <<  e.getStatusString() );
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

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
#include <unistd.h>
#include <termios.h>
#include "daqt.hpp"
#include "daqt_messages.hpp"
#include "daqt_command_line.hpp"
#include "daqt_scan.hpp"

using namespace daqt;

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
class Terminal
{
   struct termios m_originTerminal;

public:
   Terminal( void )
   {
      struct termios newTerminal;
      ::tcgetattr( STDIN_FILENO, &m_originTerminal );
      newTerminal = m_originTerminal;
      newTerminal.c_lflag     &= ~(ICANON | ECHO);  /* Disable canonic mode and echo.*/
      newTerminal.c_cc[VMIN]  = 1;  /* Reading is complete after one byte only. */
      newTerminal.c_cc[VTIME] = 0; /* No timer. */
      ::tcsetattr( STDIN_FILENO, TCSANOW, &newTerminal );
   }

   ~Terminal( void )
   {
      ::tcsetattr( STDIN_FILENO, TCSANOW, &m_originTerminal );
   }

   static int readKey( void )
   {
      int inKey = 0;
      fd_set rfds;

      struct timeval sleepTime = {0, 10};
      FD_ZERO( &rfds );
      FD_SET( STDIN_FILENO, &rfds );

      if( ::select( STDIN_FILENO+1, &rfds, NULL, NULL, &sleepTime ) > 0 )
         ::read( STDIN_FILENO, &inKey, sizeof( inKey ) );
      else
         inKey = 0;
      return (inKey & 0xFF);
   }
};


///////////////////////////////////////////////////////////////////////////////
/*-----------------------------------------------------------------------------
 */
void Attributes::set( const Attributes& rMyContainer )
{
   #define __SET_MEMBER( member )  member.set( rMyContainer.member )
   __SET_MEMBER( m_highResolution );
   __SET_MEMBER( m_postMortem );
   __SET_MEMBER( m_continueMode );
   __SET_MEMBER( m_continueTriggerSouce );
   __SET_MEMBER( m_highResTriggerSource );
   __SET_MEMBER( m_triggerEnable );
   __SET_MEMBER( m_triggerDelay );
   __SET_MEMBER( m_triggerCondition );
   __SET_MEMBER( m_blockLimit );
   __SET_MEMBER( m_restart );
   __SET_MEMBER( m_zoomGnuPlot );
   #undef __SET_MEMBER
}

///////////////////////////////////////////////////////////////////////////////
Channel::Mode::Mode( Channel* pParent, std::size_t size, std::string text )
   :m_pParent( pParent )
   ,m_size( size )
   ,m_text( text )
   ,m_notFirst( false )
{
   m_poPoint = new Point[m_size];
}

/*-----------------------------------------------------------------------------
 */
Channel::Mode::~Mode( void )
{
   delete m_poPoint;
}

/*-----------------------------------------------------------------------------
 */
void Channel::Mode::write( DAQ_DATA_T* pData, std::size_t wordLen )
{
   std::size_t len = std::min( wordLen, m_size );

   for( std::size_t i = 0; i < len; i++ )
   {
      m_poPoint[i].m_x = static_cast<double>(i); //TODO real time scale!
      m_poPoint[i].m_y = rawToVoltage( pData[i] );
   }
}

/*-----------------------------------------------------------------------------
 */
void Channel::Mode::plot( void )
{
   if( m_notFirst )
      m_pParent->m_oPlot << "replot" << endl;
   else
   {
      m_pParent->m_oPlot << "set xrange [0:" << m_size << "]" << endl;
      m_pParent->m_oPlot << "set title \"";
      if( !m_pParent->isMultiplot() )
      {
         m_pParent->m_oPlot << "Slot: " << m_pParent->getSlot()
                            << ", Channel: " << m_pParent->getNumber()
                            << ";   ";
      }
      m_pParent->m_oPlot << "Mode: " << m_text << "\"" << "font \",14\"" << endl;

      m_pParent->m_oPlot << "plot '-' title \"\" with lines" << endl;
      m_notFirst = true;
   }

   for( std::size_t i = 0; i < m_size; i++ )
      m_pParent->m_oPlot << m_poPoint[i].m_x << ' '
                         << m_poPoint[i].m_y << endl;

   m_pParent->m_oPlot << 'e' << endl;
}

/*-----------------------------------------------------------------------------
 */
Channel::Channel( unsigned int number )
   :DaqChannel( number )
   ,m_oPlot( "-noraise" )
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
   m_oPlot << "set terminal X11 title \"SCU: " << getScuDomainName()
           << "\"" << endl;

   m_oPlot << "set grid" << endl;
   m_oPlot << "set xlabel \"Time\"" << endl;
   m_oPlot << "set ylabel \"Voltage\"" << endl;

   if( !m_oAttributes.m_zoomGnuPlot.m_value )
       m_oPlot << "set yrange ["
               << -(DAQ_VSS_MAX/2) << ':' << (DAQ_VSS_MAX/2) << ']' << endl;

   if( m_oAttributes.m_continueMode.m_valid )
   {
      string sRate = getSampleRateText(m_oAttributes.m_continueMode.m_value  );
      m_poModeContinuous =
         new Mode( this,
                   DaqInterface::c_contineousPayloadLen,
                   "continuous sample rate: " + sRate );
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
bool Channel::onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen )
{
   if( isMultiplot() )
      m_oPlot << "set multiplot layout 2, 1 title \"Slot: " << getSlot() <<
                 " Channel: " << getNumber() << "\" font \",14\"" << endl;

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

   if( m_poModeContinuous != nullptr )
      m_poModeContinuous->plot();

   if( m_poModePmHires != nullptr )
      m_poModePmHires->plot();

   if( isMultiplot() )
      m_oPlot << "unset multiplot" << endl;

   return false;
}

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
bool DaqContainer::checkCommandLineParameter( void )
{
   if( empty() )
   {
      ERROR_MESSAGE( "No DAQ-device specified!" );
      return true;
   }
   for( auto& iDev: *this )
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
   for( auto& iDev: *this )
   {
      static_cast<Device*>(iDev)->m_oAttributes.set( m_oAttributes );
      for( auto& iCha: *iDev )
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
   for( auto& iDev: *this )
   {
      for( auto& iCha: *iDev )
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
   for( auto& iDev: *this )
   {
      for( auto& iCha: *iDev )
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
   for( auto& iDev: *this )
   {
      for( auto& iCha: *iDev )
         static_cast<Channel*>(iCha)->sendAttributes();
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::start( void )
{
   for( auto& iDev: *this )
   {
      for( auto& iCha: *iDev )
         static_cast<Channel*>(iCha)->start();
   }
}

/*-----------------------------------------------------------------------------
 */
void DaqContainer::showRunState( void )
{
   for( auto& iDev: *this )
   {
      cout << "Slot " << iDev->getSlot() << ':' << endl;
      for( auto& iCha: *iDev )
      {
         cout << "\tChannel " << iCha->getNumber() << ':' << endl;
         Channel* pChannel = static_cast<Channel*>(iCha);
         if( pChannel->m_oAttributes.m_continueMode.m_valid )
         {
            cout << "\t\tcontinuous: "
                 << getSampleRateText(
                      pChannel->m_oAttributes.m_continueMode.m_value )
                 << endl;
         }
         if( pChannel->m_oAttributes.m_postMortem.m_valid )
         {
            cout << "\t\tpost mortem" << endl;
         }
         if( pChannel->m_oAttributes.m_highResolution.m_valid )
         {
            cout << "\t\thigh resolution" << endl;
         }
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
inline int daqtMain( int argc, char** ppArgv )
{
   CommandLine cmdLine( argc, ppArgv );
   DaqContainer* pDaqContainer = cmdLine();
   if( pDaqContainer == nullptr )
      return EXIT_FAILURE;

   int key;
   Terminal oTerminal;
   pDaqContainer->start();
   while( (key = Terminal::readKey()) != '\e' )
   {
      pDaqContainer->distributeData();
      switch( key )
      {
         case 's':
         {
            pDaqContainer->showRunState();
            break;
         }
      }
   }
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
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

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
/*-----------------------------------------------------------------------------
 */
Channel::Channel( unsigned int number )
   :DaqChannel( number )
   ,m_poGnuplot( nullptr )
{
   m_poGnuplot = ::gnuplot_init();
}

/*-----------------------------------------------------------------------------
 */
Channel::~Channel( void )
{
   if( m_poGnuplot != nullptr )
      ::gnuplot_close( m_poGnuplot );
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
   if( m_poGnuplot != nullptr )
   {
      ::gnuplot_cmd( m_poGnuplot, "set grid" );
      ::gnuplot_setstyle( m_poGnuplot, "lines" );
      if( !m_oAttributes.m_zoomGnuPlot.m_value )
         ::gnuplot_cmd( m_poGnuplot, "set yrange [-10.0:10.0]" );
   }

   if( m_oAttributes.m_continueMode.m_valid )
      sendEnableContineous( m_oAttributes.m_continueMode.m_value,
                            m_oAttributes.m_blockLimit.m_value );

   if( m_oAttributes.m_highResolution.m_valid )
      sendEnableHighResolution( m_oAttributes.m_restart.m_value );
   else if( m_oAttributes.m_postMortem.m_valid )
      sendEnablePostMortem( m_oAttributes.m_restart.m_value );
}

/*! ---------------------------------------------------------------------------
 */
bool Channel::onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen )
{
   for( int i = 0; i < wordLen; i++ )
      cout << i << ": " << pData[i] << "\n";
   cout << flush;

   if( m_poGnuplot == nullptr )
      return true;

   double* px = new double[wordLen];
   double* py = new double[wordLen];

   for( std::size_t i = 0; i < wordLen; i++ )
   {
      px[i] = static_cast<double>(i);
      py[i] = rawToVoltage( pData[i] );
   }

   ::gnuplot_cmd( m_poGnuplot, "set xrange [0:%d]", wordLen );

   ::gnuplot_resetplot( m_poGnuplot );

   string legende = "SCU: ";
   legende += getScuDomainName();
   legende += "; Slot: ";
   legende += to_string(getSlot());
   legende += "; Channel: ";
   legende += to_string(getNumber());
   ::gnuplot_plot_xy(m_poGnuplot, px, py, wordLen, legende.c_str() );
   ::gnuplot_set_xlabel( m_poGnuplot, "Time" );
   ::gnuplot_set_ylabel( m_poGnuplot, "Voltage" );

   delete [] px;
   delete [] py;
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

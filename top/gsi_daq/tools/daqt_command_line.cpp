/*!
 *  @file daqt_command_line.cpp
 *  @brief Command line parser of DAQ-Test
 *
 *  @date 16.04.2019
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
#include "daqt_command_line.hpp"

using namespace daqt;
using namespace std;

/*! ---------------------------------------------------------------------------
*/
vector<OPTION> CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         assert( !poParser->isOptArgPersent() );
         cout << "Usage: " << poParser->getProgramName()
              << " <proto/host/port> [global-options] "
                 "[<slot-number> <channel-number> [channel-options]] \n\n"
                 "NOTE: The lowest slot-number begins at 1; "
                 "the lowest channel-number begins at 1.\n";
         poParser->list( cout );
         cout << endl;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'h',
      .m_longOpt  = "help",
      .m_helpText = "Print this help and exit"
   },
   {
      OPT_LAMBDA( poParser,
      {
         DaqAdministration* poAllDaq =
            static_cast<CommandLine*>(poParser)->m_poAllDaq;
         if( poAllDaq == nullptr )
         {
            ERROR_MESSAGE( "<proto/host/port> must be specified before!" );
            return -1;
         }
         for( unsigned int i = 1; i <= poAllDaq->getMaxFoundDevices(); i++ )
         {
            cout << poAllDaq->getSlotNumber( i ) << " "
                 <<  poAllDaq->readMaxChannels( i ) << endl;
         }
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 's',
      .m_longOpt  = "scan",
      .m_helpText = "Scan the entire SCU-bus for DAQ- devices and its"
                    " channels and print the results as table."
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 0;
      }),
      .m_hasArg   = OPTION::OPTIONAL_ARG,
      .m_id       = 0,
      .m_shortOpt = 'C',
      .m_longOpt  = "continue",
      .m_helpText = "Starts the continuous mode"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'H',
      .m_longOpt  = "high-resolution",
      .m_helpText = "Starts the high resolution mode"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'P',
      .m_longOpt  = "post-mortem",
      .m_helpText = "Starts the post-mortem mode"
   },
   {
      OPT_LAMBDA( poParser,
      {
         unsigned int limit;
         if( readInteger( limit, poParser->getOptArg() ) )
            return -1;
         if( limit > static_cast<DAQ_REGISTER_T>(~0) )
         {
            ERROR_MESSAGE( "Requested block limit: " << limit <<
                           " is out of range!" );
            return -1;
         }
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            ERROR_MESSAGE( "<proto/host/port> must be specified before!" );
            return -1;
         }
         poAttr->m_blockLimit.set( limit );
         return 1;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'l',
      .m_longOpt  = "limit",
      .m_helpText = "Limits the maximum of data blocks in the continuous"
                    "mode.\n"
                    "The value zero (default) establishes the endless mode."
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "delay",
      .m_helpText = "PARAM sets the trigger delay in samples"
                    " for the continuous mode"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'c',
      .m_longOpt  = "condition",
      .m_helpText = "PARAM sets the timing value of the trigger condition"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 't',
      .m_longOpt  = "trigger",
      .m_helpText = "Enables the trigger mode for continuous- and"
                     " high-resolution mode"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'E',
      .m_longOpt  = "continuous-extern",
      .m_helpText = "Sets the trigger source for the continuous mode "
                    "from the event-trigger (default)\n"
                    "into the external trigger input"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'e',
      .m_longOpt  = "highres-extern",
      .m_helpText = "Sets the trigger source for the high-resolution mode "
                    "from the event-trigger (default)\n"
                    "into the external trigger input"
   },
   {
      OPT_LAMBDA( poParser,
      {
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'r',
      .m_longOpt  = "restart",
      .m_helpText = "Restarts the high-resolution or post-mortem mode after"
                    " an event"
   }
};

/*! ---------------------------------------------------------------------------
*/
bool CommandLine::readInteger( unsigned int& rValue, const string& roStr )
{
   try
   {
      rValue = stoi( roStr );
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "Integer number is expected and not that: \""
                     << roStr << "\" !" );
      return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
*/
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,FSM_INIT_FSM( READ_EB_NAME )
   ,m_poAllDaq( nullptr )
   ,m_poCurrentDevice( nullptr )
   ,m_poCurrentChannel( nullptr )
{
   add( c_optList );
}

/*! ---------------------------------------------------------------------------
*/
CommandLine::~CommandLine( void )
{
   if( m_poAllDaq != nullptr )
       delete m_poAllDaq;
}

/*! ---------------------------------------------------------------------------
*/
int CommandLine::operator()( void )
{
   int ret = PARSER::operator()();
   if( ret < 0 )
      return ret;
   if( m_poAllDaq == nullptr )
      return ret;
   m_poAllDaq->prioritizeAttributes();
   return ret;
}

/*! ---------------------------------------------------------------------------
*/
Attributes* CommandLine::getAttributesToSet( void )
{
   if( m_poAllDaq == nullptr )
      return nullptr;
   if( m_poCurrentChannel != nullptr )
      return &m_poCurrentChannel->m_oAttributes;
   if( m_poCurrentDevice != nullptr )
      return &m_poCurrentDevice->m_oAttributes;
   return &m_poAllDaq->m_oAttributes;
}

/*! ---------------------------------------------------------------------------
*/
int CommandLine::onArgument( void )
{
   const string arg = getArgVect()[getArgIndex()];
   unsigned int number;
   switch( m_state )
   {
      case READ_SLOT:
      case READ_CHANNEL:
      {
         SCU_ASSERT( m_poAllDaq != nullptr );
         if( readInteger( number, arg ) )
            return -1;
         break;
      }
   }
   switch( m_state )
   {
      case READ_EB_NAME:
      {
         SCU_ASSERT( m_poAllDaq == nullptr );
         m_poAllDaq = new DaqContainer( arg, this );
         FSM_TRANSITION( READ_SLOT );
         break;
      }
      case READ_SLOT:
      {
         m_poCurrentChannel = nullptr;
         if( !gsi::isInRange( number, DaqInterface::c_startSlot,
                                      DaqInterface::c_maxSlots ) )
         {
            ERROR_MESSAGE( "Given slot " << number <<
                           " is out of the range of: " <<
                           DaqInterface::c_startSlot << " and " <<
                           DaqInterface::c_maxSlots << " !" );
            return -1;
         }
         if( !m_poAllDaq->isDevicePresent( number ) )
         {
            ERROR_MESSAGE( "In slot " << number << " isn't a DAQ!" );
            return -1;
         }
         m_poCurrentDevice = m_poAllDaq->getDeviceBySlot( number );
         if( m_poCurrentDevice == nullptr )
         {
            m_poCurrentDevice = new Device( number );
            m_poAllDaq->registerDevice( m_poCurrentDevice );
         }
         FSM_TRANSITION( READ_CHANNEL );
         break;
      }
      case READ_CHANNEL:
      {
         SCU_ASSERT( m_poCurrentDevice != nullptr );
         SCU_ASSERT( m_poCurrentChannel == nullptr );
         if( !gsi::isInRange( number, static_cast<unsigned int>(1),
                                      m_poCurrentDevice->getMaxChannels() ) )
         {
            ERROR_MESSAGE( "Requested channel " << number <<
                           " of DAQ in slot " <<
                           m_poCurrentDevice->getSlot() <<
                           " isn't present!" );
            return -1;
         }
         m_poCurrentChannel = m_poCurrentDevice->getChannel( number );
         if( m_poCurrentChannel == nullptr )
         {
            m_poCurrentChannel = new Channel( number );
            m_poCurrentDevice->registerChannel( m_poCurrentChannel );
         }
         FSM_TRANSITION( READ_SLOT );
         break;
      }
   }

   return 1;
}

//================================== EOF ======================================

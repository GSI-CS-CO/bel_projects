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

#define CONTINUE_1MS   "1MS"
#define CONTINUE_100US "100US"
#define CONTINUE_10US  "10US"


/*! ---------------------------------------------------------------------------
*/
void CommandLine::specifiedBeforeErrorMessage( void )
{
   ERROR_MESSAGE( "<proto/host/port> must be specified before!" );
}

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
         ::exit( EXIT_SUCCESS );
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
            specifiedBeforeErrorMessage();
            return -1;
         }
         for( unsigned int i = 1; i <= poAllDaq->getMaxFoundDevices(); i++ )
         {
            cout << poAllDaq->getSlotNumber( i ) << " "
                 <<  poAllDaq->readMaxChannels( i ) << endl;
         }
         ::exit( EXIT_SUCCESS );
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
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         if( poParser->getOptArg().empty() )
         {
            poAttr->m_continueMode.set( DAQ_SAMPLE_1MS );
            return 0;
         }
         if( poParser->getOptArg() == CONTINUE_1MS )
         {
            poAttr->m_continueMode.set( DAQ_SAMPLE_1MS );
            return 0;
         }
         if( poParser->getOptArg() == CONTINUE_100US )
         {
            poAttr->m_continueMode.set( DAQ_SAMPLE_100US );
            return 0;
         }
         if( poParser->getOptArg() == CONTINUE_10US )
         {
            poAttr->m_continueMode.set( DAQ_SAMPLE_10US );
            return 0;
         }
         ERROR_MESSAGE( "Wrong sample parameter: \"" <<
                        poParser->getOptArg() << "\"\n"
                        "Known values: "
                        CONTINUE_1MS ", " CONTINUE_100US ", or "
                        CONTINUE_10US );
         return -1;
      }),
      .m_hasArg   = OPTION::OPTIONAL_ARG,
      .m_id       = 0,
      .m_shortOpt = 'C',
      .m_longOpt  = "continue",
      .m_helpText = "Starts the continuous mode \n"
                    "PARAM:\n"
                    "   " CONTINUE_1MS   " Sample rate   1 ms\n"
                    "   " CONTINUE_100US " Sample rate 100 us\n"
                    "   " CONTINUE_10US  " Sample rate  10 us\n"
                    "Default value is 1ms\n"
                    "Example:\n"
                    "   C=" CONTINUE_100US "\n"
                    "   means: Continuous mode with sample rate of 100 us"

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
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_blockLimit.set( limit );
         return 0;
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
         unsigned int delay;
         if( readInteger( delay, poParser->getOptArg() ) )
            return -1;
         if( delay > static_cast<DAQ_REGISTER_T>(~0) )
         {
            ERROR_MESSAGE( "Requested trigger delay: " << delay <<
                           " is out of range!" );
            return -1;
         }
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_triggerDelay.set( delay );
         return 0;
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
         unsigned int condition;
         if( readInteger( condition, poParser->getOptArg() ) )
            return -1;
         if( condition > static_cast<uint32_t>(~0) )
         {
            ERROR_MESSAGE( "Requested trigger condition: " << condition <<
                           " is out of range!" );
            return -1;
         }
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_triggerCondition.set( condition );
         return 0;
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
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_triggerEnable.set( true );
         return 0;
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
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_continueTreggerSouce.set( true );
         return 0;
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
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_highResTriggerSource.set( true );
         return 0;
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
         Attributes* poAttr =
            static_cast<CommandLine*>(poParser)->getAttributesToSet();
         if( poAttr == nullptr )
         {
            specifiedBeforeErrorMessage();
            return -1;
         }
         poAttr->m_restart.set( true );
         return 0;
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
//#define CONFIG_DBG_ATTRIBUTES
int CommandLine::operator()( void )
{
   int ret = PARSER::operator()();
   if( ret < 0 )
      return ret;
   if( m_poAllDaq == nullptr )
      return ret;
   m_poAllDaq->prioritizeAttributes();

#ifdef CONFIG_DBG_ATTRIBUTES
   if( m_poAllDaq->m_oAttributes.m_blockLimit.m_valid )
      cerr << "Attribute: " << m_poAllDaq->m_oAttributes.m_blockLimit.m_value << endl;
   for( auto& iDev: *m_poAllDaq )
   {
      Device* pDev = static_cast<Device*>(iDev);
      cerr << "   Slot: " << pDev->getSlot();
      if( pDev->m_oAttributes.m_blockLimit.m_valid )
         cerr << ", Attribute: " << pDev->m_oAttributes.m_blockLimit.m_value;
      cerr << endl;
      for( auto& iCha: *iDev )
      {
         Channel* pCha = static_cast<Channel*>(iCha);
         cerr << "     Channel: " << pCha->getNumber();
         if( pCha->m_oAttributes.m_blockLimit.m_valid )
            cerr << ", Attribute: " << pCha->m_oAttributes.m_blockLimit.m_value;
         cerr << endl;
      }
   }
#endif
   m_poAllDaq->sendAttributes();
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

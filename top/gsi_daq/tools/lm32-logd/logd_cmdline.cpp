/*!
 *  @file logd_cmdline.cpp
 *  @brief Commandline parser for the LM32 log daemon.
 *
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <scu_env.hpp>
#include <daqt_messages.hpp>
#include "logd_cmdline.hpp"

using namespace std;
using namespace CLOP;
using namespace Scu;

#define DEFAULT_INTERVAL 1000

/*! ---------------------------------------------------------------------------
 * @brief Initializing the command line options.
 */
CommandLine::OPT_LIST_T CommandLine::c_optList =
{
#ifdef CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         string name = poParser->getProgramName().substr(poParser->getProgramName().find_last_of('/')+1);
         cout << "<toolinfo>\n"
                 "\t<name>" << name << "</name>\n"
                 "\t<topic>Development, Release, Rollout</topic>\n"
                 "\t<description>Daemon for forwarding log-messages of a LM32-application</description>\n"
                 "\t<usage>" << name << " {SCU-url}";
         for( const auto& pOption: *poParser )
         {
            if( pOption->m_id != 0 )
               continue;
            cout << " [";
            if( pOption->m_shortOpt != '\0' )
            {
               cout << '-' << pOption->m_shortOpt;
               if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                  cout << " ARG";
               if( !pOption->m_longOpt.empty() )
                  cout << ", ";
            }
            if( !pOption->m_longOpt.empty() )
            {
               cout << "--" << pOption->m_longOpt;
               if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                  cout << " ARG";
            }
            cout << ']';
         }
         cout << "\n\t</usage>\n"
                 "\t<author>Ulrich Becker</author>\n"
                 "\t<autodocversion>1.0</autodocversion>\n"
                 "</toolinfo>"
              << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 1, // will hide this option for autodoc
      .m_shortOpt = '\0',
      .m_longOpt  = "generate_doc_tagged",
      .m_helpText = "Will need from autodoc."
   },
#endif // CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         cout << "Daemon for forwarding log-messages of a LM32-application.\n"
                 "(c) 2022 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n\n"
                 "Usage on ASL:\n\t"
              << poParser->getProgramName() << " [options] <SCU URL>\n"
                 "Usage on SCU:\n\t"
              << poParser->getProgramName() << " [options]\n\n"
                 "The key 'Esc' terminates this program\n\n"
                 "Options:"
              << endl;
         poParser->list( cout );
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
         static_cast<CommandLine*>(poParser)->m_verbose = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'v',
      .m_longOpt  = "verbose",
      .m_helpText = "Be verbose."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_daemonize = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "daemonize",
      .m_helpText = "Process will run as daemon."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_noTimestamp = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'n',
      .m_longOpt  = "notime",
      .m_helpText = "Suppresses the output of the timestamp."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_humanTimestamp = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'H',
      .m_longOpt  = "human",
      .m_helpText = "Human readable timestamp."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_isForConsole = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'c',
      .m_longOpt  = "console",
      .m_helpText = "Console mode: line feed \"\\n\" becomes printed.\n"
                    "Otherwise it becomes replaced by space character and \"\\r\" will ignored.\n\n"
                    "NOTE:\n"
                    "It is recommended to use this option in combination with option -n --notime."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint interval;
         if( readInteger( interval, poParser->getOptArg() ) )
            return -1;
         static_cast<CommandLine*>(poParser)->m_interval = interval;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'I',
      .m_longOpt  = "interval",
      .m_helpText = "PARAM=\"<new poll interval in milliseconds>\"\n"
                    "Overwrites the default interval of " TO_STRING( DEFAULT_INTERVAL )
                    " milliseconds."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint filter;
         if( readInteger( filter, poParser->getOptArg() ) )
            return -1;
         if( filter >= BIT_SIZEOF( FILTER_FLAG_T ) )
         {
            ERROR_MESSAGE( "Filter value " << filter << " out of range from 0 to "
                           << BIT_SIZEOF( FILTER_FLAG_T ) - 1 << "!" );
            return -1;
         }
         static_cast<CommandLine*>(poParser)->m_filterFlags |= 1 << filter;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'f',
      .m_longOpt  = "filter",
      .m_helpText = "PARAM=\"<filter value>\"\n"
                    "Setting a filter.\n"
                    "It is possible to specify this option multiple times"
                    " with different values,\n"
                    "from which an OR link is created.\n\n"
                    "E.g. code in LM32:\n"
                    "   lm32Log( 1, \"Log-text A\" );\n"
                    "   lm32Log( 2, \"Log-text B\" );\n"
                    "   lm32Log( 3, \"Log-text C\" );\n\n"
                    "Commandline: -f1 -f3\n"
                    "In this example only \"Log-text A\" and \"Log-text B\""
                    " becomes forwarded.\n\n"
                    "NOTE:\nWhen this option is omitted,\n"
                    "then all log-messages becomes forwarded."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_printFilter = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'p',
      .m_longOpt  = "print-filter",
      .m_helpText = "Prints the filter value at the begin of each item.\n"
                    "That is the first parameter of the LM32 function: "
                    "\"lm32Log\""
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_exit = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'e',
      .m_longOpt  = "exit",
      .m_helpText = "Exit after read, otherwise the program will run in a "
                    "polling loop until the Esc-key has pressed."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_kill = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'k',
      .m_longOpt  = "kill",
      .m_helpText = "Terminates a concurrent running process of this program."
   }
}; // CommandLine::c_optList// CommandLine::c_optList

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
*/
bool CommandLine::readInteger( uint& rValue, const string& roStr )
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
   ,m_verbose( false )
   ,m_daemonize( false )
   ,m_isOnScu( isRunningOnScu() )
   ,m_noTimestamp( false )
   ,m_humanTimestamp( false )
   ,m_isForConsole( false )
   ,m_printFilter( false )
   ,m_exit( false )
   ,m_kill( false )
   ,m_interval( DEFAULT_INTERVAL )
   ,m_filterFlags( 0 )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_isOnScu )
      m_scuUrl = "dev/wbm0";
   add( c_optList );
   sortShort();
}

/*! ---------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
   DEBUG_MESSAGE_M_FUNCTION("");

   if( m_isOnScu )
   {
      WARNING_MESSAGE( "Program is running on SCU, therefore the argument \""
                       << getArgVect()[getArgIndex()]
                       << "\" becomes replaced by \""
                       << m_scuUrl << "\"!" );
      return 1;
   }

   if( !m_scuUrl.empty() )
   {
      ERROR_MESSAGE( "Only one argument is allowed!" );
      ::exit( EXIT_FAILURE );
   }

   m_scuUrl = getArgVect()[getArgIndex()];
   if( m_scuUrl.find( "tcp/" ) == string::npos )
         m_scuUrl = "tcp/" + m_scuUrl;

   return 1;
}

/*! ---------------------------------------------------------------------------
 */
std::string& CommandLine::operator()( void )
{
   DEBUG_MESSAGE( "Parsing of commandline" );
   if( PARSER::operator()() < 0 )
      ::exit( EXIT_FAILURE );

   if( !m_isOnScu && m_scuUrl.empty() )
   {
      ERROR_MESSAGE( "Missing SCU URL" );
      ::exit( EXIT_FAILURE );
   }

   if( m_humanTimestamp && m_noTimestamp )
      WARNING_MESSAGE( "Timestamp will not printed, therefore"
                       " the option for human readable timestamp"
                       " has no effect!" );

   return m_scuUrl;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedShortOption( char unrecognized )
{
   ERROR_MESSAGE( "Unknown short option: \"-" << unrecognized << "\"" );
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedLongOption( const std::string& unrecognized )
{
   ERROR_MESSAGE( "Unknown long option: \"--" << unrecognized << "\"" );
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorShortMissingRequiredArg( void )
{
   ERROR_MESSAGE( "Missing argument of option: -" << getCurrentOption()->m_shortOpt );
   return -1;
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onErrorLongMissingRequiredArg( void )
{
   ERROR_MESSAGE( "Missing argument of option: --" << getCurrentOption()->m_longOpt );
   return -1;
}

//================================== EOF ======================================

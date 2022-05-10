/*!
 *  @file mem_cmdline.cpp
 *  @brief Command-line interpreter of memory monitor.
 *
 *  @date 12.04.2022
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
#include "mem_cmdline.hpp"

//namespace Scu
//{
//namespace mmu
//{

using namespace std;
using namespace CLOP;
using namespace Scu::mmu;

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
         cout <<
            "<toolinfo>\n"
            "\t<name>" << name << "</name>\n"
            "\t<topic>Development, Release, Rollout</topic>\n"
            "\t<description>Shows the the memory partitions of the given SCU.</description>\n"
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
         cout << "Shows the partitions and memory usage of DDR3 RAM.\n"
                 "(c) 2022 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n\n"
                 "Usage on ASL:\n\t"
              << poParser->getProgramName() << " [options] <SCU URL>\n"
                 "Usage on SCU:\n\t"
              << poParser->getProgramName() << " [options]\n"
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
      .m_helpText = "Be verbose. That means, all identifiers are displayed."
   },
   {
      OPT_LAMBDA( poParser,
      {
         if( static_cast<CommandLine*>(poParser)->m_verbose )
         {
            cout << "Version: " TO_STRING( VERSION )
                    ", Git revision: " TO_STRING( GIT_REVISION ) << endl;
         }
         else
         {
            cout << TO_STRING( VERSION ) << endl;
         }
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'V',
      .m_longOpt  = "version",
      .m_helpText = "Print the software version and exit."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_tagInDecimal = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "decimal",
      .m_helpText = "Tag will print as decimal number, default is hexadecimal."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_isInBytes = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'b',
      .m_longOpt  = "bytes",
      .m_helpText = "Displays all in bytes, otherwise all will displayed\n"
                    "in the smallest addressable unit in 8 byte clusters\n"
                    "(64 bit) in the case of DDR3-RAM."
   }
}; // CommandLine::c_optList


/*! ---------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,m_verbose( false )
   ,m_tagInDecimal( false )
   ,m_isInBytes( false )
{
   m_isOnScu = Scu::isRunningOnScu();
   if( m_isOnScu )
      m_scuUrl = "dev/wbm0";
   add( c_optList );
   sortShort();
}

/*! ---------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
}

/*! ---------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
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
std::string& CommandLine::operator()( void )
{
   if( PARSER::operator()() < 0 )
      ::exit( EXIT_FAILURE );

   if( !m_isOnScu && m_scuUrl.empty() )
   {
      ERROR_MESSAGE( "Missing SCU URL" );
      ::exit( EXIT_FAILURE );
   }
   return m_scuUrl;
}

//} // namespace mmu
//} // namespace Scu

//================================== EOF ======================================

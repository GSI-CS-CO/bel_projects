/*!
 *  @file mdaqt_command_line.cpp
 *  @brief Command line parser of MIL-DAQ-Test
 *
 *  @date 26.08.2019
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
#ifndef __DOCFSM__
 #include <find_process.h>
 #include <unistd.h>
 #include <netdb.h>
#endif

#include "mdaqt_command_line.hpp"

using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;


#define FSM_INIT_FSM( state, attr... )      m_state( state )
#define FSM_TRANSITION( newState, attr... ) m_state = newState

vector<OPTION> CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         cout << "Usage: " << poParser->getProgramName()
              << " <proto/host/port> [global-options] "
                 "[<slot-number> [device-options] "
                 "<channel-number> [channel-options]] \n\n"
                 "Global-options can be overwritten by device-options "
                 "and device options can be overwritten by channel-options.\n"
                 "NOTE: The lowest slot-number begins at 1; "
                 "the lowest channel-number begins at 1.\n\n"
                 "Hot keys:\n";
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
         static_cast<CommandLine*>(poParser)->m_verbose = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'v',
      .m_longOpt  = "verbose",
      .m_helpText = "Be verbose"
   },
   {
      OPT_LAMBDA( poParser,
      {
         cout << TO_STRING( VERSION ) << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'V',
      .m_longOpt  = "version",
      .m_helpText = "Print the software version and exit"
   },
   {
      OPT_LAMBDA( poParser,
      {
    //     if( getDaqAdmin( poParser ) == nullptr )
    //        return -1;

         static_cast<CommandLine*>(poParser)->m_gnuplotBin =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'G',
      .m_longOpt  = "gnuplot",
      .m_helpText = "Replacing of the default Gnuplot binary by the in PARAM"
                    " given binary. Default is: " GPSTR_DEFAULT_GNUPLOT_EXE
   },
   {
      OPT_LAMBDA( poParser,
      {
   //      if( getDaqAdmin( poParser ) == nullptr )
   //         return -1;

         static_cast<CommandLine*>(poParser)->m_gnuplotTerminal =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'T',
      .m_longOpt  = "terminal",
      .m_helpText = "PARAM replaces the terminal which is used by Gnuplot."
                    " Default is: \"" GNUPLOT_DEFAULT_TERMINAL "\""
   },
   {
      OPT_LAMBDA( poParser,
      {
     //    if( getDaqAdmin( poParser ) == nullptr )
     //       return -1;

         static_cast<CommandLine*>(poParser)->m_gnuplotOutput =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'o',
      .m_longOpt  = "output",
      .m_helpText = "Setting the prefix and suffix file name for Gnuplot."
                    " PARAM is the path and name of the output file.\n"
                    "NOTE: The final file name becomes generated as follows:\n"
                    "      <SUFFIX>_<SCU-name>_<slot number>_<channel number>_"
                    "<wr-time stamp>.<PREFIX>\n"
                    "Example: PARAM = myFile.png:\n"
                    "         result: myFile_scuxl0035_acc_gsi_de_3_1_"
                    "12439792657334272.png"
   }

};


///////////////////////////////////////////////////////////////////////////////
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

/*-----------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,FSM_INIT_FSM( READ_EB_NAME )
   ,m_verbose( false )
   ,m_poAllDaq( nullptr )
   ,m_poCurrentDevice( nullptr )
   ,m_poCurrentChannel( nullptr )
   ,m_gnuplotBin( GPSTR_DEFAULT_GNUPLOT_EXE )
   ,m_gnuplotTerminal( GNUPLOT_DEFAULT_TERMINAL )
{
   add( c_optList );
}

/*-----------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
   if( m_poAllDaq != nullptr )
       delete m_poAllDaq;
}

/*-----------------------------------------------------------------------------
 */
MilDaqAdministration* CommandLine::operator()( void )
{
   if( getArgCount() < 2 )
   {
      ERROR_MESSAGE( "Missing argument!" );
      return nullptr;
   }

   if( PARSER::operator()() < 0 )
      return nullptr;
   if( m_poAllDaq == nullptr )
      return nullptr;

   return m_poAllDaq;
}

/*! ----------------------------------------------------------------------------
 * @brief Callback function of findProcesses in CommandLine::onArgument
 */
extern "C" {
static int onFoundProcess( OFP_ARG_T* pArg )
{  /*
    * Checking whether the found process is himself.
    */
   if( pArg->pid == ::getpid() )
      return 0; // Process has found himself. Program continue.

   string* pEbTarget = static_cast<string*>(pArg->pUser);
   string ebSelfAddr = pEbTarget->substr( pEbTarget->find( '/' ) + 1 );

   const char* currentArg = reinterpret_cast<char*>(pArg->commandLine.buffer);

   /*
    * Skipping over the program name from the concurrent process command line.
    */
   currentArg += ::strlen( currentArg ) + 1;

   for( ::size_t i = 1; i < pArg->commandLine.argc; i++ )
   {
      if( *currentArg != '-' )
         break;
      /*
       * Skipping over possibly leading options from the concurrent
       * process command line.
       */
      currentArg += ::strlen( currentArg ) + 1;
   }

   string ebConcurrentAddr = currentArg;
   ebConcurrentAddr = ebConcurrentAddr.substr( ebConcurrentAddr.find( '/' ) + 1 );

   struct hostent* pHostConcurrent = ::gethostbyname( ebConcurrentAddr.c_str() );
   struct hostent* pHostSelf       = ::gethostbyname( ebSelfAddr.c_str() );
   if( pHostConcurrent == nullptr || pHostSelf == nullptr )
   {
      if( ebConcurrentAddr != ebSelfAddr )
         return 0; // Program continue.
   }
   if( pHostConcurrent != nullptr && pHostSelf != nullptr )
   {
      if( ::strcmp( pHostConcurrent->h_name, pHostSelf->h_name ) != 0 )
         return 0; // Program continue.
   }

   if( (pHostConcurrent != nullptr) != (pHostSelf != nullptr) )
      return 0; // Program continue.

   ERROR_MESSAGE( "A concurrent process accessing \"" << *pEbTarget <<
                  "\" is already running with the PID: " << pArg->pid );

   return -1; // Program termination.
}
} // extern "C"

/*-----------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
   string arg = getArgVect()[getArgIndex()];
   uint number;
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
#if 1
         if( ::findProcesses( getProgramName().c_str(), ::onFoundProcess, &arg,
            static_cast<FPROC_MODE_T>(::FPROC_BASENAME | ::FPROC_RLINK) ) < 0 )
            return -1;
#endif
         m_poAllDaq = new MilDaqAdministration( this, arg );
         FSM_TRANSITION( READ_SLOT );
         break;
      }
      case READ_SLOT:
      {
         m_poCurrentChannel = nullptr;
#if 0
         if( !gsi::isInRange( number, DaqInterface::c_startSlot,
                                      DaqInterface::c_maxSlots ) )
         {
            ERROR_MESSAGE( "Given slot " << number <<
                           " is out of the range of: " <<
                           DaqInterface::c_startSlot << " and " <<
                           DaqInterface::c_maxSlots << " !" );
            return -1;
         }
#endif
         m_poCurrentDevice = m_poAllDaq->getDevice( number );
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
#if 0
         if( !gsi::isInRange( number, static_cast<uint>( 1 ),
                                      DaqInterface::c_maxChannels ) )
         {
            ERROR_MESSAGE( "Requested channel " << number <<
                           " is out of range of " <<
                           DaqInterface::c_maxChannels << " !" );
            return -1;
         }
#endif
         if( m_poCurrentDevice->getDaqCompare( number ) == nullptr )
         {
            m_poCurrentChannel = new DaqMilCompare( number );
            m_poCurrentDevice->registerDaqCompare( m_poCurrentChannel );
         }
         else
         {
            ERROR_MESSAGE( "Channel number specified several times: " << number );
         }
         FSM_TRANSITION( READ_SLOT );
         break;
      }
   }
   return 1;
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedShortOption( char unrecognized )
{
   ERROR_MESSAGE( "Unknown option: '-" << unrecognized << '\'' );
   return -1;
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedLongOption( const std::string& unrecognized )
{
   ERROR_MESSAGE( "Unknown option: \"--" << unrecognized << '"' );
   return -1;
}

// ================================= EOF ======================================

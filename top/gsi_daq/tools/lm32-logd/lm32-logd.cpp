/*!
 *  @file lm32-logd.cpp
 *  @brief Main module for the LM32 log daemon.
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
#include <exception>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <daqt_messages.hpp>
#include <find_process.h>
#include "logd_cmdline.hpp"
#include "logd_core.hpp"

using namespace std;
using namespace Scu;

STATIC bool g_exit = false;

/*! ---------------------------------------------------------------------------
 */
void onUnexpectedException( void )
{
   ERROR_MESSAGE( "Unexpected exception occurred!" );
   throw 0;     // throws int (in exception-specification)
}

/*! ---------------------------------------------------------------------------
 */
STATIC void handleConcurrentRunningInstance( OFP_ARG_T* pArg )
{
   CommandLine* poCmdLine = static_cast<CommandLine*>(pArg->pUser);

   if( poCmdLine->isKill() || poCmdLine->isKillOnly() )
   {
      if( poCmdLine->isVerbose() )
         cout << "killing concurrent process with PID: " << pArg->pid << endl;

      if( ::kill( pArg->pid, SIGTERM ) == -1 )
      {
         ERROR_MESSAGE( "Unable to terminate the concurrent running process:"
                        " PID: " << pArg->pid << " errno: " << errno );
         ::exit( EXIT_FAILURE );
      }
      if( poCmdLine->isKillOnly() )
         ::exit( EXIT_SUCCESS );
      return;
   }

   ERROR_MESSAGE( "Concurrent process with PID: " << pArg->pid <<
                  " is already running!" );

   ::exit( EXIT_FAILURE );
}

extern "C"
{

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by function findProcesses().
 */
STATIC int onFoundProcess( OFP_ARG_T* pArg )
{
   DEBUG_MESSAGE_FUNCTION("OFP_ARG_T*");

   if( pArg->pid == ::getpid() )
      return 0; // Process has found himself. Program continue.

   DEBUG_MESSAGE( "Concurrent process with PID: " << pArg->pid << " found." );

   CommandLine* poCmdLine = static_cast<CommandLine*>(pArg->pUser);

   if( poCmdLine->isRuningOnScu() )
   {
      handleConcurrentRunningInstance( pArg );
      return 0;
   }

   const string scuName = poCmdLine->getScuUrl().substr(poCmdLine->getScuUrl().find_first_of('/')+1);

   /*
    * Evaluating the command line of the concurrent running process.
    */
   char* currentArg = reinterpret_cast<char*>(pArg->commandLine.buffer);
   for( uint i = 0; i < pArg->commandLine.argc; i++, currentArg += ::strlen( currentArg ) + 1 )
   {
      if( *currentArg == '-' )
         continue;

      string cScuName = currentArg;
      cScuName = cScuName.substr(cScuName.find_first_of('/')+1);
      cout << cScuName << endl;
      if( scuName == cScuName )
         handleConcurrentRunningInstance( pArg );
   }
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the linux kernel, when this
 *        process a signal has been received.
 *
 * This is the case, when for example a concurrent process has sent SIGTERM.
 * @see handleConcurrentRunningInstance
 */
STATIC void onOsSignal( int sigNo )
{
   DEBUG_MESSAGE_FUNCTION( sigNo );
   g_exit = (sigNo == SIGTERM);
}

} /* extern "C" */

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   DEBUG_MESSAGE_FUNCTION("");
   set_unexpected( onUnexpectedException );

   try
   {
      CommandLine oCmdLine( argc, ppArgv );
      oCmdLine();

      ::findProcesses( ppArgv[0], onFoundProcess, &oCmdLine, FPROC_MODE_T(FPROC_BASENAME) );
                       //static_cast<FPROC_MODE_T>(FPROC_BASENAME | FPROC_RLINK) );
      if( ::signal( SIGTERM, onOsSignal ) == SIG_ERR )
         WARNING_MESSAGE( "Can't install the signal handling for SIGTERM !" );

      mmuEb::EtherboneConnection ebc( oCmdLine.getScuUrl() );
      Lm32Logd oLog( ebc, oCmdLine );
      oLog( g_exit );
      if( oCmdLine.isVerbose() )
         oLog << "Process: \"" << oCmdLine.getProgramName() << "\" terminated by "
              << (g_exit? "SIGTERM":"user") << "." << endl;
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "std::exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   catch( ... )
   {
      ERROR_MESSAGE( "Undefined exception occurred!" );
      return EXIT_FAILURE;
   }
   return EXIT_SUCCESS;
}

//================================== EOF ======================================

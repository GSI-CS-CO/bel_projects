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
#include <daqt_messages.hpp>
#include <find_process.h>
#include "logd_cmdline.hpp"
#include "logd_core.hpp"

#include <stdio.h>

using namespace std;
using namespace Scu;

/*! ---------------------------------------------------------------------------
 */
void onUnexpectedException( void )
{
   ERROR_MESSAGE( "Unexpected exception occurred!" );
   throw 0;     // throws int (in exception-specification)
}

extern "C"
{

/*! ---------------------------------------------------------------------------
 */
STATIC int onFoundProcess( OFP_ARG_T* pArg )
{
   DEBUG_MESSAGE( __FUNCTION__ );
   if( pArg->pid == getpid() )
      return 0; // Process has found himself. Program continue.

   CommandLine* poCmdLine = static_cast<CommandLine*>(pArg->pUser);
   cout << pArg->pid << " url = " << poCmdLine->getScuUrl() << endl;


   uint8_t* currentArg = pArg->commandLine.buffer;
   for( uint i = 0; i < pArg->commandLine.argc; i++ )
   {
      cout << currentArg << endl;
      currentArg += strlen( reinterpret_cast<char*>(currentArg) );
   }
   printf( "\n" );

   return 0;
}

} /* extern "C" */

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUnexpectedException );

   try
   {
      CommandLine oCmdLine( argc, ppArgv );
      oCmdLine();
      ::findProcesses( ppArgv[0], onFoundProcess, &oCmdLine, FPROC_MODE_T(FPROC_BASENAME) );
                       //static_cast<FPROC_MODE_T>(FPROC_BASENAME | FPROC_RLINK) );
      mmuEb::EtherboneConnection ebc( oCmdLine.getScuUrl() );
      Lm32Logd oLog( ebc, oCmdLine );
      oLog();
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

/*!
 *  @file daqt_onFoundProcess.cpp
 *  @brief Callback function for C-function "::findProcesses"
 *
 *  @date 29.08.2019
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
#include <netdb.h>
#include <string.h>

#include "daqt_messages.hpp"
#include "daqt_onFoundProcess.hpp"

namespace Scu
{
namespace daq
{

using namespace std;

/*! ---------------------------------------------------------------------------
 */
int __onFoundProcess( ::OFP_ARG_T* pArg )
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

} // namespace daq
} // namespace Scu

//================================== EOF ======================================

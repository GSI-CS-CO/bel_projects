/*!
 *  @file mem-mon.cpp
 *  @brief Main module for the SCU memory monitor.
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
#include <exception>
#include <cstdlib>
#include <daqt_messages.hpp>
#include "mem_cmdline.hpp"
#include "mem_browser.hpp"

using namespace std;
using namespace Scu::mmu;

/*! ---------------------------------------------------------------------------
 */
void onUnexpectedException( void )
{
  ERROR_MESSAGE( "Unexpected exception occurred!" );
  throw 0;     // throws int (in exception-specification)
}


/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   set_unexpected( onUnexpectedException );
   try
   {
      CommandLine oCmdLine( argc, ppArgv );
      mmuEb::EtherboneConnection ebc( oCmdLine() );
      Browser browse( ebc, oCmdLine );
      browse( cout );
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

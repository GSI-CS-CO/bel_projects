/*!
 *  @file mdaqt.cpp
 *  @brief Main module of MIL-Data Acquisition Tool
 *
 *  @date 14.08.2019
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

#include <stdlib.h>
#include <iostream>
#include "mdaqt.hpp"
#include "daqt_read_stdin.hpp"
#include "daqt_messages.hpp"

using namespace std;
using namespace Scu;
using namespace MiLdaq;
using namespace MiLdaqt;

/*! ---------------------------------------------------------------------------
 */
int mdaqtMain( int argc, char** ppArgv )
{
   DaqEb::EtherboneConnection ebConnection( ppArgv[1] );
   DaqAdministration milDaqAdmin( &ebConnection );
   DEBUG_MESSAGE( "Head: " << milDaqAdmin.getHeadRingIndex() );
   DEBUG_MESSAGE( "Tail: " << milDaqAdmin.getTailRingIndex() );
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   try
   {
      return mdaqtMain( argc, ppArgv );
   }
#if 1
   catch( MiLdaq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: " << e.what() );
   }
#endif
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

/*!
 *  @file feedback-example.cpp
 *  @brief Minimal example testing feedback channels of function generators.
 *
 *  @date 26.05.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <iostream>
#include <scu_fg_feedback.hpp>

using namespace Scu;
using namespace std;



int main( int argc, const char** ppArgv )
{
   if( argc < 2 )
   {
      cerr << "ERROR: No SCU-URL given!" << endl;
      return EXIT_FAILURE;
   }
   cout << "SCU-URL: " << ppArgv[1] << endl;
#if 1
   try
   {
      DaqEb::EtherboneConnection ebConnection( ppArgv[1] );
      ebConnection.connect();
      FgFeedbackAdministration oFbAdmin( &ebConnection );
      oFbAdmin.scan();
   }
   catch( exception& e )
   {
      cerr << "ERROR: Something went wrong: \"" << e.what() << '"'
           << endl;
      return EXIT_FAILURE;
   }
#endif
   return EXIT_SUCCESS;
}

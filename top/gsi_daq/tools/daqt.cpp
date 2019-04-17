/*!
 *  @file daqt.cpp
 *  @brief Main module of Data Acquisition Tool
 *
 *  @date 11.04.2019
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
#include "daqt.hpp"
#include "daqt_messages.hpp"
#include "daqt_command_line.hpp"
#include "daqt_scan.hpp"

using namespace daqt;

/*! ---------------------------------------------------------------------------
 */
bool Channel::onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen )
{
   return false;
}

/*! ---------------------------------------------------------------------------
 */
inline int daqtMain( int argc, char** ppArgv )
{
   CommandLine cmdLine( argc, ppArgv );
   if( cmdLine() < 0 )
      return EXIT_FAILURE;
   return EXIT_SUCCESS;
}

/*! ---------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   try
   {
      return daqtMain( argc, ppArgv );
   }
   catch( daq::EbException& e )
   {
      ERROR_MESSAGE( "daq::EbException occurred: " << e.what() );
   }
   catch( daq::DaqException& e )
   {
      ERROR_MESSAGE( "daq::DaqException occurred: " << e.what()
                     << "\nStatus: " <<  e.getStatusString() );
   }
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

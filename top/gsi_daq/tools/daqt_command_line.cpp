/*!
 *  @file daqt_command_line.cpp
 *  @brief Command line parser of DAQ-Test
 *
 *  @date 16.04.2019
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
#include "daqt_command_line.hpp"

using namespace daqt;
using namespace std;

vector<OPTION> CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         assert( !poParser->isOptArgPersent() );
         cout << "Usage: " << poParser->getProgramName()
              << " [options,...] <proto/host/port> \n";
         poParser->list( cout );
         cout << endl;
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
         static_cast<CommandLine*>(poParser)->m_doScan = true;
         return 1;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 's',
      .m_longOpt  = "scan",
      .m_helpText = "Scan the entire SCU-bus for DAQ- devices and its"
                    " channels"
   }//,
};

CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,m_poAllDaq( nullptr )
   ,m_doScan( false )
{
   add( c_optList );
}

CommandLine::~CommandLine( void )
{
   if( m_poAllDaq != nullptr )
       delete m_poAllDaq;
}

int CommandLine::onArgument( void )
{
   if( m_poAllDaq == nullptr )
   {
      m_poAllDaq = new DaqAdministration(getArgVect()[getArgIndex()]);
      return 1;
   }
   return 0;
}

//================================== EOF ======================================

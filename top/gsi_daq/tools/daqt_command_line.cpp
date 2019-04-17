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
         DaqAdministration* poAllDaq =
            static_cast<CommandLine*>(poParser)->m_poAllDaq;
         if( poAllDaq == nullptr )
         {
            ERROR_MESSAGE( "<proto/host/port> must be specified before!" );
            return -1;
         }
         for( unsigned int i = 1; i <= poAllDaq->getMaxFoundDevices(); i++ )
         {
            cout << poAllDaq->getSlotNumber( i ) << " "
                 <<  poAllDaq->readMaxChannels( i ) << endl;
         }
         return 0;
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
   ,FSM_INIT_FSM( READ_EB_NAME )
   ,m_poAllDaq( nullptr )
   ,m_poCurrentDevice( nullptr )
   ,m_poCurrentChannel( nullptr )
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
   const string arg = getArgVect()[getArgIndex()];
   unsigned int number;
   switch( m_state )
   {
      case READ_SLOT:
      case READ_CHANNEL:
      {
         SCU_ASSERT( m_poAllDaq != nullptr );
         try
         {
            number = stoi( arg );
         }
         catch( std::exception& e )
         {
            ERROR_MESSAGE( "Integer number is expected and not that: \""
                           << arg << "\" !" );
            return -1;
         }
         break;
      }
   }
   switch( m_state )
   {
      case READ_EB_NAME:
      {
         SCU_ASSERT( m_poAllDaq == nullptr );
         m_poAllDaq = new DaqContainer( arg, this );
         FSM_TRANSITION( READ_SLOT );
         break;
      }
      case READ_SLOT:
      {
         m_poCurrentChannel = nullptr;
         if( !gsi::isInRange( number, DaqInterface::c_startSlot,
                                      DaqInterface::c_maxSlots ) )
         {
            ERROR_MESSAGE( "Given slot " << number <<
                           " is out of the range of: " <<
                           DaqInterface::c_startSlot << " and " <<
                           DaqInterface::c_maxSlots << " !" );
            return -1;
         }
         if( !m_poAllDaq->isDevicePresent( number ) )
         {
            ERROR_MESSAGE( "In slot " << number << " isn't a DAQ!" );
            return -1;
         }
         m_poCurrentDevice = m_poAllDaq->getDeviceBySlot( number );
         if( m_poCurrentDevice == nullptr )
         {
            m_poCurrentDevice = new DaqDevice( number );
            m_poAllDaq->registerDevice( m_poCurrentDevice );
         }
         FSM_TRANSITION( READ_CHANNEL );
         break;
      }
      case READ_CHANNEL:
      {  //TODO
         SCU_ASSERT( m_poCurrentDevice != nullptr );
         cout << "Channel " << arg << endl;
         FSM_TRANSITION( READ_SLOT );
         break;
      }
   }

   return 1;
}

//================================== EOF ======================================

/*!
 *  @file daqt.hpp
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
#ifndef _DAQT_HPP
#define _DAQT_HPP
#include <eb_console_helper.h>
#include <stdlib.h>
#include <daq_administration.hpp>
#include <string>

namespace daqt
{
using namespace daq;

class CommandLine;
///////////////////////////////////////////////////////////////////////////////
class DaqContainer: public DaqAdministration
{
   CommandLine*   m_poCommandLine;

public:
   DaqContainer( const std::string ebName, CommandLine* poCommandLine )
      :DaqAdministration( ebName )
      ,m_poCommandLine( poCommandLine )
      {}

   CommandLine* getCommandLinePtr( void )
   {
      return m_poCommandLine;
   }
};

///////////////////////////////////////////////////////////////////////////////
class Channel: public DaqChannel
{
public:
   Channel( unsigned int number )
      :DaqChannel( number )
    {}

   bool onDataBlock( DAQ_DATA_T* pData, std::size_t wordLen ) override;
};

}


#endif // ifndef _DAQT_HPP
//================================== EOF ======================================

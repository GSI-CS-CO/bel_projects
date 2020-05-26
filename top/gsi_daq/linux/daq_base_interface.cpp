/*!
 * @file daq_base_interface.cpp
 * @brief DAQ common base interface for ADDAC-DAQ and MIL-DAQ.
 *
 * @date 26.05.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#include <daq_base_interface.hpp>

using namespace Scu;

///////////////////////////////////////////////////////////////////////////////
/*! --------------------------------------------------------------------------
 */
DaqBaseInterface::DaqBaseInterface( DaqEb::EtherboneConnection* poEtherbone )
   :m_poEbAccess( new daq::EbRamAccess( poEtherbone ) )
   ,m_ebAccessSelfCreated( true )
{

}

/*! --------------------------------------------------------------------------
 */
DaqBaseInterface::DaqBaseInterface( daq::EbRamAccess* poEbAccess )
   :m_poEbAccess( poEbAccess )
   ,m_ebAccessSelfCreated( false )
{

}

/*! --------------------------------------------------------------------------
 */
DaqBaseInterface:: ~DaqBaseInterface( void )
{
   if( m_ebAccessSelfCreated )
      delete m_poEbAccess;
}


//================================== EOF ======================================

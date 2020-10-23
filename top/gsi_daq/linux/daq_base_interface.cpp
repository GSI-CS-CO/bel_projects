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
#include <string>
#include <daq_base_interface.hpp>

using namespace Scu;

/*! ----------------------------------------------------------------------------
 * @ingroup DEBUG
 * @brief Converts the command return code of LM32 into a string.
 */
const std::string Scu::deviceType2String( daq::DAQ_DEVICE_TYP_T typ )
{
   #define __RET_CODE_CASE_ITEM( name ) case name: return #name
   switch( typ )
   {
      __RET_CODE_CASE_ITEM( daq::UNKNOWN );
      __RET_CODE_CASE_ITEM( daq::ADDAC );
      __RET_CODE_CASE_ITEM( daq::ACU );
      __RET_CODE_CASE_ITEM( daq::DIOB );
   #ifdef CONFIG_MIL_FG
      __RET_CODE_CASE_ITEM( daq::MIL );
   #endif
      default: break;
   }
   return "unknown";
   #undef __RET_CODE_CASE_ITEM
}

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

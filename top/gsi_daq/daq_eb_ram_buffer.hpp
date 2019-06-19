/*!
 *  @file daq_eb_ram_buffer.hpp
 *  @brief Linux whishbone/etherbone interface for accessing the SCU-DDR3 RAM
 *
 *  @see scu_ramBuffer.h
 *
 *  @see scu_ddr3.h
 *  @see scu_ddr3.c
 *  @date 19.06.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 *  @author Ulrich Becker <u.becker@gsi.de>
 *
 *******************************************************************************
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _DAQ_EB_RAM_BUFFER_HPP
#define _DAQ_EB_RAM_BUFFER_HPP
#include <EtherboneConnection.hpp>

namespace DaqEb = FeSupport::Scu::Etherbone;

namespace Scu
{
namespace daq
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Interface class for SCU DDR3 RAM access via the class
 *        EtherboneConnection
 */
class EbRamAccess
{
   DaqEb::EtherboneConnection* m_poEb;

public:
   EbRamAccess( DaqEb::EtherboneConnection* poEb )
      :m_poEb( poEb )
   {
   }

   ~EbRamAccess( void )
   {
   }

   DaqEb::EtherboneConnection* getEbObjectPtr( void )
   {
      return m_poEb;
   }
};

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQ_EB_RAM_BUFFER_HPP
//================================== EOF ======================================
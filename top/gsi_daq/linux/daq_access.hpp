/*!
 *  @file daq_access.hpp
 *  @brief Manages the MIL and ADDAC_ DAQ data access on LM32
 *
 *  @see daq_eb_ram_buffer.hpp
 *  @see scu_shared_mem.h
 *  @date 04.06.2021
 *  @copyright (C) 2021 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
 * License along with this library. If not, see <http://www.gnu.org/licenses/>
 ******************************************************************************
 */
#ifndef _DAQ_ACCESS_HPP
#define _DAQ_ACCESS_HPP

#include "daq_eb_ram_buffer.hpp"
#include <scu_shared_mem.h>

namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
/*!----------------------------------------------------------------------------
 * @brief Class probes the LM32 firmware for ADDAC and MIL- DAQs and
 *        calculates the data offsets of them.
 */
class DaqAccess: public daq::EbRamAccess
{
public:
   constexpr static uint INVALID_OFFSET       = static_cast<uint>(~0);
#ifdef CONFIG_MIL_FG
   constexpr static uint MIL_DAQ_OFFSET       = sizeof( FG::SAFT_LIB_T );
#endif
   constexpr static uint OLD_ADDAC_DAQ_OFFSET = sizeof( FG::SAFT_LIB_T )
                                              + sizeof( MiLdaq::MIL_DAQ_BUFFER_T );
   constexpr static uint ADDAC_DAQ_OFFSET     = sizeof( FG::SAFT_LIB_T )
                                              + sizeof( MiLdaq::MIL_DAQ_ADMIN_T );
private:
   uint                         m_addacDaqLM32Offset;
#ifdef CONFIG_MIL_FG
   uint                         m_milDaqLm32Offset;
#endif
public:
   /*!
    * @brief Constructor establishes the etherbone connection if it's not
    *        already been done outside and probes the LM32-firmware.
    * @param poEb Pointer to the object of type EtherboneConnection.
    */
   DaqAccess( DaqEb::EtherboneConnection* poEb );

   /*!
    * @brief Destructor terminates the ehtherbone connection if the
    *        connection was made by this object.
    */
   ~DaqAccess( void );

#ifdef CONFIG_MILDAQ_BACKWARD_COMPATIBLE
   /*!
    * @brief Returns "true" if the LM32-firmware is a old one storing
    * the MIL-DAQ data in LM32 shared memory.
    */
   bool isMilDataInLm32Mem( void ) const
   {
      return m_addacDaqLM32Offset == OLD_ADDAC_DAQ_OFFSET;
   }
#endif

   void probe( void );

   uint getAddacDaqOffset( void ) const
   {
      return m_addacDaqLM32Offset;
   }

#ifdef CONFIG_MIL_FG
   uint getMilDaqOffset( void )
   {
      return m_milDaqLm32Offset;
   }
#endif
};

} // namespace Scu
#endif // ifndef _DAQ_ACCESS_HPP
//================================== EOF ======================================

/*!
 *  @file logd_core.hpp
 *  @brief Main functionality of LM32 log daemon.
 *
 *  @date 21.04.2022
 *  @copyright (C) 2022 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _LOGD_CORE_HPP
#define _LOGD_CORE_HPP

#include <scu_mmu_fe.hpp>
#include "logd_cmdline.hpp"


namespace Scu
{

///////////////////////////////////////////////////////////////////////////////
class Lm32Logd
{
   CommandLine& m_rCmdLine;
   mmu::Mmu     m_oMmu;
   uint         m_lm32Base;

public:
   Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine );
   ~Lm32Logd( void );
   void operator()( void );

private:
   void readLm32( char* pData,
                  const std::size_t len,
                  const std::size_t offset = 0,
                  const etherbone::format_t format = EB_DATA8 )
   {
      m_oMmu.getEb()->read( m_lm32Base + offset, pData,
                            EB_BIG_ENDIAN | format, len );
   }

   uint readStringFromLm32( std::string& rStr, uint addr );
};

} // namespace Scu

#endif // ifndef _LOGD_CORE_HPP
//================================== EOF ======================================

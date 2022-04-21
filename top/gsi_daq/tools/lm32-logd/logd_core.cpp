/*!
 *  @file logd_core.cpp
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
#include <scu_mmu_tag.h>
#include "logd_core.hpp"

using namespace Scu;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
Lm32Logd::Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine )
   :m_rCmdLine( rCmdLine )
   ,m_oMmu( &roEtherbone )
{
   if( !m_oMmu.isPresent() )
   {
      throw std::runtime_error( "MMU not present!" );
   }

   mmu::MMU_ADDR_T addr;
   std::size_t     size;
   mmu::MMU_STATUS_T status = m_oMmu.allocate( mmu::TAG_LM32_LOG, addr, size );
   if( status != mmu::OK )
   {
      throw std::runtime_error( m_oMmu.status2String( status ) );
   }

   m_lm32Base = m_oMmu.getEb()->findDeviceBaseAddress( mmuEb::gsiId,
                                                       mmuEb::lm32_ram_user );
}

/*! ---------------------------------------------------------------------------
 */
Lm32Logd::~Lm32Logd( void )
{
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::operator()( void )
{
}

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readStringFromLm32( std::string& rStr, uint addr )
{
   char buffer[16];
   uint ret = 0;

   while( true )
   {
      readLm32( buffer, sizeof( buffer ), addr );
      for( uint i = 0; i < sizeof( buffer ); i++ )
      {
         if( buffer[i] == '\0' )
            return ret;
         rStr += buffer[i];
         ret++;
      }
      addr += sizeof( buffer );
   }
}

//================================== EOF ======================================

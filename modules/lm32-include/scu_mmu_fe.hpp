/*!
 * @file scu_mmu_fe.hpp
 * @brief Memory Management Unit of SCU Linux-interface for front end
 *
 * Administration of the shared memory (for SCU3 using DDR3) between
 * Linux host and LM32 application.
 *
 * @note This source code is suitable for LM32 and Linux.
 *
 * @see       scu_mmu_fe.cpp
 * @copyright GSI Helmholtz Centre for Heavy Ion Research GmbH
 * @author    Ulrich Becker <u.becker@gsi.de>
 * @date      06.04.2022
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
#ifndef _SCU_MMU_FE_HPP
#define _SCU_MMU_FE_HPP

#include <scu_mmu.h>
#include <EtherboneConnection.hpp>
#include <assert.h>

namespace mmuEb = FeSupport::Scu::Etherbone;

class Mmu
{
   mmuEb::EtherboneConnection* m_poEtherbone;
   bool                        m_selfConnected;
   uint                        m_ramBase;


public:
   Mmu( mmuEb::EtherboneConnection* poEtherbone );
   ~Mmu( void );

   mmuEb::EtherboneConnection* getEb( void )
   {
      assert( m_poEtherbone->isConnected() );
      return m_poEtherbone;
   }

   uint getBase( void )
   {
      assert( m_ramBase != 0 );
      return m_ramBase;
   }
};


#endif // ifndef _SCU_MMU_FE_HPP
//================================== EOF ======================================

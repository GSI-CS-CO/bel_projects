/*!
 * @file scu_mmu_fe.cpp
 * @brief Memory Management Unit of SCU Linux-interface for front end
 *
 * Administration of the shared memory (for SCU3 using DDR3) between
 * Linux host and LM32 application.
 *
 * @note This source code is suitable for LM32 and Linux.
 *
 * @see       scu_mmu_fe.hpp
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
#define _SCU_MMU_FE_CPP
#include <scu_mmu_fe.hpp>

#include <lm32_hexdump.h>

using namespace Scu::mmu;

static Mmu* mg_pMmu = nullptr;

/*! ---------------------------------------------------------------------------
 */
Mmu::Mmu( mmuEb::EtherboneConnection* poEtherbone )
   :m_poEtherbone( poEtherbone )
   ,m_selfConnected( false )
   ,m_ramBase( 0 )
{
   assert( mg_pMmu == nullptr );
   mg_pMmu = this;
   if( !m_poEtherbone->isConnected() )
   {
      m_poEtherbone->connect();
      m_selfConnected = true;
   }

   m_ramBase = m_poEtherbone->findDeviceBaseAddress( mmuEb::gsiId, mmuEb::wb_ddr3ram );

}

/*! ---------------------------------------------------------------------------
 */
Mmu::~Mmu( void )
{
   assert( mg_pMmu == this );
   mg_pMmu = nullptr;
   if( m_selfConnected )
      m_poEtherbone->disconnect();
}

extern "C"
{

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuWrite( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
{
   assert( mg_pMmu != nullptr );
   mg_pMmu->write( index, pItem, len );
}

/*! ---------------------------------------------------------------------------
 * @see scu_mmu.h
 */
void mmuRead( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
{
   assert( mg_pMmu != nullptr );
   mg_pMmu->read( index, pItem, len );
}

} // extern "C"

//================================== EOF ======================================

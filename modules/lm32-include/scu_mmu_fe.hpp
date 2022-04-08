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

namespace Scu
{
namespace mmu
{

///////////////////////////////////////////////////////////////////////////////
class Mmu
{
   mmuEb::EtherboneConnection* m_poEtherbone;
   bool                        m_selfConnected;
   uint                        m_ramBase;


public:
   Mmu( mmuEb::EtherboneConnection* poEtherbone );
   ~Mmu( void );

   bool isPresent( void )
   {
      assert( m_poEtherbone->isConnected() );
      return mmuIsPresent();
   }

   void clear( void )
   {
      assert( m_poEtherbone->isConnected() );
      mmuDelete();
   }

   uint getNumberOfBlocks( void )
   {
      assert( m_poEtherbone->isConnected() );
      return mmuGetNumberOfBlocks();
   }

   MMU_STATUS_T allocate( const MMU_TAG_T tag, MMU_ADDR_T& rStartAddr,
                          size_t& rLen, const bool create = false )
   {
      assert( m_poEtherbone->isConnected() );
      return mmuAlloc( tag, &rStartAddr, &rLen, create );
   }

   std::string status2String( const MMU_STATUS_T status )
   {
      return mmuStatus2String( status );
   }

   mmuEb::EtherboneConnection* getEb( void )
   {
      return m_poEtherbone;
   }

   void write( const uint index,
               const eb_user_data_t pData,
               const etherbone::format_t format,
               const uint size = 1 )
   {
      assert( m_poEtherbone->isConnected() );
      m_poEtherbone->write( m_ramBase + index * sizeof(RAM_PAYLOAD_T),
                            pData,
                            format,
                            size );
   }

   void write( MMU_ADDR_T index, const RAM_PAYLOAD_T* pItem, size_t len )
   {
      write( index,
             eb_user_data_t(pItem),
             sizeof( pItem->ad32[0] ) | EB_LITTLE_ENDIAN,
             len * ARRAY_SIZE( pItem->ad32 ) );
   }

   void read( const uint index,
              eb_user_data_t pData,
              const etherbone::format_t format,
              const uint size = 1 )
   {
      assert( m_poEtherbone->isConnected() );
      m_poEtherbone->read( m_ramBase + index * sizeof(RAM_PAYLOAD_T),
                           pData,
                           format,
                           size );
   }

   void read( MMU_ADDR_T index, RAM_PAYLOAD_T* pItem, size_t len )
   {
      read( index,
            pItem,
            sizeof( pItem->ad32[0] ) | EB_LITTLE_ENDIAN,
            len * ARRAY_SIZE( pItem->ad32 ) );
   }

}; // class Mmu

} // namespace mmu
} // namespace Scu

#endif // ifndef _SCU_MMU_FE_HPP
//================================== EOF ======================================

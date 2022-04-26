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
#include <iomanip>
#include <scu_mmu_tag.h>
#include "logd_core.hpp"

using namespace Scu;
using namespace std;

constexpr uint LM32_OFFSET = 0x10000000;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
Lm32Logd::Lm32Logd( mmuEb::EtherboneConnection& roEtherbone, CommandLine& rCmdLine )
   :m_rCmdLine( rCmdLine )
   ,m_oMmu( &roEtherbone )
   ,m_lastTimestamp( 0 )
   ,m_maxItems( 10 )
   ,m_pMiddleBuffer( nullptr )
{
   if( !m_oMmu.isPresent() )
   {
      throw std::runtime_error( "MMU not present!" );
   }

   mmu::MMU_STATUS_T status = m_oMmu.allocate( mmu::TAG_LM32_LOG, m_offset, m_capacity );
   if( status != mmu::OK )
   {
      throw std::runtime_error( m_oMmu.status2String( status ) );
   }

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Found MMU-tag:  0x" << hex << uppercase << setw( 4 ) << setfill('0')
           << mmu::TAG_LM32_LOG << dec
           << "\nAddress:        " << m_offset
           << "\nCapacity:       " << m_capacity << endl;
   }

   m_fifoAdminBase = m_offset * sizeof(mmu::RAM_PAYLOAD_T) + m_oMmu.getBase();

   m_offset   += SYSLOG_FIFO_ADMIN_SIZE;
   m_capacity -= SYSLOG_FIFO_ADMIN_SIZE;

   if( m_rCmdLine.isVerbose() )
   {
      cout << "Begin:          " << m_offset
         << "\nMax. log items: " << m_capacity / SYSLOG_FIFO_ITEM_SIZE << endl;
   }

   m_fiFoAdmin.admin.indexes.offset   = 0;
   m_fiFoAdmin.admin.indexes.capacity = 0;
   m_fiFoAdmin.admin.indexes.start    = 0;
   m_fiFoAdmin.admin.indexes.end      = 0;
   m_fiFoAdmin.admin.wasRead          = 0;
   updateFiFoAdmin( m_fiFoAdmin );

   if( m_rCmdLine.isVerbose() )
   {
      cout << "At the moment "
           << sysLogFifoGetItemSize( &m_fiFoAdmin )
           << " Log-items in FiFo." << endl;
   }

   m_lm32Base = m_oMmu.getEb()->findDeviceBaseAddress( mmuEb::gsiId,
                                                       mmuEb::lm32_ram_user );

   /*
    * The string addresses of LM32 comes from the perspective of the LM32.
    * Therefore a offset correction has to made here.
    */
   if( m_lm32Base < LM32_OFFSET )
   {
      throw std::runtime_error( "LM32 base address is corrupt!" );
   }
   m_lm32Base -= LM32_OFFSET;
}

/*! ---------------------------------------------------------------------------
 */
Lm32Logd::~Lm32Logd( void )
{
   if( m_pMiddleBuffer != nullptr )
      delete[] m_pMiddleBuffer;
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::operator()( void )
{
  // std::string str;
  // uint n = readStringFromLm32( str, 0x100020f8 );
  // std::cout << "Zeichen: " << n << ", Text: " << str << std::endl;
   readItems();
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::updateFiFoAdmin( SYSLOG_FIFO_ADMIN_T& rAdmin )
{
   assert( m_oMmu.getEb()->isConnected() );

   constexpr uint LEN32 = sizeof(SYSLOG_FIFO_ADMIN_T) / sizeof(uint32_t);

   m_oMmu.getEb()->read( m_fifoAdminBase, &rAdmin, EB_DATA32 | EB_LITTLE_ENDIAN, LEN32 );
   if( (rAdmin.admin.indexes.offset   != m_offset) ||
       (rAdmin.admin.indexes.capacity != m_capacity) )
   {
      throw std::runtime_error( "LM32 syslog fifo is corrupt!" );
   }
}

/*! ---------------------------------------------------------------------------
 */
uint Lm32Logd::readStringFromLm32( std::string& rStr, uint addr )
{
   const uint HIGHST_ADDR = 2*LM32_OFFSET;

   if( !gsi::isInRange( addr, LM32_OFFSET, HIGHST_ADDR ) )
   {
      throw std::runtime_error( "String address is corrupt!" );
   }

   char buffer[16];
   uint ret = 0;

   while( true )
   {
      readLm32( buffer, sizeof( buffer ), addr );
      for( uint i = 0; i < sizeof( buffer ); i++ )
      {
         if( (buffer[i] == '\0') || (addr + i >= HIGHST_ADDR) )
            return ret;
         rStr += buffer[i];
         ret++;
      }
      addr += sizeof( buffer );
   }
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::readItems( void )
{
   SYSLOG_FIFO_ADMIN_T fifoAdmin;

   updateFiFoAdmin( fifoAdmin );
   if( fifoAdmin.admin.wasRead != 0 )
      return;

   uint size = sysLogFifoGetSize( &fifoAdmin );
   if( size == 0 )
      return;

   if( (size % SYSLOG_FIFO_ITEM_SIZE) != 0 )
      return;

   m_fiFoAdmin = fifoAdmin;

   const uint readTotalLen = min( size, m_maxItems );
   uint len = readTotalLen;
   const uint numOfItems = readTotalLen / SYSLOG_FIFO_ITEM_SIZE;
   assert( m_pMiddleBuffer == nullptr );
   m_pMiddleBuffer = new SYSLOG_FIFO_ITEM_T[numOfItems*10]; //TODO: WHY??
   SYSLOG_FIFO_ITEM_T* pData = m_pMiddleBuffer;
   uint lenToEnd = sysLogFifoGetUpperReadSize( &m_fiFoAdmin );
   if( lenToEnd < readTotalLen )
   {
      readItems( pData, lenToEnd );
      pData += lenToEnd;
      len   -= lenToEnd;
   }
   readItems( pData, len );

   for( uint i = 0; i < numOfItems; i++ )
   {
      evaluateItem( m_pMiddleBuffer[i] );
   }

   delete[] m_pMiddleBuffer;
   m_pMiddleBuffer = nullptr;
}

/*! ---------------------------------------------------------------------------
 */
void Lm32Logd::evaluateItem( const SYSLOG_FIFO_ITEM_T& item )
{
   std::string format;
   readStringFromLm32( format, item.format );
   cout << "TEST: " << format << endl;

   m_lastTimestamp = item.timestamp;
}


//================================== EOF ======================================

/*!
 *  @file daq_eb_ram_buffer.cpp
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
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */
#include <daq_eb_ram_buffer.hpp>

using namespace Scu;
using namespace daq;

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
EbRamAccess::EbRamAccess( DaqEb::EtherboneConnection* poEb )
   :m_poEb( poEb )
   ,m_connectedBySelf( false )
   ,m_pRam( nullptr )
{
   if( !m_poEb->isConnected() )
   {
      m_poEb->connect();
      m_connectedBySelf = true;
   }
}

/*! ---------------------------------------------------------------------------
 */
EbRamAccess::~EbRamAccess( void )
{
   if( m_connectedBySelf )
      m_poEb->disconnect();
}

/*! ---------------------------------------------------------------------------
 */
void EbRamAccess::ramInit( RAM_SCU_T* pRam, RAM_RING_SHARED_OBJECT_T* pSharedObj )
{
   SCU_ASSERT( m_poEb->isConnected() );
   SCU_ASSERT( m_pRam == nullptr );

   m_pRam = pRam;
   m_pRam->pSharedObj = pSharedObj;
   ramRingReset( &m_pRam->pSharedObj->ringIndexes );
#ifdef CONFIG_SCU_USE_DDR3
   m_pRam->ram.pTrModeBase = m_poEb->findDeviceBaseAddress( DaqEb::gsiId,
                                                            DaqEb::wb_ddr3ram );
 #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   m_pRam->ram.pBurstModeBase m_poEb->findDeviceBaseAddress( DaqEb::gsiId,
                                                             DaqEb::wb_ddr3ram2 );
 #endif
#else
   #error Nothing implemented in function ramRreadItem()!
#endif
}

/*! ---------------------------------------------------------------------------
 */
int EbRamAccess::readDaqDataBlock( RAM_DAQ_PAYLOAD_T* pData,
                                   unsigned int len
                                 #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                                   , RAM_DAQ_POLL_FT poll
                                 #endif
                                 )
{
   SCU_ASSERT( m_pRam != nullptr );
#ifdef CONFIG_SCU_USE_DDR3
  #if defined( CONFIG_DDR3_NO_BURST_FUNCTIONS )
   RAM_RING_INDEXES_T indexes = m_pRam->pSharedObj->ringIndexes;

   //TODO knallt noch!!!
   m_poEb->doRead( m_pRam->ram.pTrModeBase +
                    ramRingGeReadIndex( &indexes ) * ARRAY_SIZE( pData->ad32 ),
                   reinterpret_cast<etherbone::data_t*>(pData),
                   sizeof( pData->ad32 ),
                   len * ARRAY_SIZE( pData->ad32 ));

   ramRingAddToReadIndex( &indexes, len );
   m_pRam->pSharedObj->ringIndexes = indexes;

   return EB_OK;
  #else // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
   #warning At the moment the burst mode is slow!
   int ret;
   RAM_RING_INDEXES_T indexes = m_pRam->pSharedObj->ringIndexes;
   unsigned int lenToEnd = indexes.capacity - indexes.start;

   if( lenToEnd < len )
   {
      //TODO
      ret = ddr3FlushFiFo( ramRingGeReadIndex( &indexes ),
                           lenToEnd, pData, poll );
      if( ret != EB_OK )
         return ret;
      ramRingAddToReadIndex( &indexes, lenToEnd );
      len   -= lenToEnd;
      pData += lenToEnd;
   }
   //TODO
   ret = ddr3FlushFiFo( ramRingGeReadIndex( &indexes ),
                        len, pData, poll );
   if( ret != EB_OK )
      return ret;
   ramRingAddToReadIndex( &indexes, len );
   m_pRam->pSharedObj->ringIndexes = indexes;

   return EB_OK;
  #endif
#else
   #error Unknown memory type for function: EbRamAccess::readDaqDataBlock()
#endif
}



//================================== EOF =======================================
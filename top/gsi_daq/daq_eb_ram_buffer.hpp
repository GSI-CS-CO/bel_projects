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
#include <eb_lm32_helper.h>
#include <daq_ramBuffer.h>
#include <generated/shared_mmap.h>

namespace DaqEb = FeSupport::Scu::Etherbone;

namespace Scu
{
namespace daq
{

#define EB_PADDING_SIZE sizeof( etherbone::data_t )

#define EB_PADDING_PLACE etherbone::data_t __padding;


///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Interface class for SCU DDR3 RAM access via the class
 *        EtherboneConnection
 */
class EbRamAccess
{
   DaqEb::EtherboneConnection* m_poEb;
   bool                        m_connectedBySelf;
   RAM_SCU_T*                  m_pRam;
   uint                        m_lm32SharedMemAddr;

public:
   EbRamAccess( DaqEb::EtherboneConnection* poEb );

   ~EbRamAccess( void );

   void ramInit( RAM_SCU_T* pRam, RAM_RING_SHARED_OBJECT_T* pSharedObj );

   DaqEb::EtherboneConnection* getEbPtr( void )
   {
      return m_poEb;
   }

   const std::string& getNetAddress( void )
   {
      return m_poEb->getNetAddress();
   }
#ifdef CONFIG_VIA_EB_CYCLE
   DaqEb::EtherboneConnection::MUTEX_T& getMutex( void )
   {
      return m_poEb->getMutex();
   }

   etherbone::Device& getEbDevice( void )
   {
      return m_poEb->getEbDevice();
   }
#endif
   int readDaqDataBlock( RAM_DAQ_PAYLOAD_T* pData,
                         unsigned int len
                       #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                         , RAM_DAQ_POLL_FT poll
                       #endif
                       );

   void readLM32( void* pData, std::size_t len, std::size_t offset = 0 )
   {
      m_poEb->doRead( m_lm32SharedMemAddr + offset,
                      reinterpret_cast<etherbone::data_t*>(pData),
                      EB_BIG_ENDIAN | EB_DATA8,
                      len, true );
   }

   void writeLM32( void* pData, std::size_t len, std::size_t offset = 0 )
   {
      m_poEb->doWrite( m_lm32SharedMemAddr + offset,
                       reinterpret_cast<const etherbone::data_t*>(pData),
                       EB_BIG_ENDIAN | EB_DATA8,
                       len, true );
   }
};

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQ_EB_RAM_BUFFER_HPP
//================================== EOF ======================================
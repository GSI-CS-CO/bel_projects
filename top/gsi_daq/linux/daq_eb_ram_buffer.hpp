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

   /*!
    * @brief Returns the TCP address e.g. "tcp/scuxl4711
    *        or in the case the application runs in the IPC of the SCU:
    *        "dev/wbm0"
    */
   const std::string& getNetAddress( void )
   {
      return m_poEb->getNetAddress();
   }

   const std::string getScuDomainName( void )
   {
      return getNetAddress().substr( getNetAddress().find_first_of( '/' ) + 1 );
   }

   int readDaqDataBlock( RAM_DAQ_PAYLOAD_T* pData,
                         std::size_t  len
                       #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
                         , RAM_DAQ_POLL_FT poll
                       #endif
                       );

   /*!
    * @brief Reads data from the LM32 shared memory area.
    * @note In this case a homogeneous data object is provided so
    *       the etherbone-library cant convert that in to little endian.\n
    *       Therefore in the case of the receiving of heterogeneous data e.g.
    *       structures, the conversion in little endian has to be made
    *       in upper software layers after.
    * @param pData Destination address for the received data.
    * @param len   Data length (array size).
    * @param offset Offset in bytes in the shared memory (default is zero)
    * @param format Base data size can be EB_DATA8, EB_DATA16, EB_ADDR32 or
    *               EB_ADDR64 defined in "etherbone.h" \n
    *               Default is EB_DATA8.
    */
   void readLM32( eb_user_data_t pData,
                  const std::size_t len,
                  const std::size_t offset = 0,
                  const etherbone::format_t format = EB_DATA8 )
   {
      m_poEb->read( m_lm32SharedMemAddr + offset, pData,
                    EB_BIG_ENDIAN | format,
                    len );
   }


   /*!
    * @brief Writes data in the LM32 shared memory area.
    * @note In this case a homogeneous data object is provided so
    *       the etherbone-library cant convert that in to big endian.\n
    *       Therefore in the case of the sending of heterogeneous data e.g.
    *       structures, the conversion in little endian has to be made
    *       in upper software layers at first.
    * @param pData Source address of the data to copy
    * @param len   Data length (array size)
    * @param offset Offset in bytes in the shared memory (default is zero)
    * @param format Base data size can be EB_DATA8, EB_DATA16, EB_ADDR32 or
    *               EB_ADDR64 defined in "etherbone.h" \n
    *               Default is EB_DATA8.
    */
   void writeLM32( const eb_user_data_t pData,
                   const std::size_t len,
                   const std::size_t offset = 0,
                   const etherbone::format_t format = EB_DATA8 )
   {
      m_poEb->write( m_lm32SharedMemAddr + offset, pData,
                     EB_BIG_ENDIAN | format,
                     len );
   }
};

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQ_EB_RAM_BUFFER_HPP
//================================== EOF ======================================
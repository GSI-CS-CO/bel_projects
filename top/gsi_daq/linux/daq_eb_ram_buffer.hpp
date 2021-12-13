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

#include <scu_control_config.h>
#include <EtherboneConnection.hpp>
#include <scu_assert.h>
#ifndef CONFIG_NO_SCU_RAM
 #include <daq_ramBuffer.h>
#endif
#include <daq_calculations.hpp>


// ... a little bit paranoia, I know ... ;-)
static_assert( EB_DATA8  == sizeof(uint8_t),  "" );
static_assert( EB_DATA16 == sizeof(uint16_t), "" );
static_assert( EB_DATA32 == sizeof(uint32_t), "" );
static_assert( EB_DATA64 == sizeof(uint64_t), "" );

namespace DaqEb = FeSupport::Scu::Etherbone;

namespace Scu
{
namespace daq
{

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Interface class for SCU DDR3 RAM access via the class
 *        EtherboneConnection
 */
class EbRamAccess
{
   DaqEb::EtherboneConnection* m_poEb;
   bool                        m_connectedBySelf;
   uint                        m_lm32SharedMemAddr;

#ifndef CONFIG_NO_SCU_RAM
 #ifdef CONFIG_SCU_USE_DDR3
   uint                        m_ddr3TrModeBase;
 #ifndef CONFIG_DDR3_NO_BURST_FUNCTIONS
   uint                        m_ddr3BurstModeBase;
 #endif
 #endif
#endif

#ifdef CONFIG_EB_TIME_MEASSUREMENT
public:
    struct TIME_MEASUREMENT_T
    {
       enum WB_ACCESS_T
       {
          UNKNOWN    = 0,
          LM32_READ  = 1,
          LM32_WRITE = 2,
          DDR3_READ  = 3
       };

       USEC_T      m_duration;
       USEC_T      m_timestamp;
       std::size_t m_dataSize;
       WB_ACCESS_T m_eAccess;

       TIME_MEASUREMENT_T( const USEC_T duration )
        :m_duration( duration )
        ,m_timestamp( 0 )
        ,m_dataSize( 0 )
        ,m_eAccess( UNKNOWN ) {}
    };

    struct MAX_DURATION: public TIME_MEASUREMENT_T
    {
       MAX_DURATION( void ): TIME_MEASUREMENT_T( 0 ) {}
    };

    struct MIN_DURATION: public TIME_MEASUREMENT_T
    {
       MIN_DURATION( void ): TIME_MEASUREMENT_T( static_cast<USEC_T>(~0) ) {}
    };

    using WB_ACCESS_T = TIME_MEASUREMENT_T::WB_ACCESS_T;

private:
    MAX_DURATION               m_oMaxDuration;
    MIN_DURATION               m_oMinDuration;
    USEC_T                     m_startTime;
#endif /* #ifdef CONFIG_EB_TIME_MEASSUREMENT */

public:

   /*!
    * @brief Constructor establishes the etherbone connection if it's not
    *        already been done outside.
    * @param poEb Pointer to the object of type EtherboneConnection.
    */
   EbRamAccess( DaqEb::EtherboneConnection* poEb );

   /*!
    * @brief Destructor terminates the ehtherbone connection if the connection was made
    *        by this object.
    */
   ~EbRamAccess( void );

   /*!
    * @brief Returns the pointer of the etherbone connection object of type:
    *        EtherboneConnection:
    */
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

   /*!
    * @brief Returns the IP address or - when it runs inside of a SCU - the
    *        wishbone name without the prefix "tcp/" or "dev/"
    */
   const std::string getScuDomainName( void )
   {
      return getNetAddress().substr( getNetAddress().find_first_of( '/' ) + 1 );
   }

   /*!
    * @brief Returns "true" when the etherbone/wishbone conection has been
    *        established.
    */
   bool isConnected( void ) const
   {
      return m_poEb->isConnected();
   }

#ifdef CONFIG_EB_TIME_MEASSUREMENT
private:

   void startTimeMeasurement( void )
   {
      m_startTime = getSysMicrosecs();
   }

   void stopTimeMeasurement( const std::size_t size, const WB_ACCESS_T access );

public:

   WB_ACCESS_T getWbMeasurementMaxTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize );
   WB_ACCESS_T getWbMeasurementMinTime( USEC_T& rTimestamp, USEC_T& rDuration, std::size_t& rSize );
#endif /* ifdef CONFIG_EB_TIME_MEASSUREMENT */

#ifndef CONFIG_NO_SCU_RAM
   /*!
    * @brief Reads data from the SCU-RAM.
    * @note At the time it's the DDR3-RAM yet!
    * @param pData Pointer to the destination buffer of type RAM_DAQ_PAYLOAD_T.
    * @param len Number of RAM-items of type RAM_DAQ_PAYLOAD_T to read.
    * @param offset Offset in RAM_DAQ_PAYLOAD_T units.
    */
   void readRam( RAM_DAQ_PAYLOAD_T* pData, const std::size_t len, const uint offset )
   {
      SCU_ASSERT( m_poEb->isConnected() );
      SCU_ASSERT( m_ddr3TrModeBase != 0 );
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      startTimeMeasurement();
   #endif
      m_poEb->read( m_ddr3TrModeBase + offset * sizeof(RAM_DAQ_PAYLOAD_T),
                    pData,
                    sizeof( pData->ad32[0] ) | EB_LITTLE_ENDIAN,
                    len * ARRAY_SIZE( pData->ad32 ) );
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      stopTimeMeasurement( len * sizeof( pData->ad32 ), TIME_MEASUREMENT_T::DDR3_READ );
   #endif
   }

   /*!
    * @brief Reads circular administrated data from the SCU-RAM.
    * @note At the time it's the DDR3-RAM yet!
    * @param pData Pointer to the destination buffer of type RAM_DAQ_PAYLOAD_T.
    * @param len Number of RAM-items of type RAM_DAQ_PAYLOAD_T to read.
    * @param rIndexes Ring buffer administrator object.
    */
   void readRam( RAM_DAQ_PAYLOAD_T* pData, std::size_t len, RAM_RING_INDEXES_T& rIndexes );

#endif /* ifndef CONFIG_NO_SCU_RAM */

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
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      startTimeMeasurement();
   #endif
      m_poEb->read( m_lm32SharedMemAddr + offset, pData,
                    EB_BIG_ENDIAN | format,
                    len );
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      stopTimeMeasurement( len * (format & 0xFF), TIME_MEASUREMENT_T::LM32_READ );
   #endif
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
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      startTimeMeasurement();
   #endif
      m_poEb->write( m_lm32SharedMemAddr + offset, pData,
                     EB_BIG_ENDIAN | format,
                     len );
   #ifdef CONFIG_EB_TIME_MEASSUREMENT
      stopTimeMeasurement( len * (format & 0xFF), TIME_MEASUREMENT_T::LM32_WRITE );
   #endif
   }
};

} // namespace daq
} // namespace Scu
#endif // ifndef _DAQ_EB_RAM_BUFFER_HPP
//================================== EOF ======================================
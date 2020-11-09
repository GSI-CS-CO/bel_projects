/*!
 *  @file daq_interface.hpp
 *  @brief DAQ Interface Library for Linux
 *
 *  @date 28.02.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _DAQ_INTERFACE_HPP
#define _DAQ_INTERFACE_HPP

#include <string.h>
#include <daq_base_interface.hpp>

#include <daq_command_interface.h>
#include <scu_bus_defines.h>
#include <daq_ramBuffer.h>
#include <daq_descriptor.h>
//#include <stddef.h>
//#include <string>
#include <daq_eb_ram_buffer.hpp>
#include <daq_calculations.hpp>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

#define DAQ_ERR_PROGRAM                     -100
#define DAQ_ERR_RESPONSE_TIMEOUT            -101

#define DAQ_ASSERT_CHANNEL_ACCESS( deviceNumber, channel ) \
{                                                          \
   SCU_ASSERT( deviceNumber > 0 );                         \
   SCU_ASSERT( deviceNumber <= c_maxDevices );             \
   SCU_ASSERT( channel > 0 );                              \
   SCU_ASSERT( channel <= c_maxChannels );                 \
}

namespace Scu
{
namespace daq
{

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts the status number in to the defined string.
 */
const std::string status2String( DAQ_RETURN_CODE_T status );

/*!
 * @ingroup DAQ
 * @defgroup DAQ_EXCEPTION
 * @brief Error exceptions in communication with the LM32 firmware.
 * @{
 */

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Exception class for wishbone / etherbone communication errors.
 */
class EbException: public Exception
{
public:
   EbException( const std::string& rMsg ):
      Exception( "Etherbone: " + rMsg ) {}
};

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Exception class for error returns of the DAQ LM32 firmware.
 */
class DaqException: public Exception
{
   const DAQ_RETURN_CODE_T  m_daqStatus;

public:
   DaqException( const std::string& rMsg,
                 const DAQ_RETURN_CODE_T status = DAQ_ERR_PROGRAM )
      :Exception( "DAQ: " + rMsg )
      ,m_daqStatus( status ) {}

   const DAQ_RETURN_CODE_T getStatus( void )
   {
      return m_daqStatus;
   }

   const std::string getStatusString( void )
   {
      return status2String( getStatus() );
   }
};

/*! @} */

///////////////////////////////////////////////////////////////////////////////
class DaqInterface: public DaqBaseInterface
{
   constexpr static uint        INVALID_OFFSET = static_cast<uint>(~0);

public:
   using SLOT_FLAGS_T  = SCUBUS_SLAVE_FLAGS_T;
   using RETURN_CODE_T = DAQ_RETURN_CODE_T;

private:
   DAQ_SHARED_IO_T              m_oSharedData;
   SLOT_FLAGS_T                 m_slotFlags;
   uint                         m_maxDevices;
   DAQ_LAST_STATUS_T            m_lastStatus;
   const bool                   m_doReset;
   const bool                   m_doSendCommand;
   uint                         m_daqLM32Offset;

protected:
   RAM_SCU_T                    m_oScuRam;

   /*!
    * @brief Response timeout for LM32 commands in milliseconds.
    */
   constexpr static USEC_T       c_LM32CommandResponseTimeout
                                   = MICROSECS_PER_SEC / 10;

public:
   constexpr static uint         c_maxDevices        = DAQ_MAX;
   constexpr static uint         c_maxChannels       = DAQ_MAX_CHANNELS;
   constexpr static std::size_t  c_ramBlockShortLen  = RAM_DAQ_SHORT_BLOCK_LEN;
   constexpr static std::size_t  c_ramBlockLongLen   = RAM_DAQ_LONG_BLOCK_LEN;
   constexpr static std::size_t  c_hiresPmDataLen    =
                                               DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC;
   constexpr static std::size_t  c_contineousDataLen =
                                                    DAQ_FIFO_DAQ_WORD_SIZE_CRC;
   constexpr static std::size_t  c_discriptorWordSize =
                                                      DAQ_DESCRIPTOR_WORD_SIZE;
   constexpr static std::size_t  c_contineousPayloadLen =
                                    c_contineousDataLen - c_discriptorWordSize;
   constexpr static std::size_t  c_pmHiresPayloadLen =
                                       c_hiresPmDataLen - c_discriptorWordSize;
// --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --  --
   DaqInterface( DaqEb::EtherboneConnection* poEtherbone,
                 const bool doReset = true,
                 const bool doSendCommand = true
               );

   DaqInterface( EbRamAccess* poEbAccess,
                 const bool doReset = true,
                 const bool doSendCommand = true
               );

   ~DaqInterface( void ) override;

   /*!
    * @brief Returns "true" if the LM32 firmware supports ADDAC/ACU DAQs.
    */
   bool isAddacDaqSupport( void ) const
   {
      return m_daqLM32Offset != INVALID_OFFSET;
   }

   /*!
    * @brief Returns true when the command sending to LM32 is enabled.
    *
    * It's the value of the constructors third parameter.
    */
   const bool isLM32CommandEnabled( void ) const
   {
      return m_doSendCommand;
   }

   RETURN_CODE_T getLastReturnCode( void ) const
   {
      return m_oSharedData.operation.retCode;
   }

   const std::string getLastReturnCodeString( void );

   /*!
    * @brief Indicator returns true when the in the LM32 runs concurrent to
    *        the DAQ application the FG-application.
    * @retval true FG functions available.
    * @retval false Single DAQ application.
    */
   bool isFgIntegrated( void ) const;

   RETURN_CODE_T readSlotStatus( void );
   SLOT_FLAGS_T  getSlotStatus( void ) const
   {
      return m_slotFlags;
   }

   uint getMaxFoundDevices( void ) const
   {
      return m_maxDevices;
   }

   bool isDevicePresent( const uint slot ) const
   {
      return Bus::scuBusIsSlavePresent( m_slotFlags, slot );
   }

   void sendReset( void )
   {
      sendUnlockRamAccess();
      sendCommand( DAQ_OP_RESET );
   }

   bool isDoReset( void ) const
   {
      return m_doReset;
   }

   DAQ_LAST_STATUS_T readLastStatus( void );

   const std::string getLastStatusString( void );

   DAQ_LAST_STATUS_T getLastStatus( void ) const
   {
      return m_lastStatus;
   }

   uint readMacroVersion( const uint deviceNumber );

   uint getSlotNumber( const uint deviceNumber );

   uint getDeviceNumber( const uint slotNumber );

   uint readMaxChannels( const uint deviceNumber );

   DAQ_DEVICE_TYP_T readDeviceType( const uint deviceNumber );

   int sendEnablePostMortem( const uint deviceNumber,
                             const uint channel,
                             const bool restart = false
                           );

   int sendEnableHighResolution( const uint deviceNumber,
                                 const uint channel,
                                 const bool restart = false
                               );

   int sendDisablePmHires( const uint deviceNumber,
                           const uint channel,
                           const bool restart = false
                         );

   int sendEnableContineous( const uint deviceNumber,
                             const uint channel,
                             const DAQ_SAMPLE_RATE_T sampleRate,
                             const uint maxBlocks = 0
                           );

   int sendDisableContinue( const uint deviceNumber,
                            const uint channel );


   int sendTriggerCondition( const uint deviceNumber,
                             const uint channel,
                             const uint32_t trgCondition );

   uint32_t receiveTriggerCondition( const uint deviceNumber,
                                     const uint channel );


   int sendTriggerDelay( const uint deviceNumber,
                         const uint channel,
                         const uint16_t delay );

   uint16_t receiveTriggerDelay( const uint deviceNumber,
                                 const uint channel );


   int sendTriggerMode( const uint deviceNumber,
                        const uint channel,
                        const bool mode );

   bool receiveTriggerMode( const uint deviceNumber,
                            const uint channel );

   int sendTriggerSourceContinue( const uint deviceNumber,
                                  const uint channel,
                                  const bool extInput );

   bool receiveTriggerSourceContinue( const uint deviceNumber,
                                      const uint channel );

   int sendTriggerSourceHiRes( const uint deviceNumber,
                               const uint channel,
                               const bool extInput );

   bool receiveTriggerSourceHiRes( const uint deviceNumber,
                                   const uint channel );



   /*!
    * @brief Returns the number of items which are currently in the
    *        data buffer.
    * @param update If true the indexes in the LM32 shared memory
    *               will read before.
    */
   RAM_RING_INDEX_T getCurrentRamSize( bool update = true ) override;

   /*!
    * @brief Makes the data buffer empty.
    * @param update If true the indexes in the LM32 shared memory
    *               becomes updated.
    */
   void clearBuffer( bool update = true ) override;

protected:
   void checkAddacSupport( void );

   virtual bool onCommandReadyPoll( USEC_T pollCount );

   void readLM32( eb_user_data_t pData,
                  const std::size_t len,
                  const std::size_t offset = 0,
                  const etherbone::format_t format = EB_DATA8
                )
   {
      assert( m_daqLM32Offset != INVALID_OFFSET );
      getEbAccess()->readLM32( pData, len, offset + m_daqLM32Offset, format );
   }

   void writeLM32( const eb_user_data_t pData,
                   const std::size_t len,
                   const std::size_t offset = 0,
                   const etherbone::format_t format = EB_DATA8 )
   {
      assert( m_daqLM32Offset != INVALID_OFFSET );
      getEbAccess()->writeLM32( pData, len, offset + m_daqLM32Offset, format );
   }


#ifdef CONFIG_DAQ_TEST
   void clearData( void )
   {
      ::memset( &m_oSharedData.operation, 0,
                sizeof( m_oSharedData.operation ));
   }
#endif

   void setLocation( const uint deviceNumber,
                     const uint channel )
   {
#ifdef CONFIG_DAQ_TEST
      clearData();
#endif
      m_oSharedData.operation.ioData.location.deviceNumber = deviceNumber;
      m_oSharedData.operation.ioData.location.channel      = channel;
   }

   void sendLockRamAccess( void )
   {
      sendCommand( DAQ_OP_LOCK );
   }

   void sendUnlockRamAccess( void );

   void writeRamIndexesAndUnlock( void );

   virtual void onBlockReceiveError( void );

private:
   void init( void );
   void probe( void );
   bool cmdReadyWait( void );
   void readSharedTotal( void );
   bool permitCommand( DAQ_OPERATION_CODE_T );
   RETURN_CODE_T sendCommand( DAQ_OPERATION_CODE_T );
   DAQ_OPERATION_CODE_T getCommand( void );

   RETURN_CODE_T readParam1( void );
   RETURN_CODE_T readParam12( void );
   RETURN_CODE_T readParam123( void );
   RETURN_CODE_T readParam1234( void );

   RETURN_CODE_T readRamIndexes( void );

   void writeParam1( void );
   void writeParam12( void );
   void writeParam123( void );
   void writeParam1234( void );

}; // end class DaqInterface

} //namespace daq
} //namespace Scu

#endif //ifndef _DAQ_INTERFACE_HPP
//================================== EOF ======================================

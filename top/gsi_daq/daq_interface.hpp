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

#include <daq_command_interface.h>
#include <scu_bus_defines.h>
#include <daq_ramBuffer.h>
#include <daq_descriptor.h>
#include <stddef.h>
#include <string>
#include <exception>

#include <eb_object_transfer.h>

#ifndef CONFIG_NO_FE_ETHERBONE_CONNECTION
  #include <daq_eb_ram_buffer.hpp>
#endif

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


#ifndef DAQ_VSS_MAX
   #define DAQ_VSS_MAX 20.0
#endif

namespace Scu
{
namespace daq
{


/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts raw data of the DAQ ADC in to voltage.
 *  (Nice to have function)
 * @param rawData raw data from the DAQ ADC.
 * @return Voltage in the range 0.0V to 10.0V
 */
inline float rawToVoltage( DAQ_DATA_T rawData )
{
   return (static_cast<float>(static_cast<int16_t>(rawData)) * DAQ_VSS_MAX) /
             static_cast<float>(static_cast<DAQ_DATA_T>(~0));
}

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
 * @brief Base exception class for all DAQ- exceptions
 */
class Exception: public std::exception
{
   const std::string m_message;

public:
   Exception( const std::string& rMsg ):
      m_message( rMsg ) {}

   const char* what( void ) const noexcept override
   {
      return m_message.c_str();
   }
};

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
class DaqInterface
{
   typedef eb_status_t          EB_STATUS_T;

public:
   typedef SCUBUS_SLAVE_FLAGS_T SLOT_FLAGS_T;
   typedef DAQ_RETURN_CODE_T    RETURN_CODE_T;

private:
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   const std::string            m_wbDevice;
   EB_HANDLE_T                  m_oEbHandle;
   EB_HANDLE_T*                 m_poEbHandle;
#else
protected:
   EbRamAccess                  m_oEbAccess;
private:
   bool                         m_connectedBySelf;
#endif
   DAQ_SHARED_IO_T              m_oSharedData;
   SLOT_FLAGS_T                 m_slotFlags;
   unsigned int                 m_maxDevices;
   DAQ_LAST_STATUS_T            m_lastStatus;
   const bool                   m_doReset;

protected:
   RAM_SCU_T                    m_oScuRam;

   constexpr static unsigned int c_maxCmdPoll  = 200;

public:
   constexpr static unsigned int c_maxDevices
             = DAQ_MAX;
   constexpr static unsigned int c_maxSlots
             = MAX_SCU_SLAVES;
   constexpr static unsigned int c_startSlot
             = SCUBUS_START_SLOT;
   constexpr static unsigned int c_maxChannels
             = DAQ_MAX_CHANNELS;
   constexpr static std::size_t  c_ramBlockShortLen
             = RAM_DAQ_SHORT_BLOCK_LEN;
   constexpr static std::size_t  c_ramBlockLongLen
             = RAM_DAQ_LONG_BLOCK_LEN;
   constexpr static std::size_t  c_hiresPmDataLen
             = DAQ_FIFO_PM_HIRES_WORD_SIZE_CRC;
   constexpr static std::size_t  c_contineousDataLen
             = DAQ_FIFO_DAQ_WORD_SIZE_CRC;
   constexpr static std::size_t  c_discriptorWordSize
             = DAQ_DESCRIPTOR_WORD_SIZE;
   constexpr static std::size_t  c_contineousPayloadLen
             = c_contineousDataLen - c_discriptorWordSize;
   constexpr static std::size_t  c_pmHiresPayloadLen
             = c_hiresPmDataLen - c_discriptorWordSize;

#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   DaqInterface( const std::string = DAQ_DEFAULT_WB_DEVICE, bool doReset = true );
#else
   DaqInterface( DaqEb::EtherboneConnection* poEtherbone, bool doReset = true );
#endif

   virtual ~DaqInterface( void );


   const std::string& getWbDevice( void )
   {
   #ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
      return m_wbDevice;
   #else
      return m_oEbAccess.getEbObjectPtr()->getNetAddress();
   #endif
   }

   const std::string getScuDomainName( void )
   {
      return getWbDevice().substr( getWbDevice().find_first_of( '/' ) + 1 );
   }

   const std::string getEbStatusString( void ) const
   {
   #ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
      return static_cast<const std::string>(ebGetStatusString( m_poEbHandle ));
   #else
      return static_cast<const std::string>("Noch nix");
   #endif
   }

   RETURN_CODE_T getLastReturnCode( void ) const
   {
      return m_oSharedData.operation.retCode;
   }

   const std::string getLastReturnCodeString( void );


   RETURN_CODE_T readSlotStatus( void );
   SLOT_FLAGS_T  getSlotStatus( void ) const
   {
      return m_slotFlags;
   }

   unsigned int getMaxFoundDevices( void ) const
   {
      return m_maxDevices;
   }

   bool isDevicePresent( const unsigned int slot ) const
   {
      return scuBusIsSlavePresent( m_slotFlags, slot );
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

   unsigned int readMacroVersion( const unsigned int deviceNumber );

   unsigned int getSlotNumber( const unsigned int deviceNumber );

   unsigned int getDeviceNumber( const unsigned int slotNumber );

   unsigned int readMaxChannels( const unsigned int deviceNumber );

   int sendEnablePostMortem( const unsigned int deviceNumber,
                             const unsigned int channel,
                             const bool restart = false
                           );

   int sendEnableHighResolution( const unsigned int deviceNumber,
                                 const unsigned int channel,
                                 const bool restart = false
                               );

   int sendDisablePmHires( const unsigned int deviceNumber,
                           const unsigned int channel,
                           const bool restart = false
                         );

   int sendEnableContineous( const unsigned int deviceNumber,
                             const unsigned int channel,
                             const DAQ_SAMPLE_RATE_T sampleRate,
                             const unsigned int maxBlocks = 0
                           );

   int sendDisableContinue( const unsigned int deviceNumber,
                            const unsigned int channel );


   int sendTriggerCondition( const unsigned int deviceNumber,
                             const unsigned int channel,
                             const uint32_t trgCondition );

   uint32_t receiveTriggerCondition( const unsigned int deviceNumber,
                                     const unsigned int channel );


   int sendTriggerDelay( const unsigned int deviceNumber,
                         const unsigned int channel,
                         const uint16_t delay );

   uint16_t receiveTriggerDelay( const unsigned int deviceNumber,
                                 const unsigned int channel );


   int sendTriggerMode( const unsigned int deviceNumber,
                        const unsigned int channel,
                        const bool mode );

   bool receiveTriggerMode( const unsigned int deviceNumber,
                            const unsigned int channel );

   int sendTriggerSourceContinue( const unsigned int deviceNumber,
                          const unsigned int channel,
                          const bool extInput );

   bool receiveTriggerSourceContinue( const unsigned int deviceNumber,
                                      const unsigned int channel );

   int sendTriggerSourceHiRes( const unsigned int deviceNumber,
                               const unsigned int channel,
                               const bool extInput );

   bool receiveTriggerSourceHiRes( const unsigned int deviceNumber,
                                   const unsigned int channel );


   RAM_RING_INDEX_T getCurrentRamSize( bool update = true );

protected:
   virtual bool onCommandReadyPoll( unsigned int pollCount );

#ifdef CONFIG_DAQ_TEST
   void clearData( void )
   {
      ::memset( &m_oSharedData.operation, 0,
                sizeof( m_oSharedData.operation ));
   }
#endif

   void setLocation( const unsigned int deviceNumber,
                     const unsigned int channel )
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
   void clearBuffer( bool update = true );
   void writeRamIndexesAndUnlock( void );

   virtual void onBlockReceiveError( void );

private:
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   EB_STATUS_T ebSocketRun( void )
   {
      return Scu::ebSocketRun( m_poEbHandle );
   }

   void ebClose( void );

   EB_STATUS_T ebReadObjectCycleOpen( EB_CYCLE_OR_CB_ARG_T& rCArg )
   {
      return Scu::ebObjectReadCycleOpen( m_poEbHandle, &rCArg );
   }

   EB_STATUS_T ebWriteObjectCycleOpen( EB_CYCLE_OW_CB_ARG_T& rCArg )
   {
      return Scu::ebObjectWriteCycleOpen( m_poEbHandle, &rCArg );
   }

   void ebCycleClose( void )
   {
      Scu::ebCycleClose( m_poEbHandle );
   }
#endif
   bool cmdReadyWait( void );
   void readSharedTotal( void );
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

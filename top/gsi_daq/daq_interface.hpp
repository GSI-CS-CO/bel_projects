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
#include <scu_ramBuffer.h>
#include <daq_descriptor.h>
#include <eb_object_transfer.h>
#include <string>
#include <exception>

#ifndef DAQ_DEFAULT_WB_DEVICE
   #define DAQ_DEFAULT_WB_DEVICE "dev/wbm0"
#endif

namespace daq
{

///////////////////////////////////////////////////////////////////////////////
class Exception
{
   const std::string m_message;

public:
   Exception( const std::string msg ):
      m_message( msg ) {}

   const std::string& what( void ) const
   {
      return m_message;
   }
};

///////////////////////////////////////////////////////////////////////////////
class DaqInterface
{
   typedef eb_status_t      EB_STATUS_T;
public:
   typedef SCUBUS_SLAVE_FLAGS_T SLOT_FLAGS_T;
   typedef DAQ_RETURN_CODE_T    RETURN_CODE_T;

private:
   const std::string        m_wbDevice;
   RAM_SCU_T                m_oScuRam;
   EB_HANDLE_T              m_oEbHandle;
   EB_HANDLE_T*             m_poEbHandle;
   DAQ_SHARED_IO_T          m_oSharedData;
   SLOT_FLAGS_T             m_slotFlags;
   unsigned int             m_maxDevices;

protected:
   constexpr static unsigned int c_maxCmdPoll = 100;

public:
   constexpr static unsigned int c_maxDevices   = DAQ_MAX;
   constexpr static unsigned int c_maxSlots     = MAX_SCU_SLAVES;
   constexpr static unsigned int c_startSlot    = SCUBUS_START_SLOT;
   constexpr static unsigned int c_maxChannels  = DAQ_MAX_CHANNELS;


   DaqInterface( const std::string = DAQ_DEFAULT_WB_DEVICE );

   virtual ~DaqInterface( void );

   const std::string& getWbDevice( void ) const { return m_wbDevice; }

   const std::string getEbStatusString( void ) const
   {
      static_cast<const std::string>(::ebGetStatusString( m_poEbHandle ));
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

   bool isDevicePresent( const unsigned int slot )
   {
      return ::scuBusIsSlavePresent( m_slotFlags, slot );
   }

   unsigned int getSlotNumber( const unsigned int deviceNumber );

   int readMaxChannels( const unsigned int deviceNumber,
                        unsigned int& rMaxChannels );

   int enablePostMortem( const unsigned int deviceNumber,
                         const unsigned int channel );

   int enableHighResolution( const unsigned int deviceNumber,
                             const unsigned int channel );

   int enableContineous( const unsigned int deviceNumber,
                         const unsigned int channel,
                         const DAQ_SAMPLE_RATE_T sampleRate );

   int disable( const unsigned int deviceNumber,
                const unsigned int channel );


   int setTriggerCondition( const unsigned int deviceNumber,
                            const unsigned int channel,
                            const uint32_t trgCondition );

   int getTriggerCondition( const unsigned int deviceNumber,
                            const unsigned int channel,
                            uint32_t& rTrgCondition );


   int setTriggerDelay( const unsigned int deviceNumber,
                        const unsigned int channel,
                        const uint16_t delay );

   int getTriggerDelay( const unsigned int deviceNumber,
                        const unsigned int channel,
                        uint16_t& rDelay );


   int setTriggerMode( const unsigned int deviceNumber,
                       const unsigned int channel,
                       const bool mode );

   int getTriggerMode( const unsigned int deviceNumber,
                       const unsigned int channel,
                       bool& rMode );

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

   void ramAddToReadIndex( RAM_RING_INDEX_T toAdd, bool update = true );

private:

   EB_STATUS_T ebSocketRun( void )
   {
      return ::ebSocketRun( m_poEbHandle );
   }

   void ebClose( void );

   EB_STATUS_T ebReadObjectCycleOpen( EB_CYCLE_OR_CB_ARG_T& rCArg )
   {
      return ::ebObjectReadCycleOpen( m_poEbHandle, &rCArg );
   }

   EB_STATUS_T ebWriteObjectCycleOpen( EB_CYCLE_OW_CB_ARG_T& rCArg )
   {
      return ::ebObjectWriteCycleOpen( m_poEbHandle, &rCArg );
   }

   void ebCycleClose( void )
   {
      ::ebCycleClose( m_poEbHandle );
   }

   bool cmdReadyWait( void );
   void readSharedTotal( void );
   RETURN_CODE_T setCommand( DAQ_OPERATION_CODE_T );
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

   void writeRamIndexes( void );
}; // end class DaqInterface

} //namespace daq

#endif //ifndef _DAQ_INTERFACE_HPP
//================================== EOF ======================================

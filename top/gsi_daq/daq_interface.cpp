/*!
 *  @file daq_interface.cpp
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
#include <daq_interface.hpp>
#include <sys/select.h>
#include <helper_macros.h>
using namespace daq;

#define FUNCTION_NAME_TO_STD_STRING static_cast<const std::string>(__func__)

#define __THROW_EB_EXCEPTION() \
   throw Exception( FUNCTION_NAME_TO_STD_STRING + "(): "\
   + static_cast<const std::string>(::ebGetStatusString( m_poEbHandle )) )

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 * @brief Constructor of class daq::DaqInterface
 */
DaqInterface::DaqInterface( const std::string wbDevice )
   :m_wbDevice( wbDevice )
   ,m_poEbHandle( nullptr )
   ,m_slotFlags( 0 )
   ,m_maxDevices( 0 )
{
   if( ::ebOpen( &m_oEbHandle, m_wbDevice.c_str() ) != EB_OK )
      throw Exception( ::ebGetStatusString( &m_oEbHandle ) );

   m_poEbHandle = &m_oEbHandle;

   if( ::ramInit( &m_oScuRam, &m_oSharedData.ramIndexes, m_poEbHandle ) < 0 )
   {
      ebClose();
      throw( Exception( "Could not find RAM-device!" ) );
   }

   readSharedTotal();
   setCommand( DAQ_OP_RESET );
   readSlotStatus();
}

/*! ---------------------------------------------------------------------------
 * @brief Destructor of class daq::DaqInterfaceInterface
 */
DaqInterface::~DaqInterface( void )
{
   ebClose();
}

/*! ---------------------------------------------------------------------------
 */
const std::string DaqInterface::getLastReturnCodeString( void )
{
   #define __RET_CODE_CASE_ITEM( name ) case name: return #name
   switch( getLastReturnCode() )
   {
      __RET_CODE_CASE_ITEM( DAQ_RET_RESCAN );
      __RET_CODE_CASE_ITEM( DAQ_RET_OK );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_UNKNOWN_OPERATION );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_SLAVE_NOT_PRESENT );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_CHANNEL_NOT_PRESENT );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_DEVICE_ADDRESS_NOT_FOUND );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_CHANNEL_OUT_OF_RANGE );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_SLAVE_OUT_OF_RANGE );
      __RET_CODE_CASE_ITEM( DAQ_RET_ERR_WRONG_SAMPLE_PARAMETER );
   }
   return "unknown";
   #undef __RET_CODE_CASE_ITEM
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::ebClose( void )
{
   if( m_poEbHandle == nullptr )
      return;

   if( ::ebClose( m_poEbHandle ) != EB_OK )
     __THROW_EB_EXCEPTION();

   m_poEbHandle = nullptr;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readSharedTotal( void )
{
   EB_MEMBER_INFO_T info[7];
   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.magicNumber );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.ramIndexes.ringIndexes.offset );
   EB_INIT_INFO_ITEM_STATIC( info, 2, m_oSharedData.ramIndexes.ringIndexes.capacity );
   EB_INIT_INFO_ITEM_STATIC( info, 3, m_oSharedData.ramIndexes.ringIndexes.start );
   EB_INIT_INFO_ITEM_STATIC( info, 4, m_oSharedData.ramIndexes.ringIndexes.end );
   EB_INIT_INFO_ITEM_STATIC( info, 5, m_oSharedData.operation.code );
   EB_INIT_INFO_ITEM_STATIC( info, 6, m_oSharedData.operation.retCode );

   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, magicNumber );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.offset );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.capacity );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.start );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.end );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.code );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   if( m_oSharedData.magicNumber != DAQ_MAGIC_NUMBER )
      throw Exception( "Wrong DAQ magic number" );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::onCommandReadyPoll( unsigned int pollCount )
{
   if( pollCount >= c_maxCmdPoll )
      return true;

   struct timeval sleepTime = {0, 1};
   ::select( 0, nullptr, nullptr, nullptr, &sleepTime );

   return false;
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::cmdReadyWait( void )
{
   unsigned int pollCount = 0;
   while( getCommand() != DAQ_OP_IDLE )
   {
      if( onCommandReadyPoll( pollCount ) )
         return true;
      pollCount++;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::setCommand( DAQ_OPERATION_CODE_T cmd )
{
   m_oSharedData.operation.code = cmd;
   EB_MAKE_CB_OW_ARG( cArg );

    if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
       __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData, operation.code );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   if( cmdReadyWait() )
      throw Exception( "Timeout at waiting for command feedback" );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DAQ_OPERATION_CODE_T DaqInterface::getCommand( void )
{
   EB_MEMBER_INFO_T info[2];

   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.operation.code );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.operation.retCode );
   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.code );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   return m_oSharedData.operation.code;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1( void )
{
   EB_MEMBER_INFO_T info[2];

   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.operation.retCode );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.operation.ioData.param1 );
   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param1 );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam12( void )
{
   EB_MEMBER_INFO_T info[3];

   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.operation.retCode );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.operation.ioData.param1 );
   EB_INIT_INFO_ITEM_STATIC( info, 2, m_oSharedData.operation.ioData.param2 );
   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param1 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param2 );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam123( void )
{
   EB_MEMBER_INFO_T info[4];

   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.operation.retCode );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.operation.ioData.param1 );
   EB_INIT_INFO_ITEM_STATIC( info, 2, m_oSharedData.operation.ioData.param2 );
   EB_INIT_INFO_ITEM_STATIC( info, 3, m_oSharedData.operation.ioData.param3 );
   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param1 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param2 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param3 );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();


   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1234( void )
{
   EB_MEMBER_INFO_T info[5];

   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.operation.retCode );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.operation.ioData.param1 );
   EB_INIT_INFO_ITEM_STATIC( info, 2, m_oSharedData.operation.ioData.param2 );
   EB_INIT_INFO_ITEM_STATIC( info, 3, m_oSharedData.operation.ioData.param3 );
   EB_INIT_INFO_ITEM_STATIC( info, 4, m_oSharedData.operation.ioData.param4 );
   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.retCode );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param1 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param2 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param3 );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, operation.ioData.param4 );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readRamIndexes( void )
{
   EB_MEMBER_INFO_T info[2];
   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.ramIndexes.ringIndexes.start );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.ramIndexes.ringIndexes.end );

   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.start );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T, ramIndexes.ringIndexes.end );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1( void )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.deviceNumber );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.channel );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param1 );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam12( void )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.deviceNumber );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.channel );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param1 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param2 );

   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam123( void )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.deviceNumber );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.channel );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param1 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param2 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param3 );

   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1234( void )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.deviceNumber );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.location.channel );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param1 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param2 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param3 );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               operation.ioData.param4 );

   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeRamIndexes( void )
{
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               ramIndexes.ringIndexes.start );
   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               ramIndexes.ringIndexes.end );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
}

/*! ---------------------------------------------------------------------------
 */
RAM_RING_INDEX_T DaqInterface::getCurrentRamSize( bool update )
{
   if( update )
      readRamIndexes();
   return ::ramRingGetSize( &m_oSharedData.ramIndexes.ringIndexes );
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::ramAddToReadIndex( RAM_RING_INDEX_T toAdd, bool update )
{
   ::ramRingAddToReadIndex( &m_oSharedData.ramIndexes.ringIndexes, toAdd );
   if( update )
      writeRamIndexes();
}

/*! ---------------------------------------------------------------------------
 */
unsigned int DaqInterface::getSlotNumber( const unsigned int deviceNumber )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );

   unsigned int i = 0;
   for( unsigned int slot = 1; slot <= c_maxSlots; slot++ )
   {
      if( isDevicePresent( slot ) )
         i++;
      if( i == deviceNumber )
         return slot;
   }
   return 0;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readSlotStatus( void )
{
   setCommand( DAQ_OP_GET_SLOTS );
   readParam1();
   if( m_oSharedData.operation.retCode == DAQ_RET_OK )
   {
      m_slotFlags = m_oSharedData.operation.ioData.param1;
      m_maxDevices = 0;
      for( unsigned int slot = 1; slot <= c_maxSlots; slot++ )
      {
         if( isDevicePresent( slot ) )
            m_maxDevices++;
      }
   }
   else
   {
      m_slotFlags  = 0;
      m_maxDevices = 0;
   }
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::readMaxChannels( const unsigned int deviceNumber,
                                   unsigned int& rMaxChannels )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );

   rMaxChannels = 0;
   m_oSharedData.operation.ioData.location.deviceNumber = deviceNumber;
   writeParam1();
   if( setCommand( DAQ_OP_GET_CHANNELS ) != DAQ_RET_OK )
      return getLastReturnCode();
   if( readParam1() != DAQ_RET_OK )
      return getLastReturnCode();
   rMaxChannels = m_oSharedData.operation.ioData.param1;
   return getLastReturnCode();
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::enablePostMortem( const unsigned int deviceNumber,
                                    const unsigned int channel )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   writeParam1();
   return setCommand( DAQ_OP_PM_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::enableHighResolution( const unsigned int deviceNumber,
                                        const unsigned int channel )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   writeParam1();
   return setCommand( DAQ_OP_HIRES_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::enableContineous( const unsigned int deviceNumber,
                                    const unsigned int channel,
                                    const DAQ_SAMPLE_RATE_T sampleRate )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   m_oSharedData.operation.ioData.param1 = sampleRate;
   writeParam1();
   return setCommand( DAQ_OP_CONTINUE_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::disable( const unsigned int deviceNumber,
                           const unsigned int channel )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   writeParam1();
   return setCommand( DAQ_OP_OFF );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::setTriggerCondition( const unsigned int deviceNumber,
                                       const unsigned int channel,
                                       const uint32_t trgCondition )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   m_oSharedData.operation.ioData.param1 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[0];
   m_oSharedData.operation.ioData.param2 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[1];
   writeParam12();
   return setCommand( DAQ_OP_SET_TRIGGER_CONDITION );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::getTriggerCondition( const unsigned int deviceNumber,
                                       const unsigned int channel,
                                       uint32_t& rTrgCondition )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   writeParam1();
   if( setCommand( DAQ_OP_GET_TRIGGER_CONDITION ) != DAQ_RET_OK )
      return getLastReturnCode();

   readParam12();
   reinterpret_cast<uint16_t*>(&rTrgCondition)[0] =
      m_oSharedData.operation.ioData.param1;
   reinterpret_cast<uint16_t*>(&rTrgCondition)[1] =
      m_oSharedData.operation.ioData.param2;

   return getLastReturnCode();
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::setTriggerDelay( const unsigned int deviceNumber,
                                   const unsigned int channel,
                                   const uint16_t delay )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   m_oSharedData.operation.ioData.param1 = delay;
   writeParam1();
   return setCommand( DAQ_OP_SET_TRIGGER_DELAY );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::getTriggerDelay( const unsigned int deviceNumber,
                                   const unsigned int channel,
                                   uint16_t& rDelay )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   writeParam1();
   if( setCommand( DAQ_OP_GET_TRIGGER_DELAY ) != DAQ_RET_OK )
      return getLastReturnCode();
   readParam1();
   rDelay = m_oSharedData.operation.ioData.param1;
   return getLastReturnCode();
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::setTriggerMode( const unsigned int deviceNumber,
                                  const unsigned int channel,
                                  const bool mode )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   m_oSharedData.operation.ioData.param1 = mode;
   writeParam1();
   return setCommand( DAQ_OP_SET_TRIGGER_MODE );
}
/*! ---------------------------------------------------------------------------
 */
int DaqInterface::getTriggerMode( const unsigned int deviceNumber,
                       const unsigned int channel,
                       bool& rMode )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   SCU_ASSERT( channel > 0 );
   SCU_ASSERT( channel <= c_maxChannels );

   setLocation( deviceNumber, channel );
   if( setCommand( DAQ_OP_GET_TRIGGER_MODE ) != DAQ_RET_OK )
      return getLastReturnCode();
   readParam1();
   rMode = m_oSharedData.operation.ioData.param1;
   return getLastReturnCode();
}

//================================== EOF ======================================

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
#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <daq_interface.hpp>
#include <scu_shared_mem.h>
#ifdef CONFIG_DAQ_TEST
#include <daqt_messages.hpp>
#endif

#ifndef DEBUG_MESSAGE
  #define DEBUG_MESSAGE( args... )
#endif

using namespace Scu;
using namespace daq;

#define DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel )  \
{                                                          \
   DAQ_ASSERT_CHANNEL_ACCESS( deviceNumber, channel );     \
   setLocation( deviceNumber, channel );                   \
}

/*! ----------------------------------------------------------------------------
 * @ingroup DEBUG
 * @brief Converts a operation code for the LM32 into a string.
 */
const std::string command2String( daq::DAQ_OPERATION_CODE_T op )
{
   #define __OP_CODE_CASE_ITEM( name ) case name: return #name
   switch( op )
   {
      __OP_CODE_CASE_ITEM( DAQ_OP_IDLE );
      __OP_CODE_CASE_ITEM( DAQ_OP_LOCK );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_ERROR_STATUS );
      __OP_CODE_CASE_ITEM( DAQ_OP_RESET );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_MACRO_VERSION );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_SLOTS );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_CHANNELS );
      __OP_CODE_CASE_ITEM( DAQ_OP_RESCAN );
      __OP_CODE_CASE_ITEM( DAQ_OP_PM_ON );
      __OP_CODE_CASE_ITEM( DAQ_OP_HIRES_ON );
      __OP_CODE_CASE_ITEM( DAQ_OP_PM_HIRES_OFF );
      __OP_CODE_CASE_ITEM( DAQ_OP_CONTINUE_ON );
      __OP_CODE_CASE_ITEM( DAQ_OP_CONTINUE_OFF );
      __OP_CODE_CASE_ITEM( DAQ_OP_SET_TRIGGER_CONDITION );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_TRIGGER_CONDITION );
      __OP_CODE_CASE_ITEM( DAQ_OP_SET_TRIGGER_DELAY );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_TRIGGER_DELAY );
      __OP_CODE_CASE_ITEM( DAQ_OP_SET_TRIGGER_MODE );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_TRIGGER_MODE );
      __OP_CODE_CASE_ITEM( DAQ_OP_SET_TRIGGER_SOURCE_CON );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_TRIGGER_SOURCE_CON );
      __OP_CODE_CASE_ITEM( DAQ_OP_SET_TRIGGER_SOURCE_HIR );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_TRIGGER_SOURCE_HIR );
      __OP_CODE_CASE_ITEM( DAQ_OP_GET_DEVICE_TYPE );
      __OP_CODE_CASE_ITEM( DAQ_OP_SYNC_TIMESTAMP );
   }
   return "unknown";
   #undef  __OP_CODE_CASE_ITEM
}

/*! ----------------------------------------------------------------------------
 * @ingroup DEBUG
 * @brief Converts the command return code of LM32 into a string.
 */
const std::string daq::status2String( DAQ_RETURN_CODE_T status )
{
   #define __RET_CODE_CASE_ITEM( name ) case name: return #name
   switch( status )
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
      __RET_CODE_CASE_ITEM( DAQ_ERR_PROGRAM );
      __RET_CODE_CASE_ITEM( DAQ_ERR_RESPONSE_TIMEOUT );
   }
   return "unknown";
   #undef __RET_CODE_CASE_ITEM
}


///////////////////////////////////////////////////////////////////////////////

/*! ---------------------------------------------------------------------------
 * @brief Constructor of class daq::DaqInterface
 */
DaqInterface::DaqInterface( DaqEb::EtherboneConnection* poEtherbone,
                            const bool doReset,
                            const bool doSendCommand
                          )
   :DaqBaseInterface( poEtherbone )
   ,m_slotFlags( 0 )
   ,m_maxDevices( 0 )
   ,m_doReset( doReset )
   ,m_doSendCommand( doSendCommand )
   ,m_daqLM32Offset( INVALID_OFFSET )
{
   init();
}

DaqInterface::DaqInterface( EbRamAccess* poEbAccess,
                            const bool doReset,
                            const bool doSendCommand
                          )
   :DaqBaseInterface( poEbAccess )
   ,m_slotFlags( 0 )
   ,m_maxDevices( 0 )
   ,m_doReset( doReset )
   ,m_doSendCommand( doSendCommand )
   ,m_daqLM32Offset( INVALID_OFFSET )
{
   init();
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::init( void )
{
   SCU_ASSERT( getEbAccess() != nullptr );

   probe();

   if( !isAddacDaqSupport() )
      return;

   getEbAccess()->ramInit( &m_oScuRam, &m_oSharedData.ramIndexes );
   readSharedTotal();
   sendUnlockRamAccess();
   sendReset();
   readSlotStatus();
}

/*! ---------------------------------------------------------------------------
 */
inline void DaqInterface::probe( void )
{
   m_daqLM32Offset = INVALID_OFFSET;

   uint32_t actMagicNumber;

   /*
    * First step: Investigation whether the single DAQ LM32
    * application is loaded.
    */
   getEbAccess()->readLM32( &actMagicNumber, 1,
                           offsetof( DAQ_SHARED_IO_T, magicNumber ),
                           sizeof( actMagicNumber ) );
   if( actMagicNumber == DAQ_MAGIC_NUMBER )
   {/*
     * DAQ-LM32 single application found. But...
     */
      m_daqLM32Offset = offsetof( DAQ_SHARED_IO_T, magicNumber );
   }

   /*
    * Second step: Investigation whether the FG-LM32 application
    * is loaded.
    */
   getEbAccess()->readLM32( &actMagicNumber, 1,
                           offsetof( FG::SCU_SHARED_DATA_T, fg_magic_number ),
                           sizeof( actMagicNumber ) );
   if( m_daqLM32Offset != INVALID_OFFSET )
   {/*
     * Check whether the DAQ-magic number is not a random number
     * of SCU_SHARED_DATA_T::board_id.
     */
      if( actMagicNumber == FG_MAGIC_NUMBER )
      {
         m_daqLM32Offset = INVALID_OFFSET;
      }
   }

   if( m_daqLM32Offset != INVALID_OFFSET )
   {
      return;
   }

   if( actMagicNumber != FG_MAGIC_NUMBER )
   {
      throw DaqException( "Neither DAQ-application nor FG-application "
                          "in LM32 found!" );

   }

   /*
    * FG-application is loaded.
    * Third step: Investigation whether the FG+DAQ-LM32 application
    * is loaded.
    */
   getEbAccess()->readLM32( &actMagicNumber, 1,
                           offsetof( FG::SCU_SHARED_DATA_T, sDaq.magicNumber ),
                           sizeof( actMagicNumber ) );
   if( actMagicNumber == DAQ_MAGIC_NUMBER )
   {
      m_daqLM32Offset = offsetof( FG::SCU_SHARED_DATA_T, sDaq );
      return;
   }

   m_daqLM32Offset = INVALID_OFFSET;
}

/*----------------------------------------------------------------------------
 */
void DaqInterface::checkAddacSupport( void )
{
   if( isAddacDaqSupport() )
      return;
   throw DaqException( "LM32-Firmware doesn't support ADDAC/ACU-DAQ!" );
}

/*! ---------------------------------------------------------------------------
 * @brief Destructor of class daq::DaqInterfaceInterface
 */
DaqInterface::~DaqInterface( void )
{
}

/*! ---------------------------------------------------------------------------
 */
const std::string DaqInterface::getLastReturnCodeString( void )
{
   return status2String( getLastReturnCode() );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::isFgIntegrated( void ) const
{
   if( m_daqLM32Offset == INVALID_OFFSET )
      throw( "Neither DAQ-application nor FG-application running!" );

   return m_daqLM32Offset == offsetof( FG::SCU_SHARED_DATA_T, sDaq );
}


#define CONV_ENDIAN( t, s, m ) \
   t.m = gsi::convertByteEndian( s.m )

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readSharedTotal( void )
{
   DAQ_SHARED_IO_T temp;

   readLM32( &temp, sizeof(DAQ_SHARED_IO_T) );
   CONV_ENDIAN( m_oSharedData, temp, magicNumber );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.offset );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.capacity );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.start );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.end );
   CONV_ENDIAN( m_oSharedData, temp, operation.code );
   CONV_ENDIAN( m_oSharedData, temp, operation.retCode );

   if( m_oSharedData.magicNumber != DAQ_MAGIC_NUMBER )
      throw DaqException( "Wrong DAQ magic number respectively not found" );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::onCommandReadyPoll( USEC_T timeout )
{
   if( getSysMicrosecs() > timeout )
      return true;

   ::usleep( 10 );

   return false;
}

/*! ---------------------------------------------------------------------------
 */
inline
bool DaqInterface::cmdReadyWait( void )
{
   USEC_T timeout = getSysMicrosecs() + c_LM32CommandResponseTimeout;
   while( getCommand() != DAQ_OP_IDLE )
   {
      if( onCommandReadyPoll( timeout ) )
         return true;
   }
   return false;
}

/*! ---------------------------------------------------------------------------
 */
inline bool DaqInterface::permitCommand( DAQ_OPERATION_CODE_T cmd )
{
   if( m_doSendCommand )
      return true;

   switch( cmd )
   {
      //case DAQ_OP_GET_ERROR_STATUS:
      case DAQ_OP_GET_MACRO_VERSION:
      case DAQ_OP_GET_SLOTS:
      case DAQ_OP_GET_CHANNELS:
      case DAQ_OP_GET_DEVICE_TYPE:
      {
         return true;
      }
      default: break;
   }

   DEBUG_MESSAGE( "LM32 command: " << command2String( cmd ) << " disabled!" );
   return false;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T
DaqInterface::sendCommand( DAQ_OPERATION_CODE_T cmd )
{
   checkAddacSupport();

#if 0
   if( !m_doSendCommand )
   {
      DEBUG_MESSAGE( "LM32 command: " << command2String( cmd ) << " disabled!" );
      return DAQ_RET_OK;
   }
#else

   if( !permitCommand( cmd ) )
      return DAQ_RET_OK;
#endif
   DEBUG_MESSAGE( "Send command: " << command2String( cmd ) << " to LM32." );

   m_oSharedData.operation.code = cmd;
   DAQ_OPERATION_CODE_T temp =
                        gsi::convertByteEndian( m_oSharedData.operation.code );
   writeLM32( &temp, sizeof( temp ),
                                 offsetof( DAQ_SHARED_IO_T, operation.code ) );

   if( cmdReadyWait() )
   {
      throw DaqException( "Timeout at waiting for command feedback",
                          DAQ_ERR_RESPONSE_TIMEOUT );
   }

   if( m_oSharedData.operation.retCode < DAQ_RET_OK )
   {
      throw DaqException( "DAQ firmware error",
                          m_oSharedData.operation.retCode );
   }

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DAQ_OPERATION_CODE_T DaqInterface::getCommand( void )
{
   static_assert( static_cast<int>(offsetof( DAQ_OPERATION_T, ioData )) -
                  static_cast<int>(offsetof( DAQ_OPERATION_T, code )) > 0,
                  "Wrong order in DAQ_OPERATION_T" );
   static_assert( static_cast<int>(offsetof( DAQ_OPERATION_T, retCode )) -
                  static_cast<int>(offsetof( DAQ_OPERATION_T, code )) > 0,
                  "Wrong order in DAQ_OPERATION_T" );

   checkAddacSupport();

   DAQ_OPERATION_T temp;

   readLM32( &temp, offsetof(DAQ_OPERATION_T, ioData) -
                                               offsetof(DAQ_OPERATION_T, code),
                    offsetof( DAQ_SHARED_IO_T, operation )  );

   CONV_ENDIAN( m_oSharedData.operation, temp, code );
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );

   return m_oSharedData.operation.code;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1( void )
{
   checkAddacSupport();

   DAQ_OPERATION_T temp;

   readLM32( &temp,
                         (offsetof( DAQ_OPERATION_T, ioData.param1 ) +
                                sizeof( temp.ioData.param1 )) -
                                offsetof( DAQ_OPERATION_T, code ),
                                offsetof( DAQ_SHARED_IO_T, operation ));
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param1 );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam12( void )
{
   checkAddacSupport();
   DAQ_OPERATION_T temp;

   readLM32( &temp,
                         (offsetof( DAQ_OPERATION_T, ioData.param2 ) +
                                sizeof( temp.ioData.param2 )) -
                                offsetof( DAQ_OPERATION_T, code ),
                                offsetof( DAQ_SHARED_IO_T, operation ));
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param1 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param2 );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam123( void )
{
   checkAddacSupport();
   DAQ_OPERATION_T temp;

   readLM32( &temp, (offsetof( DAQ_OPERATION_T, ioData.param3 ) +
                                sizeof( temp.ioData.param3 )) -
                                offsetof( DAQ_OPERATION_T, code ),
                                offsetof( DAQ_SHARED_IO_T, operation ));
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param1 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param2 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param3 );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1234( void )
{
   checkAddacSupport();
   DAQ_OPERATION_T temp;
   readLM32( &temp, (offsetof( DAQ_OPERATION_T, ioData.param4 ) +
                                sizeof( temp.ioData.param4 )) -
                                offsetof( DAQ_OPERATION_T, code ),
                                offsetof( DAQ_SHARED_IO_T, operation ));
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param1 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param2 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param3 );
   CONV_ENDIAN( m_oSharedData.operation, temp, ioData.param4 );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readRamIndexes( void )
{
   checkAddacSupport();
   RAM_RING_INDEXES_T temp;
   readLM32( &temp, sizeof( RAM_RING_INDEXES_T ),
                        offsetof( DAQ_SHARED_IO_T, ramIndexes.ringIndexes ));
   CONV_ENDIAN( m_oSharedData.ramIndexes.ringIndexes, temp, start );
   CONV_ENDIAN( m_oSharedData.ramIndexes.ringIndexes, temp, end );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::sendUnlockRamAccess( void )
{
   checkAddacSupport();
   m_oSharedData.ramIndexes.ramAccessLock = false;
   uint32_t temp = 0;
   writeLM32( &temp, sizeof( temp ),
                          offsetof(DAQ_SHARED_IO_T, ramIndexes.ramAccessLock ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1( void )
{
   checkAddacSupport();
   DAQ_OPERATION_IO_T temp;

   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.deviceNumber );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.channel );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param1 );
   writeLM32( &temp, GET_OFFSET_AFTER( DAQ_OPERATION_IO_T, param1 ),
                     offsetof( DAQ_SHARED_IO_T, operation.ioData ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam12( void )
{
   checkAddacSupport();
   DAQ_OPERATION_IO_T temp;

   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.deviceNumber );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.channel );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param1 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param2 );
   writeLM32( &temp, GET_OFFSET_AFTER( DAQ_OPERATION_IO_T, param2 ),
                     offsetof( DAQ_SHARED_IO_T, operation.ioData ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam123( void )
{
   DAQ_OPERATION_IO_T temp;

   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.deviceNumber );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.channel );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param1 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param2 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param3 );
   writeLM32( &temp, GET_OFFSET_AFTER( DAQ_OPERATION_IO_T, param3 ),
                     offsetof( DAQ_SHARED_IO_T, operation.ioData ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1234( void )
{
   checkAddacSupport();
   DAQ_OPERATION_IO_T temp;

   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.deviceNumber );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, location.channel );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param1 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param2 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param3 );
   CONV_ENDIAN( temp, m_oSharedData.operation.ioData, param4 );
   writeLM32( &temp, GET_OFFSET_AFTER( DAQ_OPERATION_IO_T, param4 ),
                     offsetof( DAQ_SHARED_IO_T, operation.ioData ));
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeRamIndexesAndUnlock( void )
{
   checkAddacSupport();
   m_oSharedData.ramIndexes.ramAccessLock = false;

   RAM_RING_SHARED_OBJECT_T temp;
   CONV_ENDIAN( temp, m_oSharedData.ramIndexes, ramAccessLock );
   CONV_ENDIAN( temp.ringIndexes, m_oSharedData.ramIndexes.ringIndexes, offset );
   CONV_ENDIAN( temp.ringIndexes, m_oSharedData.ramIndexes.ringIndexes, capacity );
   CONV_ENDIAN( temp.ringIndexes, m_oSharedData.ramIndexes.ringIndexes, start );
   //!! CONV_ENDIAN( temp.ringIndexes, m_oSharedData.ramIndexes.ringIndexes, end );
   writeLM32( &temp, GET_OFFSET_AFTER( RAM_RING_SHARED_OBJECT_T,
                     ringIndexes.start ),
                     offsetof( DAQ_SHARED_IO_T, ramIndexes ));
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
uint DaqInterface::getSlotNumber( const uint deviceNumber )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );

   uint i = 0;
   for( uint slot = 1; slot <= c_maxSlots; slot++ )
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
uint DaqInterface::getDeviceNumber( const uint slotNumber )
{
   SCU_ASSERT( slotNumber > 0 );
   SCU_ASSERT( slotNumber <= c_maxSlots );

   if( !isDevicePresent( slotNumber ) )
      return 0;

   uint deviceNumber = 0;
   for( uint slot = 1; slot <= slotNumber; slot++ )
   {
      if( isDevicePresent( slot ) )
         deviceNumber++;
   }
   return deviceNumber;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readSlotStatus( void )
{
   sendCommand( DAQ_OP_GET_SLOTS );
   readParam1();
   if( m_oSharedData.operation.retCode == DAQ_RET_OK )
   {
      m_slotFlags = m_oSharedData.operation.ioData.param1;
      m_maxDevices = 0;
      for( uint slot = 1; slot <= c_maxSlots; slot++ )
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
DAQ_LAST_STATUS_T DaqInterface::readLastStatus( void )
{
   sendCommand( DAQ_OP_GET_ERROR_STATUS );
   readParam1();
   m_lastStatus = *reinterpret_cast<DAQ_LAST_STATUS_T*>
                                      (&m_oSharedData.operation.ioData.param1);
   return m_lastStatus;
}

/*! ---------------------------------------------------------------------------
 */
const std::string DaqInterface::getLastStatusString( void )
{
#define __CASE_ITEM( status ) case status: retString += #status; break
   std::string retString =  "Slot: " + std::to_string( m_lastStatus.slot )
                      + ", Channel: " + std::to_string( m_lastStatus.channel )
                      + ", Error-code: ";

   switch( m_lastStatus.status )
   {
      __CASE_ITEM( DAQ_RECEIVE_STATE_OK );
      __CASE_ITEM( DAQ_RECEIVE_STATE_DATA_LOST );
      __CASE_ITEM( DAQ_RECEIVE_STATE_CORRUPT_BLOCK );
      default:
         retString += "unknown " + std::to_string(m_lastStatus.status );
         break;
   }
   return retString;
#undef __CASE_ITEM
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::onBlockReceiveError( void )
{
   std::cerr << "Block error:  " << getLastStatusString() << std::endl;
}

/*! ---------------------------------------------------------------------------
 */
uint DaqInterface::readMaxChannels( const uint deviceNumber )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   m_oSharedData.operation.ioData.location.deviceNumber = deviceNumber;
   writeParam1();
   sendCommand( DAQ_OP_GET_CHANNELS );
   readParam1();
   return m_oSharedData.operation.ioData.param1;
}

/*! ---------------------------------------------------------------------------
 */
uint DaqInterface::readMacroVersion( const uint deviceNumber )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   m_oSharedData.operation.ioData.location.deviceNumber = deviceNumber;
   writeParam1();
   sendCommand( DAQ_OP_GET_MACRO_VERSION );
   readParam1();
   return m_oSharedData.operation.ioData.param1;
}

/*! ---------------------------------------------------------------------------
 */
DAQ_DEVICE_TYP_T DaqInterface::readDeviceType( const uint deviceNumber )
{
   SCU_ASSERT( deviceNumber > 0 );
   SCU_ASSERT( deviceNumber <= c_maxDevices );
   m_oSharedData.operation.ioData.location.deviceNumber = deviceNumber;
   writeParam1();
   sendCommand( DAQ_OP_GET_DEVICE_TYPE );
   readParam1();
   return static_cast<DAQ_DEVICE_TYP_T>(m_oSharedData.operation.ioData.param1);
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendSyncronizeTimestamps( const uint32_t timeOffset,
                                            const uint32_t ecaTag )
{
   m_oSharedData.operation.ioData.param1 = GET_LOWER_HALF( timeOffset );
   m_oSharedData.operation.ioData.param2 = GET_UPPER_HALF( timeOffset );
   m_oSharedData.operation.ioData.param3 = GET_LOWER_HALF( ecaTag );
   m_oSharedData.operation.ioData.param4 = GET_UPPER_HALF( ecaTag );
   writeParam1234();
   return sendCommand( DAQ_OP_SYNC_TIMESTAMP );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendEnablePostMortem( const uint deviceNumber,
                                        const uint channel,
                                        const bool restart
                                      )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = restart;
   writeParam1();
   return sendCommand( DAQ_OP_PM_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendEnableHighResolution( const uint deviceNumber,
                                            const uint channel,
                                            const bool restart
                                          )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = restart;
   writeParam1();
   return sendCommand( DAQ_OP_HIRES_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendEnableContineous( const uint deviceNumber,
                                        const uint channel,
                                        const DAQ_SAMPLE_RATE_T sampleRate,
                                        const uint maxBlocks
                                      )
{
   SCU_ASSERT( maxBlocks <= static_cast<uint16_t>(~0) );
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = sampleRate;
   m_oSharedData.operation.ioData.param2 = maxBlocks;
   writeParam12();
   return sendCommand( DAQ_OP_CONTINUE_ON );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendDisableContinue( const uint deviceNumber,
                                       const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   writeParam1();
   return sendCommand( DAQ_OP_CONTINUE_OFF );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendDisablePmHires( const uint deviceNumber,
                                      const uint channel,
                                      const bool restart
                                    )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = restart;
   writeParam1();
   return sendCommand( DAQ_OP_PM_HIRES_OFF );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerCondition( const uint deviceNumber,
                                        const uint channel,
                                        const uint32_t trgCondition )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   m_oSharedData.operation.ioData.param1 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[1];
   m_oSharedData.operation.ioData.param2 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[0];
#else
   m_oSharedData.operation.ioData.param1 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[0];
   m_oSharedData.operation.ioData.param2 =
      reinterpret_cast<const uint16_t*>(&trgCondition)[1];
#endif
   writeParam12();
   return sendCommand( DAQ_OP_SET_TRIGGER_CONDITION );
}

/*! ---------------------------------------------------------------------------
 */
uint32_t
DaqInterface::receiveTriggerCondition( const uint deviceNumber,
                                       const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   writeParam1();
   sendCommand( DAQ_OP_GET_TRIGGER_CONDITION );
   readParam12();

   uint32_t condition;
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
   reinterpret_cast<uint16_t*>(&condition)[1] =
      m_oSharedData.operation.ioData.param1;
   reinterpret_cast<uint16_t*>(&condition)[0] =
      m_oSharedData.operation.ioData.param2;
#else
   reinterpret_cast<uint16_t*>(&condition)[0] =
      m_oSharedData.operation.ioData.param1;
   reinterpret_cast<uint16_t*>(&condition)[1] =
      m_oSharedData.operation.ioData.param2;
#endif
   return condition;
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerDelay( const uint deviceNumber,
                                    const uint channel,
                                    const uint16_t delay )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = delay;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_DELAY );
}

/*! ---------------------------------------------------------------------------
 */
uint16_t DaqInterface::receiveTriggerDelay( const uint deviceNumber,
                                            const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   writeParam1();
   sendCommand( DAQ_OP_GET_TRIGGER_DELAY );
   readParam1();
   return m_oSharedData.operation.ioData.param1;
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerMode( const uint deviceNumber,
                                   const uint channel,
                                   const bool mode )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = mode;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_MODE );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerMode( const uint deviceNumber,
                                       const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   sendCommand( DAQ_OP_GET_TRIGGER_MODE );
   readParam1();
   return (m_oSharedData.operation.ioData.param1 != 0);
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerSourceContinue( const uint deviceNumber,
                                             const uint channel,
                                             const bool extInput )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = extInput;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_SOURCE_CON );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerSourceContinue( const uint deviceNumber,
                                                 const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   sendCommand( DAQ_OP_GET_TRIGGER_SOURCE_CON );
   readParam1();
   return (m_oSharedData.operation.ioData.param1 != 0);
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerSourceHiRes( const uint deviceNumber,
                                          const uint channel,
                                          const bool extInput )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = extInput;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_SOURCE_HIR );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerSourceHiRes( const uint deviceNumber,
                                              const uint channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   sendCommand( DAQ_OP_GET_TRIGGER_SOURCE_HIR );
   readParam1();
   return (m_oSharedData.operation.ioData.param1 != 0);
}


/*! ---------------------------------------------------------------------------
 */
void DaqInterface::clearBuffer( bool update )
{
   if( !isAddacDaqSupport() )
      return;

   ramRingReset( &m_oScuRam.pSharedObj->ringIndexes );
   if( update )
      writeRamIndexesAndUnlock();
}

//================================== EOF ======================================

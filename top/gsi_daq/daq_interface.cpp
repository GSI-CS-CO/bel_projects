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
#include <iostream>

#ifndef CONFIG_NO_FE_ETHERBONE_CONNECTION
#include <BusException.hpp>
namespace EB = FeSupport::Scu::Etherbone;
namespace IPC = EB::IPC;

#define EB_SCOPED_LOCK() IPC::scoped_lock<IPC::named_mutex> \
        lock( m_oEbAccess.getMutex() );

#define EB_THROW_MESSAGE( _m )                                                 \
   {                                                                           \
      std::stringstream messageBuilder;                                        \
      messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"               \
                           << std::dec << __LINE__ << ": "                     \
                           << "ERROR: " _m " etherbone cycle failed! - "       \
                           "ErrorCode: " << status << " ErrorMsg: "            \
                           << ::eb_status(status) << std::endl;                \
      throw EB::BusException(messageBuilder.str());                            \
   }

#endif

using namespace Scu;
using namespace daq;


#define FUNCTION_NAME_TO_STD_STRING static_cast<const std::string>(__func__)

#define __THROW_EB_EXCEPTION() \
   throw EbException( FUNCTION_NAME_TO_STD_STRING + "(): "\
   + static_cast<const std::string>(::ebGetStatusString( m_poEbHandle )) )


#define DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel )  \
{                                                          \
   DAQ_ASSERT_CHANNEL_ACCESS( deviceNumber, channel );     \
   setLocation( deviceNumber, channel );                   \
}

/*! ----------------------------------------------------------------------------
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
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
DaqInterface::DaqInterface( const std::string wbDevice, bool doReset )
   :m_wbDevice( wbDevice )
   ,m_poEbHandle( nullptr )
   ,m_slotFlags( 0 )
   ,m_maxDevices( 0 )
   ,m_doReset( doReset )
{
   if( ::ebOpen( &m_oEbHandle, m_wbDevice.c_str() ) != EB_OK )
      throw EbException( ::ebGetStatusString( &m_oEbHandle ) );

   m_poEbHandle = &m_oEbHandle;

   if( ::ramInit( &m_oScuRam, &m_oSharedData.ramIndexes, m_poEbHandle ) < 0 )
   {
      ebClose();
      throw( EbException( "Could not find RAM-device!" ) );
   }

   readSharedTotal();
   sendUnlockRamAccess();
   if( m_doReset )
      sendReset();
   readSlotStatus();
}
#else
DaqInterface::DaqInterface( DaqEb::EtherboneConnection* poEtherbone,
                            bool doReset )
   :m_oEbAccess( poEtherbone )
   ,m_slotFlags( 0 )
   ,m_maxDevices( 0 )
   ,m_doReset( doReset )
{
   m_oEbAccess.ramInit( &m_oScuRam, &m_oSharedData.ramIndexes );
   readSharedTotal();
   sendUnlockRamAccess();
   if( m_doReset )
      sendReset();
   readSlotStatus();
}
#endif

/*! ---------------------------------------------------------------------------
 * @brief Destructor of class daq::DaqInterfaceInterface
 */
DaqInterface::~DaqInterface( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   ebClose();
#endif
}


/*! ---------------------------------------------------------------------------
 */
const std::string DaqInterface::getLastReturnCodeString( void )
{
   return status2String( getLastReturnCode() );
}

/*! ---------------------------------------------------------------------------
 */
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
void DaqInterface::ebClose( void )
{
   if( m_poEbHandle == nullptr )
      return;

   if( ::ebClose( m_poEbHandle ) != EB_OK )
     __THROW_EB_EXCEPTION();

   m_poEbHandle = nullptr;
}
#endif

#ifndef CONFIG_NO_FE_ETHERBONE_CONNECTION

#define EB_READ_LM32_SHARAD_OBJECT( type, object, member )                    \
   read( EB_LM32_FOR_MEMBER( type, member ),                                  \
         reinterpret_cast<etherbone::data_t*>(&object.member) )

#define EB_READ_LM32_DAQ_OBJECT( object, member )                             \
    EB_READ_LM32_SHARAD_OBJECT( DAQ_SHARED_IO_T, object, member )

#define EB_WRITE_LM32_SHARED_OBJECT( type, object, member )                   \
    write( EB_LM32_FOR_MEMBER( type, member ), object.member )

#define EB_WRITE_LM32_DAQ_OBJECT( object, member )                            \
    EB_WRITE_LM32_SHARED_OBJECT( DAQ_SHARED_IO_T, object, member )

#endif // ifndef CONFIG_NO_FE_ETHERBONE_CONNECTION

#define CONV_ENDIAN( t, s, m ) \
   t.m = gsi::convertByteEndian( s.m )

 // #define CONFIG_VIA_EB_CYCLE

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::readSharedTotal( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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

#else
#ifdef CONFIG_VIA_EB_CYCLE
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open(m_oEbAccess.getEbDevice(), this,
        eb_block)) != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, magicNumber );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.offset );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.capacity );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.start );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.end );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.code );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#else
   DAQ_SHARED_IO_T temp;
   m_oEbAccess.readLM32( &temp, sizeof(temp) );
   CONV_ENDIAN( m_oSharedData, temp, magicNumber );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.offset );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.capacity );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.start );
   CONV_ENDIAN( m_oSharedData, temp, ramIndexes.ringIndexes.end );
   CONV_ENDIAN( m_oSharedData, temp, operation.code );
   CONV_ENDIAN( m_oSharedData, temp, operation.retCode );
#endif
#endif
   if( m_oSharedData.magicNumber != DAQ_MAGIC_NUMBER )
      throw DaqException( "Wrong DAQ magic number respectively not found" );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::onCommandReadyPoll( unsigned int pollCount )
{
   if( pollCount >= c_maxCmdPoll )
      return true;

   struct timeval sleepTime = {0, 10};
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
DaqInterface::RETURN_CODE_T DaqInterface::sendCommand( DAQ_OPERATION_CODE_T cmd )
{
   m_oSharedData.operation.code = cmd;
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
      throw DaqException( "Timeout at waiting for command feedback",
                          DAQ_ERR_RESPONSE_TIMEOUT );

#else
#ifdef CONFIG_VIA_EB_CYCLE
   { /*
      * We need this validity area in {} avoiding a mutex- conflict with
      * EB_SCOPED_LOCK() in function cmdReadyWait().
      */
      EB_SCOPED_LOCK();
      etherbone::Cycle oEbCycle;
      eb_status_t status;
      if( (status = oEbCycle.open(m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
         EB_THROW_MESSAGE( "opening" );

      oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.code );

      if( (status = oEbCycle.close()) != EB_OK )
         EB_THROW_MESSAGE( "closing" );
   }
#else
   DAQ_OPERATION_CODE_T temp = gsi::convertByteEndian( m_oSharedData.operation.code );
   m_oEbAccess.writeLM32( &temp, sizeof( temp ), offsetof( DAQ_SHARED_IO_T, operation.code ) );
#endif
#endif

   if( cmdReadyWait() )
      throw DaqException( "Timeout at waiting for command feedback",
                          DAQ_ERR_RESPONSE_TIMEOUT );

   if( m_oSharedData.operation.retCode < DAQ_RET_OK )
      throw DaqException( "DAQ firmware error",
                          m_oSharedData.operation.retCode );

   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DAQ_OPERATION_CODE_T DaqInterface::getCommand( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
#ifdef CONFIG_VIA_EB_CYCLE
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.code );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#else
   DAQ_OPERATION_T temp;
   m_oEbAccess.readLM32( &temp, sizeof(temp), offsetof( DAQ_SHARED_IO_T, operation )  );
   CONV_ENDIAN( m_oSharedData.operation, temp, code );
   CONV_ENDIAN( m_oSharedData.operation, temp, retCode );

#endif
#endif
   return m_oSharedData.operation.code;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open(m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                    != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam12( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam123( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param3 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readParam1234( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.retCode );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param3 );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param4 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
DaqInterface::RETURN_CODE_T DaqInterface::readRamIndexes( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   EB_MEMBER_INFO_T info[2];
   EB_INIT_INFO_ITEM_STATIC( info, 0, m_oSharedData.ramIndexes.ringIndexes.start );
   EB_INIT_INFO_ITEM_STATIC( info, 1, m_oSharedData.ramIndexes.ringIndexes.end );

   EB_MAKE_CB_OR_ARG( cArg, info );

   if( ebReadObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T,
                         ramIndexes.ringIndexes.start );
   EB_OJECT_MEMBER_READ( m_poEbHandle, DAQ_SHARED_IO_T,
                         ramIndexes.ringIndexes.end );
   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData,
                                                ramIndexes.ringIndexes.start );
   oEbCycle.EB_READ_LM32_DAQ_OBJECT( m_oSharedData,
                                                  ramIndexes.ringIndexes.end );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
   return m_oSharedData.operation.retCode;
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::sendUnlockRamAccess( void )
{
   m_oSharedData.ramIndexes.ramAccessLock = false;
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               ramIndexes.ramAccessLock );

   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();

#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData,
                                                    ramIndexes.ramAccessLock );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData,
                                      operation.ioData.location.deviceNumber );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData,
                                           operation.ioData.location.channel );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam12( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData,
                                      operation.ioData.location.deviceNumber );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData,
                                           operation.ioData.location.channel );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam123( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.location.deviceNumber );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.location.channel );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param1 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param3 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeParam1234( void )
{
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
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
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open( m_oEbAccess.getEbDevice(), this, eb_block ))
                                                                      != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.location.deviceNumber );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.location.channel );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param3 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param2 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param3 );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, operation.ioData.param4 );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
}

/*! ---------------------------------------------------------------------------
 */
void DaqInterface::writeRamIndexesAndUnlock( void )
{
   m_oSharedData.ramIndexes.ramAccessLock = false;
#ifdef CONFIG_NO_FE_ETHERBONE_CONNECTION
   EB_MAKE_CB_OW_ARG( cArg );

   if( ebWriteObjectCycleOpen( cArg ) != EB_OK )
      __THROW_EB_EXCEPTION();

   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               ramIndexes.ringIndexes.start );
 //!!  EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
 //!!                              ramIndexes.ringIndexes.end );


   EB_LM32_OJECT_MEMBER_WRITE( m_poEbHandle, &m_oSharedData,
                               ramIndexes.ramAccessLock );

   ebCycleClose();

   while( !cArg.exit )
      ebSocketRun();

   m_poEbHandle->status = cArg.status;
   if( m_poEbHandle->status != EB_OK )
      __THROW_EB_EXCEPTION();
#else
   EB_SCOPED_LOCK();
   etherbone::Cycle oEbCycle;
   eb_status_t status;
   if( (status = oEbCycle.open(m_oEbAccess.getEbDevice(), this, eb_block))
                                                                     != EB_OK )
      EB_THROW_MESSAGE( "opening" );

   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.start );
   //!! oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ringIndexes.end );
   oEbCycle.EB_WRITE_LM32_DAQ_OBJECT( m_oSharedData, ramIndexes.ramAccessLock );

   if( (status = oEbCycle.close()) != EB_OK )
      EB_THROW_MESSAGE( "closing" );
#endif
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
unsigned int DaqInterface::getDeviceNumber( const unsigned int slotNumber )
{
   SCU_ASSERT( slotNumber > 0 );
   SCU_ASSERT( slotNumber <= c_maxSlots );

   if( !isDevicePresent( slotNumber ) )
      return 0;

   unsigned int deviceNumber = 0;
   for( unsigned int slot = 1; slot <= slotNumber; slot++ )
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
unsigned int DaqInterface::readMaxChannels( const unsigned int deviceNumber )
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
unsigned int DaqInterface::readMacroVersion( const unsigned int deviceNumber )
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
int DaqInterface::sendEnablePostMortem( const unsigned int deviceNumber,
                                        const unsigned int channel,
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
int DaqInterface::sendEnableHighResolution( const unsigned int deviceNumber,
                                            const unsigned int channel,
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
int DaqInterface::sendEnableContineous( const unsigned int deviceNumber,
                                        const unsigned int channel,
                                        const DAQ_SAMPLE_RATE_T sampleRate,
                                        const unsigned int maxBlocks
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
int DaqInterface::sendDisableContinue( const unsigned int deviceNumber,
                                       const unsigned int channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   writeParam1();
   return sendCommand( DAQ_OP_CONTINUE_OFF );
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendDisablePmHires( const unsigned int deviceNumber,
                                      const unsigned int channel,
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
int DaqInterface::sendTriggerCondition( const unsigned int deviceNumber,
                                        const unsigned int channel,
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
DaqInterface::receiveTriggerCondition( const unsigned int deviceNumber,
                                       const unsigned int channel )
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
int DaqInterface::sendTriggerDelay( const unsigned int deviceNumber,
                                   const unsigned int channel,
                                   const uint16_t delay )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = delay;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_DELAY );
}

/*! ---------------------------------------------------------------------------
 */
uint16_t DaqInterface::receiveTriggerDelay( const unsigned int deviceNumber,
                                            const unsigned int channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   writeParam1();
   sendCommand( DAQ_OP_GET_TRIGGER_DELAY );
   readParam1();
   return m_oSharedData.operation.ioData.param1;
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerMode( const unsigned int deviceNumber,
                                   const unsigned int channel,
                                   const bool mode )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = mode;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_MODE );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerMode( const unsigned int deviceNumber,
                                       const unsigned int channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   sendCommand( DAQ_OP_GET_TRIGGER_MODE );
   readParam1();
   return (m_oSharedData.operation.ioData.param1 != 0);
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerSourceContinue( const unsigned int deviceNumber,
                                             const unsigned int channel,
                                             const bool extInput )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = extInput;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_SOURCE_CON );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerSourceContinue( const unsigned int deviceNumber,
                                                 const unsigned int channel )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   sendCommand( DAQ_OP_GET_TRIGGER_SOURCE_CON );
   readParam1();
   return (m_oSharedData.operation.ioData.param1 != 0);
}

/*! ---------------------------------------------------------------------------
 */
int DaqInterface::sendTriggerSourceHiRes( const unsigned int deviceNumber,
                                          const unsigned int channel,
                                          const bool extInput )
{
   DAQ_SET_CHANNEL_LOCATION( deviceNumber, channel );

   m_oSharedData.operation.ioData.param1 = extInput;
   writeParam1();
   return sendCommand( DAQ_OP_SET_TRIGGER_SOURCE_HIR );
}

/*! ---------------------------------------------------------------------------
 */
bool DaqInterface::receiveTriggerSourceHiRes( const unsigned int deviceNumber,
                                              const unsigned int channel )
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
   ramRingReset( &m_oScuRam.pSharedObj->ringIndexes );
   if( update )
      writeRamIndexesAndUnlock();
}

//================================== EOF ======================================

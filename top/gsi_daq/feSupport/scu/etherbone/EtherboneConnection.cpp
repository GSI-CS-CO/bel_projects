/*
 * EtherboneConnection.cpp
 *
 *  Created on: Feb 14, 2013
 *      Author: Vitaliy Rapp
 *      Revised: 2018 by Ulrich Becker
 */

#include "feSupport/scu/etherbone/EtherboneConnection.hpp"
#include "feSupport/scu/etherbone/Constants.hpp"
#include "feSupport/scu/etherbone/BusException.hpp"

#ifdef CONFIG_EB_USE_NORMAL_MUTEX
 #include <iostream>
 #include <string.h>
 #include <assert.h>
#else
 #include <boost/thread.hpp>
 #include <boost/date_time.hpp>
#endif

#include <sstream>
#include <stdexcept>
#include <netdb.h>

namespace FeSupport {
namespace Scu {
namespace Etherbone {

using namespace etherbone;


#ifndef CONFIG_EB_USE_NORMAL_MUTEX
std::string EtherboneConnection::c_mutexName;

#ifdef __WIN32
#warning "Following function is not been proofed for Windows yet!"
#endif

/*! ---------------------------------------------------------------------------
 * @brief Helper function for the constructor. It creates a name for the
 *        etherbone-Mutex by the net address.
 * @author Ulrich Becker
 * @todo Check whether this function runs under MS-Windows as well.
 * @param rName Net address e.g.: "tcp/scuxl4711" or "dev/wbm0"
 * @retval "wbm0_Mutex" In the case the application runs in the SCU's IPC
 * @retval "scuxl4711.acc.gsi.de_Mutex" In the case of remote.
 */
inline
const char* EtherboneConnection::__makeMutexName( const std::string& rName )
{  /*
    * Removing prefix "tcp/" or "dev/"
    */
   c_mutexName = rName.substr( rName.find_first_of( '/' ) + 1 );

   /*
    * In the case of "tcp/" obtaining the full URL name.
    */
   struct hostent* pHost = ::gethostbyname( c_mutexName.c_str() );
   if( (pHost != nullptr) && (pHost->h_name != nullptr) &&
       (*pHost->h_name != '\0' ) )
      c_mutexName = pHost->h_name;

   c_mutexName += "_Mutex";

   return c_mutexName.c_str();
}
#endif // ifndef CONFIG_EB_USE_NORMAL_MUTEX

///////////////////////////////////////////////////////////////////////////////
/* ----------------------------------------------------------------------------
 */
EtherboneConnection::EtherboneConnection(std::string netaddress,
                                                         unsigned int timeout)
#ifdef CONFIG_EB_USE_NORMAL_MUTEX
   :netaddress_(netaddress)
#else
   :_sysMu( IPC::open_or_create, __makeMutexName( netaddress ) )
   ,netaddress_(netaddress)
#endif
   ,timeout_( timeout )
   ,connectionOpened(false)
   ,debug_(false)
{
   // check if mutex is already locked
#ifndef CONFIG_EB_USE_NORMAL_MUTEX
   checkMutex();
#endif
}

/* ----------------------------------------------------------------------------
 */
EtherboneConnection::~EtherboneConnection()
{
}

#ifndef CONFIG_EB_USE_NORMAL_MUTEX
/* ----------------------------------------------------------------------------
 */
bool EtherboneConnection::checkMutex(bool unlock)
{
   try
   {
      if (_sysMu.try_lock())
      {
        // was not locked
         _sysMu.unlock();
         return false;
      }
      else
      {
         // was locked -> wait up to 1s then unlock
         std::cout << __FILE__ << "::" << __FUNCTION__  << "::" << std::dec
                   << __LINE__ << " " << c_mutexName
                   << " is locked -> wait 1s";
         //IPC::scoped_lock<IPC::named_mutex>
         SCOPED_MUTEX_T
            lock(_sysMu, boost::get_system_time() +
                 boost::posix_time::seconds(1));
         if (lock)
         {
          // lock succeeded within timeout
            std::cout << " -> unlock" << std::endl;;
         }
         else
         {
            // unlock explicit after timeout
            _sysMu.unlock();
            std::cout << " -> timed out -> unlock" << std::endl;
         }
         return true;
      }
   }
   catch (...)
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
             << __LINE__ << ": " << " " << c_mutexName
             << " problem with try_lock() ";
      throw BusException(stream.str());
   }
   return true;
}
#endif // CONFIG_EB_USE_NORMAL_MUTEX

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::connect()
{
   eb_status_t status;

   SCOPED_MUTEX_T lock(_sysMu);

   status = eb_socket_.open(0, EB_ADDRX|EB_DATAX);
   if( status != EB_OK )
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
             << __LINE__ << ": Error opening etherbone socket: " << status;
         throw BusException(stream.str());
   }

   status = eb_device_.open( eb_socket_, netaddress_.c_str(),
                             EB_ADDRX|EB_DATAX);
   if( status != EB_OK )
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::"
             << std::dec << __LINE__ << ": Error opening etherbone device: "
             << netaddress_ << " Error code: " << status;
      throw BusException(stream.str());
   }
   this->connectionOpened = true;
}

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::disconnect()
{
   SCOPED_MUTEX_T lock(_sysMu);
   eb_device_.close();
   eb_socket_.close();
   this->connectionOpened = false;
}

/* ----------------------------------------------------------------------------
 */
uint32_t EtherboneConnection::getSlaveMacroVersion( VendorId vendorId,
                                                    DeviceId deviceId )
{
   std::vector<sdb_device> foundDevs;

   SCOPED_MUTEX_T lock(_sysMu);
   eb_device_.sdb_find_by_identity(vendorId, deviceId, foundDevs);

   if (foundDevs.size() > 1)
      throw std::runtime_error("Only devs which are once in list are checkable."); // TODO implement

   sdb_device dev     = foundDevs.at(0);
   sdb_component comp = dev.sdb_component;
   sdb_product prod   = comp.product;
   uint32_t macroVersion         = prod.version;
   return macroVersion;
}

/* ----------------------------------------------------------------------------
 */
uint64_t EtherboneConnection::findDeviceBaseAddress( VendorId vendorId,
                                                     DeviceId deviceId,
                                                     uint32_t ind)
{
   eb_status_t status;

   if (!this->connectionOpened)
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
             << __LINE__ << ": Connection need to be opened before searching"
             "for a device address";
      throw BusException(stream.str());
   }

   std::vector<sdb_device> deviceVector;
   sdb_device device;
   deviceVector.push_back(device);

   {
      SCOPED_MUTEX_T lock(_sysMu);
      status = eb_device_.sdb_find_by_identity( vendorId, deviceId,
                                                deviceVector);
   }

   if( status != EB_OK )
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
             << __LINE__ << ": Error searching for a device address:"
             " VendorId 0x" << std::hex << vendorId << " deviceId 0x"
             << deviceId << " Error code: " << status;
      throw BusException(stream.str());
   }

   if (deviceVector.size() < (ind+1))
   {
      std::stringstream stream;
      stream << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
             << __LINE__ << ": Error searching for a device address:"
             " VendorId 0x" << std::hex << vendorId << " deviceId 0x"
             << deviceId << " Not enough devices found: expected: "
             << ind+1 << " found: " << deviceVector.size();
      throw BusException(stream.str());
   }

   if (debug_)
   {
      std::cout << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
                << __LINE__ << ": searched for Vendor 0x" << std::hex
                << vendorId << " deviceId 0x" << deviceId
                << " found " << std::dec << deviceVector.size()
                << " devices " << std::endl;

      std::cout << "abi_class 0x"  << std::hex << deviceVector[ind].abi_class
                << " abi_ver_major 0x" << deviceVector[ind].abi_ver_major
                << " abi_ver_minor 0x" << deviceVector[ind].abi_ver_minor
                << " bus_specific 0x" << deviceVector[ind].bus_specific
                << std::dec << std::endl;

      std::cout << "Version 0x" << std::hex
                << deviceVector[ind].sdb_component.product.version
                << " date 0x" << deviceVector[ind].sdb_component.product.date
                << " name " << deviceVector[ind].sdb_component.product.name
                << " record-type 0x"  << std::hex
                << deviceVector[ind].sdb_component.product.record_type
                << std::dec << std::endl;
    }

    return deviceVector[ind].sdb_component.addr_first;
}

/*! ---------------------------------------------------------------------------
 * @brief Type of data-object used by callback function __onEbSocked
 * @see __onEbSocked
 * @author Ulrich Becker
 */
struct EB_USER_CB_T
{
   bool                 m_finished;     //!<@brief becomes true when transfer finished.
   eb_status_t          m_status;       //!<@brief etherbone status
   const uint           m_len;          //!<@brief length of data field to copy
   const eb_user_data_t m_pUserAddress; //!<@brief Linux user address

   /*!
    * @brief Constructor
    * @param len Number of data elements to transfer this can be
    *            8, 16, 32 or 64-Bit types.
    * @param pUserAddress Target address in the Linux memory when
    *        function EtherboneConnection::read is used. \n
    *        If function EtherboneConnection::write will used this
    *        value has to be the "nullptr" (default).
    */
   EB_USER_CB_T( uint len, eb_user_data_t pUserAddress = nullptr )
      :m_finished( false )
      ,m_status( EB_OK )
      ,m_len( len )
      ,m_pUserAddress( pUserAddress )
   {}

   /*!
    * @brief Returns true once the callback function has been invoked.
    */
   bool isFinished( void ) const
   {
      return m_finished;
   }

   /*!
    * @brief Returns the error status of the callback function.
    */
   eb_status_t getStatus( void ) const
   {
      return m_status;
   }
};

/*
 * The callback function __onEbSocked becomes invoked within
 * the C-module libEtherbone. Therefore it's necessary to declare the
 * call-convention as pure C-function.
 */
extern "C"
{

/*! ---------------------------------------------------------------------------
 * @brief Callback function becomes invoked by member function
 *        EtherboneConnection::read and EtherboneConnection::write
 * @see EtherboneConnection::read
 * @see EtherboneConnection::write
 * @author Ulrich Becker
 */
static void  __onEbSocked( eb_user_data_t pUser, eb_device_t dev,
                           eb_operation_t op, eb_status_t status )
{
   static_cast<EB_USER_CB_T*>(pUser)->m_finished = true;
   static_cast<EB_USER_CB_T*>(pUser)->m_status = status;
   if( status != EB_OK )
      return;

   uint i = 0;
   uint j = 0;
   while( (op != EB_NULL) && (i < static_cast<EB_USER_CB_T*>(pUser)->m_len) )
   {
      if( ::eb_operation_had_error( op ) )
      {
         static_cast<EB_USER_CB_T*>(pUser)->m_status = EB_SEGFAULT;
         return;
      }

      /*
       * Is in EtherboneConnection::read() ?
       */
      if( ::eb_operation_is_read( op ) )
      { /*
         * Yes, section will run by EtherboneConnection::read.
         * Copying from Wishbone/Etherbone to user-buffer.
         */
         assert( static_cast<EB_USER_CB_T*>(pUser)->m_pUserAddress != nullptr );
         const data_t      data   = ::eb_operation_data( op );
         const std::size_t wide   = ::eb_operation_format( op ) & EB_DATAX;

         ::memcpy( &(static_cast<uint8_t*>(static_cast<EB_USER_CB_T*>
                                         (pUser)->m_pUserAddress))[j],
                   &data, wide );

         j += wide;
      }
#ifndef NDEBUG
      else
      {
         assert( static_cast<EB_USER_CB_T*>(pUser)->m_pUserAddress == nullptr );
      }
#endif
      op = ::eb_operation_next( op );
      i++;
   }
   if( (op != EB_NULL) || (i != static_cast<EB_USER_CB_T*>(pUser)->m_len) )
      static_cast<EB_USER_CB_T*>(pUser)->m_status = EB_FAIL;
}

} // extern "c"

/*! ---------------------------------------------------------------------------
 * @brief Helper function for EtherboneConnection::write \n
 *        returns a  8, 16, 32 or 64 bit value depending on the parameter wide.
 * @author Ulrich Becker
 * @param pData Base address
 * @param wide Data wide in bytes (1, 2, 4 or 8)
 * @param index Read index depending on parameter wide
 * @return 8, 16, 32 or 64 bit value of type etherbone::data_t.
 */
inline data_t getValueByFormat( const eb_user_data_t pData,
                                const std::size_t wide,
                                const uint index )
{
   #define __EB_FORMATED_RETURN( type ) \
      case sizeof( type ): return static_cast<const type*>(pData)[index]

   switch( wide )
   {
      __EB_FORMATED_RETURN( uint8_t );
      __EB_FORMATED_RETURN( uint16_t );
      __EB_FORMATED_RETURN( uint32_t );
      __EB_FORMATED_RETURN( uint64_t );
   }
   #undef __EB_FORMATED_RETURN
   assert( false ); // Shall never be reached!!!
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper-macro for debug-infos.
 * @author Ulrich Becker
 */
#define EB_DEBUG_INFO( addr, offset, index, pData, wide )            \
      std::cout << __FILE__ << "::" << __FUNCTION__ << "::"          \
                << std::dec << __LINE__ << ": " << "addr 0x"         \
                << std::uppercase << std::hex << uint(addr) + offset \
                << " pData[" << std::dec << i <<"] 0x" << std::hex   \
                << std::hex << getValueByFormat( pData, wide, i )    \
                << std::dec << std::endl

/*! ---------------------------------------------------------------------------
 * @brief Helper macro for throwing errors.
 * @param status error-number
 * @param text Message text.
 * @author Ulrich Becker
 * @todo A specific exception would be nice.
 */
#define EB_THROW( status, text )                                     \
{                                                                    \
   std::string status_str(eb_status(status));                        \
   std::stringstream messageBuilder;                                 \
   messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"        \
                  << std::dec << __LINE__ << ": "                    \
                  << "ERROR: " text " - "                            \
                     "ErrorCode: " << status << " ErrorMsg: "        \
                  << status_str << std::endl;                        \
   throw BusException( messageBuilder.str() );                       \
}

/*! ---------------------------------------------------------------------------
 * @brief Helper macro for throwing a possible cycle-open error.
 * @author Ulrich Becker
 * @param status error-number
 */
#define EB_THROW_CYC_OPEN_ERROR( status )                            \
   EB_THROW( status, "Opening etherbone cycle failed!" )

/*! ---------------------------------------------------------------------------
 * @brief Helper macro for throwing a possible cycle-close error.
 * @author Ulrich Becker
 * @param status error-number
 */
#define EB_THROW_CYC_CLOSE_ERROR( status )                           \
   EB_THROW( status, "Closing etherbone cycle failed!" )

/*! ---------------------------------------------------------------------------
 * @brief Helper macro for throwing a possible error in the callback-function.
 * @author Ulrich Becker
 * @param status error-number
 */
#define EB_THROW_CB_ERROR( status )                                  \
   EB_THROW( status, "Callback function __onEbSocked failed!" )

/*! ---------------------------------------------------------------------------
 * @author Ulrich Becker
 * @see EtherboneConnection.hpp
 */
void EtherboneConnection::write( const address_t eb_address,
                                 const eb_user_data_t pData,
                                 const format_t format,
                                 const uint size )
{
   /*
    * A size of zero is legal but there is nothing to do.
    */
   if( size == 0 )
      return;

   /*
    * Initializing the argument object of the callback function "__onEbSocked"
    */
   EB_USER_CB_T userObj( size );

   eb_status_t status;
   const std::size_t wide = format & EB_DATAX;

   {
      SCOPED_MUTEX_T lock(_sysMu);

      Cycle eb_cycle;
      if( (status = eb_cycle.open(eb_device_, &userObj, __onEbSocked)) != EB_OK )
      {
         EB_THROW_CYC_OPEN_ERROR( status );
      }

      for( uint i = 0, j = 0; i < size; i++, j += wide )
      {
         eb_cycle.write( eb_address + j, format,
                         getValueByFormat( pData, wide, i ));
         if( debug_ )
            EB_DEBUG_INFO( eb_address, j, i, pData, wide );
      }

      if( (status = eb_cycle.close()) != EB_OK )
      {
         EB_THROW_CYC_CLOSE_ERROR( status );
      }

      do
      {
         run();
         if( userObj.isFinished() )
            break;
      }
      while( onSockedPoll() );
   } // End of Mutex scope.

   /*
    * Checking whether an error in the callback function "__onEbSocked"
    * has occurred.
    */
   if( userObj.getStatus() != EB_OK )
   {
      EB_THROW_CB_ERROR( userObj.getStatus() );
   }
}

/*! ---------------------------------------------------------------------------
 * @author Ulrich Becker
 * @see EtherboneConnection.hpp
 */
void EtherboneConnection::read( const address_t eb_address,
                                eb_user_data_t pData,
                                const format_t format,
                                const uint size )
{
   /*
    * A size of zero is legal but there is nothing to do.
    */
   if( size == 0 )
      return;

   /*
    * Initializing the argument object of the callback function "__onEbSocked"
    */
   EB_USER_CB_T userObj( size, pData );

   eb_status_t status;
   const std::size_t wide = format & EB_DATAX;

   {
      SCOPED_MUTEX_T lock(_sysMu);

      Cycle eb_cycle;
      if( (status = eb_cycle.open( eb_device_, &userObj, __onEbSocked )) != EB_OK )
      {
         EB_THROW_CYC_OPEN_ERROR( status );
      }

      for( uint i = 0, j = 0; i < size; i++, j += wide )
      {
         eb_cycle.read( eb_address + j, format, nullptr );
      }

      if( (status = eb_cycle.close()) != EB_OK )
      {
         EB_THROW_CYC_CLOSE_ERROR( status );
      }

      do
      {
         run();
         if( userObj.isFinished() )
            break;
      }
      while( onSockedPoll() );
   } // End of Mutex scope.

   /*
    * Checking whether an error in the callback function "__onEbSocked"
    * has occurred.
    */
   if( userObj.getStatus() != EB_OK )
   {
      EB_THROW_CB_ERROR( userObj.getStatus() );
   }

   if( !debug_ )
      return;

   for( std::size_t i = 0, j = 0; i < size; i++, j += wide )
   {
      EB_DEBUG_INFO( eb_address, j, i, pData, wide );
   }
}

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::doRead(etherbone::address_t eb_address,
                                 etherbone::data_t* data,
                                 etherbone::format_t format,
                                 const uint16_t size) {

  if ( size == 1 ) {
    SCOPED_MUTEX_T lock(_sysMu);
    {
      eb_device_.read(eb_address, format, data);
      if (debug_)
        std::cout << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__
                  << " addr 0x" << std::hex << eb_address << " data 0x" << *data
                  << std::dec << std::endl;
    }
  }
  else {
    etherbone::Cycle eb_cycle;
    eb_status_t status;
    {
      SCOPED_MUTEX_T lock(_sysMu);
      if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK) {
        // TODO: a specific exception would be nice
        std::string status_str(eb_status(status));
        std::stringstream messageBuilder;
        messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": "
                       << "ERROR: opening etherbone cycle failed! - "
                       << "ErrorCode: " << status << " ErrorMsg: " << status_str << std::endl;
        throw BusException(messageBuilder.str());
      }

      // read data block
      for(int i = 0; i < size; i++) {
        eb_cycle.read(eb_address+(i*(format & EB_DATAX)), format, &(data[i]));
      }

      if ((status = eb_cycle.close()) != EB_OK) {
        // TODO: a specific exception would be nice
        std::string status_str(eb_status(status));
        std::stringstream messageBuilder;
        messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": "
                       << "ERROR: closing etherbone cycle failed! - "
                       << "ErrorCode: " << status << " ErrorMsg: " << status_str << std::endl;
        throw BusException(messageBuilder.str());
      }
    } // End of Mutex scope
    if (debug_) {
      for(uint i = 0; i < size; i++) {
        std::cout << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": " << "addr 0x"
                  << std::hex << eb_address+(i*(format & EB_DATAX))
                  << " data[" << std::dec << i <<"] 0x" << std::hex << data[i] << std::dec << std::endl;
      }
    }
  }
}

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::doVectorRead( const etherbone::address_t &eb_address,
                                        std::vector
                                           <std::pair <etherbone::data_t,
                                                       etherbone::data_t> >& v,
                                        etherbone::format_t format )
{
   etherbone::Cycle eb_cycle;
   eb_status_t status;

   {
      SCOPED_MUTEX_T lock(_sysMu);
      if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK)
      {
        // TODO: a specific exception would be nice
         std::string status_str(eb_status(status));
         std::stringstream messageBuilder;
         messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"
                        << std::dec << __LINE__
                        << ": ERROR: opening etherbone cycle failed! - "
                        "ErrorCode: " << status << " ErrorMsg: " << status_str
                        << std::endl;
         throw BusException(messageBuilder.str());
      }

      // read data block

      for( uint i = 0; i < v.size(); i++ )
      {
         eb_cycle.read( eb_address+v[i].first, format, &(v[i].second) );
         //std::cout << "etherboneConnection 0x" << std::hex <<
         //(eb_address+v[i].first) << ", format 0x" << format <<
         //", second 0x" << v[i].second << std::dec << std::endl;
      }

      if ((status = eb_cycle.close()) != EB_OK)
      {
      // TODO: a specific exception would be nice
         std::string status_str(eb_status(status));
         std::stringstream messageBuilder;
         messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"
                        << std::dec << __LINE__
                        << ": ERROR: closing etherbone cycle failed! - "
                        "ErrorCode: " << status << " ErrorMsg: "
                        << status_str << std::endl;
         throw BusException(messageBuilder.str());
      }
   } // End of Mutex scope

   if (debug_)
   {
      for(uint i = 0; i < v.size(); i++)
      {
         std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                   << std::dec << __LINE__ << ": format 0x" << std::hex
                   << (int)format << " addr 0x" << std::hex
                   << eb_address+v[i].first << " data[" << std::dec << i
                   << "] 0x" << std::hex << v[i].second
                   << std::dec << std::endl;
      }
   }
}


/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::doWrite(const etherbone::address_t &eb_address,
                                  const etherbone::data_t* data,
                                  etherbone::format_t format,
                                  const uint16_t size) {

  if ( size == 1 ) {
    SCOPED_MUTEX_T lock(_sysMu);
    {
      eb_device_.write(eb_address, format, data[0]);
      if (debug_)
        std::cout << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__
                  << " addr 0x" << std::hex << eb_address << " data 0x" << *data
                  << std::dec << std::endl;
    }
  }
  else {
    etherbone::Cycle eb_cycle;
    eb_status_t status;

    SCOPED_MUTEX_T lock(_sysMu);
    {
      if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK) {
        // TODO: a specific exception would be nice
        std::string status_str(eb_status(status));
        std::stringstream messageBuilder;
        messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": "
                       << "ERROR: opening etherbone cycle failed! - "
                       << "ErrorCode: " << status << " ErrorMsg: " << status_str << std::endl;
        throw BusException(messageBuilder.str());
      }

      // write data block
      for(int i = 0; i < size; i++) {
        eb_cycle.write(eb_address+(i*(format & EB_DATAX)), format, data[i]);
        if (debug_)
          std::cout << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": " << "addr 0x"
                    << std::hex << eb_address+(i*(format & EB_DATAX)) << " data[" << std::dec << i <<"] 0x"
                    << std::hex << data[i] << std::endl;
      }
      if ((status = eb_cycle.close()) != EB_OK) {
        // TODO: a specific exception would be nice
        std::string status_str(eb_status(status));
        std::stringstream messageBuilder;
        messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": "
                       << "ERROR: closing etherbone cycle failed! - "
                       << "ErrorCode: " << status << " ErrorMsg: " << status_str << std::endl;
        throw BusException(messageBuilder.str());
      }
    }
  } // else
} // doWrite

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::doVectorWrite(const etherbone::address_t &eb_address,
                                        const std::vector
                                        <std::pair <etherbone::data_t,
                                                    etherbone::data_t> >& v,
                                        etherbone::format_t format)
{
   etherbone::Cycle eb_cycle;
   eb_status_t status;

   SCOPED_MUTEX_T lock(_sysMu);
   { //?
      if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK)
      {
        // TODO: a specific exception would be nice
         std::string status_str(eb_status(status));
         std::stringstream messageBuilder;
         messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec
                        << __LINE__
                        << ": ERROR: opening etherbone cycle failed! - "
                        "ErrorCode: " << status << " ErrorMsg: " << status_str
                        << std::endl;
         throw BusException(messageBuilder.str());
      }

       // write data block

      for( uint i = 0; i < v.size(); i++ )
      {
         eb_cycle.write(eb_address+v[i].first, format, v[i].second);
         if (debug_)
             std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                       << std::dec << __LINE__ << ": format 0x"
                       << std::hex << (int)format
                       << " addr 0x" << std::hex << eb_address+v[i].first
                       << " data[" << std::dec << i <<"] 0x"
                       << std::hex << v[i].second << std::dec << std::endl;
      }

      if ((status = eb_cycle.close()) != EB_OK)
      {
         // TODO: a specific exception would be nice
         std::string status_str(eb_status(status));
         std::stringstream messageBuilder;
         messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"
                        << std::dec << __LINE__
                        << ": ERROR: closing etherbone cycle failed! - "
                        "ErrorCode: " << status << " ErrorMsg: "
                        << status_str << std::endl;
         throw BusException(messageBuilder.str());
      }
   } //?
}
} /* namespace EtherboneAccesss */
} // namespace Scu
} // namespace FeSupport


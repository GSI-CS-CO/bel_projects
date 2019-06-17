/*
 * EtherboneConnection.cpp
 *
 *  Created on: Feb 14, 2013
 *      Author: Vitaliy Rapp
 */

#include "feSupport/scu/etherbone/EtherboneConnection.hpp"
#include "feSupport/scu/etherbone/Constants.hpp"
#include "feSupport/scu/etherbone/BusException.hpp"

#include <boost/thread.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/date_time.hpp>

#include <sstream>
#include <stdexcept>

namespace FeSupport {
namespace Scu {
namespace Etherbone {

#ifndef ETHERBONE_CONNECTION_MUTEX
   #define ETHERBONE_CONNECTION_MUTEX "EtherBoneConnectionMutexDaq"
#endif

namespace IPC = boost::interprocess;

//static boost::mutex _widgetMu;
static IPC::named_mutex
              _sysMu(IPC::open_or_create, ETHERBONE_CONNECTION_MUTEX);

///////////////////////////////////////////////////////////////////////////////
/* ----------------------------------------------------------------------------
 */
EtherboneConnection::EtherboneConnection(std::string netaddress,
                                                         unsigned int timeout)
   :netaddress_(netaddress)
   ,connectionOpened(false)
   ,debug_(false)
{
   // check if mutex is already locked
   checkMutex();
}

/* ----------------------------------------------------------------------------
 */
EtherboneConnection::~EtherboneConnection()
{
}

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
                   << __LINE__ << " " << ETHERBONE_CONNECTION_MUTEX
                   << " is locked -> wait 1s";
         IPC::scoped_lock<IPC::named_mutex>
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
             << __LINE__ << ": " << " " << ETHERBONE_CONNECTION_MUTEX
             << " problem with try_lock() ";
      throw BusException(stream.str());
   }
   return true;
}

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::connect()
{
   eb_status_t status;

   //boost::lock_guard<IPC::named_mutex> lock(_sysMu);
   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
   {
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
   }
   this->connectionOpened = true;
}

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::disconnect()
{
   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
   {
      eb_device_.close();
      eb_socket_.close();
      this->connectionOpened = false;
   }
}

/* ----------------------------------------------------------------------------
 */
uint32_t EtherboneConnection::getSlaveMacroVersion( VendorId vendorId,
                                                    DeviceId deviceId )
{
   std::vector<sdb_device> foundDevs;
   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
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

   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
   {
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

/* ----------------------------------------------------------------------------
 */
void EtherboneConnection::doRead( etherbone::address_t eb_address,
                                  etherbone::data_t* data,
                                  etherbone::format_t format,
                                  const uint16_t size )
{
   if ( size == 1 )
   {
      IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
      {
         eb_device_.read(eb_address, format, data);
         if (debug_)
              std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                        << std::dec << __LINE__
                        << " addr 0x" << std::hex << eb_address
                        << " data 0x" << *data << std::dec << std::endl;
      }
   }
   else
   {
      etherbone::Cycle eb_cycle;
      eb_status_t status;

      IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
      {
         if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK)
         {
              // TODO: a specific exception would be nice
            std::string status_str(eb_status(status));
            std::stringstream messageBuilder;
            messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::"
                           << std::dec << __LINE__ << ": "
                           << "ERROR: opening etherbone cycle failed! - "
                           "ErrorCode: " << status << " ErrorMsg: "
                           << status_str << std::endl;
            throw BusException(messageBuilder.str());
         }

            // read data block
         for( int i = 0; i < size; i++)
         {
            eb_cycle.read(eb_address+(i*(format & EB_DATAX)), format, &(data[i]));
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
      }

      if (debug_)
      {
         for(uint i = 0; i < size; i++)
         {
            std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                      << std::dec << __LINE__ << ": " << "addr 0x"
                      << std::hex << eb_address+(i*(format & EB_DATAX))
                      << " data[" << std::dec << i <<"] 0x" << std::hex
                      << data[i] << std::dec << std::endl;
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

   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
   {
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
   }

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
void EtherboneConnection::doWrite( const etherbone::address_t &eb_address,
                                   const etherbone::data_t* data,
                                   etherbone::format_t format,
                                   const uint16_t size )
{
   if ( size == 1 )
   {
      IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
      {
         eb_device_.write( eb_address, format, data[0] );
         if (debug_)
            std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                      << std::dec << __LINE__ << " addr 0x" << std::hex
                      << eb_address << " data 0x" << *data
                      << std::dec << std::endl;
      }
   }
   else
   {
      etherbone::Cycle eb_cycle;
      eb_status_t status;

      IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
      {
         if ((status = eb_cycle.open(eb_device_, this, eb_block)) != EB_OK)
         {
              // TODO: a specific exception would be nice
              std::string status_str(eb_status(status));
              std::stringstream messageBuilder;
              messageBuilder << __FILE__ << "::" << __FUNCTION__ << "::" << std::dec << __LINE__ << ": "
                             << "ERROR: opening etherbone cycle failed! - " 
                             << "ErrorCode: " << status << " ErrorMsg: " << status_str << std::endl;
              throw BusException(messageBuilder.str());
         }

         // write data block
         for( int i = 0; i < size; i++ )
         {
            eb_cycle.write(eb_address+(i*(format & EB_DATAX)), format, data[i]);
            if (debug_)
               std::cout << __FILE__ << "::" << __FUNCTION__ << "::"
                         << std::dec << __LINE__ << ": " << "addr 0x"
                         << std::hex << eb_address+(i*(format & EB_DATAX))
                         << " data[" << std::dec << i <<"] 0x"
                         << std::hex << data[i] << std::endl;
         }
         if ((status = eb_cycle.close()) != EB_OK)
         {
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

   IPC::scoped_lock<IPC::named_mutex> lock(_sysMu);
   {
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
   }
}

} /* namespace EtherboneAccesss */
} // namespace Scu
} // namespace FeSupport


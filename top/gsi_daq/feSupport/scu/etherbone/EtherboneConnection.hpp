/*
 * EtherboneConnection.hpp
 *
 *  Created on: Feb 14, 2013
 *      Author: Vitaliy Rapp
 *      Revised: 2019 by Ulrich Becker
 */
#pragma once

#include <etherbone.h>
#include <utility>      // std::pair, std::make_pair
#include <string>

#define CONFIG_EB_USE_NORMAL_MUTEX

#ifdef CONFIG_EB_USE_NORMAL_MUTEX
  #include <mutex>
#else
  #include <boost/interprocess/sync/named_mutex.hpp>
  #include <boost/interprocess/sync/scoped_lock.hpp>
#endif

#include "feSupport/scu/etherbone/Constants.hpp"

#ifndef EB_DEFAULT_TIMEOUT
   #define EB_DEFAULT_TIMEOUT 5000
#endif

namespace FeSupport {
  namespace Scu {
    /*!
     * Namespace used to organize all etherbone related
     * classes.
     */
    namespace Etherbone {
#ifndef CONFIG_EB_USE_NORMAL_MUTEX
      namespace IPC = boost::interprocess;
#endif
      /*!
       * \brief Connection abstraction for an etherbone bus.
       */
      class EtherboneConnection {
        public:

       #ifdef CONFIG_EB_USE_NORMAL_MUTEX
          using MUTEX_T        = std::mutex;
          using SCOPED_MUTEX_T = std::lock_guard<MUTEX_T>;
       #else
          using MUTEX_T        = IPC::named_mutex;
          using SCOPED_MUTEX_T = IPC::scoped_lock<MUTEX_T>;
       #endif
          /*!
           * \brief Basic constuctor
           */
          EtherboneConnection(std::string netaddress, uint timeout = EB_DEFAULT_TIMEOUT );

          /*!
           * \brief Destructor
           */
          virtual ~EtherboneConnection();

#ifndef CONFIG_EB_USE_NORMAL_MUTEX
          /*!
           * \brief check if named mutex is locked
           * @param true: unlock mutex, false: don't
           */
          bool checkMutex(bool unlock=false);
#endif
          /*!
           * \brief Connects to the etherbone bus
           */
          void connect();

          /*!
           * \brief Disconnects from the bus
           */
          void disconnect();
          /*!
           * \brief Searches for a particular device address
           */
          uint64_t findDeviceBaseAddress(Etherbone::VendorId vendorId,
                                         Etherbone::DeviceId deviceId,
                                         uint32_t ind=0);

          /*!
           * @brief Copies a data array in 1:1 manner form the bus.
           * @author Ulrich Becker
           * @param eb_address Address to read from
           * @param pData Destination address to store data
           * @param format Or-link of endian convention and data format
           *               (8, 16, 32 or 64) bit.
           * @param size Length of data array.
           */
          void read( const etherbone::address_t eb_address,
                     eb_user_data_t pData,
                     const etherbone::format_t format,
                     const uint size = 1 );

          /*!
           * \brief Reads a value from the bus in etherbone format.
           *
           * It wraps the asynchronous
           * etherbone call into a synchronous read method.
           *
           * @param eb_address Address to read from
           * @param data Array to store read data
           * @param format 32 or 16 bit
           * @param size Length of data array
           */
          void doRead(etherbone::address_t eb_address, etherbone::data_t* data,
                      etherbone::format_t format,
                      const uint16_t size = 1);

          void doVectorRead(const etherbone::address_t &eb_address, 
                            std::vector  <std::pair <etherbone::data_t, etherbone::data_t> >& v,
                            etherbone::format_t format);

          /*!
           * @brief Copies a data array in 1:1 manner to the bus.
           * @author Ulrich Becker
           * @param eb_address Address to write to
           * @param pData Array of data to write
           * @param format Or-link of endian convention and data format
           *               (8, 16, 32 or 64) bit.
           * @param size Length of data array.
           */
          void write( const etherbone::address_t eb_address,
                      const eb_user_data_t pData,
                      const etherbone::format_t format,
                      const uint size = 1 );

          /*!
           * \brief Writes a single etherbone value to the bus.
           *
           * It wraps the asynchronous etherbone
           * call into a synchronous write method.
           *
           * @param eb_address Address to write to
           * @param data Array of data to write
           * @param format 32 or 16 bit
           * @param size Length of data array
           */
          void doWrite(const etherbone::address_t &eb_address,
                       const etherbone::data_t* data,
                       etherbone::format_t format,
                       const uint16_t size = 1);

          void doVectorWrite(const etherbone::address_t &eb_address, 
                             const std::vector <std::pair <etherbone::data_t, etherbone::data_t> >& v,
                             etherbone::format_t format);

          /*!
           * \brief Activate debugging outputs on console. 
           *
           * @param activate/deactivate debug output to console
           */
          void setDebug(const bool deb) { debug_ = deb; }

          uint32_t getSlaveMacroVersion(VendorId vendorId, DeviceId deviceId);

          /*!
           * @brief Returns true if connection has been successful established.
           * @author UB
           */
          bool isConnected() const
          {
             return connectionOpened;
          }

          /*!
           * @brief Returns the net address given as first argument of
           *        the constructor.
           * @author UB
           */
          const std::string& getNetAddress()
          {
             return netaddress_;
          }

        protected:
          /*!
           * @brief Function will used from EtherboneConnection::write
           *        and EtherboneConnection::read
           * @note The real WB/EB hardware access will made within the
           *       function etherbone::Socket::run().\n
           *       If a real cycle callback function is given instead of
           *       "eb_block" then etherbone::Cycle::close() wont invoke
           *       this.\n
           *       The both functions mentioned above have to do this self.\n
           *
           * @author UB
           */
          int run()
          {
             return eb_socket_.run( timeout_ );
          }

          /*!
           * @brief Optional callback function becomes invoked during
           *        the final socked polling in the functions
           *        EtherboneConnection::write and EtherboneConnection::read.
           * @retval true Continue the polling loop
           * @retval false Polling loop will break.
           * @author UB
           */
          virtual bool onSockedPoll()
          {
             return true;
          }

        private:
          /*!
           * @brief System mutex
           */
          MUTEX_T _sysMu;
          // EB-Device to talk to
          //
          std::string netaddress_;

          // How long we want to wait for eb to answer [ms]
          // TODO Only used in function run() respectively in write()
          //      and read()
          unsigned int timeout_;

          // TODO Add RAII, initialize eb things in ctor
          // The socket, used for this connection
          etherbone::Socket eb_socket_;

          // device to talk too
          etherbone::Device eb_device_;

          // This flag shows weather the connection was opened
          bool connectionOpened;

          // Contains the error Message in case the operation fails
          std::string errorMessage_;

          // debug flag, when set detailed infos for each etherbone access
          // will be printed to console output
          bool debug_;

          static std::string c_mutexName;
          static const char* __makeMutexName( const std::string& rName );
      };

    } // namespace Etherbone 
  } // namespace Scu
} // namepace FeSupport



#pragma once

#include <etherbone.h>
#include <utility>      // std::pair, std::make_pair
#include <string>

#include "feSupport/scu/etherbone/Constants.hpp"

namespace FeSupport {
  namespace Scu {
    /*!
     * Namespace used to organize all etherbone related
     * classes.
     */
    namespace Etherbone {
      /*!
       * \brief Connection abstraction for an etherbone bus.
       */
      class EtherboneConnection {
        public:
          /*!
           * \brief Basic constuctor
           */
          EtherboneConnection(std::string netaddress, unsigned int timeout = 5000);

          /*!
           * \brief Destructor
           */
          virtual ~EtherboneConnection();

          /*!
           * \brief check if named mutex is locked
           * @param true: unlock mutex, false: don't
           */
          bool checkMutex(bool unlock=false);

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

          bool isConnected( void ) const
          {
             return connectionOpened;
          }

        private:
          // EB-Device to talk to
          //
          std::string netaddress_;

          // How long we want to wait for eb to answer [ms] TODO NOT USED!
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
      };

    } // namespace Etherbone 
  } // namespace Scu
} // namepace FeSupport



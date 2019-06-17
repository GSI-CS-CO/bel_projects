#pragma once

#include <exception>
#include <string>
#include <stdarg.h>
namespace FeSupport {
  namespace Scu {
    namespace Etherbone
    {

      /*!
       * \brief A basic Exception for bus errors.
       *
       * It indicates, that the communication with the
       * Hardware device wasn't successful.
       */
      class BusException: public std::exception
      {
      public:
          /*!
           * \brief Constructor used in the derive classes since we can't use the one
           * with variable parameters
           */
        BusException();
        /*!
         * \brief Constructor with an error message as parameter
         */
        BusException(const char* errorMessage, ...);
        /*!
         * \brief Constructor with an error message as parameter
        */
        BusException(const std::string errorMessage);
        /*!
         * @return Containing error message
         */
        virtual const char* what() const throw ();
        /*!
         * \brief Basic destructor
         */
        virtual ~BusException() throw();
      private:
        /*!
         * \brief Method, which initializes the parameter-list of this exception
         */
        void buildMessage(va_list vl);

        /*!
         * \brief Message to return
         */
        std::string message_;
      };

    } // namespace Etherbone
  } // namespace Scu
} // namespace FeSupport


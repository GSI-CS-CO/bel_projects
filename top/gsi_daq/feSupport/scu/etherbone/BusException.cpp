/*
 * BusAdapterException.cpp
 *
 *  Created on: Feb 12, 2013
 *      Author: rapp
 */

#include "feSupport/scu/etherbone/BusException.hpp"
#include <cstdarg>
#include <sstream>
#include <stdint.h>
#include <stdio.h>

namespace FeSupport {
  namespace Scu {
    namespace Etherbone
    {

    BusException::BusException() :
                    message_("")
    {
    }


    BusException::BusException(const char* errorMessage, ...)
    {
            va_list vl;
            va_start(vl, errorMessage);
            buildMessage(vl);
            va_end(vl);
    }

    BusException::BusException(const std::string errorMessage) :
                    message_(errorMessage)
    {
    }

    const char* BusException::what() const throw()
    {
            return message_.c_str();
    }

    BusException::~BusException() throw ()
    {
    }

    void BusException::buildMessage(va_list vl)
    {
            char* parameter = NULL;
            uint32_t i = 0;

            // replace the appearances of the strings $1,$2 ... by the parameters
            while (true)
            {
                    ++i;
                    char buf[8];
                    int32_t nb = snprintf(buf, 8, "$%d", i);
                    int32_t pos = static_cast<int32_t>(message_.find(buf, 0));
                    if (pos == -1)
                            break;

                    parameter = NULL;
                    parameter = va_arg(vl,char*);
                    //if something looks strange, just break ...we don't want to have a segmentation fault here
                    if (parameter == NULL || parameter == (void*) -1)
                            break;

                    message_ = message_.replace(pos, nb, parameter);
            }
            std::stringstream result;
            result << "Exception: " << message_;
            message_ = result.str();
    }

    } // namespace Etherbone
  } // namespace Scu
} // namespace FeSupport

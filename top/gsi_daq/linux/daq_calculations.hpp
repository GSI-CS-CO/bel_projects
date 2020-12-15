/*!
 *  @file daq_calculations.hpp
 *  @brief Some helper templates for DAQ calculations.
 *  @note Header only!
 *  @date 22.08.2019
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
#ifndef _DAQ_CALCULATIONS_HPP
#define _DAQ_CALCULATIONS_HPP

#include <scu_control_config.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <system_error>

namespace Scu
{
namespace daq
{

/*!
 * @ingroup DAQ
 * @brief Calculation factor converting nanoseconds to seconds and vice versa.
 *
 * Preventing of the possible miscounting of zeros. ;-)
 */
constexpr uint64_t NANOSECS_PER_SEC  = 1000000000;

/*!
 * @ingroup DAQ
 * @brief Calculation factor converting microseconds to seconds and vice versa.
 *
 * Preventing of the possible miscounting of zeros. ;-)
 */
constexpr uint32_t MICROSECS_PER_SEC = 1000000;

/*!
 * @ingroup DAQ
 * @brief Calculation factor converting nanoseconds to milliseconds and vice
 *        versa.
 */
constexpr uint NANOSECS_PER_MILISEC = 1000000;

using USEC_T = uint64_t;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts nanoseconds to seconds.
 */
template< typename FT = double >
FT nanoSecsToSecs( const uint64_t nanoSecs )
{
   return nanoSecs / static_cast<FT>(NANOSECS_PER_SEC);
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Returns the absolute system time of Linux in microseconds.
 *
 * Can be used for time measurement and/or timeout detection.
 */
inline USEC_T getSysMicrosecs( void )
{
   struct ::timeval oTime;
   int fail;

   do
   {
      fail = ::gettimeofday( &oTime, nullptr );
   }
   while( (fail == -1) && (errno == EAGAIN) );

   if( fail == -1 )
      throw( std::system_error( errno, std::generic_category(), __func__ ));

   return oTime.tv_sec * MICROSECS_PER_SEC + oTime.tv_usec;
}

/*!
 * @ingroup DAQ
 * @brief Type of POSIX time object.
 *
 * Removing the old fashioned structure definition.
 */
typedef struct ::tm TIME_DATE_T;

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts the 64 bit white rabbit value in to the POSIX time object
 *        of type "struct tm",
 * @param rTm Reverence to the POSIX time object
 * @param wrt White rabbit time
 * @return Reverence to the POSIX time object including the actual time.
 */
inline TIME_DATE_T& wrToTimeDate( TIME_DATE_T& rTm, uint64_t wrt )
{
   ::time_t seconds = wrt / NANOSECS_PER_SEC;
   TIME_DATE_T* pTmpTm = ::localtime( &seconds );
   rTm = *pTmpTm;
   return rTm;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts the POSIX time object in a human readable string.
 * @param rTm Reverence to the POSIX time object.
 * @return Human readable string without linefeed.
 */
inline std::string timeToString( TIME_DATE_T& rTm )
{
   std::string ret( ::asctime( &rTm ) );
   ret.pop_back(); // Removes the final linefeed character ('\n').
   return ret;
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts a 64 bit white rabbit value in a human readable
 *        string including the date and time.
 * @param wrt 64 bit white rabbit time.
 * @return Human readable string with date and time.
 */
inline std::string wrToTimeDateString( uint64_t wrt )
{
   TIME_DATE_T time;
   return timeToString( wrToTimeDate( time, wrt ) );
}

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Performing of a linear interpolation between two given
 *        points in a two dimensional coordinate system.
 * @param x  X-value for which the Y-value shall be calculated.
 * @param x1 X-value of the known first point.
 * @param y1 Y-value of the known first point.
 * @param x2 X-value of the known second point.
 * @param y2 Y-value of the known second point.
 * @return Interpolated Y-value of the given X-value.
 */
template< typename XT = float, typename YT = float >
YT interpolate( const XT x, const XT x1, const YT y1, const XT x2, const YT y2 )
{
   if( x2 == x1 )
      return (y1 + y2) / 2; //TODO Workaround prevents a division by zero.

   return (y2 - y1) * static_cast<YT>(x - x1) /
              static_cast<YT>(x2 - x1) + y1;
}

#ifndef DAQ_VPP_MAX
   #define DAQ_VPP_MAX 20.0
#endif

/*! ---------------------------------------------------------------------------
 * @ingroup DAQ
 * @brief Converts raw data of the DAQ ADC in to voltage.
 * @param rawData raw data from the DAQ ADC.
 * @param maxVss Difference of minimum and maximum voltage
 * @return Voltage in the range -maxVpp/2 to +maxVpp/2
 */
template< typename IT, typename FT = float >
FT rawToVoltage( const IT rawData, const FT maxVpp = DAQ_VPP_MAX )
{
#define __CALC_VOLTAGE_CASE( size )                                           \
   case sizeof( uint##size##_t ):                                             \
     return (static_cast<FT>(static_cast<int##size##_t>(rawData)) * maxVpp) / \
             static_cast<FT>(static_cast<IT>(~0))

   switch( sizeof( IT ) )
   {
      __CALC_VOLTAGE_CASE(  8 );
      __CALC_VOLTAGE_CASE( 16 );
      __CALC_VOLTAGE_CASE( 32 );
      __CALC_VOLTAGE_CASE( 64 );
      default: assert( false );
   }

#undef __CALC_VOLTAGE_CASE
}

/*! ---------------------------------------------------------------------------
 * @brief Calculating of the Y value of a quadratic polynomial.
 *
 * @f$ f(x) = a \times x^2 + b \times x + c @f$
 *
 * @param a Quadratic coefficient.
 * @param b Linear coefficient,
 * @param c Offset coefficient.
 * @param x Value of x-axis.
 * @return Value of y-axis.
 */
template <typename T>
T calcPolynom( const T a, const T b, const T c, const T x )
{
  return x * (a * x + b) + c;
}


} // namespace daq
} // namespace Scu

#endif // ifndef _DAQ_CALCULATIONS_HPP
// ================================= EOF ======================================

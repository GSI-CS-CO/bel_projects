/*!
 *  @file daq_calculations.hpp
 *  @brief Some helper templates for DAQ calculations.
 *
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

namespace Scu
{
namespace daq
{

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
template< typename T >
float rawToVoltage( const T rawData, const float maxVpp = DAQ_VPP_MAX )
{
#define __CALC_VOLTAGE_CASE( size )                                           \
   case sizeof( uint##size##_t ):                                             \
   return (static_cast<float>(static_cast<int##size##_t>(rawData)) * maxVpp) /\
             static_cast<float>(static_cast<T>(~0))

   switch( sizeof( T ) )
   {
      __CALC_VOLTAGE_CASE(  8 );
      __CALC_VOLTAGE_CASE( 16 );
      __CALC_VOLTAGE_CASE( 32 );
      __CALC_VOLTAGE_CASE( 64 );
      default: assert( false );
   }

#undef __CALC_VOLTAGE_CASE
}

} // namespace daq
} // namespace Scu

#endif // ifndef _DAQ_CALCULATIONS_HPP
// ================================= EOF ======================================

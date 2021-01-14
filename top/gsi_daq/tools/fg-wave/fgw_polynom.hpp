/*!
 *  @file fgw_polynom.hpp
 *  @brief Calculates the polymoms of a given polynominal vector.
 *
 *  @date 10.12.2020
 *  @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#ifndef _FGW_POLYNOM_HPP
#define _FGW_POLYNOM_HPP

#include <gnuplotstream.hpp>
#include <fgw_parser.hpp>
#include "fgw_commandline.hpp"

namespace fgw
{

///////////////////////////////////////////////////////////////////////////////
class Polynom
{
public:
   static constexpr long double F_MAX = static_cast<long double>(static_cast<uint64_t>(~0));
   const static double c_timeTab[8];
   static uint calcStep( const uint i )
   {
      assert( i <= 7 );
      return (250 << i);
   }

private:
   CommandLine& m_rCommandline;

public:
   Polynom( CommandLine& rCmdLine );
   ~Polynom( void );

   void plot( std::ostream& out, const POLYMOM_VECT_T& rVect );

private:
   double calcPolynom( const POLYNOM_T& polynom, const int64_t x );
};

} // namespace fgw
#endif // ifndef _FGW_POLYNOM_HPP
//================================== EOF ======================================

/*!
 *  @file fgw_polynom.cpp
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
#include <daq_calculations.hpp>
#include "fgw_polynom.hpp"

namespace fgw
{
   
using namespace std;
using namespace Scu;

/*!
 * @brief Establishing a upper and lower margin of Y axis, so that the
 *        maximum voltages can be plot as well.
 *
 * The dimension is voltage.
 */
constexpr float Y_PADDING = 0.5;

   constexpr uint c_frequencyTab[] =
   {
       16,
       32,
       64,
      125,
      250,
      500,
      1000,
      2000
   };

///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
 */
Polynom::Polynom(  CommandLine& rCmdLine )
   :m_rCommandline( rCmdLine )
{
}

/*! ---------------------------------------------------------------------------
 */
Polynom::~Polynom( void )
{
}

/*! ---------------------------------------------------------------------------
 */
double Polynom::calcPolynom( const POLYNOM_T& polynom, const int64_t x )
{
   constexpr int64_t ZERO = static_cast<int64_t>(0);
#if 1
   return daq::calcPolynom( (m_rCommandline.isNoSquareTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.a ) << polynom.shiftA)),
                            (m_rCommandline.isNoLinearTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.b ) << polynom.shiftB)),
                            static_cast<int64_t>( polynom.c ) << 32,
                            x
                          );
#else
   return daq::calcPolynom( static_cast<long double>(m_rCommandline.isNoSquareTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.a ) << polynom.shiftA)),
                            static_cast<long double>(m_rCommandline.isNoLinearTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.b ) << polynom.shiftB)),
                            static_cast<long double>(static_cast<int64_t>( polynom.c ) << 32),
                            static_cast<long double>(x)
                          );

#endif

}

/*! ---------------------------------------------------------------------------
 */
void Polynom::plot( ostream& out, const POLYMOM_VECT_T& rVect )
{
   const uint repeat = m_rCommandline.getRepetitions();
   out << "set terminal " << m_rCommandline.getGnuplotTerminal()
       << " title \"File: " << m_rCommandline.getFileName() << '"' << endl;
   out << "set grid" << endl;
   if( !m_rCommandline.isZoomYAxis() )
   {
      out << "set yrange [" << -(DAQ_VPP_MAX/2 + Y_PADDING) << ':'
                            << (DAQ_VPP_MAX/2 + Y_PADDING) << ']' << endl;
   }
   out << "set ylabel \"Voltage\"" << endl;                           
   double xRange = 0.0;
   uint daConversions = 0; 
   for( const auto& polynom: rVect )
   {
      assert( polynom.frequ < ARRAY_SIZE( c_frequencyTab ) );
      const uint steps = calcStep( polynom.step );
      xRange += static_cast<double>(c_frequencyTab[polynom.frequ] * steps);
      daConversions += steps;
   }
   assert( xRange > 0.0 );
   xRange /= (SCU_FREQUENCY * 2);
   const double frequency = 1.0 / xRange;
   const double loadFrequency = rVect.size() / xRange;
   xRange *= repeat;
   out << "set xrange [0:" << xRange << ']' << endl;
   out << "set xlabel \"Time\"" << endl;

   out << "set title \"file-repeat-frequency: " << frequency << " Hz"
       << ", fg-load frequ.: " << loadFrequency << " Hz"
       << ", tuples: " << rVect.size()
       << ", dots/tuple: ";
   if( m_rCommandline.getDotsPerTuple() == 0 )
      out << "all";
   else
      out << m_rCommandline.getDotsPerTuple();
   out << ", D/A- conversions: " << daConversions;
   if( repeat > 1 )
      out << ", repeats: " << repeat;
   out << '"' << endl;

   out << "plot '-' title '' with " << m_rCommandline.getLineStyle() << " lc rgb 'green'" << endl;
   double tOrigin = 0.0;
   for( uint r = 0; r < repeat; r++ )
   {
      for( const auto& polynom: rVect )
      {
         const uint steps = calcStep( polynom.step );
         assert( steps > 0 );
         uint dotsPerTuple = m_rCommandline.getDotsPerTuple();
         if( dotsPerTuple == 0 )
            dotsPerTuple = steps;
         const uint interval = steps / dotsPerTuple;
         assert( polynom.frequ < ARRAY_SIZE( c_frequencyTab ) );
         const double tPart = static_cast<double>(c_frequencyTab[polynom.frequ]) / (SCU_FREQUENCY * 2);
         for( uint i = 0; i < steps; i++ )
         {
            if( (i % interval) == 0 )
               out << tOrigin << ' ' << (calcPolynom( polynom, i ) * DAQ_VPP_MAX / F_MAX) << endl;
            tOrigin += tPart;
         }
      }
   }
   out << 'e' << endl;
}


} // namespace fgw
//================================== EOF ======================================

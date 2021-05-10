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

///////////////////////////////////////////////////////////////////////////////
/*!
 * @brief Assignment table for POLYNOM_T::frequ.
 * 
 * Containing the period time.
 */
const double Polynom::c_timeTab[] =
{
   1.0 /   16000.0,
   1.0 /   32000.0,
   1.0 /   64000.0,
   1.0 /  125000.0,
   1.0 /  250000.0,
   1.0 /  500000.0,
   1.0 / 1000000.0,
   1.0 / 2000000.0
};

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

   return daq::calcPolynom( (m_rCommandline.isNoSquareTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.coeff_a ) << polynom.control.bv.shift_a)) / 2,
                            (m_rCommandline.isNoLinearTerm()?
                               ZERO:
                               (static_cast<int64_t>( polynom.coeff_b ) << polynom.control.bv.shift_b)),
                            static_cast<int64_t>( polynom.coeff_c ) << 32,
                            x
                          );
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
      assert( polynom.control.bv.frequency < ARRAY_SIZE( c_timeTab ) );
      const uint steps = calcStep( polynom.control.bv.step );
      xRange += c_timeTab[polynom.control.bv.frequency] * steps;
      daConversions += steps;
   }
   assert( xRange > 0.0 );
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

   out << "plot '-' title ";
   if( m_rCommandline.isPlotCoeffC() )
   {
      out << "'polynomials' with " << m_rCommandline.getLineStyle() << " lc rgb 'green', "
          << "'-' title 'coefficient c' with " << m_rCommandline.getCoeffCLineStyle()
          << " lc rgb 'red'" << endl; 
   }
   else
      out << "'' with " << m_rCommandline.getLineStyle() << " lc rgb 'green'" << endl;

   constexpr double TO_VOLTAGE = DAQ_VPP_MAX / F_MAX;
   /*
    * Potting of all polynomials
    */
   double tOrigin = 0.0;
   for( uint r = 0; r < repeat; r++ )
   {
      for( const auto& polynom: rVect )
      {
         const uint steps = calcStep( polynom.control.bv.step );
         assert( steps > 0 );
         uint dotsPerTuple = m_rCommandline.getDotsPerTuple();
         if( dotsPerTuple == 0 )
            dotsPerTuple = steps;
         const uint interval = steps / dotsPerTuple;
         assert( polynom.control.bv.frequency < ARRAY_SIZE( c_timeTab ) );
         const double tPart = c_timeTab[polynom.control.bv.frequency];
         for( uint i = 0; i < steps; i++ )
         {
            if( ((i % interval) == 0) || (i == (steps-1)) )
               out << tOrigin << ' ' << (calcPolynom( polynom, i ) * TO_VOLTAGE) << endl;
            tOrigin += tPart;
         }
      }
   }
   out << 'e' << endl;

   if( !m_rCommandline.isPlotCoeffC() )
      return;

   /*
    * Potting of c-coefficient only
    */
   tOrigin = 0.0;
   for( uint r = 0; r < repeat; r++ )
   {
      for( const auto& polynom: rVect )
      {
         out << tOrigin << ' '
             << (static_cast<double>(static_cast<int64_t>( polynom.coeff_c ) << 32)
                * TO_VOLTAGE)
             << endl;
         tOrigin += calcStep( polynom.control.bv.step ) * c_timeTab[polynom.control.bv.frequency];
      }
   }
   out << 'e' << endl;
}

} // namespace fgw
//================================== EOF ======================================

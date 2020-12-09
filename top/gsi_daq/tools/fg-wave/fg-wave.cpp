/*!
 *  @file fg-wave.cpp
 *  @brief Main module for plotting SCU function generator polynomial files
 *         via Gnuplot.
 *
 *  @date 07.12.2020
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
#include <cmath>
#include <fgw_parser.hpp>
#include <gnuplotstream.hpp>

using namespace std;
using namespace Scu;
using namespace fgw;

#ifndef DEFAULT_LINE_STYLE
   #define DEFAULT_LINE_STYLE "lines"
#endif


/*
 * -- constants for frequency and step value counters
type int_array is array(7 downto 0) of integer;

constant c_freq_cnt: int_array := (
                                    (CLK_in_Hz / 2000000) - 2,
                                    (CLK_in_Hz / 1000000) - 2,
                                    (CLK_in_Hz / 500000) -2,
                                    (CLK_in_Hz / 250000) - 2,
                                    (CLK_in_Hz / 125000) - 2,
                                    (CLK_in_Hz / 64000) - 2,
                                    (CLK_in_Hz / 32000) - 2,
                                    (CLK_in_Hz / 16000) - 2
                                    );
*/
constexpr uint SCU_FREQUENCY = 125000000;

constexpr long double PERIOD_TIME = 1.0 / SCU_FREQUENCY;

#if 0
const uint g_frequencyTab[] =
{
   SCU_FREQUENCY /   16000,
   SCU_FREQUENCY /   32000,
   SCU_FREQUENCY /   64000,
   SCU_FREQUENCY /  125000,
   SCU_FREQUENCY /  250000,
   SCU_FREQUENCY /  500000,
   SCU_FREQUENCY / 1000000,
   SCU_FREQUENCY / 2000000
};
#else
const uint g_frequencyTab[] =
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
#endif


/*
constant c_step_cnt: int_array := (
                                   32000 - 2,
                                   16000 - 2,
                                   8000 - 2,
                                   4000 - 2,
                                   2000 - 2,
                                   1000 - 2,
                                   500 - 2,
                                   250 - 2
                                   );

*/
uint calcStep( const uint i )
{
   return 250 << i;
}

void printPolynomVect( const POLYMOM_VECT_T& rVect )
{
   for( const auto& polynom: rVect )
   {
      cout  << polynom.a << '\t'
            << polynom.shiftA << '\t'
            << polynom.b << '\t'
            << polynom.shiftB << '\t'
            << polynom.c << '\t'
            << polynom.step << '\t'
            << polynom.frequ << endl;
   }
}

constexpr long double F_MAX = static_cast<long double>(static_cast<uint64_t>(~0));

/*!
 * @brief Establishing a upper and lower margin of Y axis, so that the
 *        maximum voltages can be plot as well.
 *
 * The dimension is voltage.
 */
constexpr float Y_PADDING = 0.5;

double calcPolynomTuple( const POLYNOM_T& polynom, const int64_t x )
{
   return daq::calcPolynom( static_cast<int64_t>( polynom.a ) << polynom.shiftA,
                            static_cast<int64_t>( polynom.b ) << polynom.shiftB,
                            static_cast<int64_t>( polynom.c ) << 32,
                            x
                          );
}

void printPolynomVectFloat( ostream& out, const POLYMOM_VECT_T& rVect, const uint repeat = 1 )
{
   out << "set grid" << endl;
   //out << "set yrange [" << -(DAQ_VPP_MAX/2 + Y_PADDING) << ':'
   //                           << (DAQ_VPP_MAX/2 + Y_PADDING) << ']' << endl;
   out << "set ylabel \"Voltage\"" << endl;                           
   double xRange = 0.0;
   for( const auto& polynom: rVect )
   {
      assert( polynom.frequ < ARRAY_SIZE( g_frequencyTab ) );
      xRange += static_cast<double>(g_frequencyTab[polynom.frequ] * calcStep( polynom.step ));
   }
   assert( xRange > 0.0 );
   xRange /= (SCU_FREQUENCY * 2);
   const double frequency = 1.0 / xRange;
   xRange *= repeat;
   out << "set xrange [0:" << xRange << ']' << endl;
   out << "set xlabel \"Time\"" << endl;
   out << "set title \"Frequency: " << frequency << " Hz\"" << endl;
   
   //out << "plot '-' title 'set value' with " << DEFAULT_LINE_STYLE << " lc rgb 'green'" << endl;
   out << "plot '-' title '' with " << DEFAULT_LINE_STYLE << " lc rgb 'green'" << endl;
   double tOrigin = 0.0;
   for( uint r = 0; r < repeat; r++ )
   {
      for( const auto& polynom: rVect )
      {
         double tStep = 0.0;
         uint step = calcStep( polynom.step );
         assert( step > 0 );
         assert( polynom.frequ < ARRAY_SIZE( g_frequencyTab ) );
         const double tPart = static_cast<double>(g_frequencyTab[polynom.frequ]) / step / SCU_FREQUENCY;
         for( uint i = 0; i < step; i++ )
         {
            out << tOrigin << ' ' << (calcPolynomTuple( polynom, i ) * DAQ_VPP_MAX / F_MAX) << endl;
            tStep += tPart;
            tOrigin += tStep;
         }
      }
   }
   out << 'e' << endl;
}


int main( int argc, char** ppArgv )
{
  // cout << "arg=" << ppArgv[0] << endl;

   POLYMOM_VECT_T oPolyVect;

   ifstream fInput;
   try
   {
      if( argc > 1 )
      {
         fInput.open( ppArgv[1] );
         parseInStream( oPolyVect, fInput );
      }
      else
         parseInStream( oPolyVect, cin );
      
      gpstr::PlotStream oPlot( "-p" );
      printPolynomVectFloat( oPlot, oPolyVect );
   }
   catch( daq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   
   return EXIT_SUCCESS;
}


//================================== EOF ======================================

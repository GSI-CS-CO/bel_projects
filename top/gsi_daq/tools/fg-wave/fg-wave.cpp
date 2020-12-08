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
#ifndef __DOCFSM__
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits.h>
#include <daq_calculations.hpp>
#include <daq_exception.hpp>
#include <daqt_messages.hpp>
#endif



using namespace std;
using namespace Scu;


#ifndef DEFAULT_LINE_STYLE
   #define DEFAULT_LINE_STYLE "lines"
#endif


struct POLYNOM_T
{
   int   a;
   uint  shiftA;
   int   b;
   uint  shiftB;
   int   c;
   uint  step;
   uint  frequ;
};

using POLYMOM_VECT_T = vector<POLYNOM_T>;

#define FSM_DECLARE_STATE( state, attr... ) state
enum STATE_T
{
   FSM_DECLARE_STATE( READ_COEFF_A ),
   FSM_DECLARE_STATE( READ_SHIFT_A ),
   FSM_DECLARE_STATE( READ_COEFF_B ),
   FSM_DECLARE_STATE( READ_SHIFT_B ),
   FSM_DECLARE_STATE( READ_COEFF_C ),
   FSM_DECLARE_STATE( READ_STEP ),
   FSM_DECLARE_STATE( READ_FREQUENCY )
};

string printReadState( const STATE_T state )
{
   #define _CASE_STATE( _s ) case _s: return #_s
   switch( state )
   {
      _CASE_STATE( READ_COEFF_A );
      _CASE_STATE( READ_SHIFT_A );
      _CASE_STATE( READ_COEFF_B );
      _CASE_STATE( READ_SHIFT_B );
      _CASE_STATE( READ_COEFF_C );
      _CASE_STATE( READ_STEP );
      _CASE_STATE( READ_FREQUENCY );
   }
   #undef _CASE_STATE
   return "unknown";
}

void throwStateMessage( const STATE_T state, const int line )
{
   string errorMessage = "Reading in line: ";
   errorMessage += to_string( line );
   errorMessage += ": Value of state: ";
   errorMessage += printReadState( state );
   errorMessage += " out of range!";
   throw( daq::Exception( errorMessage ) );
}

#define FSM_INIT_FSM( _state, attr... )      STATE_T state = _state
#define FSM_TRANSITION( newState, attr... ) state = newState; nextState = false; break
#define FSM_TRANSITION_NEXT( newState, attr... ) state = newState; break

int readFile( POLYMOM_VECT_T& rVect, istream& rInput )
{
   int i = 1;

   for( string oLine; getline( rInput, oLine ); i++ )
   {
      POLYNOM_T polynom;
      size_t pos = 0;
      bool nextState = true;
      FSM_INIT_FSM( READ_COEFF_A );
      do
      {
         int number;
         try
         {
            number = stoi( oLine, &pos );
         }
         catch( ... )
         {
            string errorMessage = "Reading in line: ";
            errorMessage += to_string( i );
            errorMessage += " on state: ";
            errorMessage += printReadState( state );
            errorMessage += " out of range!";
            throw( daq::Exception( errorMessage ) );
         }
         oLine = oLine.substr( pos );
         switch( state )
         {
            case READ_COEFF_A: // No break here.
            case READ_COEFF_B:
            {
               if( !gsi::isInRange( number, SHRT_MIN, SHRT_MAX ) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_SHIFT_A: // No break here.
            case READ_SHIFT_B:
            {
               if( !gsi::isInRange( number, 0,  static_cast<int>(BIT_SIZEOF(uint64_t)-BIT_SIZEOF(uint16_t))) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_COEFF_C:
            {
               if( !gsi::isInRange( number, INT_MIN, INT_MAX ) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_STEP:     // No break here.
            case READ_FREQUENCY:
            { /*
               * The value shall not exceed the size of 3 bits!
               */
               if( !gsi::isInRange( number, 0, 7 ) )
                  throwStateMessage( state, i );
               break;
            }

            default: break;
         }

         switch( state )
         {

            case READ_COEFF_A:
            {
               polynom.a = number;
               FSM_TRANSITION_NEXT( READ_SHIFT_A );
            }

            case READ_SHIFT_A:
            {
               polynom.shiftA = number;
               FSM_TRANSITION_NEXT( READ_COEFF_B );
            }

            case READ_COEFF_B:
            {
               polynom.b = number;
               FSM_TRANSITION_NEXT( READ_SHIFT_B );
            }

            case READ_SHIFT_B:
            {
               polynom.shiftB = number;
               FSM_TRANSITION_NEXT( READ_COEFF_C );
            }

            case READ_COEFF_C:
            {
               polynom.c = number;
               FSM_TRANSITION_NEXT( READ_STEP );
            }

            case READ_STEP:
            {
               polynom.step = number;
               FSM_TRANSITION_NEXT( READ_FREQUENCY );
            }

            case READ_FREQUENCY:
            {
               polynom.frequ = number;
               FSM_TRANSITION( READ_COEFF_A );
            }

            default: break;
         }
      }
      while( nextState );
      rVect.push_back( polynom );
   }
   return i;
}

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
   16000,
   32000,
   64000,
   125000,
   250000,
   500000,
   1000000,
   2000000
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


double calcPolynom( const double a, const double b, const double c, const double x )
{
   return x * (a * x + b) + c;
   //return c;
}

double calcPolynomTuple( const POLYNOM_T& polynom, const double x )
{
   return calcPolynom( static_cast<int64_t>( polynom.a ) << polynom.shiftA,
                       static_cast<int64_t>( polynom.b ) << polynom.shiftB,
                       static_cast<int64_t>( polynom.c ) << 32,
                       x
                     );
}



void printPolynomVectFloat( const POLYMOM_VECT_T& rVect )
{
   double t = 0.0;

   cout << "set grid" << endl;
   cout <<"set yrange [" << -(DAQ_VPP_MAX/2 + Y_PADDING) << ':'
                              << (DAQ_VPP_MAX/2 + Y_PADDING) << ']' << endl;
   cout << "plot '-' title 'set value' with " << DEFAULT_LINE_STYLE << " lc rgb 'green'" << endl;

   //for( int r = 0; r < 3; r++ )
   for( const auto& polynom: rVect )
   {
      double tStep = 0.0;
      for( int i = calcStep(polynom.step) ; i >= 0; i-- )
      {
         cout << t << ' ' << (calcPolynomTuple( polynom, tStep ) * DAQ_VPP_MAX / F_MAX) << endl;
         assert( polynom.frequ < ARRAY_SIZE( g_frequencyTab ) );
         tStep +=  (double)g_frequencyTab[polynom.frequ] / SCU_FREQUENCY;
        // tStep +=  1.0 / SCU_FREQUENCY;
         t += tStep;
      }

   }
   cout << 'e' << endl;
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
         readFile( oPolyVect, fInput );
      }
      else
         readFile( oPolyVect, cin );
   }
   catch( daq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   printPolynomVectFloat( oPolyVect );
   return EXIT_SUCCESS;
}


//================================== EOF ======================================

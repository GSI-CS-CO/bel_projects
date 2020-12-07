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
#include <daq_calculations.hpp>
#endif
using namespace std;

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
         int number = stoi( oLine, &pos );
         oLine = oLine.substr( pos );
         switch( state )
         {
            case READ_COEFF_A:
            case READ_COEFF_B:
            {
               break;
            }

            case READ_SHIFT_A:
            case READ_SHIFT_B:
            {
               break;
            }

            case READ_COEFF_C:
            {
               break;
            }

            case READ_STEP:
            {
               break;
            }

            case READ_FREQUENCY:
            {
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

void printPolynomVectFloat( const POLYMOM_VECT_T& rVect )
{
   for( const auto& polynom: rVect )
   {
#if 0
      long double coeffA = static_cast<double>(polynom.a) * pow( 2.0, polynom.shiftA );
      long double coeffB = static_cast<double>(polynom.b) * pow( 2.0, polynom.shiftB );
      long double coeffC = static_cast<double>(polynom.b) * pow( 2.0, 32.0 );
#else
      int64_t coeffA = static_cast<int64_t>( polynom.a ) << polynom.shiftA;
      int64_t coeffB = static_cast<int64_t>( polynom.b ) << polynom.shiftB;
      int64_t coeffC = static_cast<int64_t>( polynom.c ) << 32;
#endif
       cout  << coeffA << '\t'
            << coeffB << '\t'
            << coeffC << '\t' << (( coeffC * DAQ_VPP_MAX ) / F_MAX ) << '\t'
            << polynom.step << '\t'
            << polynom.frequ << endl;
   }
}


int main( int argc, char** ppArgv )
{
   cout << "arg=" << ppArgv[0] << endl;

   POLYMOM_VECT_T oPolyVect;

   ifstream fInput;
   if( argc > 1 )
   {
      fInput.open( ppArgv[1] );
      readFile( oPolyVect, fInput );
   }
   else
      readFile( oPolyVect, cin );

   printPolynomVect( oPolyVect );
   printPolynomVectFloat( oPolyVect );
   return EXIT_SUCCESS;
}


//================================== EOF ======================================

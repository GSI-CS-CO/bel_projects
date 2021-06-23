/*!
 * @file fgw_parser.cpp
 * @brief Module for parsing polynom input file-streams for
 *        SCU function generators
 *
 * @date 09.12.2020
 * @copyright (C) 2020 GSI Helmholtz Centre for Heavy Ion Research GmbH
 *
 * @author Ulrich Becker <u.becker@gsi.de>
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
#include "fgw_parser.hpp"

using namespace std;
using namespace Scu;

namespace fgw
{

#define FSM_DECLARE_STATE( state, attr... ) state
enum STATE_T
{
   FSM_DECLARE_STATE( READ_COEFF_A ),
   FSM_DECLARE_STATE( READ_SHIFT_A ),
   FSM_DECLARE_STATE( READ_COEFF_B ),
   FSM_DECLARE_STATE( READ_SHIFT_B ),
   FSM_DECLARE_STATE( READ_COEFF_C ),
   FSM_DECLARE_STATE( READ_STEP ),
   FSM_DECLARE_STATE( READ_FREQUENCY ),
   FSM_DECLARE_STATE( LINE_READY )
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
      _CASE_STATE( LINE_READY );
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

int parseInStream( POLYMOM_VECT_T& rVect, istream& rInput )
{
   int i = 1;

   for( string oLine; getline( rInput, oLine ); i++ )
   {
      if( oLine.empty() )
         continue;

      if( oLine.find_first_of( '#' ) != string::npos )
         continue;

      POLYNOM_T polynom;
      size_t pos = 0;
      bool nextState = true;

      FSM_INIT_FSM( READ_COEFF_A );
      do
      {
         long number;
         try
         {
            number = stol( oLine, &pos );
         }
         catch( ... )
         {
            string errorMessage = "Reading in line: ";
            errorMessage += to_string( i );
            errorMessage += " on state: ";
            errorMessage += printReadState( state );
            errorMessage += " unknown characters: \"";
            errorMessage += oLine;
            errorMessage += "\"!";
            throw( daq::Exception( errorMessage ) );
         }

         oLine = oLine.substr( pos );
         switch( state )
         {
            case READ_COEFF_A: // No break here.
            case READ_COEFF_B:
            {
               if( !gsi::isInRange( number, static_cast<long>(SHRT_MIN), static_cast<long>(SHRT_MAX) ) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_SHIFT_A: // No break here.
            case READ_SHIFT_B:
            {
               if( !gsi::isInRange( number, 0L,  static_cast<long>(BIT_SIZEOF(uint64_t)-BIT_SIZEOF(uint16_t))) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_COEFF_C:
            {
               if( !gsi::isInRange( number, static_cast<long>(LONG_MIN), static_cast<long>(LONG_MAX) ) )
                  throwStateMessage( state, i );
               break;
            }

            case READ_STEP:     // No break here.
            case READ_FREQUENCY:
            { /*
               * The value shall not exceed the size of 3 bits!
               */
               if( !gsi::isInRange( number, 0L, 7L ) )
                  throwStateMessage( state, i );
               break;
            }

            default: break;
         }

         switch( state )
         {

            case READ_COEFF_A:
            {
               polynom.coeff_a = number;
               FSM_TRANSITION_NEXT( READ_SHIFT_A );
            }

            case READ_SHIFT_A:
            {
               polynom.control.bv.shift_a = number;
               FSM_TRANSITION_NEXT( READ_COEFF_B );
            }

            case READ_COEFF_B:
            {
               polynom.coeff_b = number;
               FSM_TRANSITION_NEXT( READ_SHIFT_B );
            }

            case READ_SHIFT_B:
            {
               polynom.control.bv.shift_b = number;
               FSM_TRANSITION_NEXT( READ_COEFF_C );
            }

            case READ_COEFF_C:
            {
               polynom.coeff_c = number;
               FSM_TRANSITION_NEXT( READ_STEP );
            }

            case READ_STEP:
            {
               polynom.control.bv.step = number;
               FSM_TRANSITION_NEXT( READ_FREQUENCY );
            }

            case READ_FREQUENCY:
            {
               polynom.control.bv.frequency = number;
               FSM_TRANSITION( LINE_READY );
            }

            default: break;
         }
      }
      while( nextState );

      for( const auto& c: oLine )
      {
         if( c != ' ' )
         {
            string errorMessage = "Extra characters found in line ";
            errorMessage += to_string( i );
            errorMessage += ": \"";
            errorMessage += oLine;
            errorMessage += "\" !";
            throw( daq::Exception( errorMessage ) );
         }
      }

      rVect.push_back( polynom );
   }
   return i;
}

} // namespace fgw
//================================== EOF ======================================

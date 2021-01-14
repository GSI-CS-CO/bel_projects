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
#include <cfloat>
#include <daqt_read_stdin.hpp>
#include <fgw_parser.hpp>
#include <fgw_commandline.hpp>
#include <gnuplotstream.hpp>
#include <fgw_polynom.hpp>

using namespace std;
using namespace Scu;
using namespace fgw;

/*! ---------------------------------------------------------------------------
 */
void static printPolynomVect( const POLYMOM_VECT_T& rVect )
{
   for( const auto& polynom: rVect )
   {
      cout << polynom.a << ' '
           << polynom.shiftA << ' '
           << polynom.b << ' '
           << polynom.shiftB << ' '
           << polynom.c << ' '
           << polynom.step << ' '
           << polynom.frequ
           << endl;
   }
}

/*! ---------------------------------------------------------------------------
 */
int static printInfo( const POLYMOM_VECT_T& rVect, const bool beVerbose )
{
   if( rVect.size() == 0 )
      return EXIT_FAILURE;

   double interruptTimeMax = 0.0;
   double interruptTimeMin = DBL_MAX;
   double periodTime = 0.0;
   for( const auto& polynom: rVect )
   {
      assert( polynom.frequ < ARRAY_SIZE( Polynom::c_timeTab ) );
      double iTime = Polynom::calcStep( polynom.step ) * Polynom::c_timeTab[polynom.frequ];
      interruptTimeMax = max( interruptTimeMax, iTime );
      interruptTimeMin = min( interruptTimeMin, iTime );
      periodTime += iTime;
   }

   assert( rVect.size() != 0 );
   const double interruptTimeAvr = periodTime / rVect.size();

   if( beVerbose )
      cout << "min: ";
   cout << interruptTimeMin << ' ';

   if( beVerbose )
      cout << "sec,\t";
   if( interruptTimeMin != 0.0 )
   {
      cout << (1.0 / interruptTimeMin);
      if( beVerbose )
         cout << " Hz";
   }
   cout << '\n';

   if( beVerbose )
      cout << "avr: ";
   cout << interruptTimeAvr << ' ';
   if( beVerbose )
      cout << "sec,\t";
   if( interruptTimeAvr != 0.0 )
   {
      cout << (1.0 / interruptTimeAvr);
      if( beVerbose )
         cout << " Hz";
   }
   cout << '\n';

   if( beVerbose )
      cout << "max: ";
   cout << interruptTimeMax << ' ';
   if( beVerbose )
      cout << "sec,\t";
   if( interruptTimeMax != 0.0 )
   {
      cout << (1.0 / interruptTimeMax);
      if( beVerbose )
         cout << " Hz";
   }
   cout << '\n';

   if( beVerbose )
      cout << "per: ";
   cout << periodTime << ' ';
   if( beVerbose )
      cout << "sec,\t";
   if( periodTime != 0.0 )
   {
      cout << (1.0 / periodTime);
      if( beVerbose )
         cout << " Hz";
   }
   cout << '\n';

   if( beVerbose )
      cout << "Tuples: ";
   cout << rVect.size() << endl;

   return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
int main( int argc, char** ppArgv )
{
   try
   {
      CommandLine oCmdLine( argc, ppArgv );
      istream* pIstream = oCmdLine();
      if( pIstream == nullptr )
         return EXIT_SUCCESS;

      POLYMOM_VECT_T oPolyVect;
      parseInStream( oPolyVect, *pIstream );

      if( oCmdLine.isDoStrip() )
      {
         for( uint i = oCmdLine.getRepetitions(); i > 0; i-- ) 
            printPolynomVect( oPolyVect );
         return EXIT_SUCCESS;
      }

      if( oCmdLine.isDoInfo() )
      {
         return printInfo( oPolyVect, oCmdLine.isVerbose() );
      }

      string gnuplotCmdLine;
      if( oCmdLine.isDoQuit() )
         gnuplotCmdLine +=  "-p";
      gpstr::PlotStream oPlot( gnuplotCmdLine );

      Polynom polynom( oCmdLine );

      polynom.plot( oPlot, oPolyVect );

      if( oCmdLine.isDoQuit() )
         return EXIT_SUCCESS;

      Terminal oTerminal;
      int key;
      if( oCmdLine.isVerbose() )
         cout << "Press Esc to exit." << endl;
      while( (key = Terminal::readKey()) != '\e' )
      {
         ::usleep( 100 );
      }
   }
   catch( daq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   
   return EXIT_SUCCESS;
}

//================================== EOF ======================================

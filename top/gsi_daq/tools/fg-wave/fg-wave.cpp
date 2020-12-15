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

///////////////////////////////////////////////////////////////////////////////
int main( int argc, char** ppArgv )
{
#if 0
   for( int i = 0; i < 10; i++ )
      cout << i << ": " << daq::calcPolynom( 0, 1, 0, i ) << endl;
   return EXIT_SUCCESS;
#else
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
      gpstr::PlotStream oPlot( "-p -noraise" );
      Polynom polynom( oCmdLine );
      polynom.plot( oPlot, oPolyVect );
   }
   catch( daq::Exception& e )
   {
      ERROR_MESSAGE( "Exception occurred: \"" << e.what() << '"' );
      return EXIT_FAILURE;
   }
   
   return EXIT_SUCCESS;
#endif
}

//================================== EOF ======================================

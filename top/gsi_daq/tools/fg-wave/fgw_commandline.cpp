/*!
 *  @file fgw_commandline.cpp
 *  @brief Commandline parser for program fg-wave
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
#include <helper_macros.h>
#include "fgw_commandline.hpp"


namespace fgw
{

using namespace std;
using namespace CLOP;

#ifndef GNUPLOT_DEFAULT_TERMINAL
   #define GNUPLOT_DEFAULT_TERMINAL "X11 size 1200,600"
#endif
#ifndef DEFAULT_DOTS_PER_TUPLE
   #define DEFAULT_DOTS_PER_TUPLE 10
#endif

/*! ---------------------------------------------------------------------------
 * @brief Initializing the command line options.
 */
CommandLine::OPT_LIST_T CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         cout << "Wave viewer plotting wave-files for SCU- function generators.\n"
                 "(c) 2020 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n\n"
                 "Usage:\n\t"
              << poParser->getProgramName() << " [options] <wave-file.fgw>\n"
                 "or\n"
                 "\techo -e <coeff_a shift_a coeff_b shift_b coeff_c step frequ_select> "
                 "| " << poParser->getProgramName() << " [options]\n\n"
                 "The format of the \"wave-file\" is identical like for the"
                 " saft-lib service program \"saft-fg-ctl\".\n"
                 "The calculation of the quadratic polynomial is:\n\t" ESC_BOLD
                 "f(x) = (coeff_a * 2^shift_a) / 2 * x^2 + (coeff_b * 2^shift_b) * x + (coeff_c * 2^32);\n"
                 "\t{0 <= x < (250*2^step)}\n"
                 "\t{0 <= step <= 7}\n\n" ESC_NORMAL
                 "Options:\n";
         poParser->list( cout );
         cout << "\n----------------------------------------\n"
                 "Example 1 file reading:\n\t"
              << poParser->getProgramName() << " sinus.fgw\n\n"
                 "Example 2 pipe mode, plotting three tuples for five times with 100 dots per tuple:\n\t"
              << "echo -e \"0 0 0 0 2147483647 3 6\\n0 0 0 0 -2147483648 4 6\\n0 0 0 0 0 3 6\" | "
              << poParser->getProgramName() << " -Qd100 -r5\n\n"
                 "Example 3 creating a fgw-file with 120 polynomials:\n\t"
                 "echo -e \"0 0 0 0 2147483647 3 6\\n0 0 0 0 -2147483648 4 6\\n0 0 0 0 0 3 6\" | "
              << poParser->getProgramName() << " -Sr40 >rect120.fgw\n"
              << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'h',
      .m_longOpt  = "help",
      .m_helpText = "Print this help and exit"
   },
#ifdef CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         string name = poParser->getProgramName().substr(poParser->getProgramName().find_last_of('/')+1);
         cout <<
            "<toolinfo>\n"
               "\t<name>" << name << "</name>\n"
               "\t<topic>Development, Release, Rollout</topic>\n"
               "\t<description>Plots the graph of files for SCU function generators via Gnuplot.</description>\n"
               "\t<usage>" << name << " {wave-file.fgw}";
               for( const auto& pOption: *poParser )
               {
                  if( pOption->m_id != 0 )
                     continue;
                  cout << " [";
                  if( pOption->m_shortOpt != '\0' )
                  {
                     cout << '-' << pOption->m_shortOpt;
                     if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                        cout << " ARG";
                     if( !pOption->m_longOpt.empty() )
                        cout << ", ";
                  }
                  if( !pOption->m_longOpt.empty() )
                  {
                     cout << "--" << pOption->m_longOpt;
                     if( pOption->m_hasArg == OPTION::REQUIRED_ARG )
                        cout << " ARG";
                  }
                  cout << ']';
               }
               cout << "\n\t</usage>\n"
               "\t<author>Ulrich Becker</author>\n"
               "\t<tags>graphics</tags>\n"
               "\t<version>" TO_STRING( VERSION ) "</version>\n"
               "\t<documentation></documentation>\n"
               "\t<environment></environment>\n"
               "\t<requires>Gnuplot</requires>\n"
               "\t<autodocversion>1.0</autodocversion>\n"
            "</toolinfo>"
         << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 1, // will hide this option for autodoc
      .m_shortOpt = '\0',
      .m_longOpt  = "generate_doc_tagged",
      .m_helpText = "Will need from autodoc."
   },
#endif // ifdef CONFIG_AUTODOC_OPTION
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_verbose = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'v',
      .m_longOpt  = "verbose",
      .m_helpText = "Be verbose"
   },
   {
      OPT_LAMBDA( poParser,
      {
         if( static_cast<CommandLine*>(poParser)->m_verbose )
         {
            cout << "Version: " TO_STRING( VERSION )
                    ", Git revision: " TO_STRING( GIT_REVISION ) << endl;
         }
         else
         {
            cout << TO_STRING( VERSION ) << endl;
         }
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'V',
      .m_longOpt  = "version",
      .m_helpText = "Print the software version and exit."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_zoomYAxis = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'z',
      .m_longOpt  = "zoom",
      .m_helpText = "Zooming of the Y-axis (voltage axis) in GNUPLOT "
                    "(auto-scaling)."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_noSquareTerm = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'q',
      .m_longOpt  = "nosquare",
      .m_helpText = "Suppresses the calculation of the square term.\n"
                    "From the full calculation of [f(x) = a * x^2 + b * x + c],"
                    " only [f(x) = b * x + c] becomes calculated."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_noLinearTerm = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'l',
      .m_longOpt  = "nolinear",
      .m_helpText = "Suppresses the calculation of the linear term.\n"
                    "From the full calculation of [f(x) = a * x^2 + b * x + c],"
                    " only [f(x) = a * x^2 + c] becomes calculated."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint repeat;
         if( readInteger( repeat, poParser->getOptArg() ) )
            return -1;
         if( repeat == 0 )
         {
            ERROR_MESSAGE( "The repeat value of 0 is not permitted!" );
            return -1;
         }
         if( repeat > 10 )
         {
            WARNING_MESSAGE( "A repeat value of " << repeat << " could take"
                             " some time for calculation!" );
         }
         static_cast<CommandLine*>(poParser)->m_repetitions = repeat;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'r',
      .m_longOpt  = "repeat",
      .m_helpText = "Repeats the plot of the waveform for PARAM times."
                    " The value of 1 is the default."
                    " The value of 0 is not permitted!"
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_gnuplotLineStyle =
                                                        poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 's',
      .m_longOpt  = "style",
      .m_helpText = "Setting of the Gnuplot line-style default is: \""
                     DEFAULT_LINE_STYLE "\""
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_gnuplotTerminal =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'T',
      .m_longOpt  = "terminal",
      .m_helpText = "PARAM replaces the terminal which is used by Gnuplot."
                    " Default is: \"" GNUPLOT_DEFAULT_TERMINAL "\""
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint dotsPerTuple;
         if( readInteger( dotsPerTuple, poParser->getOptArg() ) )
            return -1;
         static_cast<CommandLine*>(poParser)->m_dotsPerTuple = dotsPerTuple; 
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "dotsPerTuple",
      .m_helpText = "PARAM is the number of dots per tuple which will plot.\n"
                    "This reduces the calculation time but it increases the granularity.\n"
                    "The default value is " TO_STRING( DEFAULT_DOTS_PER_TUPLE ) " dots/tuple.\n"
                    "The exception is zero that means that all dots will plotted."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_doStrip = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'S',
      .m_longOpt  = "strip",
      .m_helpText = "Prints the entire fgw-file in stdout but without empty "
                    "lines and/or comment-lines with the leading character #.\n"
                    "NOTE:\n"
                    "In combination with option \"-r, --repeat\" it becomes possible "
                    "to expand the number of polynomials.\n"
                    "Because by the current version of SaftLib and LM32 firmware, "
                    "the number of polynomials should not fall below a minimum number.\n"
                    "If not, the frequency of \"REFILL_REQUEST\" to the SaftLib becomes increased.\n"
                    "At the moment the number of polynomials is 121 per channel."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_doInfo = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'i',
      .m_longOpt  = "info",
      .m_helpText = "Prints the information of minimum and maximum interrupt frequency of the given wave-file."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_doQuit = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'Q',
      .m_longOpt  = "quit",
      .m_helpText = "Program will terminate immediately after plotting.\n"
                    "This is meaningful when the data has been read in via the standard input rather than a file.\n"
                    "The disadvantage is that in this way its not possible to zoom the graph via mouse."
   },
   {
      OPT_LAMBDA( poParser,
      {
         if( poParser->isOptArgPersent() )
            static_cast<CommandLine*>(poParser)->m_gnuplotCoeffCLineStyle = poParser->getOptArg();
         else
            static_cast<CommandLine*>(poParser)->m_gnuplotCoeffCLineStyle = "points";
         return 0;
      }),
      .m_hasArg   = OPTION::OPTIONAL_ARG,
      .m_id       = 0,
      .m_shortOpt = 'c',
      .m_longOpt  = "ccoeff",
      .m_helpText = "Extra plot of the coefficient \"c\" of the polynomial"
                    " \"ax^2 + bx + c\".\n"
                    "The optional parameter PARAM gives the line-style of Gnuplot."
                    " The default is \"points\"."
   }
}; // CommandLine::c_optList
   
///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
*/
bool CommandLine::readInteger( uint& rValue, const string& roStr )
{
   try
   {
      rValue = stoi( roStr );
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "Integer number is expected and not that: \""
                     << roStr << "\" !" );
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,m_verbose( false )
   ,m_zoomYAxis( false )
   ,m_noSquareTerm( false )
   ,m_noLinearTerm( false )
   ,m_doStrip( false )
   ,m_doInfo( false )
   ,m_doQuit( false )
   ,m_repetitions( 1 )
   ,m_dotsPerTuple( DEFAULT_DOTS_PER_TUPLE )
   ,m_gnuplotTerminal( GNUPLOT_DEFAULT_TERMINAL )
   ,m_gnuplotLineStyle( DEFAULT_LINE_STYLE )
   ,m_fileName( "stdin" )
   ,m_pInStream( nullptr )
{
   add( c_optList );
   sortShort();
}

/*-----------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
   if( m_pInStream != nullptr )
      delete m_pInStream;
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
   if( m_pInStream != nullptr )
   {
      ERROR_MESSAGE( "To much arguments! Only the filename is required." );
      return -1;
   }
   m_fileName = getArgVect()[getArgIndex()];
   m_pInStream = new ifstream;
   m_pInStream->open( m_fileName, ifstream::in );
   return 1;
}

/*-----------------------------------------------------------------------------
 */
std::istream& CommandLine::getIStream( void )
{
   if( m_pInStream != nullptr )
      return *m_pInStream;

   return cin;
}

/*-----------------------------------------------------------------------------
 */
std::istream* CommandLine::operator()( void )
{
   if( PARSER::operator()() < 0 )
      return nullptr;

   return &getIStream();
}

} // namespace fgw
//================================== EOF ======================================

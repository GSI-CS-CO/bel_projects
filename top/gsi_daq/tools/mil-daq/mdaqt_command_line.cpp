/*!
 *  @file mdaqt_command_line.cpp
 *  @brief Command line parser of MIL-DAQ-Test
 *
 *  @date 26.08.2019
 *  @copyright (C) 2019 GSI Helmholtz Centre for Heavy Ion Research GmbH
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
#include <mdaqt_command_line.hpp>

using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;

vector<OPTION> CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         cout << "Usage: " << poParser->getProgramName()
              << " <proto/host/port> [global-options] "
                 "[<slot-number> [device-options] "
                 "<channel-number> [channel-options]] \n\n"
                 "Global-options can be overwritten by device-options "
                 "and device options can be overwritten by channel-options.\n"
                 "NOTE: The lowest slot-number begins at 1; "
                 "the lowest channel-number begins at 1.\n\n"
                 "Hot keys:\n";
         poParser->list( cout );
         cout << endl;
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'h',
      .m_longOpt  = "help",
      .m_helpText = "Print this help and exit"
   },
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
    //     if( getDaqAdmin( poParser ) == nullptr )
    //        return -1;

         static_cast<CommandLine*>(poParser)->m_gnuplotBin =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'G',
      .m_longOpt  = "gnuplot",
      .m_helpText = "Replacing of the default Gnuplot binary by the in PARAM"
                    " given binary. Default is: " GPSTR_DEFAULT_GNUPLOT_EXE
   },
   {
      OPT_LAMBDA( poParser,
      {
   //      if( getDaqAdmin( poParser ) == nullptr )
   //         return -1;

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
     //    if( getDaqAdmin( poParser ) == nullptr )
     //       return -1;

         static_cast<CommandLine*>(poParser)->m_gnuplotOutput =
                                                       poParser->getOptArg();
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'o',
      .m_longOpt  = "output",
      .m_helpText = "Setting the prefix and suffix file name for Gnuplot."
                    " PARAM is the path and name of the output file.\n"
                    "NOTE: The final file name becomes generated as follows:\n"
                    "      <SUFFIX>_<SCU-name>_<slot number>_<channel number>_"
                    "<wr-time stamp>.<PREFIX>\n"
                    "Example: PARAM = myFile.png:\n"
                    "         result: myFile_scuxl0035_acc_gsi_de_3_1_"
                    "12439792657334272.png"
   }

};


///////////////////////////////////////////////////////////////////////////////
/*! ---------------------------------------------------------------------------
*/
bool CommandLine::readInteger( unsigned int& rValue, const string& roStr )
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
   ,m_gnuplotBin( GPSTR_DEFAULT_GNUPLOT_EXE )
   ,m_gnuplotTerminal( GNUPLOT_DEFAULT_TERMINAL )
{
   add( c_optList );
}

/*-----------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onArgument( void )
{
   string arg = getArgVect()[getArgIndex()];
   return 1;
}

// ================================= EOF ======================================

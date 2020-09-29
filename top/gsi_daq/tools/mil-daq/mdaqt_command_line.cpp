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
#ifndef __DOCFSM__
 #include <daqt_onFoundProcess.hpp>
#endif

#include "mdaqt_command_line.hpp"

using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;

#ifndef MINIMUM_X_AXIS
   #define MINIMUM_X_AXIS 1.0
#endif
#ifndef MAXIMUM_X_AXIS
   #define MAXIMUM_X_AXIS 300.0
#endif

#define FSM_INIT_FSM( state, attr... )      m_state( state )
#define FSM_TRANSITION( newState, attr... ) m_state = newState

/*! ---------------------------------------------------------------------------
 */
STATIC MilDaqAdministration* getDaqAdministration( CommandLine* pCmdLine )
{
   MilDaqAdministration* pAllDaq = pCmdLine->getDaqAdminPtr();
   if( pAllDaq == nullptr )
   {
      ERROR_MESSAGE( "SCU target has to be specified before!" );
   }
   return pAllDaq;
}

/*! ---------------------------------------------------------------------------
 * @brief Helper function for the options "-S" and "-L"
 * @param pCmdLine Pointer to command line object.
 * @param doScan If true so the LM32 performs a re-scan.
 */
STATIC int listOrScanFGs( CommandLine* pCmdLine, bool doScan )
{
   MilDaqAdministration* pAllDaq = getDaqAdministration( pCmdLine );
   if( pAllDaq == nullptr )
      return -1;

   const bool verbose = pCmdLine->isVerbose();
   if( doScan )
   {
      if( verbose )
         cout << "scanning..." << endl;
      pAllDaq->scan();
   }
   for( const auto& fg: pAllDaq->getFgList() )
   {
      if( !fg.isMIL() && !verbose )
         continue;
      if( verbose )
      {
         cout << "Slot: " << fg.getSlot() <<
                 ", Bits: " << fg.getOutputBits() <<
                 ", Version: " << fg.getVersion() << ",\t" <<
                 (fg.isMIL()? "MIL":"ADAC") << " device\t";
      }
      cout << "fg-" << fg.getSocket() << '-' << fg.getDevice() << endl;
   }
   ::exit( EXIT_SUCCESS );
   return 0;
}

/*! ---------------------------------------------------------------------------
 * @brief Initializing the command line options.
 */
vector<OPTION> CommandLine::c_optList =
{
   {
      OPT_LAMBDA( poParser,
      {
         cout << "MIL-DAQ Plotter\n"
                 "(c) 2019 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n"
              << "Usage: " << poParser->getProgramName()
              << " <SCU- target IP-address> [options] [slot channel [slot channel ...]]\n\n"
                 "Hot keys:\n"
              << HOT_KEY_RESET    << ": Reset zooming of all plot windows\n"
              << HOT_KEY_RECEIVE  << ": Toggling receiving on / off\n"
              << HOT_KEY_TOGGLE_SINGLE_SHOOT << ": Toggling single shoot mode on / off\n"
                 "Esc: Program termination\n"
                 "\nCommandline options:\n";
         poParser->list( cout );
         cout << "\n--------------------------------------------------" << endl;
         cout << "Example a:\n\t" ESC_BOLD << poParser->getProgramName() << " scuxl4711 -ac" ESC_NORMAL
                 "\n\tor"
                 "\n\t" ESC_BOLD << poParser->getProgramName() <<  " tcp/scuxl4711 -ac" ESC_NORMAL
                 "\n\n\tWill make a plot of all found MIL-function-generators.\n\n"
                 "Example b:\n"
                 "\tStep 1: Scanning for connected MIL-function generators:\n"
                 "\n\t\t" ESC_BOLD << poParser->getProgramName() << " scuxl4711 -S" ESC_NORMAL
                 "\n\n\tResut (e.g.):\n"
                 "\t\tfg-39-1\n"
                 "\t\tfg-39-2\n"
                 "\t\tfg-39-129\n"
                 "\t\tfg-39-130\n"
                 "\tIn this example four MIL- function generators was found.\n\n"
                 "\tStep 2: Now we intend to see the plots of \"fg-39-2\" and \"fg-39-130\" only:\n"
                 "\n\t\t" ESC_BOLD << poParser->getProgramName() << " scuxl4711 -c 39 2 39 130\n" ESC_NORMAL
                 << endl;
         cout << "NOTE:\n\tPrerequisite for this program is, that the port forwarder demon \"socat\" runs\n"
                 "\tin the concerning SCU.\n"
                 "\tIf not already running so you can accomplish that by invoking the"
                 " shell script \"start-socat.sh\".\n" << endl;

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
               "\t<description>Display actual- and set-values of MIL function generators via Gnuplot.</description>\n"
               "\t<usage>" << name << " {SCU- target IP-address}";
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
               "\t<tags>graphics,etherbone</tags>\n"
               "\t<version>" TO_STRING( VERSION ) "</version>\n"
               "\t<documentation></documentation>\n"
               "\t<environment></environment>\n"
               "\t<requires>Gnuplot, socat, LM32-firmware scu_control</requires>\n"
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
         static_cast<CommandLine*>(poParser)->m_plotAlwaysSetValue = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'l',
      .m_longOpt  = "always",
      .m_helpText = "Plots always the set-value, even within a gap.\n"
                    "Within a gab the last valid set-value will used.\n"
                    "NOTE: In general, a gap-plotting needs a actual LM32 firmware,"
                    " written by UB."
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
         static_cast<CommandLine*>(poParser)->m_autoBuilding = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'a',
      .m_longOpt  = "auto",
      .m_helpText = "Automatically building of channel plot windows.\n"
                    "That means no further arguments of slot and channel"
                    " number necessary."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_deviationEnable = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'd',
      .m_longOpt  = "deviation",
      .m_helpText = "Enabling of plotting the deviation graph: "
                    "set value minus actual value."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_continuePlotting = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'c',
      .m_longOpt  = "continue",
      .m_helpText = "Plotting continuously and not when enough data present "
                     "only."
   },
   {
      OPT_LAMBDA( poParser,
      {
         float temp;
         if( readFloat( temp, poParser->getOptArg() ) )
            return -1;
         if( temp < 0.0 )
         {
            ERROR_MESSAGE( "A negative time of " << temp
                          << "  doesn't exist!" );
            return -1;
         }
         if( temp < MINIMUM_X_AXIS )
         {
            ERROR_MESSAGE( "Value of X axis is to small, expecting at least "
                           TO_STRING(MINIMUM_X_AXIS) " and not " << temp
                           << " !" );
            return -1;
         }
         if( temp > MAXIMUM_X_AXIS )
         {
            ERROR_MESSAGE( "Value of X axis it to large, expecting a maximum "
                           "of " TO_STRING(MAXIMUM_X_AXIS) " and not " << temp
                           << " !" );
            return -1;
         }
         static_cast<CommandLine*>(poParser)->m_xAxisLen = temp;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 't',
      .m_longOpt  = "time",
      .m_helpText = "Length of the X-axis (time axis) in a range of "
                    TO_STRING(MINIMUM_X_AXIS) " to " TO_STRING(MAXIMUM_X_AXIS)
                    " in seconds.\n"
                    "If this option not given, so the default value of "
                    TO_STRING(DEFAULT_X_AXIS_LEN) " seconds will used."
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
#if 0
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
                    "         result: myFile_scuxl4711_acc_gsi_de_39_130_"
                    "12439792657334272.png"
   }
#endif
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
         return listOrScanFGs( static_cast<CommandLine*>(poParser), true );
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'S',
      .m_longOpt  = "scan",
      .m_helpText = "Scanning for all connected function generators.\n"
                    "NOTE: The verbosity mode (option -v has to be set before) "
                    "will show all function-generators,\notherwise only MIL- "
                    "function-generators will shown.\n"
                    ESC_BOLD "CAUTION: Don't use this option during function-"
                    "generators are running! Otherwise the timing becomes disturbed \n"
                    "and the function-generators will stopped!\n" ESC_NORMAL
                    "If you will list the found function generators without scanning only,"
                    " so use the option \"-L\" respectively \"--list\"."
   },
   {
      OPT_LAMBDA( poParser,
      {
         return listOrScanFGs( static_cast<CommandLine*>(poParser), false );
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'L',
      .m_longOpt  = "list",
      .m_helpText = "Lists all connected function generators.\n"
                    "NOTE: The verbosity mode (option -v has to be set before) "
                    "will show all function-generators,\notherwise only MIL- "
                    "function-generators will shown.\n"
   },
   {
      OPT_LAMBDA( poParser,
      {
         MilDaqAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
         if( pAllDaq == nullptr )
            return -1;
         pAllDaq->sendSwi( FG::FG_OP_PRINT_HISTORY );
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'H',
      .m_longOpt  = "history",
      .m_helpText = "Triggers the LM32-firmware to print the entire history "
                    "into the etherbone-console.\n"
                    "NOTE: This option is only meaningful if a etherbone-console via "
                    "program \"eb-console\" eg: \"eb-console tcp/scuxl4711\" is open "
                    "before.\n"
                    ESC_BOLD "CAUTION: Using this option will destroy the timing! "
                    "Don't use it in the real production environment!" ESC_NORMAL
   },
   {
      OPT_LAMBDA( poParser,
      {
         MilDaqAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
         if( pAllDaq == nullptr )
            return -1;

         uint gapInterval;
         if( readInteger( gapInterval, poParser->getOptArg() ) )
            return -1;

         pAllDaq->sendSwi( FG::FG_OP_MIL_GAP_INTERVAL, gapInterval );
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'A',
      .m_longOpt  = "gap",
      .m_helpText = "Activates or deactivates the gap reading. "
                    "PARAM is the gap reading interval in milliseconds. "
                    "A value of zero deactivates the gap reading."
   }
};


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

/*! ---------------------------------------------------------------------------
*/
bool CommandLine::readFloat( float& rValue, const string& roStr )
{
   try
   {
      rValue = stof( roStr );
   }
   catch( std::exception& e )
   {
      ERROR_MESSAGE( "Floating point number is expected and not that: \""
                     << roStr << "\" !" );
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,FSM_INIT_FSM( READ_EB_NAME )
   ,m_targetUrlGiven( false )
   ,m_numDevs( 0 )
   ,m_numChannels( 0 )
   ,m_optionError( false )
   ,m_verbose( false )
   ,m_autoBuilding( false )
   ,m_deviationEnable( false )
   ,m_continuePlotting( false )
   ,m_plotAlwaysSetValue( false )
   ,m_zoomYAxis( false )
   ,m_xAxisLen( DEFAULT_X_AXIS_LEN )
   ,m_poAllDaq( nullptr )
   ,m_poCurrentDevice( nullptr )
   ,m_poCurrentChannel( nullptr )
   ,m_gnuplotBin( GPSTR_DEFAULT_GNUPLOT_EXE )
   ,m_gnuplotTerminal( GNUPLOT_DEFAULT_TERMINAL )
   ,m_gnuplotLineStyle( DEFAULT_LINE_STYLE )
{
   add( c_optList );
   sortShort();
}

/*-----------------------------------------------------------------------------
 */
CommandLine::~CommandLine( void )
{
   if( m_poAllDaq != nullptr )
       delete m_poAllDaq;
}

/*-----------------------------------------------------------------------------
 */
MilDaqAdministration* CommandLine::operator()( void )
{
   if( getArgCount() < 2 )
   {
      ERROR_MESSAGE( "Missing argument!" );
      return nullptr;
   }

   if( PARSER::operator()() < 0 )
      return nullptr;

   if( m_poAllDaq != nullptr )
      return m_poAllDaq;

   if( !m_targetUrlGiven )
   {
      ERROR_MESSAGE( "Missing target!" );
      return nullptr;
   }

   if( m_numDevs == 0 )
   {
      ERROR_MESSAGE( "No slot(s) given!" );
      return nullptr;
   }

   if( m_numChannels == 0 )
   {
      ERROR_MESSAGE( "No channel(s) given!" );
      return nullptr;
   }
   return nullptr;
}

/*!----------------------------------------------------------------------------
 * @dotfile mdaqt_command_line.gv
 */
int CommandLine::onArgument( void )
{
   string arg = getArgVect()[getArgIndex()];
   uint number;
   switch( m_state )
   {
      case READ_SLOT:
      case READ_CHANNEL:
      {
         assert( m_poAllDaq != nullptr );
         if( readInteger( number, arg ) )
            return -1;
         break;
      }
      default: break;
   }
   switch( m_state )
   {
      case READ_EB_NAME:
      {
         assert( m_poAllDaq == nullptr );
         m_targetUrlGiven = true;
         if( arg.find( "tcp/" ) == string::npos )
            arg = "tcp/" + arg;
#if 0
         if( daq::isConcurrentProcessRunning( getProgramName(), arg ) )
            return -1;
#endif

         m_poAllDaq = new MilDaqAdministration( this, arg );
         FSM_TRANSITION( READ_SLOT );
         break;
      }
      case READ_SLOT:
      {
         m_numDevs++;
         m_poCurrentChannel = nullptr;
#if 0
         if( !gsi::isInRange( number, DaqInterface::c_startSlot,
                                      DaqInterface::c_maxSlots ) )
         {
            ERROR_MESSAGE( "Given slot " << number <<
                           " is out of the range of: " <<
                           DaqInterface::c_startSlot << " and " <<
                           DaqInterface::c_maxSlots << " !" );
            return -1;
         }
#endif
         if( !m_poAllDaq->isSocketUsed( number ) )
         {
            ERROR_MESSAGE( "No device in socket " << number << " present!" );
            return -1;
         }
         m_poCurrentDevice = m_poAllDaq->getDevice( number );
         if( m_poCurrentDevice == nullptr )
         {
            m_poCurrentDevice = new Device( number );
            m_poAllDaq->registerDevice( m_poCurrentDevice );
         }
         FSM_TRANSITION( READ_CHANNEL );
         break;
      }
      case READ_CHANNEL:
      {
         assert( m_poCurrentDevice != nullptr );
         assert( m_poCurrentChannel == nullptr );
         m_numChannels++;
#if 0
         if( !gsi::isInRange( number, static_cast<uint>( 1 ),
                                      DaqInterface::c_maxChannels ) )
         {
            ERROR_MESSAGE( "Requested channel " << number <<
                           " is out of range of " <<
                           DaqInterface::c_maxChannels << " !" );
            return -1;
         }
#endif
         if( !m_poAllDaq->isPresent( m_poCurrentDevice->getLocation(), number ) )
         {
            ERROR_MESSAGE( "No device in socket " << m_poCurrentDevice->getLocation()
                            << " with the number " << number << " present!" );
            return -1;
         }
         if( m_poCurrentDevice->getDaqCompare( number ) == nullptr )
         {
            m_poCurrentChannel = new DaqMilCompare( number );
            m_poCurrentDevice->registerDaqCompare( m_poCurrentChannel );
         }
         else
         {
            ERROR_MESSAGE( "Channel number specified several times: " << number );
         }
         FSM_TRANSITION( READ_SLOT );
         break;
      }
   }
   return 1;
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedShortOption( char unrecognized )
{
   ERROR_MESSAGE( "Unknown option: '-" << unrecognized << '\'' );
   m_optionError = true;
   return -1;
}

/*-----------------------------------------------------------------------------
 */
int CommandLine::onErrorUnrecognizedLongOption( const std::string& unrecognized )
{
   ERROR_MESSAGE( "Unknown option: \"--" << unrecognized << '"' );
   m_optionError = true;
   return -1;
}

// ================================= EOF ======================================

/*!
 *  @file fb_command_line.cpp
 *  @brief Command line parser of MIL-DAQ-Test
 *
 *  @date 09.10.2020
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
 #include <daqt_onFoundProcess.hpp>
#include <scu_env.hpp>
#endif

#include "fb_command_line.hpp"
//#include "scu_fg_feedback.hpp"
//using namespace Scu::MiLdaq::MiLdaqt;
using namespace std;
using namespace Scu;

#ifndef MINIMUM_X_AXIS
   #define MINIMUM_X_AXIS 1.0
#endif
#ifndef MAXIMUM_X_AXIS
   #define MAXIMUM_X_AXIS 300.0
#endif

#ifndef DEFAULT_THROTTLE_THRESHOLD
  #define DEFAULT_THROTTLE_THRESHOLD 10
#endif
#ifndef DEFAUT_THROTTLE_TIMEOUT
  #define DEFAULT_THROTTLE_TIMEOUT   10
#endif

#ifndef DEFAULT_MAX_EB_BLOCK_LEN
  #define DEFAULT_MAX_EB_BLOCK_LEN    10
#endif
#ifndef DEFAULT_EB_CYCLE_GAP_TIME
   #define DEFAULT_EB_CYCLE_GAP_TIME  100
#endif

#define FSM_INIT_FSM( state, attr... )      m_state = state
#define FSM_TRANSITION( newState, attr... ) m_state = newState

/*! ---------------------------------------------------------------------------
 */
STATIC FgFeedbackAdministration* getDaqAdministration( CommandLine* pCmdLine )
{
   FgFeedbackAdministration* pAllDaq = pCmdLine->getDaqAdminPtr();
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
   FgFeedbackAdministration* pAllDaq = getDaqAdministration( pCmdLine );
   if( pAllDaq == nullptr )
      return -1;

   const bool verbose = pCmdLine->isVerbose();
   if( !pAllDaq->isAddacDaqSupport() )
   {
      WARNING_MESSAGE( "LM32-firmware doesn't support ADDAC/ACU- DAQs!" );
   }

   if( doScan )
   {
      if( verbose )
         cout << "scanning..." << endl;
      pAllDaq->scan( doScan );
   }
   for( const auto& fg: pAllDaq->getFgList() )
   {
      if( verbose )
      {
         cout << "Slot: " << fg.getSlot() <<
                 ", Bits: " << fg.getOutputBits() <<
                 ", Version: " << fg.getVersion() << ",\t" <<
                 (fg.isMIL()? "MIL":"ADDAC/ACU") << " device\t";
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
         cout << "Feedback-Plotter for SCU function generators\n"
                 "(c) 2020 GSI; Author: Ulrich Becker <u.becker@gsi.de>\n\n"
              << "Usage when running outside of SCU:\n" << poParser->getProgramName()
              << " <SCU- target IP-address> [options] [slot channel [slot channel ...]]\n\n"
                 "Usage when running directly on SCU:\n" << poParser->getProgramName()
              << " [options] [slot channel [slot channel ...]]\n\n"
              << "NOTE: When running directly on SCU then no graphic output is possible,\n"
                 "except Gnuplot has been copied on this SCU and the environment variable\n"
                 "DISPLAY was initialized by a server-IP address where a X-server is running.\n"
                 "(Not tested yet.)\n\n"
                 "Hot keys:\n"
              << HOT_KEY_RESET    << ": Reset zooming of all plot windows\n"
              << HOT_KEY_CLEAR_BUFFER << ": Clearing of the DDR3 buffer\n"
              << HOT_KEY_RECEIVE  << ": Toggling receiving on / off\n"
              << HOT_KEY_TOGGLE_SINGLE_SHOOT << ": Toggling single shoot mode on / off\n"
              << HOT_KEY_PRINT_HISTORY << ": Prints the current LM32 history in a eb-console. (See option -H)\n"
              << HOT_KEY_BUILD_NEW << ": Rebuilds the objects respectively restart.\n"
         #ifdef CONFIG_EB_TIME_MEASSUREMENT
              << HOT_KEY_SHOW_TIMING << ": Shows the maximum and minimum access time in microseconds"
                                        " of wishbone cycles.\n"
         #endif
                 "Esc: Program termination\n"
                 "\nCommandline options:\n";
         poParser->list( cout );
         cout << "\n--------------------------------------------------" << endl;
         cout << "Example a:\n\t" ESC_BOLD << poParser->getProgramName() << " scuxl4711 -ac" ESC_NORMAL
                 "\n\tor"
                 "\n\t" ESC_BOLD << poParser->getProgramName() <<  " tcp/scuxl4711 -ac" ESC_NORMAL
                 "\n\n\tWill make a plot of all found MIL-function-generators.\n\n"
                 "Example b:\n"
                 "\tStep 1: Scanning for connected function generators:\n"
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
               "\t<description>Display actual- and set- feedback values of SCU function generators via Gnuplot.</description>\n"
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
            cout << "Version: "
         #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
                 "* "
         #endif
                 TO_STRING( VERSION )
                    ", Git revision: "
                 TO_STRING( GIT_REVISION ) << endl;
         }
         else
         {
            cout <<
         #ifdef _CONFIG_WAS_READ_FOR_ADDAC_DAQ
            "* "
         #endif
            TO_STRING( VERSION ) << endl;
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
      .m_helpText = "Plots in the case of MIL-DAQs always the set-value, even within a gap.\n"
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
         uint plotInterval;
         if( readInteger( plotInterval, poParser->getOptArg() ) )
            return -1;
         if( plotInterval == 0 )
         {
            ERROR_MESSAGE( " plot-interval of " << plotInterval << " is not allowed!" );
            return -1;
         }
         static_cast<CommandLine*>(poParser)->m_plotInterval = plotInterval;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'i',
      .m_longOpt  = "plot-interval",
      .m_helpText = "In the case of ADDAC/ACU-DAQs it is necessary to reduce the points to plot\n"
                    "for performance reasons.\n"
                    "PARAM is used to specify after how many samples will be plotted again.\n"
                    "The default value is: " TO_STRING(DEFAULT_PLOT_INTERVAL)
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_doClearBuffer = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'r',
      .m_longOpt  = "reset",
      .m_helpText = "Reset of the whole data buffer, that means clearing of history data."
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
                    "NOTE: In the case of verbosity mode the option -v has to be set before\n"
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
                    "NOTE: In the case of verbosity mode the option -v has to be set before"
   },
   {
      OPT_LAMBDA( poParser,
      {
         FgFeedbackAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
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
         FgFeedbackAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
         if( pAllDaq == nullptr )
            return -1;
         pAllDaq->clearBuffer();
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'C',
      .m_longOpt  = "clear",
      .m_helpText = "Clears the whole shared memory buffer of LM32 and Linux and exit."
   },
   {
      OPT_LAMBDA( poParser,
      {
         FgFeedbackAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
         if( pAllDaq == nullptr )
            return -1;

         uint gapInterval;
         if( readInteger( gapInterval, poParser->getOptArg() ) )
            return -1;

         pAllDaq->sendGapReadingInterval( gapInterval );
         ::exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'A',
      .m_longOpt  = "gap",
      .m_helpText = "Activates or deactivates the gap reading in the case of MIL DAQs.\n"
                    "PARAM is the gap reading interval in milliseconds. "
                    "A value of zero deactivates the gap reading."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint timeout;
         if( readInteger( timeout, poParser->getOptArg() ) )
            return -1;
         static_cast<CommandLine*>(poParser)->m_throttleTimeout = timeout;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'm',
      .m_longOpt  = "tTimeout",
      .m_helpText = "Sets the throttle-timeout in milliseconds. "
                    "The default value is " TO_STRING( DEFAULT_THROTTLE_TIMEOUT ) " ms.\n"
                    "After this time a value tupel will plot in any cases.\n"
                    "NOTE: A value of zero means the timeout is infinite."
   },
   {
      OPT_LAMBDA( poParser,
      {
         uint threshold;
         if( readInteger( threshold, poParser->getOptArg() ) )
            return -1;
         static_cast<CommandLine*>(poParser)->m_throttleThreshold = threshold;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'n',
      .m_longOpt  = "tThreshold",
      .m_helpText = "Sets the throttle-threshold in DAQ-ADC raw units. "
                    "The default value is " TO_STRING( DEFAULT_THROTTLE_THRESHOLD ) ".\n"
                    "By this parameter it becomes possible to filtering "
                    "out ripple and noise voltage, which increases the "
                    "performance in plotting."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_exitOnError = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'e',
      .m_longOpt  = "exit-on-error",
      .m_helpText = "Program termination in the case of a detected sequence"
                    "-error instead a warning only."

   },
   {
      OPT_LAMBDA( poParser,
      {
         FgFeedbackAdministration* pAllDaq = getDaqAdministration( static_cast<CommandLine*>(poParser) );
         if( pAllDaq == nullptr )
            return -1;

         uint timeOffset = daq::DEFAULT_SYNC_TIMEOFFSET;
         uint ecaTag =     daq::DEFAULT_ECA_SYNC_TAG;
         if( poParser->isOptArgPersent() )
         {
            readTwoIntegerParameters( timeOffset, ecaTag, poParser->getOptArg() );
         }
         if( static_cast<CommandLine*>(poParser)->m_verbose )
         {
            cout << "Timestamp synchronization:\n"
                    "   Time-offset: " << timeOffset << " ms\n"
                    "   ECA-Tag:     0x" << hex << ecaTag << dec << endl;
         }
         pAllDaq->sendSyncronizeTimestamps( timeOffset, ecaTag );
         exit( EXIT_SUCCESS );
         return 0;
      }),
      .m_hasArg   = OPTION::OPTIONAL_ARG,
      .m_id       = 0,
      .m_shortOpt = 'y',
      .m_longOpt  = "sync",
      .m_helpText = "Synchronizing of the timestamp-counter of all found"
                    " ADDAC/SCU-DAQ slaves on SCU bus.\n"
                    "PARAM: =<time-offset in milliseconds>,<ECA tag>\n\n"
                    ESC_BOLD "CAUTION: The timing ECA has to be appear within the "
                    "given time-offset!" ESC_NORMAL "\n\n"
                    "Example 1:\n"
                    ESC_BOLD "-y" ESC_NORMAL "      Without parameter will send a default offset time of "
                    TO_STRING( __DAQ_DEFAULT_SYNC_TIMEOFFSET__ ) " milliseconds "
                    "and the default ECA-TAG of "
                    TO_STRING( __DAQ_DEFAULT_ECA_SYNC_TAG__ ) ".\n\n"
                    "Example 2:\n"
                    ESC_BOLD "-y=2000,0xDADABAFF" ESC_NORMAL "  Will send a offset time of 2 seconds and a ECA-tag of 0xDADABAFF.\n\n"
                    "Example 3:\n"
                    ESC_BOLD "-y=3000" ESC_NORMAL "   Will send a 0ffset time of 3 seconds and the default ECA-tag of "
                    TO_STRING( __DAQ_DEFAULT_ECA_SYNC_TAG__ ) ".\n\n"
                    "Example 4:\n"
                    ESC_BOLD "-y=,0xDACAFFEE" ESC_NORMAL "  Will send the default offset time of "
                    TO_STRING( __DAQ_DEFAULT_SYNC_TIMEOFFSET__ ) " milliseconds "
                    "and the ECA-tag of 0xDACAFFEE."
   },
   {
      OPT_LAMBDA( poParser,
      {
         CommandLine* pCmdLine = static_cast<CommandLine*>(poParser);
         readTwoIntegerParameters( pCmdLine->m_maxEbCycleDataLen,
                                   pCmdLine->m_blockReadEbCycleGapTimeUs,
                                   pCmdLine->getOptArg() );
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'b',
      .m_longOpt  = "block",
      .m_helpText = "PARAM=\"n,t\"\n"
                    "Adjusting of the maximum etherbone data block length of"
                    " to read out of data in the DDR3 RAM,\n"
                    "whereby the length of zero has a special meaning, in this"
                    " case the data will not divide.\n"
                    "When the first parameter \"n\" is not equal to zero,"
                    " then the etherbone block will divided in partial cycles with \"n\""
                    " DDR3-payload items per cycle.\nIn this case the second parameter"
                    " \"t\" is the gap-time in mycroseconds between two data blocks."
                    " (Time for the SaftLib.)\n"
                    "The default values are: \"" TO_STRING( DEFAULT_MAX_EB_BLOCK_LEN )
                    "," TO_STRING( DEFAULT_EB_CYCLE_GAP_TIME ) "\"\n"
                    "In the case of vintage LM32-firmware were the MIL-data is still be"
                    " in the LM32 shared memory, this option is without any effect for MIL DAQs\n\n"
                    "Example 1:\n"
                    ESC_BOLD "-b5,100" ESC_NORMAL "  Divides a etherbone block to receive"
                    " in blocks with 5 DDR3-payload items per cycle and a"
                    " waiting time of 100 microseconds between two etherbone cycles.\n\n"
                    "Example 2:\n"
                    ESC_BOLD "-b0" ESC_NORMAL "  Etherbone cycle will not divided in smaller ones.\n"

   },
   {
      OPT_LAMBDA( poParser,
      {
         uint pollWaitingTime;
         if( readInteger( pollWaitingTime, poParser->getOptArg() ) )
            return -1;
         static_cast<CommandLine*>(poParser)->m_distributeDataPollIntervall = pollWaitingTime;
         return 0;
      }),
      .m_hasArg   = OPTION::REQUIRED_ARG,
      .m_id       = 0,
      .m_shortOpt = 'I',
      .m_longOpt  = "poll-interval",
      .m_helpText = "PARAM=\"<poll interval in milliseconds>\"\n"
                    "A value of zero (default) means no waiting time between two calls of the\n"
                    "polling function \"distributeData()\", otherwise the in this option given\n"
                    "waiting time will expired between consecutive calls of \"distributeData()\"."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_noPlot = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'N',
      .m_longOpt  = "noplot",
      .m_helpText = "Suppressing the calling of Gnuplot, that means no graphic output.\n"
                    "In this case set- and actual values becomes printed directly on standard out.\n"
                    "NOTE: The graphic output becomes also suppressed when the hardware doesn't support it.\n"
                    "      E.g.: This program runs directly on a SCU."
   },
   {
      OPT_LAMBDA( poParser,
      {
         static_cast<CommandLine*>(poParser)->m_pairingBySequence = true;
         return 0;
      }),
      .m_hasArg   = OPTION::NO_ARG,
      .m_id       = 0,
      .m_shortOpt = 'q',
      .m_longOpt  = "sequence",
      .m_helpText = "Pairing of set and actual value for non-MIL DAQs will made by"
                    " block- sequence number of the device descriptor,\n"
                    "by default the pairing will made by WR- timestamp."
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

/*! ---------------------------------------------------------------------------
*/
void CommandLine::readTwoIntegerParameters( uint& rParam1, uint& rParam2, const string& rArgStr )
{
   string single;
   istringstream input( rArgStr );
   for( uint i = 0; getline( input, single, ',' ); i++ )
   {
      if( i >= 2 )
      {
         ERROR_MESSAGE( "To much arguments in option!" );
         ::exit( EXIT_FAILURE );
      }
      if( single.empty() )
         continue;

      if( i == 0 )
      {
         if( readInteger( rParam1, single ) )
            ::exit( EXIT_FAILURE );
         continue;
      }
      if( readInteger( rParam2, single ) )
        ::exit( EXIT_FAILURE );
   }
}

/*-----------------------------------------------------------------------------
 */
CommandLine::CommandLine( int argc, char** ppArgv )
   :PARSER( argc, ppArgv )
   ,m_targetUrlGiven( false )
   ,m_numDevs( 0 )
   ,m_numChannels( 0 )
   ,m_optionError( false )
   ,m_verbose( false )
   ,m_autoBuilding( false )
   ,m_deviationEnable( false )
   ,m_continuePlotting( false )
   ,m_plotAlwaysSetValue( false )
   ,m_doClearBuffer( false )
   ,m_zoomYAxis( false )
   ,m_exitOnError( false )
   ,m_noPlot( false )
   ,m_pairingBySequence( false )
   ,m_xAxisLen( DEFAULT_X_AXIS_LEN )
   ,m_plotInterval( DEFAULT_PLOT_INTERVAL )
   ,m_throttleThreshold( DEFAULT_THROTTLE_THRESHOLD )
   ,m_throttleTimeout( DEFAULT_THROTTLE_TIMEOUT )
   ,m_maxEbCycleDataLen( DEFAULT_MAX_EB_BLOCK_LEN )
   ,m_blockReadEbCycleGapTimeUs( DEFAULT_EB_CYCLE_GAP_TIME )
   ,m_distributeDataPollIntervall( 0 )
   ,m_isRunningOnScu( Scu::isRunningOnScu() )
   ,m_poAllDaq( nullptr )
   ,m_poCurrentDevice( nullptr )
   ,m_poCurrentChannel( nullptr )
   ,m_gnuplotBin( GPSTR_DEFAULT_GNUPLOT_EXE )
   ,m_gnuplotTerminal( GNUPLOT_DEFAULT_TERMINAL )
   ,m_gnuplotLineStyle( DEFAULT_LINE_STYLE )
{
   if( m_isRunningOnScu )
   {
      m_poAllDaq = new AllDaqAdministration( this, "dev/wbm0" );
      FSM_INIT_FSM( READ_SLOT, label='running inside of SCU' );
   }
   else
   {
      FSM_INIT_FSM( READ_EB_NAME, label='running outside of SCU' );
   }
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

/*! ---------------------------------------------------------------------------
 */
inline
void CommandLine::autoBuild( void )
{
   for( const auto& fg: m_poAllDaq->getFgList() )
   {
      Device* pDev = m_poAllDaq->getDevice( fg.getSocket() );
      if( pDev == nullptr )
      {
         pDev = new Device( fg.getSocket() );
         m_poAllDaq->registerDevice( pDev );
      }
      FbChannel* pChannel = static_cast<FbChannel*>(pDev->getChannel( fg.getDevice() ));
      if( pChannel == nullptr )
      {
         pChannel = new FbChannel( fg.getDevice() );
         pDev->registerChannel( pChannel );
      }
   }
}

/*-----------------------------------------------------------------------------
 */
AllDaqAdministration* CommandLine::operator()( void )
{
   if( getArgCount() < 2 )
   {
      ERROR_MESSAGE( "Missing argument!" );
      return nullptr;
   }

   if( PARSER::operator()() < 0 )
      return nullptr;

   if( m_poAllDaq != nullptr )
   {
      m_poAllDaq->setPairingBySequence( m_pairingBySequence );
      m_poAllDaq->setThrottleThreshold( m_throttleThreshold );
      m_poAllDaq->setThrottleTimeout( m_throttleTimeout );
      m_poAllDaq->setMaxEbCycleDataLen( m_maxEbCycleDataLen );
      m_poAllDaq->setBlockReadEbCycleTimeUs( m_blockReadEbCycleGapTimeUs );
      if( m_autoBuilding )
         autoBuild();
      if( m_doClearBuffer )
         m_poAllDaq->clearBuffer();
      if( m_verbose )
      {
         constexpr float MB = 1024.0 * 1024.0;
         const uint addacOfs = m_poAllDaq->getAddacBufferOffset();
         const uint addacCap = m_poAllDaq->getAddacBufferCapacity();
         cout << "Offset of ADDAC-DAQ buffer   : "
              << addacOfs << " item\t"
              << addacOfs * sizeof(daq::RAM_DAQ_PAYLOAD_T) << " byte\t"
              << ((addacOfs * sizeof(daq::RAM_DAQ_PAYLOAD_T)/MB )) << " MB" << endl;
         cout << "Capacity of ADDAC-DAQ buffer : "
              << addacCap << " item\t"
              << addacCap * sizeof(daq::RAM_DAQ_PAYLOAD_T) << " byte\t"
              << ((addacCap * sizeof(daq::RAM_DAQ_PAYLOAD_T))/MB ) << " MB" << endl;
      #ifdef CONFIG_MIL_FG
         const uint milOfs = m_poAllDaq->getMilBufferOffset();
         const uint milCap = m_poAllDaq->getMilBufferCapacity();
         cout << "Offset of MIL-DAQ buffer     : "
              << milOfs << " item\t"
              << milOfs * sizeof(daq::RAM_DAQ_PAYLOAD_T) << " byte\t"
              << ((milOfs * sizeof(daq::RAM_DAQ_PAYLOAD_T)/MB )) << " MB" << endl;

         cout << "Capacity of MIL-DAQ buffer   : "
              << milCap << " item\t"
              << milCap * sizeof(daq::RAM_DAQ_PAYLOAD_T) << " byte\t"
              << ((milCap * sizeof(daq::RAM_DAQ_PAYLOAD_T)/MB )) << " MB" << endl;
      #endif
      }
      return m_poAllDaq;
   }

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

         m_poAllDaq = new AllDaqAdministration( this, arg );
         FSM_TRANSITION( READ_SLOT );
         break;
      }
      case READ_SLOT:
      {
         assert( dynamic_cast<AllDaqAdministration*>(m_poAllDaq) != nullptr );
         m_numDevs++;
         m_poCurrentChannel = nullptr;
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
         if( !m_poAllDaq->isPresent( m_poCurrentDevice->getSocket(), number ) )
         {
            ERROR_MESSAGE( "No device in socket " << m_poCurrentDevice->getSocket()
                            << " with the number " << number << " present!" );
            return -1;
         }
         if( m_poCurrentDevice->getChannel( number ) == nullptr )
         {
            m_poCurrentChannel = new FbChannel( number );
            m_poCurrentDevice->registerChannel( m_poCurrentChannel );
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

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <unistd.h>


#include "carpeDM.h"
#include "filenames.h"





static void help(const char *program) {
  fprintf(stderr, "\ndm-sched v%s, build date %s\nCreates Binary Data for the DataMaster (DM) from Schedule Graphs (.dot files) and\nuploads/downloads to/from CPU Core <m> of the DM (CPU currently specified in schedule as cpu=<m>).\n", TOOL_VER, BUILD_DATE);
  fprintf(stderr, "\nUsage: %s <etherbone-device> <Command> <.dot file> \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nCommands:\n");
  fprintf(stderr, "  status                    Gets current DM schedule state (default) \n");
  fprintf(stderr, "  dump                      Gets current DM schedule\n");
  fprintf(stderr, "  clear                     Clear DM, existing nodes will be erased. \n");
  fprintf(stderr, "  add        <.dot file>    Add a Schedule from input file to DM, nodes with identical hashes (names) on the DM will be ignored.\n");
  fprintf(stderr, "  overwrite  <.dot file>    Overwrites all Schedules on DM with the one in the input file, already existing nodes on the DM will be erased. \n");
  fprintf(stderr, "  remove     <.dot file>    Removes the schedule in the input file from the DM, nodes with hashes (names) not present on the DM will be ignored \n");
  fprintf(stderr, "  keep       <.dot file>    Removes everything BUT the schedule in the input file from the DM, nodes with hashes (names) not present on the DM will be ignored.\n");
  fprintf(stderr, "  chkrem     <.dot file>    Checks if all patterns in given dot can be removed safely\n");
  fprintf(stderr, "  -n                        No verify, status will not be read after operation\n");
  fprintf(stderr, "  -o         <.dot file>    Specify output file name, default is '%s'\n", outfile);
  fprintf(stderr, "  -s                        Show Meta Nodes. Download will not only contain schedules, but also queues, etc. \n");
  fprintf(stderr, "  -v                        Verbose operation, print more details\n");
  fprintf(stderr, "  -d                        Debug operation, print everything\n");
  fprintf(stderr, "  -f                        Force, overrides the safety check for clear, remove, overwrite and keep\n");

  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {

  Graph g;
  char dirnameBuff[80];

  bool update = true, verbose = false, strip=true, cmdValid = false, force = false, debug=false;
  bool reqStatus = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *inputFilename = NULL, *cmdName = NULL, *outputFilename = outfile;
  const char *dirname = (const char *)getcwd(dirnameBuff, 80);
  int32_t error=0;


// start getopt
   while ((opt = getopt(argc, argv, "fnshvo:d")) != -1) {
      switch (opt) {

         case 'o':
            outputFilename  = optarg;
            if (outputFilename == NULL) {
              std::cerr << std::endl << program << ": option -o expects a filename" << std::endl;
            }
            error = -1;
            break;
         case 'd':
            debug = true;
            break;

         case 'n':
            update = false;
            break;
         case 'f':
            force = true;
            break;

         case 'v':
            verbose = true;
            break;

         case 's':
            strip = false;
            break;

         case 'h':
            help(program);
            return 0;

         case ':':

         case '?':
            error = -2;
            break;

         default:
            std::cerr << std::endl << program << ": bad getopt result" << std::endl;
            error = -3;
      }
   }


  if (error) return error;

   if (optind >= argc) {
   std::cerr << std::endl << program << ": expecting one non-optional argument: <etherbone-device>" << std::endl;
   //help();
   return -4;
   }

   // process command arguments
  //std::cerr << program << ": " << argc << " " << optind << " " << argv[optind] << " " << argv[optind+1] << " " << argv[optind+2] << " " << std::endl;

   netaddress = argv[optind];

   if (optind+1 < argc) cmdName        = argv[optind+1];
   if (optind+2 < argc) inputFilename  = argv[optind+2];



  CarpeDM cdm = CarpeDM();


  if(verbose) cdm.verboseOn();
  if(debug)   cdm.debugOn();


  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }

  cdm.updateModTime();

  if (cmdName != NULL ) {


    std::string cmd(cmdName);

    if ((cmd == "add" || cmd == "overwrite" || cmd == "remove" || cmd == "keep") && inputFilename == NULL) { std::cerr << std::endl << program << "Command <" << cmd << "> requires a .dot file" << std::endl; return -8; }

    try {
      if (cmd == "clear")     { cdm.clear(force); cmdValid = true;}
      if (cmd == "add")       { cdm.download(); cdm.addDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "overwrite") { cdm.overwriteDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "remove")    { cdm.download(); cdm.removeDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "keep")      { cdm.download(); cdm.keepDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "status")    { cdm.downloadDotFile(outputFilename, strip); cmdValid = true; reqStatus = true;}
      if (cmd == "dump")      { cdm.download(); std::cout << cdm.createDot(cdm.getDownGraph(), strip) << std::endl; cmdValid = true; reqStatus = false; update=false;}
      if (cmd == "chkrem")    {
        cdm.download();

        std::string report;
        Graph gTmp0, gTmp1;
        bool isSafe = cdm.isSafeToRemove(cdm.parseDot(cdm.readTextFile(inputFilename), gTmp0), report);

        cdm.writeTextFile(std::string(dirname) + "/" + std::string(debugfile), report);

        std::cout << std::endl << "Dot file " << inputFilename << " content removal: " << (isSafe ? "SAFE" : "FORBIDDEN" ) << std::endl;
        cmdValid = true;
      }

      if(verbose) cdm.showUp(false);
    } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Failed to execute <"<< cmd << ". Cause: " << err.what() << std::endl;
      return -6;
    }
  }

  if (cmdName == NULL ) { cmdValid = true;reqStatus = true; }

  if ( ! cmdValid ) { std::cerr << std::endl << program << ": Unknown command <" << std::string(cmdName) << ">" << std::endl; return -8; }

  if ( update ) {
    try {
      cdm.downloadDotFile(outputFilename, strip);
      if(verbose || reqStatus) cdm.showDown(false);

    } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Failed to execute <status>. Cause: " << err.what() << std::endl;
      return -7;
    }
  }

  std::string report;
  bool tabOk = cdm.tableCheck(report);

  if (debug or !tabOk) std::cout << report << std::endl;


  cdm.disconnect();

  return 0;
}

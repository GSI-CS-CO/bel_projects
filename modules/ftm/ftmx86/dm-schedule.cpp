#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"


const char defOutputFilename[] = "download.dot";



static void help(const char *program) {
  fprintf(stderr, "\nUsage: %s <etherbone-device> <Command> <.dot file> \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nSchedule Generator. Creates Binary Data for the DataMaster (DM) from Schedule Graphs (.dot files) and\nuploads/downloads to/from CPU Core <m> of the DM (CPU currently specified in schedule as cpu=<m>).\n");
  fprintf(stderr, "\nCommands:\n");
  fprintf(stderr, "  status                    Gets current DM schedule state (default) \n");
  fprintf(stderr, "  clear                     Clear DM, existing nodes will be erased. \n");
  fprintf(stderr, "  add        <.dot file>    Add a Schedule from input file to DM, nodes with identical hashes (names) on the DM will be ignored.\n");
  fprintf(stderr, "  overwrite  <.dot file>    Overwrites all Schedules on DM with the one in the input file, already existing nodes on the DM will be erased. \n");
  fprintf(stderr, "  remove     <.dot file>    Removes the schedule in the input file from the DM, nodes with hashes (names) not present on the DM will be ignored \n");
  fprintf(stderr, "  keep       <.dot file>    Removes everything BUT the schedule in the input file from the DM, nodes with hashes (names) not present on the DM will be ignored.\n");
  fprintf(stderr, "  chkrem     <.dot file>    Checks if a pattern can be removed safely\n");
  fprintf(stderr, "  -n                        No verify, status will not be read after operation\n");
  fprintf(stderr, "  -o         <.dot file>    Specify output file name, default is '%s'\n", defOutputFilename);
  fprintf(stderr, "  -s                        Show Meta Nodes. Download will not only contain schedules, but also queues, etc. \n");  
  fprintf(stderr, "  -v                        Verbose operation, print more details\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {

  Graph g;

  bool update = true, verbose = false, strip=true, cmdValid = false, force = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *inputFilename = NULL, *cmdName = NULL, *outputFilename = defOutputFilename;
  int32_t error=0;


// start getopt 
   while ((opt = getopt(argc, argv, "fnshvo:")) != -1) {
      switch (opt) {
 
         case 'o':
            outputFilename  = optarg;
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


  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }

  //TODO we need a dictionary independent of dot files, otherwise, how do we update?

  try { cdm.loadHashDictFile("dm.dict"); } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Warning - Could not load dictionary file. Cause: " << err.what() << std::endl;
    }
  std::cout << std::endl << program << ": Loaded " << cdm.getHashDictSize() << " Node / Hash entries" << std::endl;  

  try { cdm.loadGroupsDictFile("dm.groups"); } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Warning - Could not load groups file. Cause: " << err.what() << std::endl;
    }
  std::cout << std::endl << program << ": Loaded " << cdm.getGroupsSize() << " Node / Pattern / Beamprocess entries" << std::endl;    


  if (inputFilename == NULL) {
    if (cdm.isHashDictEmpty()) std::cerr << std::endl << program << ": Warning - No Nodename/Hash dictionary available. Your download will show only hashes." << std::endl;
  }




  if (cmdName != NULL ) {


    std::string cmd(cmdName);

    if ((cmd == "add" || cmd == "overwrite" || cmd == "remove" || cmd == "keep") && inputFilename == NULL) { std::cerr << std::endl << program << "Command <" << cmd << "> requires a .dot file" << std::endl; return -8; }
    
    try {
      if (cmd == "clear")     { cdm.clear(); cmdValid = true;}
      if (cmd == "add")       { cdm.addDotFile(inputFilename); cmdValid = true;}
      if (cmd == "overwrite") { cdm.overwriteDotFile(inputFilename); cmdValid = true;}
      if (cmd == "remove")    { cdm.removeDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "keep")      { cdm.keepDotFile(inputFilename, force); cmdValid = true;}
      if (cmd == "status")    { cdm.downloadDotFile(outputFilename, strip); cmdValid = true;}
      if (cmd == "chkrem")    {
        cdm.download();

        std::string report;
        Graph gTmp0, gTmp1;
        bool isSafe = cdm.isSafeToRemove(cdm.parseDot(cdm.readTextFile(inputFilename), gTmp0), report);
        
        cdm.writeTextFile("debug.dot", report);

        std::cout << std::endl << "Dot file " << inputFilename << " content removal: " << (isSafe ? "SAFE" : "FORBIDDEN" ) << std::endl;
        cmdValid = true;
      }

      if(verbose) cdm.showUp(false);
    } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Failed to execute <"<< cmd << ". Cause: " << err.what() << std::endl;
      return -6;
    }
  }
  
  if (cmdName == NULL ) cmdValid = true;

  if ( ! cmdValid ) { std::cerr << std::endl << program << ": Unknown command <" << std::string(cmdName) << ">" << std::endl; return -8; }

  if ( update ) {
    try { 
      cdm.downloadDotFile(outputFilename, strip);
      if(verbose) cdm.showDown(false);

    } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Failed to execute <status>. Cause: " << err.what() << std::endl;
      return -7;
    }
  }


  cdm.storeHashDictFile("dm.dict");  
  cdm.storeGroupsDictFile("dm.groups");

  //cdm.showGroupsDict();

  cdm.disconnect();

  return 0;
}

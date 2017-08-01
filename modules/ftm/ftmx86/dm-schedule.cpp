#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"


const char defOutputFilename[] = "download.dot";



static void help(const char *program) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <input .dot file>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nSchedule Generator. Creates Binary Data for the DataMaster (DM) from Schedule Graphs (.dot files) and\nuploads/downloads to/from CPU Core <m> of the DM. For download, a Schedule Graph is not manadatory ,\nbut hashes will not be resolved to node names. To render a download, use the 'dot -Tpng -o<outputfile.dot>'\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              Select CPU core by index, default is 0\n");
  fprintf(stderr, "  -w                        Wipe Graph of selected CPU core\n");
  fprintf(stderr, "  -a                        Add Graph from input file it to selected CPU core.\n");
  fprintf(stderr, "  -r                        Remove Graph from input file from selected CPU core.\n");
  fprintf(stderr, "  -o <output file>          Specify output file name, default is '%s'\n", defOutputFilename);
  fprintf(stderr, "  -s                        Strip Meta Nodes. Download writes out only schedule paths, no queues etc \n");  
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {

  Graph g;

  bool doUpload = false, doUpdate = false, doRemove = false, doClear = false, verbose = false, strip=false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *inputFilename = NULL, *outputFilename = defOutputFilename;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0;

// start getopt 
   while ((opt = getopt(argc, argv, "rashvc:o:w")) != -1) {
      switch (opt) {
 
         case 'w':
            doUpload = false;
            doUpdate = false;
            doRemove = false;
            doClear  = true;
            break;
         
         case 'o':
            outputFilename  = optarg;
            break;
 
         case 'a':
            doUpload = true;
            doUpdate = true;
            doRemove = false;
            doClear  = false;
            break;

         case 'r':
            doUpload = false;
            doUpdate = false;
            doRemove = true;
            doClear  = false;
            break;  

         case 'v':
            verbose = true;
            break;

         case 's':
            strip = true;
            break;   

         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << std::endl << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
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
   if (optind+1 < argc) inputFilename   = argv[optind+1];

   

  CarpeDM cdm = CarpeDM();


  if(verbose) cdm.verboseOn();


  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }

  //TODO we need a dictionary independent of dot files, otherwise, how do we update?

  cdm.getHashMap().load("dm.dict");

  if (inputFilename != NULL) {
    try { cdm.addDotToDict(inputFilename); }
    catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Warning - Could not insert your .dot file into dictionary. Cause: " << err.what() << std::endl;
    }
  } else {
    if (cdm.getHashMap().size() == 0) std::cerr << std::endl << program << ": Warning - No Nodename/Hash dictionary available. Your download will show only hashes." << std::endl;
  }
  
  
    if(doUpload | doUpdate) {
      if (inputFilename != NULL) {
        try {

          if(doUpdate) cdm.download();
          cdm.prepareUpload(cdm.parseDot(inputFilename, g));
          cdm.upload();
          if(verbose) cdm.showUp(strip);
        } catch (std::runtime_error const& err) {
          std::cerr << std::endl << program << ": Failed to upload to CPU#"<< cpuIdx << ". Cause: " << err.what() << std::endl;
          return -6;
        }
      } else {std::cerr << program << ": Failed - a .dot file is required" << std::endl; return -7;}
    } else if (doRemove) {
      if (inputFilename != NULL) {
        try {
          cdm.download();
          cdm.removeDot(inputFilename);
          cdm.removeDotFromDict(inputFilename);
        } catch (std::runtime_error const& err) {
          std::cerr << std::endl << program << ": Failed to remove from CPU#"<< cpuIdx << ". Cause: " << err.what() << std::endl;
          return -6;
        }
      } else {std::cerr << std::endl << program << ": Failed - a .dot file is required" << std::endl; return -7;}
    } else if (doClear) {
        cdm.clear();
    }
  
  
    
    try { 
      cdm.download();
      cdm.writeDownDot(outputFilename, strip);
      if(verbose) cdm.showDown(strip);
    } catch (std::runtime_error const& err) {
      std::cerr << std::endl << program << ": Failed to download from CPU#"<< cpuIdx << " Cause: " << err.what() << std::endl;
      return -7;
    }

  cdm.getHashMap().store("dm.dict");

  cdm.disconnect();

  return 0;
}

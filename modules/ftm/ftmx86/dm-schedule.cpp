#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"


const char defOutputFilename[] = "download.dot";
const char defInputFilename[] = "";


static void help(const char *program) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> <input .dot file>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "\nSchedule Generator. Creates Binary Data for the DataMaster (DM) from Schedule Graphs (.dot files) and\nuploads/downloads to/from CPU Core <m> of the DM. For download, a Schedule Graph is not manadatory ,\nbut hashes will not be resolved to node names. To render a download, use the 'dot -Tpng -o<outputfile.dot>'\n");
  fprintf(stderr, "\nGeneral Options:\n");
  fprintf(stderr, "  -c <cpu-idx>              Select CPU core by index, default is 0\n");
  fprintf(stderr, "  -w                        Generate Schedule Graph from input file and uploads it to selected CPU core\n");
  fprintf(stderr, "  -o <output file>          Specify output file name, default is '%s'\n", defOutputFilename);
  fprintf(stderr, "  -s                        Strip Meta Nodes. Download writes out only schedule paths, no queues etc \n");  
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "\n");
}

int main(int argc, char* argv[]) {



  bool doUpload = false, verbose = false, strip=false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *inputFilename = defInputFilename, *outputFilename = defOutputFilename;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0;

// start getopt 
   while ((opt = getopt(argc, argv, "shvc:o:w")) != -1) {
      switch (opt) {
         case 'w':
            doUpload = true;
            break;
         
         case 'o':
            outputFilename  = optarg;
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
              std::cerr << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
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
            std::cerr << program << ": bad getopt result" << std::endl; 
            error = -3;
      }
   }


  if (error) return error;

   if (optind >= argc) {
   std::cerr << program << ": expecting one non-optional argument: <etherbone-device>" << std::endl;
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
    std::cerr << "ERROR - Could not connect to DM: " << err.what() << std::endl; return -20;
  }

  //TODO we need a dictionary independent of dot files, otherwise, how do we update?

  try { cdm.addDotToDict(inputFilename); }
  catch (std::runtime_error const& err) {
    if(doUpload) {
      std::cerr << "ERROR: No dictionary available, mandatory for upload. Cause: " << err.what() << std::endl;
      return -5;
    } else {std::cerr << "WARNING: No Nodename/Hash dictionary available. Cause: " << err.what() << std::endl;}
  }
  
  if(doUpload) {

    try { 
      cdm.uploadDot(cpuIdx, inputFilename);
      if(verbose) cdm.showUp(cpuIdx, strip);
    } catch (std::runtime_error const& err) {
      std::cerr << "ERROR: Upload to CPU#"<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
      return -6;
    }
  }
  
  try { 
    cdm.downloadAndParse(cpuIdx);
    cdm.writeDownDot(outputFilename, cpuIdx, strip);
    if(verbose) cdm.showDown(cpuIdx, strip);
  } catch (std::runtime_error const& err) {
    std::cerr << "ERROR: Download from CPU#"<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
    return -7;
  }
 

  cdm.disconnect();

  return 0;
}

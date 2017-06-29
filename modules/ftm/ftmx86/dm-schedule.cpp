#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"


const char defOutputFilename[] = "download.dot";
const char defInputFilename[] = "";

int main(int argc, char* argv[]) {



  bool doUpload = false, verbose = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *inputFilename = defInputFilename, *outputFilename = defOutputFilename;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0;

// start getopt 
   while ((opt = getopt(argc, argv, "vc:o:w")) != -1) {
      switch (opt) {
         case 'w':
            doUpload = true;
            break;
         
         case 'o':
            outputFilename  = optarg;
            break;
 
         case 'v':
            verbose = 1;
            break;

         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
            break;

         case 'h':
            //help();
          std::cout << program << "<FIXME insert help here > " << std::endl;
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

  Graph gUp;
  try {
    cdm.connect(std::string(netaddress));
  } catch (std::runtime_error const& err) {
    std::cerr << "ERROR - Could not connect to DM: " << err.what() << std::endl; return -20;
  }



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
      if(verbose) cdm.showUp(cpuIdx);
    } catch (std::runtime_error const& err) {
      std::cerr << "ERROR: Upload to CPU#"<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
      return -6;
    }
  }
  
  try { 
    cdm.downloadAndParse(cpuIdx);
    cdm.writeDownDot(outputFilename, cpuIdx);
    if(verbose) cdm.showDown(cpuIdx);
  } catch (std::runtime_error const& err) {
    std::cerr << "ERROR: Download from CPU#"<< cpuIdx << " failed. Cause: " << err.what() << std::endl;
    return -7;
  }
 

  cdm.disconnect();

  return 0;
}

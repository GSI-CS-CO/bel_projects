#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "ftm_shared_mmap.h"
#include "carpeDM.h"


const char defOutputFilename[] = "download.dot";


int main(int argc, char* argv[]) {



  bool doUpload = false, readBlock = false, verbose = false;

  int opt;
  const char *program = argv[0];
  const char *netaddress, *blockName = NULL, *inputFilename = NULL, *outputFilename = defOutputFilename;
  int32_t tmp, error=0;
  uint32_t cpuIdx = 0, thrIdx = 0;

// start getopt 
   while ((opt = getopt(argc, argv, "vb:c:o:t:w")) != -1) {
      switch (opt) {
         case 'w':
            doUpload = true;
            break;
         case 'o':
            outputFilename  = optarg;
            break;
         case 'b':
            blockName = optarg;
            readBlock = true;
            break;       
         case 'v':
            verbose = 1;
            break;
         case 't':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid thr idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {thrIdx = (uint32_t)tmp;}
            break;
         case 'c':
            tmp = atol(optarg);
            if (tmp < 0 || tmp > 8) {
              std::cerr << program << ": invalid cpu idx -- '" << optarg << "'" << std::endl;
              error = -1;
            } else {cpuIdx = (uint32_t)tmp;}
            break;
         /*    
         case 'h':
            help();
            return 0;
         */ 
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
   

   /*
   if (optind+1 < argc)  command = argv[++optind];
   else                 {command = "status"; cpuId = -1;}
   if (!strcasecmp(command, "loadfw")) overrideFWcheck = 1;  
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw")))
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], FILENAME_LEN);

         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   } 
   */

  CarpeDM cdm = CarpeDM();
  Graph gUp;
  std::cout << "Connecting to " << netaddress << "... ";
  cdm.connect(std::string(netaddress));
  std::cout << "Done."  << std::endl << "Found " << cdm.getCpuQty() << " Cores." << std::endl;
  std::cout << "Creating Dictionary from " << inputFilename << " ... ";
  try { cdm.addDotToDict(inputFilename); }
  catch (...) {
    if(doUpload) {
      return -5;
    }
  }
  std::cout << "Done. " << std::endl;
  if(doUpload) {
    std::cout << "Preparing Upload to CPU #" << cpuIdx << "... " ;
    cdm.parseUpDot(inputFilename, gUp);
    cdm.prepareUploadToCpu(gUp, cpuIdx);
    std::cout << "Done." << std::endl << "Uploading... ";
    cdm.upload(cpuIdx);
    std::cout << "Done." << std::endl;
    cdm.showUp(cpuIdx);
  }
  
  std::cout << "Downloading from CPU #" << cpuIdx << "... ";
  cdm.downloadAndParse(cpuIdx);
  std::cout << "Done." << std::endl << "Writing Output File " << outputFilename << "... ";
  cdm.writeDownDot(outputFilename, cpuIdx);
  std::cout << "Done." << std::endl;


  cdm.showDown(cpuIdx);

  cdm.disconnect();

  return 0;
}

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <unistd.h>


#include "carpeDM.h"
#include "filenames.h"
#include <chrono>
#include <thread>





int main(int argc, char* argv[]) {

  Graph g;
  

  int opt, error;

  
  const char *program = argv[0];


  CarpeDM cdm = CarpeDM();



  try {
    cdm.connect("", true);
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }

    uint64_t cursor = 0x2ULL;
  uint64_t defInit  = 0x012ULL;
  uint64_t dynInit  = 0x012012ULL;

  uint64_t seed = (dynInit << (12 + 4)) | (defInit << 4) | cursor; 

  // start getopt 
  while ((opt = getopt(argc, argv, "s:")) != -1) {
     switch (opt) {
       case 's':
          seed  = (uint64_t)strtol(optarg, NULL, 0);
         break;  
       case ':':
       
       case '?':
          error = -2;
          break;
          
       default:
          std::cerr << program << ": bad getopt result" << std::endl; 
          error = -3;
     }
   }






  cdm.coverageUpload3(seed);
  std::string report;
  bool isSafe = cdm.isSafeToRemove("A", report, true);
  cdm.writeTextFile("./debug.dot", report);
  std::cout << "0x" << std::setfill('0') << std::setw(10) <<  std::hex << seed << " - " << isSafe << std::endl;

/*
  //cdm.verboseOn();
  uint64_t before, after, sum, average;
  sum = 0;
  for (unsigned i=0; i < 10; i++) {

    before = cdm.getDmWrTime();
    cdm.overwriteDotFile("debug_sloppy_safe2remove.dot", true);
    cdm.download();
    cdm.setThrOrigin(0, 0, "B_BLOCK");
    cdm.forceThrCursor(0, 0);
    cdm.sendCommandsDotFile("debug_cmd_flow_sloppy_safe2remove.dot");
    cdm.download();
    std::string report;
    try{ 
      cdm.isSafeToRemove("E", report, true);
      cdm.writeTextFile("./" + std::string(debugfile), report);
    } catch (...) {
      std::cout << "FUCKUP" << std::endl << report << std::endl;
      

    }  
    after = cdm.getDmWrTime();
    sum += (after - before);
  }
  sum /= 1000;
  std::cout << "Time Avg: " << sum << std::endl;
*/

  cdm.disconnect();

  return 0;
}

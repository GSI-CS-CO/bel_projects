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
  uint64_t reps = 1;
  // start getopt 
  while ((opt = getopt(argc, argv, "s:r:")) != -1) {
     switch (opt) {
       case 's':
          seed  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'r':
          reps  = (uint64_t)strtol(optarg, NULL, 0);
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








  uint64_t before, after, sum, average;
  sum = 0;
  for (unsigned i=0; i < reps; i++) {
    uint64_t thisSeed = seed + i;
    before = cdm.getDmWrTime();
    cdm.coverageUpload3(thisSeed);
    std::string report;
    bool isSafe = cdm.isSafeToRemove("A", report, true);
    cdm.writeTextFile("./debug.dot", report);
    std::cout << "0x" << std::setfill('0') << std::setw(10) <<  std::hex << thisSeed << " - " << isSafe << std::endl;
    
    after = cdm.getDmWrTime();
    sum += (after - before);
  }
  average = sum / reps;
  std::cout << "Time Avg: " << std::dec << average << std::endl;


  cdm.disconnect();

  return 0;
}

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
  

  

  
  const char *program = argv[0];


  CarpeDM cdm = CarpeDM();


  try {
    cdm.connect("tcp/tsl403.acc.gsi.de");
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }



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


  cdm.disconnect();

  return 0;
}

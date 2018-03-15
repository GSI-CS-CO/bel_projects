#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <unistd.h>

#include "ftm_shared_mmap.h"
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

  try { cdm.loadHashDictFile("./dm.hashes"); } catch (std::runtime_error const& err) {}

  try { cdm.loadHashDictFile("./dm.groups"); }  catch (std::runtime_error const& err) {}


  //cdm.verboseOn();

  cdm.addDotFile("pps.dot");
  cdm.download();
  cdm.sendCommandsDotFile("pps_cmd_start.dot");
  printf("waiting 1 sec\n");
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  printf("done\n");
  cdm.showGroupsDict();
  try {
    printf("trying to clear while running\n");
    cdm.clear(false);
  }  catch (...) {
    printf("clear told us to fuck off\n");

  }  
  cdm.showGroupsDict();
  
  try {
    printf("trying to stop\n");
    cdm.sendCommandsDotFile("pps_cmd_stop.dot");
  }  catch (...) {
    printf("stop went south, cleaning up\n");

    cdm.halt();
    cdm.clear(false);
  }

   


  cdm.disconnect();

  return 0;
}

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
    cdm.connect("dev/ttyUSB0", true);
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }



  uint64_t seed   = (0x20800 << (6 + 3)) | (0x1 << 3) | 0x2; //test with optimisable edges for new safe2remove
  uint64_t reps   = 1;
  uint64_t parts  = 1;
  uint64_t beginning, before, after, sum=0, average;
  bool verbose = false;

  // start getopt 
  while ((opt = getopt(argc, argv, "s:r:p:v")) != -1) {
     switch (opt) {
       case 's':
          seed  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'r':
          reps  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'p':
          parts  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'v':
          verbose  = true;
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

  beginning = cdm.getDmWrTime();

  before = cdm.getDmWrTime();
  std::vector<std::vector<uint64_t>> testData = cdm.coverage3TestData(seed, reps, parts );
  after = cdm.getDmWrTime();
  size_t cases = 0;
  for (auto& itThr : testData) {
    //std::cout << "Part --- " << std::endl;
    cases += itThr.size();
    //for (auto& itVal : itThr) {
    //  std::cout << std::hex << "0x" << itVal << std::endl;
    //}  
  }

  std::string dirname = "dm-test-250418";
  std::string cmd = "exec rm -r " + dirname + "/*";
  system(cmd.c_str());
  std::ofstream outfile;
  outfile.open(dirname + "/dm-test.log");
  //  
  //outfile << "Created " << std::dec << cases << " test cases in " << std::dec << ((after - before) / 1000000ULL) << " ms" << std::endl;
  std::cout << "Created " << std::dec << cases << " test cases in " << std::dec << ((after - before) / 1000000ULL) << " ms" << std::endl;

  const uint64_t sample = 200;
  uint64_t sampleCnt = 0;
  uint64_t fuckUpCnt = 0;

  

  for (auto& itThr : testData) {
    for (auto& itVal : itThr) {

      before = cdm.getDmWrTime();
      cdm.coverage3Upload(itVal);
      std::string report, queueDbg;
      bool isSafe = false;
      if (verbose) cdm.verboseOn();
      //cdm.inspectQueues("A_B", queueDbg);
      //cdm.inspectQueues("B_B", queueDbg); 
      //cdm.inspectQueues("C_B", queueDbg);
      //std::cout << queueDbg << std::endl;

      try {
        cdm.updateModTime();
        isSafe = cdm.isSafeToRemove("A", report, true);
        //outfile << "Sample " << std::dec << sampleCnt << "0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe << std::endl;
      } catch (const std::runtime_error& re) {
        std::stringstream auxstream;
        auxstream << "0x" << std::setfill('0') << std::setw(10) << std::hex << itVal;
        cdm.writeTextFile(dirname + "/debug_" + auxstream.str() + ".dot", report);
        cdm.writeDotFile(dirname + "/coverage_" + auxstream.str() + ".dot", cdm.getDownGraph(), false);
        fuckUpCnt++;
        outfile << "Sample " << std::dec << std::setfill('0') << std::setw(8) << sampleCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - FUCKUP" << std::endl;
        outfile << re.what() << std::endl;
        std::cout << "Sample " << std::dec << std::setfill('0') << std::setw(8) << sampleCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - FUCKUP" << std::endl;
        std::cout << re.what() << std::endl;
        std::flush(outfile);
      }

      cdm.verboseOff();
      if ((sampleCnt % sample) == 0) {
        std::stringstream auxstream;
        auxstream << "0x" << std::setfill('0') << std::setw(10) << std::hex << itVal;
        cdm.writeTextFile(dirname + "/debug_" + auxstream.str() + "_sample.dot", report);
        cdm.writeDotFile(dirname + "/coverage_" + auxstream.str() + "_sample.dot", cdm.getDownGraph(), false);
        std::cout << "Sample " << std::dec << sampleCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe << std::endl;
        outfile << "Sample " << std::dec << sampleCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe << std::endl;
        std::flush(outfile);
      }
      sampleCnt++;
      
      
      after = cdm.getDmWrTime();
      sum += (after - before);
    }
  }  
  average = sum / sampleCnt;
  outfile << "Ran " << std::dec << sampleCnt << " of " << cases << " test cases. " << fuckUpCnt << " f**kups" << std::endl;
  outfile << "Time Total: " << std::dec << ((after - beginning) / 1000000ULL) << " Avg: " << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
  std::cout << "Ran " << std::dec << sampleCnt << " of " << cases << " test cases. " << fuckUpCnt << " f**kups" << std::endl;
  std::cout << "Time Total: " << std::dec << ((cdm.getDmWrTime() - beginning) / 1000000ULL) << " Avg: " << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
  outfile.close();

  cdm.disconnect();

  return 0;
}

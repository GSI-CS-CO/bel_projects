#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <unistd.h>


#include "carpeDM.h"
#include "filenames.h"
#include <chrono>
#include <thread>
#include <sys/stat.h>



int main(int argc, char* argv[]) {

  Graph g;


  int opt, error = 0;
  std::string dirname, netaddress;
  int dir_err, rem_err;

  const char *program = argv[0];
  uint64_t seed   = (0x20800 << (6 + 3)) | (0x1 << 3) | 0x2; //test with optimisable edges for new safe2remove
  uint64_t range  = 1;   //range to test
  uint64_t percentage  = 100;   //percentage to test
  uint64_t parts  = 1;   //parts to split testdata into
  uint64_t interval = 200; //sample generation interval
  uint64_t beginning, before, after, sum=0, average;
  bool sim = false, verbose = false;


  // start getopt
  while ((opt = getopt(argc, argv, "i:s:r:c:p:v")) != -1) {
     switch (opt) {
       case 'i':
          interval  = (uint64_t)strtol(optarg, NULL, 0);
          break;
       case 's':
          seed  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'r':
          range  = (uint64_t)strtol(optarg, NULL, 0);
         break;
       case 'c':
          percentage  = (uint64_t)strtol(optarg, NULL, 0);
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

  if (optind+1 >= argc) {
   std::cerr << std::endl << program << ": expecting one non-optional arguments: <etherbone/simulation-device> <filepath>" << std::endl;
   exit(4);
   }

   if(error != 0) {
    std::cerr << std::endl << program << ": unknown parameters" << std::endl;
    exit(1);
  }


  const uint64_t maxSeed = (1 << (27))-1;


  if (seed > maxSeed) {
    std::cerr << std::endl << program << ": Seed must be less or equal " <<  std::setfill('0') << std::setw(10) << std::hex << maxSeed << std::endl;
    exit(5);
  }

  netaddress = std::string(argv[optind]);
  dirname = std::string(argv[optind+1]);


  dir_err = mkdir(dirname.c_str(), ACCESSPERMS);



  if (netaddress == "sim") sim = true;




  CarpeDM cdm = CarpeDM();
  try {
    cdm.connect(netaddress.c_str(), sim, true);
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << program << ": Failed to connect to DM: " << err.what() << std::endl; return -20;
  }

  beginning = cdm.getDmWrTime();

  before = cdm.getDmWrTime();
  std::vector<std::vector<uint64_t>> testData = cdm.coverage3TestData(seed, range, parts, percentage );
  after = cdm.getDmWrTime();
  size_t genCases = 0, partCnt = 0;
  for (auto& itThr : testData) {
    partCnt++;
    std::cout << "Part " << std::dec << partCnt << " - Elements " << itThr.size() << " - Startseed 0x" <<  std::setfill('0') << std::setw(10) << std::hex << *itThr.begin() <<  std::endl;
    genCases += itThr.size();
  }


  std::string cmd = "exec rm " + dirname + "/*";
  if (dir_err == -1) rem_err = system(cmd.c_str());
  else {std::cout << "Created directory" << dirname << std::endl;}

  if (rem_err == 0) std::cout << "Cleared directory" << dirname << std::endl;

  if (dir_err == -1 && rem_err != 0)
  {
    std::cerr << "Error creating/clearing directory" << dirname << std::endl;
    exit(1);
  }

  std::ofstream outfile;
  outfile.open(dirname + "/dm-test.log");
  //
  //outfile << "Created " << std::dec << range << " test range in " << std::dec << ((after - before) / 1000000ULL) << " ms" << std::endl;
  std::cout << "Given Startseed 0x" <<  std::setfill('0') << std::setw(10) << std::hex << seed <<  std::endl;
  outfile << "Given Startseed 0x" <<  std::setfill('0') << std::setw(10) << std::hex << seed <<  std::endl;
  std::cout << "Given Range" <<  std::setfill(' ') << std::setw(10) << std::dec << range << ", " << percentage << "% coverage, Final Range " << (( range * percentage) / 100 ) <<  std::endl;
  outfile << "Given Range" <<  std::setfill(' ') << std::setw(10) << std::dec << range << ", " << percentage << "% coverage, Final Range " << (( range * percentage) / 100 ) <<  std::endl;
  std::cout << "Created " << std::dec << genCases << " test cases in " << std::dec << ((after - before) / 1000000ULL) << " ms" << std::endl;


  uint64_t intervalCnt = 0;
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
        isSafe = cdm.isSafeToRemove("A", report);
        //outfile << "Sample " << std::dec << intervalCnt << "0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe << std::endl;
      } catch (const std::runtime_error& re) {
        std::stringstream auxstream;
        auxstream << "0x" << std::setfill('0') << std::setw(10) << std::hex << itVal;
        cdm.writeTextFile(dirname + "/debug_" + auxstream.str() + "_fuckup.dot", report);
        cdm.writeDotFile(dirname + "/coverage_" + auxstream.str() + "_fuckup.dot", cdm.getDownGraph(), false);
        fuckUpCnt++;
        outfile << "Sample " << std::dec << std::setfill('0') << std::setw(8) << intervalCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - FUCKUP" << std::endl;
        outfile << re.what() << std::endl;
        std::cout << "Sample " << std::dec << std::setfill('0') << std::setw(8) << intervalCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - FUCKUP" << std::endl;
        std::cout << re.what() << std::endl;
        std::flush(outfile);
      }
      after = cdm.getDmWrTime();
      sum += (after - before);
      intervalCnt++;

      cdm.verboseOff();
      if ((intervalCnt % interval) == 0) {
        average = sum / intervalCnt;
        std::stringstream auxstream;
        auxstream << "0x" << std::setfill('0') << std::setw(10) << std::hex << itVal;
        cdm.writeTextFile(dirname + "/debug_" + auxstream.str() + "_interval.dot", report);
        cdm.writeDotFile(dirname + "/coverage_" + auxstream.str() + "_interval.dot", cdm.getDownGraph(), true);
        std::cout << "Sample " << std::setfill(' ') << std::setw(10) << std::dec << intervalCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe;
        std::cout << " Time Total: " << std::setfill(' ') << std::setw(10) << std::dec << ((after - beginning) / 1000000ULL) << " Avg: " << std::setfill(' ') << std::setw(10) << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
        outfile << "Sample " << std::setfill(' ') << std::setw(10) << std::dec << intervalCnt << " 0x" << std::setfill('0') << std::setw(10) <<  std::hex << itVal << " - " << isSafe;
        outfile << " Time Total: " << std::setfill(' ') << std::setw(10) << std::dec << ((after - beginning) / 1000000ULL) << " Avg: " << std::setfill(' ') << std::setw(10) << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
        std::flush(outfile);
      }


    }
  }
  average = sum / intervalCnt;
  outfile << "Ran " << std::dec << intervalCnt << " of " << (( range * percentage) / 100 ) << " test range. " << fuckUpCnt << " f**kups" << std::endl;
  outfile << "Time Total: " << std::setfill(' ') << std::setw(10) << std::dec << ((after - beginning) / 1000000ULL) << " Avg: " << std::setfill(' ') << std::setw(10) << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
  std::cout << "Ran " << std::dec << intervalCnt << " of " << (( range * percentage) / 100 ) << " test range. " << fuckUpCnt << " f**kups" << std::endl;
  std::cout << "Time Total: " << std::setfill(' ') << std::setw(10) << std::dec << ((cdm.getDmWrTime() - beginning) / 1000000ULL) << " Avg: " << std::setfill(' ') << std::setw(10) << std::dec << (average / 1000000ULL) << " ms"  << std::endl;
  outfile.close();

  cdm.disconnect();

  return 0;
}

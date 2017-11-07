#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "carpeDM.h"


int main(int argc, char* argv[]) {


   

  CarpeDM cdm = CarpeDM();

  //TODO we need a dictionary independent of dot files, otherwise, how do we update?

  try {
    cdm.loadPatternsFile("dm.dict2");
  } catch (std::runtime_error const& err) {
   std::cerr << std::endl << " No pattern file" << std::endl;
  }

  

  std::string tests(argv[optind]);

  std::cout << "Found " << tests << std::endl;

  cdm.testPattern(tests);  


  cdm.storePatternsFile("dm.dict2");  
  

  return 0;
}

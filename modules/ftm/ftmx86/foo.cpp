#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>

#include "etherbone.h"

#define ETHERBONE_THROWS

using namespace etherbone;

int main(int argc, char* argv[]) {

  Socket ebs;
  Device ebd;
  std::vector<struct sdb_device> cpuDevs;

  ebs.open(0, EB_DATAX|EB_ADDRX);
  ebd.open(ebs, ebdevname.c_str(), EB_DATAX|EB_ADDRX, 3);



  return 0;
}

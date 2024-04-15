
#include <boost/assign.hpp>
#include <iostream>
#include "reflocation.h"
#include "dotstr.h"

using namespace DotStr::Misc;
namespace dmv   = DotStr::Misc;
namespace dloc  = DotStr::Locations::base;
namespace dfld  = DotStr::Locations::fields;

MemLocMap mlm = boost::assign::list_of< MemLocMap::relation >
( dmv::sZero, 0x0 )
( dloc::sRegisters, SHCTL_REGS );


MemFieldMap mfm = boost::assign::list_of< MemFieldMap::relation >
( dmv::sZero, 0x0 );


template<typename LeftType, typename RightType>
void printBimap(const boost::bimap<LeftType, RightType>& bm) {
  for (const auto& pair : bm) {
        std::cout << "key: " << pair.left << ", value: 0x" << std::hex << pair.right << std::endl;
    }
}


void showMemLocMap() {
  std::cout << "Memory Location Map:" << std:: endl;
  printBimap(mlm);
}

void showMemFieldMap() {
  std::cout << "Memory Field Map:" << std:: endl;
  printBimap(mfm);
}
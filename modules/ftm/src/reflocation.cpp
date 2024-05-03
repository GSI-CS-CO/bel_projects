
#include <boost/assign.hpp>
#include <iostream>
#include "reflocation.h"
#include "dotstr.h"
#include "graph.h"
#include "ebwrapper.h"



namespace dmv   = DotStr::Misc;
namespace dloc  = DotStr::Locations::base;
namespace dfld  = DotStr::Locations::fields;


void RefLocation::init(EbWrapper* ebd, const uint32_t sharedOffs) {
  ml.clear();
  mf.clear();
  //std::cout << "Init RefMaps" << std::hex << sharedOffs << std::endl;
  //std::cout << "Shared Offs: 0x" << std::hex << sharedOffs << std::endl;
  ml.insert({dmv::sZero, 0x0 });
  ml.insert({dloc::sRegisters, ebd->getCtlAdr(ADRLUT_SHCTL_REGS) + sharedOffs});
  mf.insert({dmv::sZero, 0x0 });
  //add more locations that should be commonly known here. Could even add one per thread with a loop etc

}

template<typename LeftType, typename RightType>
void RefLocation::printBimap(const boost::bimap<LeftType, RightType>& bm) const {
  for (const auto& pair : bm) {
        std::cout << "key: " << pair.left << ", value: 0x" << std::hex << pair.right << std::endl;
    }
}

void RefLocation::showMemLocMap() const {
  std::cout << "Memory Location Map:" << std:: endl;
  printBimap(ml);
}

void RefLocation::showMemFieldMap() const {
  std::cout << "Memory Field Map:" << std:: endl;
  printBimap(mf);
}

uint32_t RefLocation::getLocVal(const std::string& s) const {
  auto it = ml.left.find(s);
  return it->second;
}

uint32_t RefLocation::getFieldVal(const std::string& s) const {
  auto it = ml.left.find(s);
  return it->second;
}

std::string RefLocation::getLocName(const uint32_t a) const {
  auto it = ml.right.upper_bound(a);
  if(it == ml.right.begin()) throw std::out_of_range("Invalid upper_bound for MemLoc");
  it--;
  return it->second;
}

std::string RefLocation::getFieldName(const uint32_t a) const {
  auto itL = ml.right.upper_bound(a);
  if(itL == ml.right.begin()) throw std::out_of_range("Invalid upper_bound for MemLoc");
  itL--;

  auto itF = mf.right.upper_bound(a - itL->first);
  if(itF == mf.right.begin()) throw std::out_of_range("Invalid upper_bound for MemFieldc");
  itF--;
  return itF->second;
}




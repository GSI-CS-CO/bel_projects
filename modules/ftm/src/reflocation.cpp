
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
  mlm.clear();
  mfm.clear();
  std::cout << "Shared Offs: 0x" << std::hex << sharedOffs << std::endl;
  mlm.insert({dmv::sZero, 0x0 });
  mlm.insert({dloc::sRegisters, ebd->getCtlAdr(ADRLUT_SHCTL_REGS) + sharedOffs});

  mfm.insert({dmv::sZero, 0x0 });

}

template<typename LeftType, typename RightType>
void RefLocation::printBimap(const boost::bimap<LeftType, RightType>& bm) const {
  for (const auto& pair : bm) {
        std::cout << "key: " << pair.left << ", value: 0x" << std::hex << pair.right << std::endl;
    }
}

void RefLocation::showMemLocMap() const {
  std::cout << "Memory Location Map:" << std:: endl;
  printBimap(mlm);
}

void RefLocation::showMemFieldMap() const {
  std::cout << "Memory Field Map:" << std:: endl;
  printBimap(mfm);
}

RefLocationSearch RefLocation::getSearch(const std::string& sLoc, const std::string& sField) const {
  RefLocationSearch rls = RefLocationSearch(*this, sLoc, sField);
  return rls;
}

RefLocationSearch RefLocation::getSearch(uint32_t searchAdr) const {
  RefLocationSearch rls = RefLocationSearch(*this, searchAdr); 
  return rls;
  
}

RefLocationSearch::RefLocationSearch(const RefLocation& rl, const std::string& sLoc, const std::string& sField) : rl(rl) {
  MemLocMap::left_iterator itL    = rl.getMlm().left.find(sLoc);
  if(itL == rl.getMlm().left.end()) {throw std::runtime_error("ERROR: Reference Location lookup for '" + sLoc + "' failed");}
  MemFieldMap::left_iterator itF  = rl.getMfm().left.find(sField);
  if(itF == rl.getMfm().left.end()) {throw std::runtime_error("ERROR: Reference Field lookup for '" + sField + "' failed");}

  this->itL = rl.getMlm().project_up(itL);
  this->itF = rl.getMlm().project_up(itF);
};

RefLocationSearch::RefLocationSearch(const RefLocation& rl, uint32_t searchAdr) : rl(rl) {
  std::cout << "I am a Reflocsearch" << std:: endl;
  MemLocMap::right_iterator itL = rl.getMlm().right.upper_bound(searchAdr);
  if (itL == rl.getMlm().right.begin()) {
    throw std::out_of_range("Invalid upper_bound operation");
  }
  --itL;
  std::cout << "wtf... " << itL->second << std:: endl;
  std::cout << "Is this a bad alloc? " << itL->second << std:: endl;
  auto projectedL = rl.getMlm().project_up(itL);

  MemFieldMap::right_iterator itF = rl.getMfm().right.upper_bound(searchAdr - itL->first);
  if (itF == rl.getMfm().right.begin()) {
    throw std::out_of_range("Invalid upper_bound for MemFieldMap");
  }
  --itF;
  auto projectedF = rl.getMfm().project_up(itF); 

/*
  if (projectedL == rl.getMlm().left.end() || projectedF == rl.getMfm().left.end()) {
    throw std::runtime_error("Invalid iterator after projection");
  }
*/
  this->itL = projectedL;
  this->itF = projectedF;


  std::cout << "Is this a bad alloc? " << this->itL->left << std:: endl;
};

RefLocationSearch::RefLocationSearch(const RefLocation& rl, MemLocMap::iterator itL, MemFieldMap::iterator itF) : rl(rl), itL(itL), itF(itF) {};

std::string RefLocationSearch::getLocName() {
  return this->itL->left;
}

std::string RefLocationSearch::getFieldName() {
  return this->itF->left;
}

uint32_t RefLocationSearch::getLocVal() {
  return this->itL->right;
}

uint32_t RefLocationSearch::getFieldVal() {
  return this->itF->right;
}
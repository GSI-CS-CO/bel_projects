
#include <boost/assign.hpp>
#include <iostream>
#include "reflocation.h"
#include "dotstr.h"
#include "graph.h"
#include "ebwrapper.h"



namespace dmv   = DotStr::Misc;
namespace dloc  = DotStr::Locations::base;
namespace dfld  = DotStr::Locations::fields;


void RefLocation::init(EbWrapper* ebd) {
  mlm.clear();
  mfm.clear();

  mlm.insert({dmv::sZero, 0x0 });
  mlm.insert({dloc::sRegisters, ebd->getCtlAdr(ADRLUT_SHCTL_REGS) });

  mfm.insert({dmv::sZero, 0x0 });

}

template<typename LeftType, typename RightType>
void RefLocation::printBimap(const boost::bimap<LeftType, RightType>& bm) {
  for (const auto& pair : bm) {
        std::cout << "key: " << pair.left << ", value: 0x" << std::hex << pair.right << std::endl;
    }
}

void RefLocation::showMemLocMap() {
  std::cout << "Memory Location Map:" << std:: endl;
  printBimap(mlm);
}

void RefLocation::showMemFieldMap() {
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
  MemLocMap::right_iterator itL = rl.getMlm().right.upper_bound(searchAdr);
  --itL;
  MemFieldMap::right_iterator itF = rl.getMfm().right.upper_bound(searchAdr - itL->first);
  --itF;

  this->itL = rl.getMlm().project_up(itL);
  this->itF = rl.getMlm().project_up(itF);
};

RefLocationSearch::RefLocationSearch(const RefLocation& rl, MemLocMap::iterator itL, MemFieldMap::iterator itF) : rl(rl), itL(itL), itF(itF) {};

std::string RefLocationSearch::getLocName() {
  return itL->left;
}

std::string RefLocationSearch::getFieldName() {
  return itF->left;
}

uint32_t RefLocationSearch::getLocVal() {
  return itL->right;
}

uint32_t RefLocationSearch::getFieldVal() {
  return itF->right;
}
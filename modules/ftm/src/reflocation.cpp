
#include <boost/assign.hpp>
#include <iostream>
#include "reflocation.h"
#include "dotstr.h"
#include "graph.h"
#include "ebwrapper.h"



namespace dmv   = DotStr::Misc;
namespace dloc  = DotStr::Locations::base;
namespace dfld  = DotStr::Locations::fields;

/** Sets the global memory location table
   * This allows graph nodes referencing external memory adr. such as registers
   * @param ebd Active etherbone connection to a DM instance
   * @param sharedOffs memory shared offset read from Firmware Info ROM
   * @return existence of hash
  */
void RefLocation::init(EbWrapper* ebd, const uint32_t sharedOffs) {
  ml.clear();
  mf.clear();

  ml.insert({dmv::sZero, 0x0 });                                                      //zero as an always working trst
  ml.insert({dloc::sThrCtl,     sharedOffs + SHCTL_THR_CTL});                         //allows thread start/halt from other platfroms
  ml.insert({dloc::sRegisters,  sharedOffs + ebd->getCtlAdr(ADRLUT_SHCTL_REGS)});     //'mail boxes' for interplatform communication

  //Thread staging areas (pretime, starttime etc). Allows remote manipulation of these parameters (be)for(e) thread starts
  for(int i=0; i < ebd->getThrQty(); i++) {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << i;
    ml.insert({dloc::sThrStaging + "_" + oss.str(), sharedOffs + ebd->getCtlAdr(ADRLUT_SHCTL_THR_STA) + i * _T_TS_SIZE_ + T_TS_STARTTIME});
    ml.insert({dloc::sThrDataCt    + "_" + oss.str(), sharedOffs + ebd->getCtlAdr(ADRLUT_SHCTL_THR_DAT) + i * _T_TD_SIZE_ + T_TD_CURRTIME});
    ml.insert({dloc::sThrDataDl    + "_" + oss.str(), sharedOffs + ebd->getCtlAdr(ADRLUT_SHCTL_THR_DAT) + i * _T_TD_SIZE_ + T_TD_DEADLINE});
  }


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




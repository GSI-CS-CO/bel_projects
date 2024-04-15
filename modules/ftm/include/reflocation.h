#ifndef _REFLOCATION_H_
#define _REFLOCATION_H_

#include <stdint.h>
#include <string>
#include <boost/bimap.hpp>

typedef boost::bimap< std::string, uint32_t > MemLocMap;
typedef boost::bimap< std::string, uint32_t > MemFieldMap;
extern MemLocMap mlm;
extern MemFieldMap mfm;

template<typename LeftType, typename RightType>
void printBimap(const boost::bimap<LeftType, RightType>& bm);
void showMemLocMap();
void showMemFieldMap();

struct RefLocation {
  MemLocMap::iterator itL;
  MemFieldMap::iterator itF;

  RefLocation(const std::string& sLoc, const std::string& sField) {
    MemLocMap::left_iterator itL    = mlm.left.find(sLoc);
    if(itL == mlm.left.end()) {throw std::runtime_error("ERROR: Reference Location lookup for '" + sLoc + "' failed");}
    MemFieldMap::left_iterator itF  = mfm.left.find(sField);
    if(itF == mfm.left.end()) {throw std::runtime_error("ERROR: Reference Field lookup for '" + sField + "' failed");}

    this->itL = mlm.project_up(itL);
    this->itF = mlm.project_up(itF);
  };

  RefLocation(uint32_t searchAdr) {
    MemLocMap::right_iterator itL = mlm.right.upper_bound(searchAdr);
    --itL;
    MemFieldMap::right_iterator itF = mfm.right.upper_bound(searchAdr - itL->first);
    --itF;

    this->itL = mlm.project_up(itL);
    this->itF = mlm.project_up(itF);
  };

  RefLocation(MemLocMap::iterator itL, MemFieldMap::iterator itF) : itL(itL), itF(itF) {};

  std::string getLocName() {
    return itL->left;
  }

  std::string getFieldName() {
    return itF->left;
  }

  uint32_t getLocVal() {
    return itL->right;
  }

  uint32_t getFieldVal() {
    return itF->right;
  }

};

#endif
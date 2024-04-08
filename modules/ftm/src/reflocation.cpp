
#include "reflocation.h"
#include <boost/assign.hpp>
#include <iostream>

MemLocMap mlm = boost::assign::list_of< MemLocMap::relation >
( "VOID"  , 0x0 )
( "one"  , 0x100 )
( "two"  , 0x200 )
( "three", 0x300 )
( "OOB", 0x400 );

MemFieldMap mfm = boost::assign::list_of< MemFieldMap::relation >
( "fieldZero" , 0x00 )
( "fieldOne"  , 0x10 )
( "fieldTwo"  , 0x20 )
( "fieldThree", 0x30 );


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
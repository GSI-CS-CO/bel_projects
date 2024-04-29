#ifndef _REFLOCATION_H_
#define _REFLOCATION_H_

#include <stdint.h>
#include <string>
#include <boost/bimap.hpp>

typedef boost::bimap< std::string, uint32_t > MemLocMap;
typedef boost::bimap< std::string, uint32_t > MemFieldMap;




typedef uint32_t (*lutPtr)  ( uint32_t );

class RefLocationSearch;
class EbWrapper;



class RefLocation {
  MemLocMap mlm;
  MemFieldMap mfm;

  public:
    RefLocation() {};
    ~RefLocation(){};

    void init(EbWrapper* ebd, const uint32_t sharedOffs);

    RefLocationSearch getSearch(const std::string& sLoc, const std::string& sField) const;
    RefLocationSearch getSearch(uint32_t searchAdr) const;

    template<typename LeftType, typename RightType>
    void printBimap(const boost::bimap<LeftType, RightType>& bm) const;
    
    void showMemLocMap() const;
    
    void showMemFieldMap() const;

    MemLocMap getMlm() const { return mlm;}
    MemFieldMap getMfm() const { return mfm;}
};

class RefLocationSearch {
  MemLocMap::iterator itL;
  MemFieldMap::iterator itF;
  const RefLocation& rl;

  public:

    RefLocationSearch(const RefLocation& rl, const std::string& sLoc, const std::string& sField);
  
    RefLocationSearch(const RefLocation& rl, uint32_t searchAdr);
  
    RefLocationSearch(const RefLocation& rl, MemLocMap::iterator itL, MemFieldMap::iterator itF);
  
    std::string getLocName();
  
    std::string getFieldName();
  
    uint32_t getLocVal();
  
    uint32_t getFieldVal();

};



#endif
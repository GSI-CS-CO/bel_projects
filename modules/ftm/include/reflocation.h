#ifndef _REFLOCATION_H_
#define _REFLOCATION_H_

#include <stdint.h>
#include <string>
#include <boost/bimap.hpp>

typedef boost::bimap< std::string, uint32_t > MemMap;


class EbWrapper;



class RefLocation {
  MemMap ml;
  MemMap mf;

  public:
    RefLocation() {};
    ~RefLocation(){};

    void init(EbWrapper* ebd, const uint32_t sharedOffs);

    template<typename LeftType, typename RightType>
    void printBimap(const boost::bimap<LeftType, RightType>& bm) const;
    
    void showMemLocMap() const;

    void showMemFieldMap() const;

    uint32_t getLocVal(const std::string& s) const;
    
    uint32_t getFieldVal(const std::string& s) const;
    
    std::string getLocName(const uint32_t a) const;
    
    std::string getFieldName(const uint32_t a) const;

};

#endif
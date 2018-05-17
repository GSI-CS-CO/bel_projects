#ifndef _COVENANT_TABLE_H_
#define _COVENANT_TABLE_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "graph.h"
#include "hashmap.h"

using boost::multi_index_container;
using namespace boost::multi_index;


using namespace DotStr::Misc;

struct CovenantMeta {
  std::string name;
  uint8_t     prio;
  uint8_t     slot;
  uint32_t    chkSum;

  
  CovenantMeta(std::string name, uint8_t prio, uint8_t slot, uint32_t chkSum) : name(name), prio(prio), slot(slot), chkSum(chkSum){}
  CovenantMeta() : name(""), prio(-1), slot(-1), chkSum(-1) {}  
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & name;
      ar & prio;
      ar & slot;
      ar & chkSum;
  }
};


struct Name{};


typedef boost::multi_index_container<
  CovenantMeta,
  indexed_by<
    hashed_unique <
      tag<Name>,  BOOST_MULTI_INDEX_MEMBER(CovenantMeta, std::string, name)>  
  >    
 > CovenantMeta_set;


typedef CovenantMeta_set::iterator cmI;

class CovenantTable {

  CovenantMeta_set a;

private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & a;
  }
  



public:

  CovenantTable(){};
  ~CovenantTable(){};
  CovenantTable(CovenantTable const &src) : a(src.a) {};

  CovenantTable &operator=(const CovenantTable &src)
  {
    a = src.a;
    return *this;
  }

  std::string store() { 
    std::stringstream os;
    boost::archive::text_oarchive oa(os);
    oa << BOOST_SERIALIZATION_NVP(*this);
    return os.str();
  }

  void load(const std::string& s){
    std::stringstream is;
    is.str(s);
    boost::archive::text_iarchive ia(is);
    ia >> BOOST_SERIALIZATION_NVP(*this);
  }

  uint32_t genChecksum(const QueueElement& qe) {
    //easiest to just convert to string and hash
    std::string tmp = std::to_string(qe.pending) + std::to_string(qe.validTime) + qe.sType + qe.flowDst + qe.flowDstPattern + std::to_string(qe.flowPerma);
    return HashMap::hash(tmp);
  }

  bool insert(const std::string& name, uint8_t prio, uint8_t slot, const QueueElement& qe) {
    CovenantMeta m = CovenantMeta(name, prio, slot, genChecksum(qe));
    auto x = a.insert(m); 
    return x.second; 
  }


 
  cmI lookup(const std::string& s)   {auto tmp = a.get<Name>().find(s); return a.iterator_to(*tmp);}

  bool isOk(cmI it) const {return (it != a.end()); }

  bool insert(cmI e) {
    CovenantMeta m = CovenantMeta(e->name, e->prio, e->slot, e->chkSum);
    auto x = a.insert(m); 
    return x.second; 
  }

  bool remove(const std::string& s) {
    auto range = a.get<Name>().equal_range(s);
    auto it = a.get<Name>().erase(range.first, range.second); 
    return (it != a.end() ? true : false);
  };

  bool remove(cmI e) {
    auto it = a.get<Name>().erase(e); 
    return (it != a.end() ? true : false);
  };


  void clear() { a.clear(); }

  void debug(std::ostream& os) {
    os << "Active Safe2remove-Covenants:" << std::endl; 
    for (cmI x = a.begin(); x != a.end(); x++) { 
      os << x->name << " prio: " << std::dec << (int)x->prio <<  ", slot: " <<  (int)x->slot <<  ", ChkSum: 0x" << std::hex <<  x->chkSum << std::endl;       
    }
  }

  const CovenantMeta_set& getTable() const { return a; }
  const size_t getSize()          const { return a.size(); }


 


};

#endif 
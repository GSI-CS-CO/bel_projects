#ifndef _STATIC_TABLE_H_
#define _STATIC_TABLE_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <boost/optional.hpp>
#include <boost/container/vector.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "dotstr.h"
#include "common.h"


using boost::multi_index_container;
using namespace boost::multi_index;


using namespace DotStr::Misc;

struct StaticMeta {
  std::string node;
  int         cpu;
  uint32_t    adr;
  uint32_t    offs;
  uint32_t    size;
  int         thread;
  vertex_t    v;

//ext int / global local statt CPU ? gefährlich (blockade) ?
  StaticMeta(const std::string& sNode, int cpu, uint32_t adr, uint32_t size, uint32_t offs, int thread, vertex_t v) : node(sNode), cpu(cpu), adr(adr), size(size), offs(offs), thread(thread), v(v) {}
  StaticMeta(const std::string& sNode, int cpu, uint32_t adr, uint32_t size, uint32_t offs, int thread) : node(sNode), cpu(cpu), adr(adr), size(size), offs(offs), thread(thread), v(null_vertex) {}
  StaticMeta(const std::string& sNode, int cpu, uint32_t adr, uint32_t size, uint32_t offs, int thread) : node(sNode), cpu(cpu), adr(adr), size(size), offs(offs), thread(0), v(null_vertex) {}
  
    // Multiindexed Elements are immutable, must use the modify function of the container to change attributes

  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & node;
      ar & cpu;
      ar & adr;
      ar & offs;
      ar & size;
      ar & thread;
      ar & v;

  }
};



//necessary to avoid confusion with classnames elsewhere
namespace Static {

struct Name{};
struct Vertex{};
struct CpuAdr{};



}




typedef boost::multi_index_container<
  StaticMeta,

  indexed_by<
    ordered_unique <
      tag<Static::Name>,  BOOST_MULTI_INDEX_MEMBER(StaticMeta, std::string, node)>,
    hashed_unique<
      tag<Static::Vertex>,  BOOST_MULTI_INDEX_MEMBER(StaticMeta,vertex_t,v)>,
    ordered_unique<
      tag<Static::CpuAdr>,
      composite_key<
        StaticMeta,
        BOOST_MULTI_INDEX_MEMBER(StaticMeta,uint8_t,cpu),
        BOOST_MULTI_INDEX_MEMBER(StaticMeta,uint32_t,adr)
      >
    >
  >
 > StaticMeta_set;






typedef StaticMeta_set::iterator pmI;

/* accepted

  //the iterator dilemma

ivs.get<1>() gives you index, not iterator. You need to call begin(), end() and other methods on that index to get iterator (like you do on containers). You better use typedef though:

indexed_vertex_set ivs;
typedef indexed_vertex_set::nth_index<1>::type sorted_index;
sorted_index &idx = ivs.get<1>();
for( sorted_index::iterator it = idx.begin(); it != idx.end(); ++it ) {
    it->vid = 123; // getting access to fields
}

*/



class StaticTable {
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & a;
  }

  StaticMeta_set a;


public:

  StaticTable(){};
  ~StaticTable(){};
  StaticTable(StaticTable const &src) : a(src.a) {};

  StaticTable &operator=(const StaticTable &src)
  {
    a = src.a;
    return *this;
  }

  std::string store();
  void load(const std::string& s);

  bool insert(const std::string& sNode);

  template <typename Tag>
  pmI lookup(const std::string& s)   {auto tmp = a.get<Tag>().find(s); return a.iterator_to(*tmp);}


  //Lookup a node, create if non existent. Single return value as node names are uniqu./d
  pmI lookupOrCreateNode(const std::string& sNode);


  template <typename Tag>
  bool remove(const std::string& s) {
    auto range = a.get<Tag>().equal_range(s);
    auto it = a.get<Tag>().erase(range.first, range.second);
    return (it != a.end() ? true : false);
  };

  void clear() { a.clear(); }

  void setPattern (pmI it, const std::string& sNew, bool entry, bool exit)  { a.modify(it, [sNew, entry, exit](StaticMeta& p){p.pattern  = sNew; p.patternEntry  = entry; p.patternExit  = exit;}); }
  void setPattern (pmI it, const std::string& sNew) { setPattern(it, sNew, false, false); }
  void setPattern (const std::string& sNode, const std::string& sNew, bool entry, bool exit) { setPattern(lookupOrCreateNode(sNode), sNew, entry, exit); }
  void setPattern (const std::string& sNode, const std::string& sNew) { setPattern(sNode, sNew, false, false); }

  template <typename Tag, std::string StaticMeta::*group>
  vStrC getGroups(const std::string& sNode) {
    vStrC res; auto x  = a.get<Tag>().equal_range(sNode);
    for (auto it = x.first; it != x.second; ++it) {res.push_back(*it.*group); }
    return res;
  }





  const StaticMeta_set& getTable() const { return a; }
  const size_t getSize()          const { return a.size(); }


  void debug(std::ostream& os);


};

#endif

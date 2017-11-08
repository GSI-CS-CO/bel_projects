#ifndef _GROUP_TABLE_H_
#define _GROUP_TABLE_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "dotstr.h"


using boost::multi_index_container;
using namespace boost::multi_index;


using namespace DotStr::Misc;

struct GroupMeta {
  std::string node;  //name of pattern
  std::string pattern;  //name of pattern
  bool patternEntry, patternExit;
  std::string beamproc; //name of entry node
  bool beamprocEntry, beamprocExit;

  
  GroupMeta(const std::string& node) : node(node), pattern(sUndefined),  patternEntry(false), patternExit(false), beamproc(sUndefined), beamprocEntry(false), beamprocExit(false) {}
  GroupMeta() : node(sUndefined), pattern(sUndefined),  patternEntry(false), patternExit(false), beamproc(sUndefined), beamprocEntry(false), beamprocExit(false) {}

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & node;
      ar & pattern;
      ar & patternEntry;
      ar & patternExit;
      ar & beamproc;
      ar & beamprocEntry;
      ar & beamprocExit;


  }
  
};

namespace Groups {

struct Node{};
struct Pattern{};
struct BeamProc{};

}



typedef boost::multi_index_container<
  GroupMeta,
  indexed_by<
    hashed_unique <
      tag<Groups::Node>,  BOOST_MULTI_INDEX_MEMBER(GroupMeta, std::string, node)>,
    hashed_non_unique <
      tag<Groups::Pattern>,  BOOST_MULTI_INDEX_MEMBER(GroupMeta, std::string, pattern)>,
    hashed_non_unique <
      tag<Groups::BeamProc>,  BOOST_MULTI_INDEX_MEMBER(GroupMeta, std::string, beamproc)>  
  >    
 > GroupMeta_set;

typedef GroupMeta_set::iterator pmI;
typedef std::pair<pmI, pmI> pmRange;


class GroupTable {
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & a;
  }
  
  GroupMeta_set a;


public:

  GroupTable(){};
  ~GroupTable(){};

  std::string store();


  void load(const std::string& s);

  bool insert(const std::string& sNode);

  template <typename Tag>
  pmRange lookup(std::string s)   {auto test = a.get<Tag>().equal_range(s); return pmRange({a.iterator_to(*test.first), a.iterator_to(*test.second)});}
  pmI lookUpOrCreateNode(const std::string& sNode);

  bool isOk(pmI it) const {return (it != a.end()); }

  template <typename Tag>
  bool contains(const std::string& s) const {return isOk(lookup<Tag>(s));}

  template <typename Tag>
  bool remove(const std::string& s) {auto it = a.get<Tag>().erase(s); return (it != a.end() ? true : false);};

  void clear() { a.clear(); }

  void setPattern (pmI it, const std::string& sNew, bool entry, bool exit)  { a.modify(it, [sNew, entry, exit](GroupMeta& p){p.pattern  = sNew; p.patternEntry  = entry; p.patternExit  = exit;}); } 
  void setPattern (pmI it, const std::string& sNew) { setPattern(it, sNew, false, false); }
  void setPattern (const std::string& sNode, const std::string& sNew, bool entry, bool exit) { setPattern(lookUpOrCreateNode(sNode), sNew, entry, exit); }
  void setPattern (const std::string& sNode, const std::string& sNew) { setPattern(sNode, sNew, false, false); }
  
  void setBeamProc (pmI it, const std::string& sNew, bool entry, bool exit)  { a.modify(it, [sNew, entry, exit](GroupMeta& p){p.beamproc = sNew; p.beamprocEntry = entry; p.beamprocExit = exit;}); } 
  void setBeamProc (pmI it, const std::string& sNew) { setBeamProc(it, sNew, false, false); }
  void setBeamProc (const std::string& sNode, const std::string& sNew, bool entry, bool exit) { setBeamProc(lookUpOrCreateNode(sNode), sNew, entry, exit); } ;
  void setBeamProc (const std::string& sNode, const std::string& sNew) { setBeamProc(sNode, sNew, false, false); }
  

  const std::string& getPatternEntry(const std::string& sPattern) {
    pmRange x  = lookup<Groups::Pattern>(sPattern); 
    if (isOk(x.first) && isOk(x.second)) {

    }

    return sUndefined;
  }
  const GroupMeta_set& getTable() const { return a; }
  const size_t getSize()            const { return a.size(); }


  void debug();


};

#endif
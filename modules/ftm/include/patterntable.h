#ifndef _PATTERN_TABLE_H_
#define _PATTERN_TABLE_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "dotstr.h"


using boost::multi_index_container;
using namespace boost::multi_index;


using namespace DotStr::Misc;

struct PatternMeta {
  std::string pattern;  //name of pattern
  std::string entry; //name of entry node
  std::string exit;  //name of exit node

  PatternMeta(const std::string& pattern, const std::string&  entry, const std::string&  exit) : pattern(pattern), entry(entry),       exit(exit) {}
  PatternMeta(const std::string& pattern)                                      : pattern(pattern), entry(sUndefined),  exit(sUndefined) {}
  PatternMeta()                                      : pattern(sUndefined), entry(sUndefined),  exit(sUndefined) {}

  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & pattern;
      ar & entry;
      ar & exit;
  }
  
};


struct Pattern{};
struct Entry{};
struct Exit{};





typedef boost::multi_index_container<
  PatternMeta,
  indexed_by<
    ordered_non_unique <
      tag<Pattern>,  BOOST_MULTI_INDEX_MEMBER(PatternMeta, std::string, pattern)>,
    ordered_non_unique <
      tag<Entry>,  BOOST_MULTI_INDEX_MEMBER(PatternMeta, std::string, entry)>,  
    ordered_non_unique <
      tag<Exit>,  BOOST_MULTI_INDEX_MEMBER(PatternMeta, std::string, exit)>
  >    
 > PatternMeta_set;

typedef PatternMeta_set::iterator pmI;



class PatternTable {
private:
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & a;
  }
  
  PatternMeta_set a;


public:

  PatternTable(){};
  ~PatternTable(){};

  std::string store();


  void load(const std::string& s);

  pmI insertPattern(const std::string& pattern, bool& success);

  template <typename Tag>
  pmI lookup(const std::string& s)   const {auto it = a.get<Tag>().find(s); return a.iterator_to( *it ); }

  bool isOk(pmI it) const {return (it != a.end()); }

  template <typename Tag>
  bool remove(const std::string& s) {auto it = a.get<Tag>().erase(s); return (it != a.end() ? true : false);};

  void clear() { a.clear(); }

  void setEntry(pmI it, const std::string& sNew) { a.modify(it, [sNew](PatternMeta& p){p.entry  = sNew;}); } 
  void setExit(pmI it, const std::string& sNew)   { a.modify(it, [sNew](PatternMeta& p){p.exit   = sNew;}); }

  
  const PatternMeta_set& getTable() const { return a; }
  const size_t getSize()            const { return a.size(); }


  void debug();


};

#endif
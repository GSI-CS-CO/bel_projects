#ifndef _GLOBAL_REF_TABLE_H_
#define _GLOBAL_REF_TABLE_H_


#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <boost/optional.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include "common.h"
#include "bmiContainers.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>

using boost::multi_index_container;
using namespace boost::multi_index;


struct RefMeta {
  uint8_t     cpu;
  uint32_t    adr;
  uint32_t    hash;

  RefMeta() : cpu(0), adr(0), hash(0) {};
  RefMeta(uint8_t cpu, uint32_t adr, uint32_t hash) : cpu(cpu), adr(adr), hash(hash) {};


  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & cpu;
      ar & adr;
      ar & hash;
 
  }
};

struct RefMeta_set {
  using type = boost::multi_index_container<
  RefMeta,
  indexed_by<
    hashed_unique<
      tag<Hash>,  BOOST_MULTI_INDEX_MEMBER(RefMeta,uint32_t,hash)>,
    ordered_unique<
      tag<CpuAdr>,
      composite_key<
        RefMeta,
        BOOST_MULTI_INDEX_MEMBER(RefMeta,uint8_t,cpu),
        BOOST_MULTI_INDEX_MEMBER(RefMeta,uint32_t,adr)
      >
    >
  >
 >;
};

typedef RefMeta_set::type::iterator refIt;

class GlobalRefTable {
    

private:
  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) ///< Serializer implementing boost string serialization
  {
      ar & BOOST_SERIALIZATION_NVP(m);
  }

  RefMeta_set::type m;

public:
    

    GlobalRefTable() = default;  // Default constructor
    ~GlobalRefTable(){};
    GlobalRefTable(GlobalRefTable const &src) : m(src.m) {};
    GlobalRefTable &operator=(const GlobalRefTable &src) { m = src.m; return *this; }

    const RefMeta_set::type& getTable() const { return m; }

    bool insert(uint8_t cpu, uint32_t adr, uint32_t hash) {auto x = m.insert({cpu, adr, hash}); return x.second;}
  

    bool removeByAdr(uint8_t cpu, uint32_t adr);
    bool removeByHash(uint32_t hash);
    void clear() { m.clear(); }

    refIt lookupHash(uint32_t hash, const std::string& exMsg = "")  const;
    refIt lookupHashNoEx(uint32_t hash) const;
    refIt lookupAdr(uint8_t cpu, uint32_t adr, const std::string& exMsg = "")    const;


    bool isOk(refIt it) const {return (it != m.end()); }

    void debug(std::ostream& os);

    //could this be generic to all stored containers?

    // Store the object in a serialized form (to a string)
    std::string store();

    // Load the object from a serialized form (from a string)
    void load(const std::string& s);
};


#endif

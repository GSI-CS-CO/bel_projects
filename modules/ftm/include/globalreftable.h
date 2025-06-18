#ifndef _GLOBAL_REF_TABLE_H_
#define _GLOBAL_REF_TABLE_H_


#include <stdint.h>
#include <string>
#include <iostream>
#include <map>
#include <boost/serialization/map.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


typedef std::map< uint32_t, uint32_t > refMap;
typedef refMap::iterator refIt;

class GlobalRefTable {
    refMap m;

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


public:
    

    GlobalRefTable() = default;  // Default constructor

    bool insert(uint32_t adr, uint32_t hash)  {auto x = m.insert(std::make_pair(adr, hash)); return x.second; }

    bool remove(uint32_t adr)                 {auto it = m.erase(adr); return (it != 0);}

    uint32_t getHash(uint32_t adr);

    refIt lookupAdr(uint32_t adr) {return m.find(adr);}

    void clear() { m.clear(); }

    std::pair<refIt, refIt> getMapRange() { return std::make_pair(m.begin(), m.end());}

    bool isOk(refIt it) const {return (it != m.end()); }

    void debug(std::ostream& os);

    //could this be generic to all stored containers?

    // Store the object in a serialized form (to a string)
    std::string store();

    // Load the object from a serialized form (from a string)
    void load(const std::string& s);
};


#endif

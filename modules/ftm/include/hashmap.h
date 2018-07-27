#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdint.h>
#include <boost/bimap.hpp>
#include <boost/optional.hpp>
#include <fstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

/*
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
*/
typedef boost::bimap< uint32_t, std::string > hBiMap;
typedef hBiMap::value_type hashValue;


class HashMap {
private:
  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & BOOST_SERIALIZATION_NVP(hm);
  }

  static const unsigned int FNV_PRIME     = 16777619u;
  static const unsigned int OFFSET_BASIS  = 2166136261u;
  hBiMap hm;



protected:
  

public:

  HashMap()   {};
  ~HashMap()  {};

  HashMap &operator=(const HashMap &src)
  {
    hm = src.hm;
    return *this;
  }


  static uint32_t hash(const std::string& s); 
  static uint32_t fnvHash(const char* str);
  boost::optional<const uint32_t&> add(const std::string& name);
  bool remove(const std::string& name);
  bool remove(const uint32_t hash);

  const std::string& lookup(const uint32_t hash, const std::string& exMsg = "")     const;
  const uint32_t&    lookup(const std::string& name, const std::string& exMsg = "") const;
  bool contains(const uint32_t hash)     const;
  bool contains(const std::string& name) const;
  void clear() {hm.clear();}
  std::string store();
  void load(const std::string& s);
  int size() {return hm.size();}
  void debug(std::ostream& os);


};

#endif
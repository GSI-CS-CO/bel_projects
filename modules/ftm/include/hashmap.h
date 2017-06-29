#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdint.h>
#include <boost/bimap.hpp>
#include <boost/optional.hpp>


typedef boost::bimap< uint32_t, std::string > hBiMap;
typedef hBiMap::value_type hashValue;


class HashMap {
private:

  const unsigned int FNV_PRIME     = 16777619u;
  const unsigned int OFFSET_BASIS  = 2166136261u;
  hBiMap hm;

protected:
  uint32_t fnvHash(const char* str);

public:

  HashMap()   {};
  ~HashMap()  {};

  boost::optional<const uint32_t&> add(const std::string& name);
  bool remove(const std::string& name);
  bool remove(const uint32_t hash);
  boost::optional<const std::string&> lookup(const uint32_t hash)     const;
  boost::optional<const uint32_t&>    lookup(const std::string& name) const;
  bool contains(const uint32_t hash)     const;
  bool contains(const std::string& name) const;
  void clear() {hm.clear();}

};  

#endif
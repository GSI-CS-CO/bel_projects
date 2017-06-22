#include "hashmap.h"
#include <iostream>

uint32_t HashMap::fnvHash(const char* str)
{
    const size_t length = strlen(str) + 1;
    uint32_t hash = OFFSET_BASIS;
    for (size_t i = 0; i < length; ++i)
    {
        hash ^= *str++;
        hash *= FNV_PRIME;
    }
    return hash;
}

boost::optional<const uint32_t&> HashMap::add(const std::string& name) {
  uint32_t hash = fnvHash(name.c_str());
  try {
    hm.insert( hashValue(hash, name) ); 
  } catch (...) {
    std::cout << "Failed to add " << name << std::endl;
    throw; 
    return boost::optional<const uint32_t&>();
  } 

  return boost::optional<const uint32_t&>(hm.right.at(name));
}

bool HashMap::remove(const std::string& name) {
  if (hm.right.count(name) > 0) {hm.right.erase(name); return true;}
  return false;
}

bool HashMap::remove(const uint32_t hash) {
  if (hm.left.count(hash) > 0) {hm.left.erase(hash); return true;}
  return false;
}

boost::optional<const std::string&> HashMap::lookup(const uint32_t hash)     const  { try { return hm.left.at(hash);}  catch (...) {throw; return boost::optional<const std::string&>(); }  }
boost::optional<const uint32_t&>    HashMap::lookup(const std::string& name) const  { try { return hm.right.at(name);} catch (...) {throw; return boost::optional<const uint32_t&>();    }  }  


#include "hashmap.h"
#include <iostream>
#include <sstream>


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
    std::cout << "Failed to add " << name;
    if(lookup(hash)) throw std::runtime_error("'" + name + "' would cause a hash collision with '" + lookup(hash).get() + "'");
    else throw; 
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

boost::optional<const std::string&> HashMap::lookup(const uint32_t hash)     const  { try { return hm.left.at(hash);}  catch (...) {//throw std::runtime_error("Hash " + std::to_string(hash) + " not found"); 
                                                                                                                                    return boost::optional<const std::string&>(); }  
                                                                                                                                   }

boost::optional<const uint32_t&>    HashMap::lookup(const std::string& name) const  { try { return hm.right.at(name);} catch (...) {//throw std::runtime_error("Name " + name + " not found"); 
                                                                                                                                    return boost::optional<const uint32_t&>();    }  }  

bool HashMap::contains(const uint32_t hash)     const {if (hm.left.count(hash)  > 0) {return true;} else {return false;} };

bool HashMap::contains(const std::string& name) const {if (hm.right.count(name) > 0) {return true;} else {return false;} };

std::string HashMap::store() {
  std::stringstream os;
  boost::archive::text_oarchive oa(os);
  //boost::archive::xml_oarchive oa(ofs);
  oa << BOOST_SERIALIZATION_NVP(*this);
  return os.str();
}

void HashMap::load(const std::string& s) {
  std::stringstream is;
  is.str(s);
  
  boost::archive::text_iarchive ia(is);
  //boost::archive::xml_iarchive ia(ifs);
  ia >> BOOST_SERIALIZATION_NVP(*this);
}

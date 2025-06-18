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

/** Hashmap. Provides hashing and storage for reverse lookup
 *  The hashmap is intended for identifying data nodes on the DM by a 32b hash based on FNV algorithm. The hashtable is a bidirectional for easy reverse lookup
 */
class HashMap {
private:
  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) ///< Serializer implementing boost string serialization
  {
      ar & BOOST_SERIALIZATION_NVP(hm);
  }

  static const unsigned int FNV_PRIME     = 16777619u;    ///< FNV hashing algorithm, prime number constant
  static const unsigned int OFFSET_BASIS  = 2166136261u;  ///< FNV hashing algorithm, offset constant
  hBiMap hm; ///< Bidirectional hashmap



protected:


public:
  /** Standard Constructor
  *  
  */
  HashMap()   {};
  /** Standard Destructor
  *  
  */
  ~HashMap()  {};

  HashMap &operator=(const HashMap &src) ///< assignment operator for hashmap class
  {
    hm = src.hm;
    return *this;
  }

  /** Hashes a std:string and returns 32b FNV hash
   * Wrapper for character array version
   * @param s String to be hashed
   * @return 32b hash
   * @sa fnvHash()
  */
  static uint32_t hash(const std::string& s); ///< Hashes a std:string and returns 32b FNV hash

  /** Hashes a std:string and returns 32b FNV hash
   * Hashes the input character array using th FNV algorithm to 32 bit hash. Expects null termination
   * @param str Character array to be hashed
   * @return 32b hash
  */
  static uint32_t fnvHash(const char* str);  
  
  /** Add name/hash pair to hashmap
   * Hashes the input name string, creates new pair and adds it to the hashmap. Returns true on success, false otherwise. No insertion on hash collision
   * @param name String to be hashed
   * @return Outcome of addition to hashmap
  */
  boost::optional<const uint32_t&> add(const std::string& name);


  /** Removes pair from hashmap by name
   * Removes the entry with the given name from the hashmap. Returns true on success, false otherwise
    \param name String name of element to be removed 
    \return Outcome of removal from hashmap
  */
  bool remove(const std::string& name); ///< Removes pair from hashmap by name string
  
  /** Removes pair from hashmap by hash
   * Removes the entry with the given hash from the hashmap. Returns true on success, false otherwise
   * @param hash Hash of element to be removed 
   * @return Outcome of removal from hashmap
  */
  bool remove(const uint32_t hash); ///< Removes pair from hashmap by hash

  /** Lookup name to given hash from map
   * Looks up entry with the given hash in the hashmap and returns its string name. On error, an exception is thrown containing the exMsg string as first part of the message, usually information about the caller.
   * This is useful for debugging lookup is called very often and from many different methods
   * @param hash Hash of element to be lookup up
   * @param exMsg Message to be included in the error text on exception.
   * @return Corresponding name
  */
  const std::string& lookup(const uint32_t hash, const std::string& exMsg = "")     const;
  
  /** Lookup hash to given name from map
   * Looks up entry with the given name in the hashmap and returns its hash. On error, an exception is thrown containing the exMsg string as frist part of the message
   * @param hash Hash of element to be lookup up
   * @param exMsg Message to be included in the error text on exception.
   * @return Corresponding hash
  */
  const uint32_t&    lookup(const std::string& name, const std::string& exMsg = "") const;

  /** Checks if hashmap contains hash
   * If an entry with the given hash exists, return true, else false
   * @param hash Hash of element to be checked
   * @return existence of hash
  */
  bool contains(const uint32_t hash)     const;
  

  /** Checks if hashmap contains name
   * If an entry with the given name exists, return true, else false
   * @param name Name of element to be checked
   * @return existence of name
  */
  bool contains(const std::string& name) const;


  /** Clear hashmap
   * Removes all entries from map container
  */
  void clear() {hm.clear();}  


  /** Store hashmap
   * Serializes the hashmap container into string format which can be saved to disk
   * @return String of serialized hash
  */
  std::string store();


  /** Load hashmap
   * Load the hashmap container from a string
   * @param s String of serialized hash to be loaded
  */
  void load(const std::string& s);

  /** Size of hashmap
   * Returns number of elements currently in hashmap container
   * @return Number of elements
  */  
  int size() {return hm.size();}  ///< Number of pairs in hashmap

  /** Debug output of hashmap
   * Outputs the a human readable table version of the hashmap
   * @param os Outstream used for debug
  */
  void debug(std::ostream& os); ///< Debug prints hashmap content

  const size_t getSize()          const { return hm.size(); }

};

#endif
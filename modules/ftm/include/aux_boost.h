
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/container/vector.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/optional.hpp>
#include "ftm_common.h"
#include "dotstr.h"



#if BOOST_VERSION >= 106200 //endian conversian was included in boost 1.62
  #include <boost/endian/conversion.hpp>
  using namespace boost::endian;
#else
   // avoiding boost 1.62+, stolen from boost/endian/conversion.hpp to  //////////
  inline uint16_t endian_reverse(uint16_t x)
  {
    return (x << 8)
      | (x >> 8);
  }

  inline uint32_t endian_reverse(uint32_t x)
  {
    uint32_t step16;
    step16 = x << 16 | x >> 16;
    return
        ((step16 << 8) & 0xff00ff00)
      | ((step16 >> 8) & 0x00ff00ff);
  }

  inline uint64_t endian_reverse(uint64_t x)
  {
    uint64_t step32, step16;
    step32 = x << 32 | x >> 32;
    step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16
           | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
    return   (step16 & 0x00FF00FF00FF00FFULL) << 8
           | (step16 & 0xFF00FF00FF00FF00ULL) >> 8;
  }

 inline int16_t endian_reverse(int16_t x) BOOST_NOEXCEPT
  {
    return (static_cast<uint16_t>(x) << 8)
      | (static_cast<uint16_t>(x) >> 8);
  }

  inline int32_t endian_reverse(int32_t x) BOOST_NOEXCEPT
  {
    uint32_t step16;
    step16 = static_cast<uint32_t>(x) << 16 | static_cast<uint32_t>(x) >> 16;
    return
        ((static_cast<uint32_t>(step16) << 8) & 0xff00ff00)
      | ((static_cast<uint32_t>(step16) >> 8) & 0x00ff00ff);
  }

  inline int64_t endian_reverse(int64_t x) BOOST_NOEXCEPT
  {
    uint64_t step32, step16;
    step32 = static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(x) >> 32;
    step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16
           | (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
    return static_cast<int64_t>((step16 & 0x00FF00FF00FF00FFULL) << 8
           | (step16 & 0xFF00FF00FF00FF00ULL) >> 8);
  }

#endif




/** @name Types for use of boost graph library
 * Additional typedefs used for the carpeDM version of boost graphs, tailored to represent accelerator schedules
 */
//@{
class Node;
class MiniCommand;

typedef boost::shared_ptr<Node> node_ptr;
typedef boost::shared_ptr<MiniCommand> mc_ptr;
typedef std::vector<node_ptr> npBuf;




/** @name Auxiliary templated type converters
 * Templated helper functions for easy conversion of types and endianess
 */
//@{ 
template<typename T>
inline void writeLeNumberToBeBytes(uint8_t* pB, T val) {
  T x = endian_reverse(val);
  std::copy(static_cast<const uint8_t*>(static_cast<const void*>(&x)),
            static_cast<const uint8_t*>(static_cast<const void*>(&x)) + sizeof x,
            pB);
}

template<typename T>
inline void writeLeNumberToBeBytes(vBuf& vB, T val) {
  uint8_t b[sizeof(T)];

  T x = endian_reverse(val);
  std::copy(static_cast<const uint8_t*>(static_cast<const void*>(&x)),
            static_cast<const uint8_t*>(static_cast<const void*>(&x)) + sizeof x,
            b);

  vB.insert( vB.end(), b, b + sizeof(T) );
}

template<typename T>
inline T writeBeBytesToLeNumber(uint8_t* pB) {
  return endian_reverse(*((T*)pB));
}

template<typename T>
inline T writeBeBytesToLeNumber(vBuf& vB) {
  uint8_t* pB = (uint8_t*)&vB[0];
  return endian_reverse(*((T*)pB));
}

template<typename T>
inline void writeBeNumberToLeBytes(uint8_t* pB, T val) {
  T x = endian_reverse(val);
  std::copy(static_cast<const uint8_t*>(static_cast<const void*>(&x)),
            static_cast<const uint8_t*>(static_cast<const void*>(&x)) + sizeof x,
            pB);
}

template<typename T>
inline T writeLeBytesToBeNumber(uint8_t* pB) {
  return endian_reverse(*((T*)pB));
}
//@}

/// Inserts a fixed archive version into a serialised boost data container
/** Boost archive versions are forward compatible, and, by implementation, also backward compatible. 
  * However, using a newer lib version to create (fully data compatible) archives and opening them with older lib versions fails.
  * The workaround is to change the version tag inside the data container string to a known older version, so the old library accepts it.
  */
inline std::string fixArchiveVersion(const std::string& s) {
    //hack to ensure correct boost textarchive version
    //not nice, but I'm fed up to here with the crappy boost archive documentation
    const std::string tag = "serialization::archive ";
    const std::string myVer = "10"; // Boost Version 1.53 Archiver Version is 10
    std::string sRet = s;
    size_t pos = sRet.find(tag, 0) + tag.length();
    sRet.replace(pos, myVer.length(), myVer);

    return sRet;

}

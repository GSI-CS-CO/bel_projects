#ifndef _COMMON_H_
#define _COMMON_H_

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
#endif


#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


enum class AdrType {EXTERNAL, INTERNAL, MGMT, PEER};
enum class Direction {UPLOAD, DOWNLOAD};
enum class FwId { FWID_RAM_TOO_SMALL      = -1, 
                  FWID_BAD_MAGIC          = -2,
                  FWID_BAD_PROJECT_NAME   = -3,
                  FWID_NOT_FOUND          = -4,
                  FWID_BAD_VERSION_FORMAT = -5,
                  VERSION_MAJOR           = 0,
                  VERSION_MINOR           = 1,
                  VERSION_REVISION        = 2,
                  VERSION_MAJOR_MUL       = 10000,
                  VERSION_MINOR_MUL       = 100,
                  VERSION_REVISION_MUL    = 1};  


class Node;
class MiniCommand;

typedef boost::shared_ptr<Node> node_ptr;
typedef boost::shared_ptr<MiniCommand> mc_ptr;


typedef boost::container::vector<node_ptr> npBuf;
typedef boost::container::vector<uint8_t> vBuf;
typedef boost::container::vector<uint32_t> vAdr;
typedef boost::container::vector<uint32_t> ebBuf;
typedef boost::container::vector<std::string> vStrC;
typedef struct {
  vAdr va;
  vBuf vb;
} vEbwrs;

template <typename T>
boost::container::vector<T> operator+(const boost::container::vector<T> &A, const boost::container::vector<T> &B)
{
    boost::container::vector<T> AB;
    AB.reserve( A.size() + B.size() );                // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );        // add A;
    AB.insert( AB.end(), B.begin(), B.end() );        // add B;
    return AB;
}

template <typename T>
boost::container::vector<T> &operator+=(boost::container::vector<T> &A, const boost::container::vector<T> &B)
{
    A.reserve( A.size() + B.size() );                // preallocate memory without erase original data
    A.insert( A.end(), B.begin(), B.end() );         // add B;
    return A;                                        // here A could be named AB
}




template<typename T>
inline void writeLeNumberToBeBytes(uint8_t* pB, T val) {
  T x = endian_reverse(val);
  std::copy(static_cast<const uint8_t*>(static_cast<const void*>(&x)),
            static_cast<const uint8_t*>(static_cast<const void*>(&x)) + sizeof x,
            pB);
}

template<typename T>
inline T writeBeBytesToLeNumber(uint8_t* pB) {
  return endian_reverse(*((T*)pB));
}


template<typename T>
inline T s2u(const std::string& s) {
  using namespace DotStr::Misc;
  //check for boolean strings
  if(s == sTrue)  {return (T)1;}
  if(s == sFalse) {return (T)0;}
  T ret;
  try { ret = (T)std::stoull(s, 0, 0);} catch (...) { throw std::runtime_error("Cannot convert string '" + s + "' to number\n"); }
  return ret;

}

//FIXME just fuckin overload it, why are there two hexdump functions here ?
inline void hexDump (const char *desc, const void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addr;

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  printf ("\n");  
}

inline void vHexDump (const char *desc, vBuf pc) {
    int i, len = pc.size();
    unsigned char buff[17];

    // Output description if given.
    if (desc != NULL)
       printf ("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
               printf ("  %s\n", buff);

            // Output the offset.
           printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
       printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
  printf ("\n");  
}


#endif

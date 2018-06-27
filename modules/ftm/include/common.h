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


enum class AdrType {EXT, INT, MGMT, PEER, UNKNOWN};
enum class TransferDir {UPLOAD, DOWNLOAD};
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


namespace ECA {
  //TODO import values from eca_regs.h
  const uint32_t timeHiW = 0x18;
  const uint32_t timeLoW = 0x1c;
  const uint64_t vendID  = 0x00000651;
  const uint32_t devID   = 0xb2afc251;
}

const uint64_t processingTimeMargin = 100000000ULL; // 100 ms. Is set to 0 when testmode is on to speed coverage test

typedef struct {
  uint8_t   cpu; 
  uint64_t  msgCnt;
  uint64_t  bootTime;
  uint64_t  smodTime;
  char      smodIssuer[9];
  char      smodHost[9];
  std::string smodOpType;
  uint32_t  smodCnt;
  uint64_t  cmodTime;
  char      cmodIssuer[9];
  char      cmodHost[9];
  std::string cmodOpType;
  uint32_t  cmodCnt;
  int64_t   minTimeDiff;
  int64_t   maxTimeDiff;
  int64_t   avgTimeDiff;
  int64_t   warningThreshold;
  uint32_t  warningCnt;
  std::string warningNode;
  uint64_t  warningTime;
  uint32_t  maxBacklog;
  uint32_t  stat;
} HealthReport;


typedef struct {
  uint32_t     extAdr = LM32_NULL_PTR;
  bool       orphaned = false;
  bool        pending = false;
  uint64_t  validTime = 0;
  bool       validAbs = false;
  uint8_t        type = 0;
  std::string   sType = DotStr::Misc::sUndefined;
  uint32_t        qty = 0;
  //Flow properties
  std::string        flowDst = DotStr::Misc::sUndefined;
  std::string flowDstPattern = DotStr::Misc::sUndefined;
  bool flowPerma = false;
  //Flush Properties
  bool flushIl = false;
  bool flushHi = false;
  bool flushLo = false;
  //wait Properties
  uint64_t waitTime = 0;
  bool      waitAbs = false;
} QueueElement;


typedef struct {
  uint8_t wrIdx, rdIdx, pendingCnt;
  QueueElement aQe[4];
} QueueBuffer;

typedef struct {
  std::string name;
  bool hasQ[3] = {false, false, false};
  QueueBuffer aQ[3];
} QueueReport;


class Node;
class MiniCommand;

typedef boost::shared_ptr<Node> node_ptr;
typedef boost::shared_ptr<MiniCommand> mc_ptr;


typedef std::vector<node_ptr> npBuf;
typedef std::vector<uint8_t> vBuf;
typedef std::vector<uint32_t> vAdr;
typedef std::vector<uint32_t> ebBuf;
typedef std::vector<std::string> vStrC;
typedef std::vector<bool> vBl;

typedef struct {
  vAdr va;
  vBuf vb;
  vBl  vcs;
} vEbwrs;

typedef struct {
  vAdr va;
  vBl  vcs;
} vEbrds;




vBl leadingOne(size_t length);

template <typename T>
std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B)
{
    std::vector<T> AB;
    AB.reserve( A.size() + B.size() );                // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );        // add A;
    AB.insert( AB.end(), B.begin(), B.end() );        // add B;
    return AB;
}

template <typename T>
std::vector<T> &operator+=(std::vector<T> &A, const std::vector<T> &B)
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

void hexDump (const char *desc, const char* addr, int len);

void hexDump (const char *desc, vBuf vb);


#endif

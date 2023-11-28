#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <vector>
#include <map>
#include <stdexcept>
#include "ftm_common.h"
#include "dotstr.h"

/** @name Enums for communication with DM
 * Provides enums for the handling of firmware meta information, WB bus adress type conversion and upload/download buffers
 */
//@{
enum class AdrType {EXT, INT, MGMT, PEER, UNKNOWN}; ///< WB Address types for perspectives of different WB bus masters
enum class TransferDir {UPLOAD, DOWNLOAD}; ///< DM communication transfer direction. Upload (DM to host) or download (host to DM)


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
                  VERSION_REVISION_MUL    = 1}; ///< Firmware ID tags for status and version info
//@}


/** @name Definitions for accessing Cluster Time Hardware Module
 * Provides necessary SDB and register definitions for Cluster Time Hardware Module
 */
//@{
namespace CluTime {
  const uint32_t timeHiW = 0x0; ///< Time high word. Reading high word freezes low word until it's read
  const uint32_t timeLoW = 0x4; ///< Time low word
  const uint64_t vendID  = 0x00000651; ///< SDB Vendor ID
  const uint32_t devID   = 0x10041233; ///< SDB Device ID
}
//@}



const uint64_t processingTimeMargin = 500000000ULL; // 500 ms. Is set to 0 when testmode is on to speed coverage test


/** @name Operator overload collcetion for std::vector
 * Templated helper functions for easy concatenation std::vector
 */
//@{

/// Template to Overload + operator for std::vectors
/** This allows concatenation of two std::vectors A and B of type T into one new vector AB
  * @param A constant vector of type T
  * @param B constant vector of type T
  * @return new joint vector of type T
  */
template <typename T>
inline std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B)
{
    std::vector<T> AB;
    AB.reserve( A.size() + B.size() );                // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );        // add A;
    AB.insert( AB.end(), B.begin(), B.end() );        // add B;
    return AB;
}

/// Template to Overload + operator for std::vector and a constant
/** This allows appending a constant B to std::vectors A of type T into one new vector AB
  * @param A constant vector of type T
  * @param B literal of type T
  * @return new joint vector of type T
  */
template <typename T>
inline std::vector<T> operator+(const std::vector<T> &A, const T &B)
{
    std::vector<T> AB = A;
    AB.reserve( A.size() + 1 );                // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );
    AB.push_back(B);
    return AB;
}

/// Template to Overload += operator for std::vectors
/** This allows appending a vector B of type T to vector A of type T
  * @param A vector of type T
  * @param B constant vector of type T
  * @return modified vector A of type T
  */
template <typename T>
inline std::vector<T> &operator+=(std::vector<T> &A, const std::vector<T> &B)
{
    A.reserve( A.size() + B.size() );                // preallocate memory without erase original data
    A.insert( A.end(), B.begin(), B.end() );         // add B;
    return A;                                        // here A could be named AB
}

/// Template to Overload += operator for std::vector and a constant
/** This allows appending a constant B of type T to std::vector A of type T
  * @param A vector of type T
  * @param B literal constant of type T
  * @return modified vector A of type T
  */
template <typename T>
inline std::vector<T> &operator+=(std::vector<T> &A, const T &B)
{
    A.push_back(B);
    return A;
}
//@}


/** @name Debug output functions
 * Binary and Hexdump utilitites
 */
//@{

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c" ///< Format string to place 1/0 for binary ascii representation of a byte
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') ///< Convert byte to ascii sequence of 1/0

/** @name Diagnostic data structures
 * Report structures for diagnostics and queue report
 */
//@{

/// Global report on DM health
/** Contains all the usual suspects of diagnostic values from DM gateware, such as global msg count, the CPU boot timestamp,
 *  last schedule/command updates with user and timestamp, remainingd lead delta T on msg dispatch (min, max, avg),
 *  warning count for late dispatch and maximum observer message backlog count
 *
 */
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
  uint32_t  badWaitCnt;
  uint32_t  stat;
} HealthReport;

/// Report on DM's WB bus stall behaviour
/** Report contains DM's diagnostic values for the maximum wait time (stall) CPUs have experienced when trying to access the WB system bus.
 *
 */
typedef struct {
  uint32_t  stallStreakMax;
  uint32_t  stallStreakCurrent;
  uint64_t  stallStreakMaxUDts;
} StallDelayReport;

/// Report on DM's WR time behaviour
/** Report contains DM's diagnostic values for the linearity of WR timestamps vs system clock and when the last jump of WR time was detected
 *  Also contains the StallDelayReport as a subfield
 *
 */
typedef struct {
  bool      enabled;
  uint64_t  timeObservIntvl;
  int64_t   timeMaxPosDif;
  uint64_t  timeMaxPosUDts;
  int64_t   timeMinNegDif;
  uint64_t  timeMinNegUDts;
  uint32_t  stallObservIntvl;
  std::vector<StallDelayReport> sdr;
} HwDelayReport;

/// Report on a queued command
/** Report contains the command content (type, timestamp, parameters, etc) in a format easily convertible to human readable
 *
 */
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
  std::string flushOvr = DotStr::Node::Special::sIdle;
  //wait Properties
  uint64_t waitTime = 0;
  bool      waitAbs = false;
} QueueElement;

/// Report on a command queue
/** Report contains the queue state and meta infos as well as its content. Uses QueueElement structs to map the command content.
 *
 *
 */
typedef struct {
  uint8_t wrIdx, rdIdx, pendingCnt;
  QueueElement aQe[4];
} QueueBuffer;

/// Report on a block node's command queues
/** Report contains the content and meta information of all of a block node's command queues. This lists the content of each queue
 * (pending & processed commands) as well as the meta information. uses QueueBuffer struct to map the queue state and meta infos
 * and QueueElement structs to map the content of each queue
 *
 *
 */
typedef struct {
  std::string name;
  bool hasQ[3] = {false, false, false};
  QueueBuffer aQ[3];
} QueueReport;
//@}



typedef std::map<unsigned, uint32_t> mVal; ///< exchange map for override values/ptrs

//non destructive map merge, preference in case of existing key is A (first operand)
inline mVal operator+(const mVal &A, const mVal &B)
{
    mVal AB;
    AB.insert(A.begin(), A.end());
    AB.insert(B.begin(), B.end());
    return AB;
}



/** @name Etherbone external cycle staging
 * Etherbone lib does not allow cancellation of prepared cycles and icompleted cycles cannot be stored for later sending.
 * So these structs are a workaround to allow a more flexible staging of transmissions from carpeDM to DM
 */
//@{
typedef std::vector<uint8_t> vBuf; ///< buffer for WB payload data
typedef std::vector<uint32_t> vAdr; ///< buffer for WB addresses
typedef std::vector<uint32_t> ebBuf; ///< total number of cpus on the DM
typedef std::vector<bool> vBl; ///< buffer for cycle line control. 1 - close the ongoing cycle and start a new one, 0 - continue with current cycle
typedef std::vector<std::string> vStrC; ///< vector of strings
/// Struct for staging EB write operations
/** Staging EB write ops requires a buffer of WB addresses, a buffer of WB payload data and a buffer of cycle line control bits
 *
 */
typedef struct {
  vAdr va;
  vBuf vb;
  vBl  vcs;
} vEbwrs;

/// Struct for staging EB read operations
/** Staging EB write ops requires a buffer of WB addresses, and a buffer of cycle line control bits
 *
 */
typedef struct {
  vAdr va;
  vBl  vcs;
} vEbrds;

/// Template to Overload + operator for structs of EB Write Operations
/** This allows concatenation of two structs A and B of type EB Write Operations into one new struct AB
  * @param A constant EB Write Struct
  * @param B constant EB Write Struct
  * @return new EB Write Struct A
  */
inline vEbwrs operator+(const vEbwrs &A, const vEbwrs &B)
{
    vEbwrs AB;
    AB.va = A.va + B.va;
    AB.vb = A.vb + B.vb;
    AB.vcs = A.vcs + B.vcs;
    return AB;
}

/// Template to Overload + operator for structs of EB Write Operations
/** This allows appending a struct B of type EB Write Operations to struct A of type EB Write Operations
  * @param A constant EB Write Struct
  * @param B constant EB Write Struct
  * @return modified EB Write Struct A
  */
inline vEbwrs& operator+=(vEbwrs &A, const vEbwrs &B)
{

    A.va = A.va + B.va;
    A.vb = A.vb + B.vb;
    A.vcs = A.vcs + B.vcs;
    return A;
}

/// Template to Overload + operator for struct of EB Read Operations
/** This allows concatenation of two structs A and B of type EB Read Operations into one new struct AB
  * @param A constant EB Read Struct
  * @param B constant EB Read Struct
  * @return new EB Read Struct
  */
inline vEbrds operator+(const vEbrds& A, const vEbrds &B)
{
    vEbrds AB;
    AB.va = A.va + B.va;
    AB.vcs = A.vcs + B.vcs;
    return AB;
}

/// Template to Overload + operator for structs of EB Read Operations
/** This allows appending a struct B of type EB Read Operations to struct A of type EB Read Operations
  * @param A constant EB Read Struct
  * @param B constant EB Read Struct
  * @return modified EB Read Struct A
  */
inline vEbrds& operator+=(vEbrds& A, const vEbrds &B)
{

    A.va = A.va + B.va;
    A.vcs = A.vcs + B.vcs;
    return A;
}


/// Helper function to create a cycle line control sqequence
/** The function accepts a cycle length and outputs a matching bit sequence of a leading 1 (start new cycle) followed by 0s (continue current cycle)
 *
 */
vBl leadingOne(size_t length);
//@}



/// Helper function to convert a string to a number or bool
/** Convert a string to a number or bool. Helper to convert values found in dot files
  */
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
//@}






/// Hexdump to std::out
/** Creates a formatted hexadecimal version of a char array and puts it on std::cout
 * @param desc Char array containig dump title
 * @param addr Char array containig bytes to be hex dumped
 * @param len Number of bytes to be hex dumped
 */
std::string hexDump (const char *desc, const char* addr, int len);

/// Hexdump to std::out
/** Creates a formatted hexadecimal version of a vector of bytes and puts it on std::cout
 * @param desc Char array containig dump title
 * @param vb Vector of bytes to be hex dumped
 */
void hexDump (const char *desc, vBuf vb);
//@}

/// Helper function to convert a nanosecond timestamp into a human readable string
/** Convert a 64 bit nano second timestamp as can be obtained from getWrTime into a human readable string. If noSpaces is set, underscores will be used (useful for filename generation)
  */
std::string nsTimeToDate(uint64_t t, bool noSpaces=false);

bool hasEnding (std::string const &fullString, std::string const &ending);

#endif

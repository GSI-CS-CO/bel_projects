%module dm
%include stdint.i
%include std_string.i
%include typemaps.i
%include cpointer.i
%include exception.i
%include std_vector.i

//%template(vEbwrs) vector< StringPair >;



// Instantiate templates used by example
namespace std {
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
   %template(StringVector) vector<string>;
   %template(ConstCharVector) vector<const char*>;
}

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




%exception {
    try {
        $action
        //throw std::runtime_error("Hallo");
    } catch(const std::exception& e) {
    	std::string msg = e.what();
    	msg = "Standard Exception: " + msg;
        SWIG_exception(SWIG_UnknownError, msg.c_str());
	} catch(...) {
        SWIG_exception(SWIG_RuntimeError, "Unknown exception");
    }
}

// Make SWIG look into this header:
%include "../include/carpeDM.h"

// Make dm_wrap.cxx include this header:
%{
#include "../include/carpeDM.h"
%}

#include "dotstr.h"

namespace DotStr {

  namespace Misc { //mostly stuff to mark uninitialised props and some generic tags for nodes and edges alike

    const unsigned char deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    const std::string needle(deadbeef, deadbeef + 4);

    const std::string sHexZero      = "0x0";
    const std::string sZero         = "0";
    const std::string sOne          = "1";
    const std::string sUndefined64  = "0xD15EA5EDDEADBEEF";
    const std::string sUndefined32  = "0xDEADBEEF";
    const uint32_t    uUndefined32  = 0xDEADBEEF; //yeah yeah, it's not a string. I know
    const uint64_t    uUndefined64  = 0xD15EA5EDDEADBEEFULL; //yeah yeah, it's not a string. I know
    const std::string sUndefined    = "undefined";
    const std::string sHashType     = "HASH_";

    // tag constants for both nodes and edges

    // command priorities
    const std::string sPrioHi       = "prioil"; //FIXME string is still fitting for 'Interlock' as highest priority
    const std::string sPrioMd       = "priohi"; //FIXME string is still fitting for 'Interlock' as highest priority
    const std::string sPrioLo       = "priolo";
    //bools
    const std::string sTrue         = "true";
    const std::string sFalse        = "false";


  }

 namespace Edge {
    // edge properties
    namespace Prop {
      namespace Base {
        const std::string sType = "type"; // type specifier for edges (see namespace TypeVal )
      }
    }

    namespace TypeVal {
      // edge type tags
      const std::string sQPrio[]      = {Misc::sPrioLo, Misc::sPrioMd, Misc::sPrioHi}; // array of priorities for ease of use
      const std::string sDstList      = "listdst";    // Links to Destination list (carpeDM internal)
      const std::string sDefDst       = "defdst";     // Links to Default Destination
      const std::string sAltDst       = "altdst";     // Links to Alternative Destination
      const std::string sBadDefDst    = "baddefdst";  // Links to Bad Default Destination
      const std::string sCmdTarget    = "target";     // Links to Command's Target Block
      const std::string sCmdFlowDst   = "flowdst";    // Links to Flow Command's destination node
      const std::string sDynId        = "dynid";      // Links to Source for dynamic ID field in Tmsg nodes
      const std::string sDynPar0      = "dynpar0";    // Links to Source for dynamic par (high word) field in Tmsg nodes
      const std::string sDynPar1      = "dynpar1";    // Links to Source for dynamic par (low  word) field in Tmsg nodes
      const std::string sDynTef       = "dyntef";     // Links to Source for dynamic TEF field in Tmsg nodes
      const std::string sDynRes       = "dynres";     // Links to Source for dynamic reserved field in Tmsg nodes
      const std::string sMeta         = "meta";       // Links to Source for dynamic reserved field in Tmsg nodes
      const std::string sAny          = "";           // Wildcard type (carpeDM internal)
      const std::string sDynFlowDst   = "dynflowdst"; // Auxiliary dynamic type for safe removal check  (carpeDM internal)
      const std::string sResFlowDst   = "resflowdst"; // Auxiliary resident type for safe removal check  (carpeDM internal)
      const std::string sDomFlowDst   = "domflowdst"; // Auxiliary dominant type for safe removal check  (carpeDM internal)
    }
  }

  namespace Node {
    namespace Special {
      const std::string sIdle         = "idle"; // the 'idle' target means safely stopping a thread/pattern
    }
    // node properties
    namespace Prop {
      namespace Base {
        const std::string sType          = "type";      // type specifier for nodes (see namespace TypeVal )
        const std::string sName          = "node_id";   // - (carpeDM internal)
        const std::string sCpu           = "cpu";       // specifies CPU (and RAM) this node is located. For cmd dots, designates target cpu
        const std::string sThread        = "thread";    // For cmd dots, designates target thread
        const std::string sFlags         = "flags";     // debug field (carpeDM internal)
        const std::string sPatEntry      = "patentry";  // pattern entry point
        const std::string sPatExit       = "patexit";   // pattern exit point
        const std::string sPatName       = "pattern";   // pattern name
        const std::string sBpEntry       = "bpentry";   // beam process entry point
        const std::string sBpExit        = "bpexit";    // beam process exit point
        const std::string sBpName        = "beamproc";  // beam process name
      }

      namespace Block {
        // Block Parameters
        const std::string sTimePeriod    = "tperiod"; // a block's period (duration) which is added to thread's cumulative time sum in ns
        const std::string sGenQPrioHi    = "qil";     // order to generate (if none exists) and attach a high priority queue to this block
        const std::string sGenQPrioMd    = "qhi";     // order to generate (if none exists) and attach a medium priority queue to this block
        const std::string sGenQPrioLo    = "qlo";     // order to generate (if none exists) and attach a low priority queue to this block
        const std::string sGenQPrio[]      = {sGenQPrioLo, sGenQPrioMd, sGenQPrioHi}; // array of priority generation orders for ease of use
      }

      namespace TMsg {
        // Timing Message Parameters
        const std::string sTimeOffs      = "toffs"; // a timing messages' offset to thread's cumulative time sum in ns
        const std::string sId            = "id"; // timing event ID in dec/hex
        // subfields of timing event ID, use is format ID dependent
        namespace SubId {
          const std::string sFid           = "fid";       // Format ID
          const std::string sGid           = "gid";       // Group ID
          const std::string sEno           = "evtno";     // Event Number
          const std::string sSid           = "sid";       // Sequence ID
          const std::string sBpid          = "bpid";      // Beam process ID
          const std::string sBin           = "beamin";    // Beam In
          const std::string sReqNoB        = "reqnobeam"; // Request no beam
          const std::string sVacc          = "vacc";      // Virtual accelerator
          const std::string sRes           = "res";       // reserved
        }
        const std::string sPar           = "par"; // transparent Parameter field
        const std::string sTef           = "tef"; // time fraction (TEF) field
      }
      // Command Parameters
      namespace Cmd {

        const std::string sTimeValid   = "tvalid";        // time after which command becomes valid in ns
        const std::string sVabs        = "vabs";          // valid time is absolute (True) or relative (false)
        const std::string sPrio        = "prio";          // priority of this command
        const std::string sQty         = "qty";           // quantity (repetitions) of this command
        const std::string sTimeWait    = "twait";         // wait time (wait commands only) in ns
        const std::string sTarget      = "target";        // a command's target block. only used in cmd dots, schedules use edges instead
        const std::string sDst         = "dest";          // a flow command's destination node. only used in cmd dots, schedules use edges instead
        const std::string sDstPattern  = "destpattern";   // a flow command's destination pattern. only used in cmd dots, schedules use edges instead
        const std::string sDstBeamproc = "destbeamproc";  // a flow command's destination beam proccess. only used in cmd dots, schedules use edges instead
        const std::string sPermanent   = "permanent";     // specifies if changes by this command are permanent
      }
    }


    namespace MetaGen {
      //name prefixes, tags and suffixes for automatic meta node generation (carpeDM internal)
      const std::string sDstListSuffix = "_ListDst";
      const std::string sQPrioPrefix[] = {"Lo", "Hi", "Il"};
      const std::string sQBufListTag   = "_QBl_";
      const std::string sQBufTag       = "_Qb_";
      const std::string s1stQBufSuffix = "0";
      const std::string s2ndQBufSuffix = "1";
    }


    // node type tags
    namespace TypeVal {
      const std::string sQPrio[]       = {Misc::sPrioLo, Misc::sPrioMd, Misc::sPrioHi}; // array of priorities for ease of use
      const std::string sTMsg          = "tmsg";        // timing message
      const std::string sCmdNoop       = "noop";        // no operation command (dummy/padding)
      const std::string sCmdFlow       = "flow";        // flow command (changes path through schedule)
      const std::string sCmdFlush      = "flush";       // flush command (clears a command queue)
      const std::string sCmdWait       = "wait";        // wait command (relative prolongs block duration, abs waits til given time is reached )
      const std::string sCmdStart      = "start";       // For cmd dots, starts a thread (by cpu/thread, patternname or node name)
      const std::string sCmdStop       = "stop";        // For cmd dots, stops a thread (by cpu/thread, patternname or node name)
      const std::string sCmdAbort      = "abort";       // For cmd dots, aborts a thread (by cpu/thread, patternname or node name)
      const std::string sCmdOrigin     = "origin";      // For cmd dots, sets origin to node name
      const std::string sBlock         = "block";       // block
      const std::string sBlockFixed    = sBlock;        // same as 'block'
      const std::string sBlockAlign    = "blockalign";  // auto aligning block, prolongs duration to match time grid (currently hardcoded in FW to 10µs starting at TAI 0)
      const std::string sQInfo         = "qinfo";       // queue buffer list (carpeDM internal)
      const std::string sDstList       = "listdst";     // destination list (carpeDM internal)
      const std::string sQBuf          = "qbuf";        // queue buffer (carpeDM internal)
      const std::string sMeta          = "meta";        // generic meta node (carpeDM internal)
      const bool bMetaNode             = true;          // as comparison against isMeta() Node class member function.
      const bool bRealNode             = false;         //yeah yeah, it's not a string. I know

    }
  }

  namespace Graph {
    namespace Special {
      const std::string sCmd    = "!CMD!"; // magic word to show this dot was abused to contain commands
    }
    namespace Prop {
      const std::string sName   = "name";
      const std::string sRoot   = "root";
    }
    const std::string sDefName  = "Demo";
  }

  //Configures how a dot will be rendered (carpeDM internal)
  namespace EyeCandy {

    namespace Graph {
      // global layout options for the dot renderer
      const std::string sLookVert      = "rankdir   = TB, nodesep           = 0.6, mindist     = 1.0, ranksep = 1.0, overlap = false";
      const std::string sLookHor       = "rankdir   = LR, nodesep           = 0.6, mindist     = 1.0, ranksep = 1.0, overlap = false";
    }

    namespace Node {
      namespace Base {
        const std::string sLookDef       = "style     = \"filled\", fillcolor = \"white\", color = \"black\"";
        const std::string sLookPaintNone = "fillcolor = \"white\"";
        const std::string sLookPaint0    = "fillcolor = \"green\"";
        const std::string sLookPaint1    = "fillcolor = \"rosybrown1\"";
        const std::string sLookDebug0    = "fillcolor = \"crimson\", fontname=\"Times-Bold\", fontcolor = \"cyan\"";
        const std::string sLookDebug1    = "fontname=\"Times-Bold\", fontcolor = \"blue2\", fontsize=\"16\"";
        const std::string sLookDebug2    = "fontname=\"Times-Bold\", fillcolor = \"cyan\"";

        const std::string sLookPatEntry  = "penwidth=2, color = \"darkorange3\"";
        const std::string sLookPatExit   = "penwidth=2, color = \"purple\"";
      }

      namespace Block {
        const std::string sLookDef       = "shape     = \"rectangle\"";
        const std::string sLookFix       = sLookDef;
        const std::string sLookAlign     = sLookDef;


      }
      namespace TMsg {
        const std::string sLookDef       = "shape     = \"oval\"";

      }
      namespace Cmd {
        const std::string sLookDef       = "shape     = \"hexagon\"";


 }
      namespace Meta {
        const std::string sLookDef       = "shape     = \"rectangle\", color  = \"gray\", style  = \"dashed\"";
      }
    }

    namespace Edge {

      const std::string sLookDefDst    = "color     = \"red\"";
      const std::string sLookAltDst    = "color     = \"black\"";
      const std::string sLookMeta      = "color     = \"gray\"";
      const std::string sLookTarget    = "color     = \"blue\"";
      const std::string sLookArgument  = "color     = \"pink\"";
      const std::string sLookDebug0    = "color     = \"maroon3\"";
      const std::string sLookDebug1    = "color     = \"maroon4\"";
      const std::string sLookDebug2    = "color     = \"cyan\"";
      const std::string sLookbad       = "color     = \"orange\", style     = \"dashed\"";

    }
  }


}

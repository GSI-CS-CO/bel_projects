#include "dotstr.h"

namespace DotStr {

  namespace Misc {
    //pattern for uninitialised properties and their detection
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

    // tag constants for both nodes and edges
    const std::string sPrioHi       = "prioil"; //FIXME string is still fitting for 'Interlock' as highest priority
    const std::string sPrioMd       = "priohi"; //FIXME string is still fitting for 'Interlock' as highest priority
    const std::string sPrioLo       = "priolo";
    const std::string sTrue         = "true";
    const std::string sFalse        = "false";


  }

 namespace Edge {
    // edge properties
    namespace Prop {
      namespace Base {
        const std::string sType = "type";
      }
    }  

    namespace TypeVal {
      // edge type tags
      const std::string sQPrio[]      = {Misc::sPrioLo, Misc::sPrioMd, Misc::sPrioHi};
      const std::string sDstList      = "listdst";
      const std::string sDefDst       = "defdst";
      const std::string sAltDst       = "altdst";
      const std::string sBadDefDst    = "baddefdst";
      const std::string sCmdTarget    = "target";
      const std::string sCmdFlowDst   = "flowdst";
      const std::string sDynId        = "dynid";
      const std::string sDynPar0      = "dynpar0";
      const std::string sDynPar1      = "dynpar1";
      const std::string sDynTef       = "dyntef";
      const std::string sDynRes       = "dynres";
      const std::string sMeta         = "meta";
      const std::string sAny          = "";
      const std::string sDynFlowDst   = "dynflowdst";
    }
  }

  namespace Node {
    namespace Special {
      const std::string sIdle         = "idle";
    }
    // node properties
    namespace Prop {
      namespace Base {
        const std::string sType          = "type";
        const std::string sName          = "node_id";
        const std::string sCpu           = "cpu";
        const std::string sThread        = "thread";
        const std::string sFlags         = "flags";
        const std::string sPatEntry      = "patentry";
        const std::string sPatExit       = "patexit";
        const std::string sPatName       = "pattern";
        const std::string sBpEntry       = "bpentry";
        const std::string sBpExit        = "bpexit";
        const std::string sBpName        = "beamproc";
      }

      namespace Block {
 
        const std::string sTimePeriod    = "tperiod";
        const std::string sGenQPrioHi    = "qil";
        const std::string sGenQPrioMd    = "qhi";
        const std::string sGenQPrioLo    = "qlo";
      }  

      namespace TMsg {

        const std::string sTimeOffs      = "toffs";
        const std::string sId            = "id";
        namespace SubId {
          const std::string sFid           = "fid";
          const std::string sGid           = "gid";
          const std::string sEno           = "evtno";
          const std::string sSid           = "sid";
          const std::string sBpid          = "bpid";
          const std::string sBin           = "beamin";
          const std::string sReqNoB        = "reqnobeam";
          const std::string sVacc          = "vacc";
          const std::string sRes           = "res";
        } 
        const std::string sPar           = "par";
        const std::string sTef           = "tef";
      }

      namespace Cmd {
     
        const std::string sTimeValid   = "tvalid";
        const std::string sPrio        = "prio";
        const std::string sQty         = "qty";
        const std::string sTimeWait    = "twait";
        const std::string sTarget      = "target";
        const std::string sDst         = "dest";
        const std::string sDstPattern  = "destpattern";
        const std::string sDstBeamproc = "destbeamproc";
        const std::string sPermanent   = "permanent"; 
      }  
    }


    namespace MetaGen {
      //name prefixes, tags and suffixes for automatic meta node generation
      const std::string sDstListSuffix = "_ListDst";
      const std::string sQPrioPrefix[] = {"Lo", "Hi", "Il"};
      const std::string sQBufListTag   = "_QBl_";
      const std::string sQBufTag       = "_Qb_";
      const std::string s1stQBufSuffix = "0";
      const std::string s2ndQBufSuffix = "1";
    }


    // node type tags
    namespace TypeVal {
      const std::string sQPrio[]       = {Misc::sPrioLo, Misc::sPrioMd, Misc::sPrioHi};
      const std::string sTMsg          = "tmsg";
      const std::string sCmdNoop       = "noop";
      const std::string sCmdFlow       = "flow";
      const std::string sCmdFlush      = "flush";
      const std::string sCmdWait       = "wait";
      const std::string sCmdStart      = "start";
      const std::string sCmdStop       = "stop";
      const std::string sCmdAbort      = "abort";
      const std::string sCmdOrigin     = "origin";
      const std::string sBlock         = "block";
      const std::string sBlockFixed    = sBlock;
      const std::string sBlockAlign    = "blockalign";
      const std::string sQInfo         = "qinfo";
      const std::string sDstList       = "listdst";
      const std::string sQBuf          = "qbuf";
      const std::string sMeta          = "meta";
      const bool bMetaNode             = true;  // as comparison against isMeta() Node class member function
      const bool bRealNode             = false; //yeah yeah, it's not a string. I know

    }  
  }  

  namespace Graph {
    namespace Special {
      const std::string sCmd         = "!CMD!";
    }
    namespace Prop {
      const std::string sName   = "name"; 
      const std::string sRoot   = "root";
    }  
    const std::string sDefName  = "Demo";
  }
  
  //Configures how a dot will be rendered
  namespace EyeCandy {

    namespace Graph {
      const std::string sLookVert      = "rankdir   = TB, nodesep           = 0.6, mindist     = 1.0, ranksep = 1.0, overlap = false";
      const std::string sLookHor       = "rankdir   = LR, nodesep           = 0.6, mindist     = 1.0, ranksep = 1.0, overlap = false";
    }
    
    namespace Node {
      namespace Base {
        const std::string sLookDef       = "style     = \"filled\", fillcolor = \"white\", color = \"black\"";
        const std::string sLookPaintNone = "fillcolor = \"white\"";
        const std::string sLookPaint0    = "fillcolor = \"green\"";
        const std::string sLookPaint1    = "fillcolor = \"rosybrown1\"";
        const std::string sLookDebug0    = "fillcolor = \"crimson\", fontname=\"Times-Bold\", fontcolor = \"blue2\"";
        const std::string sLookDebug1    = "fontname=\"Times-Bold\", fontcolor = \"blue2\"";
        const std::string sLookDebug2    = "fillcolor = \"cyan\"";

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
      const std::string sLookbad       = "color     = \"orange\", style     = \"dashed\"";
      
    }   
  }


}

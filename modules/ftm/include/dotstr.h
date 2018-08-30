#ifndef _DOTSTR_H_
#define _DOTSTR_H_

#include <string>

namespace DotStr {

  namespace Misc {
    //pattern for uninitialised properties and their detection
    extern const unsigned char deadbeef[4];
    extern const std::string needle;

    extern const std::string sHexZero;
    extern const std::string sZero;
    extern const std::string sOne;
    extern const std::string sUndefined64;
    extern const std::string sUndefined32;
    extern const uint32_t    uUndefined32;
    extern const uint64_t    uUndefined64;
    extern const std::string sUndefined;
    extern const std::string sHashType;

    // tag constants for both nodes and edges
    extern const std::string sPrioHi;
    extern const std::string sPrioMd;
    extern const std::string sPrioLo;
    extern const std::string sTrue;
    extern const std::string sFalse;

  }

 namespace Edge {
    // edge properties
    namespace Prop {
      namespace Base {
        extern const std::string sType;
      }
    }

    namespace TypeVal {
      // edge type tags
      extern const std::string sQPrio[];
      extern const std::string sDstList;
      extern const std::string sDefDst;
      extern const std::string sAltDst;
      extern const std::string sBadDefDst;
      extern const std::string sCmdTarget;
      extern const std::string sCmdFlowDst;
      extern const std::string sDynId;
      extern const std::string sDynPar0;
      extern const std::string sDynPar1;
      extern const std::string sDynTef;
      extern const std::string sDynRes;
      extern const std::string sMeta;
      extern const std::string sAny;
      extern const std::string sDynFlowDst;
      extern const std::string sResFlowDst;
      extern const std::string sDomFlowDst;
    }
  }

  namespace Node {
    namespace Special {
      extern const std::string sIdle;
    }
    // node properties
    namespace Prop {
      namespace Base {
        extern const std::string sType;
        extern const std::string sName;
        extern const std::string sCpu;
        extern const std::string sThread;
        extern const std::string sFlags;
        extern const std::string sPatEntry;
        extern const std::string sPatExit;
        extern const std::string sPatName;
        extern const std::string sBpEntry;
        extern const std::string sBpExit;
        extern const std::string sBpName;
      }

      namespace Block {

        extern const std::string sTimePeriod;
        extern const std::string sGenQPrioHi;
        extern const std::string sGenQPrioMd;
        extern const std::string sGenQPrioLo;
        extern const std::string sGenQPrio[];
      }

      namespace TMsg {

        extern const std::string sTimeOffs;
        extern const std::string sId;
        namespace SubId {
          extern const std::string sFid;
          extern const std::string sGid;
          extern const std::string sEno;
          extern const std::string sSid;
          extern const std::string sBpid;
          extern const std::string sBin;
          extern const std::string sReqNoB;
          extern const std::string sVacc;
          extern const std::string sRes;
        }
        extern const std::string sPar;
        extern const std::string sTef;
      }

      namespace Cmd {

        extern const std::string sTimeValid;
        extern const std::string sVabs;
        extern const std::string sPrio;
        extern const std::string sQty;
        extern const std::string sTimeWait;
        extern const std::string sTarget;
        extern const std::string sDst;
        extern const std::string sDstPattern;
        extern const std::string sDstBeamproc;
        extern const std::string sPermanent;


      }
    }


    namespace MetaGen {
      //name prefixes, tags and suffixes for automatic meta node generation
      extern const std::string sDstListSuffix;
      extern const std::string sQPrioPrefix[];
      extern const std::string sQBufListTag;
      extern const std::string sQBufTag;
      extern const std::string s1stQBufSuffix;
      extern const std::string s2ndQBufSuffix;
    }


    // node type tags
    namespace TypeVal {
      extern const std::string sQPrio[];
      extern const std::string sTMsg;
      extern const std::string sCmdNoop;
      extern const std::string sCmdFlow;
      extern const std::string sCmdFlush;
      extern const std::string sCmdWait;
      extern const std::string sCmdStart;
      extern const std::string sCmdStop;
      extern const std::string sCmdAbort;
      extern const std::string sCmdOrigin;
      extern const std::string sBlock;
      extern const std::string sBlockFixed;
      extern const std::string sBlockAlign;
      extern const std::string sQInfo;
      extern const std::string sDstList;
      extern const std::string sQBuf;
      extern const std::string sMeta;
      extern const bool bMetaNode;
      extern const bool bRealNode;

    }
  }

  namespace Graph {
    namespace Special {
      extern const std::string sCmd;
    }

    namespace Prop {
      extern const std::string sName;
      extern const std::string sRoot;
    }
    extern const std::string sDefName;
  }

  //Configures how a dot will be rendered
  namespace EyeCandy {

    namespace Graph {
      extern const std::string sLookVert;
      extern const std::string sLookHor;
    }

    namespace Node {
      namespace Base {
        extern const std::string sLookDef;
        extern const std::string sLookPaintNone;
        extern const std::string sLookPaint0;
        extern const std::string sLookPaint1;
        extern const std::string sLookDebug0;
        extern const std::string sLookDebug1;
        extern const std::string sLookDebug2;
        extern const std::string sLookPatEntry;
        extern const std::string sLookPatExit;
      }
      namespace Block {
        extern const std::string sLookDef;
        extern const std::string sLookFix;
        extern const std::string sLookAlign;

      }
      namespace TMsg {
        extern const std::string sLookDef;
      }
      namespace Cmd {
        extern const std::string sLookDef;

      }
      namespace Meta {
        extern const std::string sLookDef;
      }
    }

    namespace Edge {

      extern const std::string sLookDefDst;
      extern const std::string sLookAltDst;
      extern const std::string sLookMeta;
      extern const std::string sLookTarget;
      extern const std::string sLookArgument;
      extern const std::string sLookDebug0;
      extern const std::string sLookDebug1;
      extern const std::string sLookDebug2;
      extern const std::string sLookbad;

    }
  }



}


#endif
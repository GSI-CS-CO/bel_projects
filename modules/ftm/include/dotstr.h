#ifndef _DOTSTR_H_
#define _DOTSTR_H_

#include <string>

namespace DotStr {
    //pattern for uninitialised properties and their detection
    extern const unsigned char deadbeef[4];
    extern const std::string needle;
    extern const std::string tHexZero;
    extern const std::string tZero;
    extern const std::string tUndefined64;
    extern const std::string tUndefined32;
    extern const uint32_t    uUndefined32;
    extern const std::string tUndefined;

    extern const std::string defGraphName;

    //name prefixes, tags and suffixes for automatic meta node generation
    extern const std::string tDstListSuffix;  
    extern const std::string tQPrioPrefix[3];  
    extern const std::string tQBufListTag;    
    extern const std::string tQBufTag;
    extern const std::string t1stQBufSuffix;
    extern const std::string t2ndQBufSuffix;        

    // tag constants for both nodes and edges
    extern const std::string tPrioHi;
    extern const std::string tPrioMd;
    extern const std::string tPrioLo;

    // edge type tags
    extern const std::string eQPrio[3];
    extern const std::string eDstList;
    extern const std::string eDefDst;
    extern const std::string eAltDst;
    extern const std::string eBadDefDst;
    extern const std::string eCmdTarget;
    extern const std::string eCmdFlowDst;
    extern const std::string eDynId;
    extern const std::string eDynPar0;
    extern const std::string eDynPar1;
    extern const std::string eDynTef;
    extern const std::string eDynRes;

    // node type tags
    extern const std::string nQPrio[3];
    extern const std::string nTMsg;
    extern const std::string nCmdNoop;
    extern const std::string nCmdFlow;
    extern const std::string nCmdFlush;
    extern const std::string nCmdWait;
    extern const std::string nBlock;
    extern const std::string nBlockFixed;
    extern const std::string nBlockAlign;
    extern const std::string nQInfo;
    extern const std::string nDstList;
    extern const std::string nQBuf;
    extern const std::string nMeta;
};



#endif


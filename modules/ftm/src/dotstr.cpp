#include "dotstr.h"

namespace DotStr {
  //pattern for uninitialised properties and their detection
  const unsigned char deadbeef[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  const std::string needle(deadbeef, deadbeef + 4);

  const std::string tHexZero     = "0x0";
  const std::string tZero        = "0";
  const std::string tUndefined64 = "0xD15EA5EDDEADBEEF";
  const std::string tUndefined32 = "0xDEADBEEF";
  const uint32_t    uUndefined32 = 0xDEADBEEF;

  const std::string tUndefined   = "UNDEFINED";

  const std::string defGraphName = "Demo";

  //name prefixes, tags and suffixes for automatic meta node generation
  const std::string tDstListSuffix  = "_ListDst";
  const std::string tQPrioPrefix[]  = {"Lo", "Hi", "Il"};
  const std::string tQBufListTag    = "_QBl_";
  const std::string tQBufTag        = "_Qb_";
  const std::string t1stQBufSuffix  = "0";
  const std::string t2ndQBufSuffix  = "1";


  // tag constants for both nodes and edges
  const std::string tPrioHi       = "prioil"; //FIXME string is still fitting for 'Interlock' as highest priority
  const std::string tPrioMd       = "priohi"; //FIXME string is still fitting for 'Interlock' as highest priority
  const std::string tPrioLo       = "priolo";


  // edge type tags
  const std::string eQPrio[]      = {tPrioLo, tPrioMd, tPrioHi};
  const std::string eDstList      = "listdst";
  const std::string eDefDst       = "defdst";
  const std::string eAltDst       = "altdst";
  const std::string eBadDefDst    = "baddefdst";
  const std::string eCmdTarget    = "target";
  const std::string eCmdFlowDst   = "flowdst";
  const std::string eDynId        = "dynid";
  const std::string eDynPar0      = "dynpar0";
  const std::string eDynPar1      = "dynpar1";
  const std::string eDynTef       = "dyntef";
  const std::string eDynRes       = "dynres";

  // node type tags
  const std::string nQPrio[]      = {tPrioLo, tPrioMd, tPrioHi};
  const std::string nTMsg         = "tmsg";
  const std::string nCmdNoop      = "noop";
  const std::string nCmdFlow      = "flow";
  const std::string nCmdFlush     = "flush";
  const std::string nCmdWait      = "wait";
  const std::string nBlock        = "block";
  const std::string nBlockFixed   = "blockfixed";
  const std::string nBlockAlign   = "blockalign";
  const std::string nQInfo        = "qinfo";
  const std::string nDstList      = "listdst";
  const std::string nQBuf         = "qbuf";
  const std::string nMeta         = "meta";

}

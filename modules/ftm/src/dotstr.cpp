#include "dotstr.h"

namespace DotStr {

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
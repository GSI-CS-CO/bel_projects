#include "idformat.h"
#include "dotstr.h"

  namespace ssi = DotStr::Node::Prop::TMsg::SubId;

std::vector<vPf> idFormats = {

  //Format ID 0
  {
    {ssi::sFid,   60, 4},
    {ssi::sGid,   48, 12},
    {ssi::sEno,   36, 12},
    {ssi::sSid,   24, 12},
    {ssi::sBpid,  10, 14}
  },
  //Format ID 1
  {
    {ssi::sFid,   60, 4},
    {ssi::sGid,   48, 12},
    {ssi::sEno,   36, 12},
    {ssi::sBin,   35, 1},
    {ssi::sSid,   20, 12},
    {ssi::sBpid,   6, 14},
    {ssi::sReqNoB, 4, 1},
    {ssi::sVacc,   0, 4}
  }
};






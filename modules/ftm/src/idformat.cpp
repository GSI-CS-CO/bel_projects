#include "idformat.h"


namespace ssi = DotStr::Node::Prop::TMsg::SubId;

const std::vector<const vPf> idFormats = {
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
}  



void completeId(vertex_d v, Graph& g) { // deduce SubID fields from ID or vice versa, depending on whether ID is defined
  std::stringstream ss;
  uint8_t fid;
  boost::dynamic_properties dp = createParser(g); //create current property map

  if (g[v].id == DotStr::Misc::sUndefined64) { // from SubID fields to ID
  
    fid = (s2u<uint8_t>(g[v].id_fid) & ID_FID_MSK); //get fid
    if (fid >= idFormats.length()) throw ;
    vPf& vTmp = idFormats[fid]; //choose conversion vector by fid
    
    for(auto& it : vTmp) {  //for each format vector element 
      auto iMap = dp[it.s]; //use dot property tag string as key to dp map (map of tags to (maps of vertex_indices to values))
      uint64_t val = s2u<uint64_t>(iMap[v]); // use vertex index v as key in this property map to obtain value
      id |= (val & ((1 << vTmp.bits ) - 1) ) << vTmp.pos; // OR the masked and shifted value to id
    }
    ss << std::dec << id;
    g[v].id = ss.str();

  } else { //from ID to SubID fields
    fid = ((s2u<uint8_t>(g[v].id) >> ID_FID_POS) & ID_FID_MSK);
    if (fid >= idFormats.length()) throw ;
    vPf& vTmp = idFormats[fid];

    for(auto& it : vTmp) {
      auto iMap = dp[it.s];
      iMap[v] = it.s;
    }  
  }

}  


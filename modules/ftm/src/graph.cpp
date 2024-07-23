#include "graph.h"
#include "node.h"

#include "hashmap.h"
#include "alloctable.h"
#include "grouptable.h"
#include "log.h"

//cpy ctor
//would be nicer to do this in the ctors initilisier, but Class Node's circular inclusion must be resolved
myVertex::myVertex(myVertex const &src) {
  this->name = src.name;
  this->patName   = src.patName;
  this->bpName    = src.bpName;
  this->cpu = src.cpu;
  this->section = src.section;
  this->thread = src.thread;
  this->hash = src.hash;

  this->np = (src.np != nullptr) ? src.np->clone() : nullptr;
  this->patEntry  = src.patEntry;
  this->patExit   = src.patExit;
  this->bpEntry   = src.bpEntry;
  this->bpExit    = src.bpExit;
  this->type = src.type;
  this->flags = src.flags;
  //Meta

  //Block
  this->tPeriod = src.tPeriod;
  this->rdIdxIl = src.rdIdxIl;
  this->rdIdxHi = src.rdIdxHi;
  this->rdIdxLo = src.rdIdxLo;
  this->wrIdxIl = src.wrIdxIl;
  this->wrIdxHi = src.wrIdxHi;
  this->wrIdxLo = src.wrIdxLo;

  //Event
  this->tOffs = src.tOffs;

  //Timing Message
  this->id = src.id;
  this->id_fid = src.id_fid;
  this->id_gid = src.id_gid;
  this->id_evtno = src.id_evtno;
  this->id_sid = src.id_sid;
  this->id_bpid = src.id_bpid;
  this->id_res = src.id_res;
  this->id_bin = src.id_bin;
  this->id_bpcstart = src.id_bpcstart;
  this->id_reqnob = src.id_reqnob;
  this->id_vacc = src.id_vacc;
  this->id_evtidatt = src.id_evtidatt;
  this->par = src.par;
  this->tef = src.tef;
  this->res = src.res;

  //StartThread
  this->startOffs = src.startOffs;

  //Command
  this->tValid = src.tValid;
  this->vabs   = src.vabs;


  // Flush
  this->qIl = src.qIl;
  this->qHi = src.qHi;
  this->qLo = src.qLo;

  this->frmIl= src.frmIl;
  this->toIl = src.toIl;
  this->frmHi= src.frmHi;
  this->toHi = src.toHi;
  this->frmLo= src.frmLo;
  this->toLo = src.toLo;

  //Flow, Noop
  this->prio = src.prio;
  this->qty = src.qty;
  this->perma = src.perma;

  //Wait
  this->tWait  = src.tWait;

  //for .dot-cmd abuse
  this->cmdTarget   = src.cmdTarget;
  this->cmdDest     = src.cmdDest;
  this->cmdDestBp   = src.cmdDestBp;
  this->cmdDestPat  = src.cmdDestPat;
  this->cmdDestThr  = src.cmdDestThr;

}

//cpy ctor
//would be nicer to do this in the ctors initilisier, but Class Node's circular inclusion must be resolved
myEdge::myEdge(myEdge const &src) {
  this->type      = src.type  ;
  this->fhead     = src.fhead ;
  this->ftail     = src.ftail ;
  this->bwidth    = src.bwidth;
};


Graph& updown_copy_graph(const Graph& original, Graph& cpy, vertex_map_t& vmap, AllocTable &at, HashMap &hm, GroupTable &gt) {

  BOOST_FOREACH( vertex_t v, vertices(original) ) {
    if (original[v].type != dnt::sDstList) {
      
      vertex_t i = boost::add_vertex(original[v], cpy);
      vmap[v] = i; // must keep track of descriptors as they can differ between graphs
    } else {
      log<DEBUG_LVL0>(L"updown: skipping node %1% and clearing its alloctable, hashmap and grouptable entries.") % original[v].name.c_str();
      //we don't copy dstlists. remove their stuff from tables.
      at.deallocate(original[v].hash); //using the hash is independent of vertex descriptors, so no remapping necessary yet
      hm.remove(original[v].hash);
      gt.remove<Groups::Node>(original[v].name);  
    }
  }

  //now we have a problem: all vertex descriptors in the alloctable just got invalidated by the removal ... repair them
  std::vector<amI> itAtVec; //because elements change order during repair loop, we need to store iterators first

  for( amI it = at.getTable().begin(); it != at.getTable().end(); it++) { itAtVec.push_back(it); }
  for( auto itIt : itAtVec ) {  at.modV(itIt, vmap[itIt->v]); }

  BOOST_FOREACH( vertex_t v, vertices(original) ) {
    typename Graph::out_edge_iterator out_begin, out_end, out_cur;
    boost::tie(out_begin, out_end) = out_edges(v, original);
    for (out_cur = out_begin; out_cur != out_end; ++out_cur) {
      if (vmap.find(v) != vmap.end()) boost::add_edge(vmap[v], vmap[target(*out_cur, original)], myEdge(original[*out_cur]), cpy);
    }
  }

  return cpy;
}
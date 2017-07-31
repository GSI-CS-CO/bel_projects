#include "graph.h"
#include "node.h"

//would be nicer to do this in the ctors initilisier, but Class Node's circular inclusion must be resolved
myVertex::myVertex(myVertex const &src) {
  this->name = src.name;
  this->cpu = src.cpu;
  this->hash = src.hash;
  if (src.np != NULL) this->np = src.np->clone();
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

  this->par = src.par;
  this->tef = src.tef;
  this->res = src.res;

  //Command
  this->tValid = src.tValid;


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

  //Wait
  this->tWait  = src.tWait;

  

}
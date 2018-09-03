#include "graph.h"
#include "node.h"

//FIXME OLD !!!!! Update !!!

//would be nicer to do this in the ctors initilisier, but Class Node's circular inclusion must be resolved
myVertex::myVertex(myVertex const &src) {
  this->name = src.name;
  this->patName   = src.patName;
  this->bpName    = src.bpName;
  this->cpu = src.cpu;
  this->thread = src.thread;
  this->hash = src.hash;

  this->np = (src.np != nullptr) ? src.np->clone() : nullptr;
  //std::cout << "Original " << src.np << " cpy " << this->np << std::endl;

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
  this->id_reqnob  = src.id_reqnob;
  this->id_vacc    = src.id_vacc;
  this->par = src.par;
  this->tef = src.tef;
  this->res = src.res;

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


}
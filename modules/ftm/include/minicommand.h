#ifndef _MINICOMMAND_H_
#define _MINICOMMAND_H_

#include <stdlib.h>
#include <stdint.h>
#include "ftm_common.h"


class MiniCommand  {
protected:
  uint64_t tValid;
  uint32_t act;

  MiniCommand(uint64_t tValid, uint32_t act) : tValid(tValid), act(act) {}
public:

  ~MiniCommand() {};
  virtual uint32_t getAct() const {return this->act;};
  virtual void serialise(uint8_t* b) const {
    writeLeNumberToBeBytes(b + (ptrdiff_t)T_CMD_TIME, this->tValid);
    writeLeNumberToBeBytes(b + (ptrdiff_t)T_CMD_ACT,  this->act);
  };


};

// Makes receiving Q do nothing when leaving block for N times
class MiniNoop : public MiniCommand {

public:
  MiniNoop(uint64_t tValid, uint8_t prio, uint32_t qty)
  : MiniCommand(tValid, (ACT_TYPE_NOOP << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (qty & ACT_QTY_MSK) << ACT_QTY_POS ) {}
  ~MiniNoop() {};

  void serialise(uint8_t* b) const {MiniCommand::serialise(b);}



};

// Makes receiving Q select destination when leaving block for N times
class MiniFlow : public MiniCommand {
  uint32_t destAdr;
public:
  MiniFlow(uint64_t tValid, uint8_t prio, uint32_t qty, uint32_t destAdr, bool permanent)
      : MiniCommand(tValid, (ACT_TYPE_FLOW << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (qty & ACT_QTY_MSK) << ACT_QTY_POS | ( permanent & ACT_CHP_MSK) << ACT_CHP_POS ), destAdr(destAdr) {}
  ~MiniFlow() {};

  uint32_t getDst() {return destAdr;}
  void serialise(uint8_t* b) const {
    MiniCommand::serialise(b);
    writeLeNumberToBeBytes(b + (ptrdiff_t)T_CMD_FLOW_DEST, this->destAdr);
    writeLeNumberToBeBytes(b + (ptrdiff_t)T_CMD_RES, (uint32_t)0);
  };

};

// Makes receiving Q add tWait instead of tPeriod to current time when leaving block once
class MiniWait : public MiniCommand {
  uint64_t tWait;
public:
  MiniWait(uint64_t tValid,  uint8_t prio, uint64_t tWait, bool permanent, bool abs)
  : MiniCommand(tValid, (ACT_TYPE_WAIT << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | 1 << ACT_QTY_POS), tWait(tWait) {}
  ~MiniWait() {};
  void serialise(uint8_t* b) const {
    MiniCommand::serialise(b);
    writeLeNumberToBeBytes(b + (ptrdiff_t)T_CMD_WAIT_TIME, this->tWait);
  };



};

// Makes receiving Q clear <prio> queue buffer when leaving block once
class MiniFlush : public MiniCommand {
  uint8_t prio, mode;
  bool qIl, qHi, qLo;

  uint8_t frmIl, toIl;
  uint8_t frmHi, toHi;
  uint8_t frmLo, toLo;


public:

  MiniFlush(uint64_t tValid, uint8_t prio, bool qIl, bool qHi, bool qLo )
        : MiniCommand(tValid, (ACT_TYPE_FLUSH << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (1 & ACT_QTY_MSK) << ACT_QTY_POS | ( ((qIl << PRIO_IL) | (qHi << PRIO_HI) | (qLo << PRIO_LO)) & ACT_FLUSH_PRIO_MSK) << ACT_FLUSH_PRIO_POS ), qIl(qIl), qHi(qHi), qLo(qLo), frmIl(0), toIl(0), frmHi(0), toHi(0), frmLo(0), toLo(0) {}
  MiniFlush(uint64_t tValid, uint8_t prio, bool qIl, bool qHi, bool qLo, uint8_t frmIl, uint8_t toIl, uint8_t frmHi, uint8_t toHi, uint8_t frmLo, uint8_t toLo)
        : MiniCommand(tValid, (ACT_TYPE_FLUSH << ACT_TYPE_POS) | (prio & ACT_PRIO_MSK) << ACT_PRIO_POS | (1 & ACT_QTY_MSK) << ACT_QTY_POS | ( ((qIl << PRIO_IL) | (qHi << PRIO_HI) | (qLo << PRIO_LO)) & ACT_FLUSH_PRIO_MSK) << ACT_FLUSH_PRIO_POS ), qIl(qIl), qHi(qHi), qLo(qLo), frmIl(frmIl), toIl(toIl), frmHi(frmHi), toHi(toHi), frmLo(frmLo), toLo(toLo) {}
  ~MiniFlush() {};

  void serialise(uint8_t* b) const {
    MiniCommand::serialise(b);
    /*
    b[CMD_FLUSHRNG_IL_FRM]  = this->frmIl;
    b[CMD_FLUSHRNG_IL_TO]   = this->toIl;
    b[CMD_FLUSHRNG_HI_FRM]  = this->frmHi;
    b[CMD_FLUSHRNG_HI_TO]   = this->toHi;
    b[CMD_FLUSHRNG_LO_FRM]  = this->frmLo;
    b[CMD_FLUSHRNG_LO_TO]   = this->toLo;
    */
  }




};

#endif

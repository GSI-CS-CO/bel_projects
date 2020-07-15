#ifndef _BLOCK_LOCK_H_
#define _BLOCK_LOCK_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include <etherbone.h>
#include "common.h"


typedef struct {
  bool set;
  bool clr;
  bool stat;
  bool act;
} LockFlags;

class BlockLock {
private:
  const uint32_t baseAdr, qflagsAdr, wridxAdr, rdidxAdr;

public:
  const std::string name;
  LockFlags rd;
  LockFlags wr;

  BlockLock(const std::string& name, const uint32_t adr) 
    : baseAdr(adr), qflagsAdr(adr + BLOCK_CMDQ_FLAGS), 
    wridxAdr(adr + BLOCK_CMDQ_WR_IDXS), rdidxAdr(adr + BLOCK_CMDQ_RD_IDXS), name(name), rd({false, false, false, false, }), wr({false, false, false, false, }) {};
/*  
  BlockLock(const std::string& name, const uint32_t adr, bool setDNR, bool clrDNR, bool setDNW, bool clrDNW)
    : baseAdr(adr), qflagsAdr(adr + BLOCK_CMDQ_FLAGS), 
    wridxAdr(adr + BLOCK_CMDQ_WR_IDXS), rdidxAdr(adr + BLOCK_CMDQ_RD_IDXS), name(name), 
    rd.set(setDNR), rd.clr(clrDNR), wr.set(setDNW), wr.clr(clrDNW) {};
 */
  ~BlockLock() {};


  vEbwrs& lock(vEbwrs& ew);

  vEbwrs& unlock(vEbwrs& ew);

  vEbrds& updateStat(vEbrds& er);

  vEbrds& updateAct(vEbrds& er);

  bool isAllSet() const; // returns true if all 'set' requests are false or have a matching 'stat'
  bool isAnySet() const; // returns true if any 'set' request is true and has a matching 'stat'

  bool isAct() const;

};  

 

#endif
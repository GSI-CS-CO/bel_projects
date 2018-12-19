#include "blocklock.h"

vEbwrs& BlockLock::lock(vEbwrs& ew) {
  ew.va  += qflagsAdr;
  uint32_t ret = (((uint32_t)(wr.set | wr.stat) << BLOCK_CMDQ_DNW_POS) | ((uint32_t)(rd.set | rd.stat) << BLOCK_CMDQ_DNR_POS));
  std::cout << "setting locks: wrset " << std::boolalpha << wr.set << " wrclr " << wr.clr <<" wrstat " << wr.stat << " rdset " << rd.set << " rdclr " << rd.clr << " rdstat " << rd.stat << std::endl;
  std::cout << "intended outcome: 0x " << std::hex << ret << std::endl;
  writeLeNumberToBeBytes(ew.vb, ret );
  ew.vcs += leadingOne(1);
  return ew;
}

vEbwrs& BlockLock::unlock(vEbwrs& ew ) {
  ew.va  += qflagsAdr;

  // no set and no clear -> do nothing
  //    set and no clear -> 1
  // no set and    clear -> 0
  //    set and    clear -> stat

  uint32_t ret = ((uint32_t)(((wr.stat & (wr.set == wr.clr)) | (wr.set & ~wr.clr)) << BLOCK_CMDQ_DNW_POS)) | ((uint32_t)(((rd.stat & (rd.set == rd.clr)) | (rd.set & ~rd.clr)) << BLOCK_CMDQ_DNR_POS));
  


  std::cout << "setting unlocks: wrset " << std::boolalpha << wr.set << " wrclr " << wr.clr <<" wrstat " << wr.stat << " rdset " << rd.set << " rdclr " << rd.clr << " rdstat " << rd.stat << std::endl;
  std::cout << "intended outcome: 0x " << std::hex << ret << std::endl;
  writeLeNumberToBeBytes(ew.vb, ret );
  ew.vcs += leadingOne(1);
  return ew;
}

vEbrds& BlockLock::updateStat(vEbrds& er) {
  er.va  += qflagsAdr;
  er.vcs += leadingOne(1);
  return er;
}

vEbrds& BlockLock::updateAct(vEbrds& er) {
  vAdr tmp = {wridxAdr, rdidxAdr}; 
  er.va  += tmp;
  er.vcs += leadingOne(tmp.size());
  return er;
}

bool BlockLock::isAllSet()  const {
  //check if any lock bits is requested but not present. If so, return false.
  return ( ((wr.set & wr.stat) | ~wr.set) & ((rd.set & rd.stat) | ~rd.set) );
}

bool BlockLock::isAnySet() const {
  //check if any lock bits is requested but not present. If so, return false.
    
    bool res =  ((wr.set & wr.stat) | (rd.set & rd.stat));
    std::cout << "AnySet:" 
            << " wrSet " << std::boolalpha << wr.set << " rdSet " << rd.set
            << " wrStat " << wr.stat << " rdStat " << rd.stat << " res " << res << std::endl;
  return res;
}

bool BlockLock::isAct() const {
  //check if any lock bits is requested but not present. If so, return false.

  return ( ((wr.set & wr.act) | ~wr.set) & ((rd.set & rd.act) | ~rd.set) );
}


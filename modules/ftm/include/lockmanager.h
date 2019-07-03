#ifndef _LOCK_MANAGER_H_
#define _LOCK_MANAGER_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <inttypes.h>
#include "ebwrapper.h"
#include "common.h"
#include "hashmap.h"
#include "alloctable.h"
#include "covenanttable.h"
#include "blocklock.h"


    //save lock states -> set requested locks -> send commands -> restore previous lock states
    
    //if both set AND clr bits are true, previous state is restored. Otherwise, set / clr override previous lock state
    //this is used to enable explicit lock or unlock commands

    //Behaviour Table for rd and wr locks: 
    // old set clr    new
    // 0   0   0      0   // -> old state
    // 0   0   1      0   // -> cleared
    // 0   1   0      1   // -> set
    // 0   1   1      0   // -> old state
    // 1   0   0      1   // -> old state
    // 1   0   1      0   // -> cleared
    // 1   1   0      1   // -> set
    // 1   1   1      1   // -> old state

class LockManager {
private:
  EbWrapper& ebd;
  std::vector<BlockLock> vBl;
  HashMap& hm;
  CovenantTable& ct;
  AllocTable& at;

  uint32_t getBaseAdr(const std::string& name);
  bool hasCovenant(const std::string& name);


public:

  LockManager(EbWrapper& ebd, HashMap& hm, CovenantTable& ct, AllocTable& at) : ebd(ebd), hm(hm), ct(ct), at(at) {};
  ~LockManager(){};

  const std::vector<BlockLock>& getLockVec() {return vBl;}


  BlockLock& add(const std::string& name);
  //BlockLock& add(const std::string& name, bool setDNR, bool clrDNR, bool setDNW, bool clrDNW);
  //not necessary to remove individuals
  void processLockRequests();
  void processUnlockRequests();
  void readInStat();
  bool isReady();
  void clear() {vBl.clear();};


};  

#endif
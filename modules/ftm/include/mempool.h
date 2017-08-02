
#ifndef _MEM_POOL_H_
#define _MEM_POOL_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <stdlib.h>
#include "common.h"


 

typedef std::set<uint32_t> aPool; // contains all available addresses in LM32 memory area


class MemPool {
  friend class AllocTable;
  friend class CarpeDM;


  
  const uint8_t cpu;
  const uint32_t extBaseAdr;  //where to find the B-Port of this CPU RAM from the Host's perspective
  const uint32_t intBaseAdr;  //where to find the A-Port of this CPU RAM from this CPU's perspective
  const uint32_t peerBaseAdr; //where to find the B-Port of this CPU RAM from another CPU's perspective
  const uint32_t  sharedOffs;
  const uint32_t  nodeQty;
  const uint32_t  bmpBits;
  const uint32_t  bmpSize;
  const uint32_t  startOffs; // baseAddress + bmpLen rounded up to next multiple of MEM_BLOCK_SIZE to accomodate BMP
  const uint32_t  endOffs;   // baseAddress + nodeQty rounded down to next multiple of MEM_BLOCK_SIZE, can only use whole blocks 
  vBuf  bmp;
  aPool pool;

protected:
  

public:  
  // actually not used by this class, but best place to keep the info


  MemPool(uint8_t cpu, uint32_t extBaseAdr, uint32_t intBaseAdr, uint32_t peerBaseAdr, uint32_t sharedOffs, uint32_t space)
        : cpu(cpu),
          extBaseAdr(extBaseAdr),
          intBaseAdr(intBaseAdr),
          peerBaseAdr(peerBaseAdr),
          sharedOffs(sharedOffs),
          nodeQty(space / _MEM_BLOCK_SIZE), 
          bmpBits(nodeQty),
          bmpSize((bmpBits + 8 * _MEM_BLOCK_SIZE -1) / (8 * _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE), 
          startOffs(sharedOffs + bmpSize), 
          endOffs(startOffs + (nodeQty * _MEM_BLOCK_SIZE)),
          bmp(bmpSize)
          { init();  }
          
  ~MemPool() { };

  void initBmp();
  void initPool();
  void init() {initBmp(); initPool();}
  void syncPoolToBmp();
  bool syncBmpToPool();

  bool getBmpBit(unsigned int bitIdx) {return bmp[bitIdx / 8] & (1 << (7 - bitIdx % 8));}


  void setBmp(const vBuf& aBmp) { bmp = aBmp;}
  const vBuf& getBmp()           const { return bmp; }
  
  uint32_t getFreeChunkQty()  const { return pool.size(); }
  uint32_t getFreeSpace()     const { return pool.size() * _MEM_BLOCK_SIZE; }
  uint32_t getUsedSpace()     const { return nodeQty - (pool.size() * _MEM_BLOCK_SIZE); }

  bool acquireChunk(uint32_t &adr);
  bool freeChunk(uint32_t &adr);
  void removeChunk(uint32_t adr) { pool.erase(adr); }

  

};

#endif
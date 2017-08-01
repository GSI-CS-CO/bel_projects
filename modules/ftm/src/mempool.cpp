#include "mempool.h"

void MemPool::initPool() { 
    pool.clear();
    //Never issue <baseAddress - (baseAddress + bmpBits -1) >, as this is where Mgmt bitmap vector resides  
    for(uint32_t adr = startOffs; adr < endOffs; adr += _MEM_BLOCK_SIZE) { pool.insert(adr); }
  }

void MemPool::initBmp() {
  //awkward initialistion because bmp in bytes can be  > bmpBits / 8
  for (auto& it : bmp) { it = 0; }
  for (uint32_t i=0; i< bmpBits/8; i++)     { bmp[i] = 0xff; }
  for (uint32_t i=0; i< (bmpBits % 8); i++) { bmp[bmpBits/8] |= (1 << (7 - (i % 8))); }
}

void MemPool::syncPoolToBmp() {
  uint32_t nodeAdr;
  initPool(); //init normally, then remove everything already used in bitmap
  for(unsigned int bitIdx = 0; bitIdx < bmpBits; bitIdx++) {
    if (bmp[bitIdx / 8] & (1 << (7 - bitIdx % 8))) {
      nodeAdr = sharedOffs + bitIdx * _MEM_BLOCK_SIZE;
      removeChunk(nodeAdr);
    }
  }
}

bool MemPool::syncBmpToPool() {
  initBmp();
  //Go through pool and update Bmp
  for (auto& adr : pool ) {
    if( (adr >= startOffs) && (adr <= endOffs)) {
      int bitIdx        = (adr - sharedOffs) / _MEM_BLOCK_SIZE;
      uint8_t       tmp = ~(1 << (7 - (bitIdx % 8)));
      bmp[bitIdx / 8]  &= tmp;
    } else {//something's awfully wrong, address out of range!
      return false;
    }
  }
  return true;
}


bool MemPool::acquireChunk(uint32_t &adr) {
  bool ret = true;
  if ( pool.empty() ) {
    ret = false;
  } else {
    adr = *(pool.begin());
    pool.erase(adr);
  }
  return ret;
}


bool MemPool::freeChunk(uint32_t &adr) {
  //bool ret = true;
  uint32_t a = adr - startOffs;
  //if out of range or unaligned, return false
  if ((a >= endOffs - startOffs) || (a % _MEM_BLOCK_SIZE)) {return false;}
  //if address exists in the pool, don't try to return it again
  if (pool.count(adr) > 0)  {return false;}
  pool.insert(adr);
  return true;
}        
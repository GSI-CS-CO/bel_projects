#ifndef _MEM_UNIT_H_
#define _MEM_UNIT_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <stdlib.h>
#include "common.h"
#include "ftm_common.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_IDX 32
#define IDX_BMPS (MAX_IDX / 32)





//typedef std::map< std::string, myData >::iterator itMap ;



  typedef struct  {
    uint32_t  adr;
    uint32_t  hash;
    uint8_t   b[_MEM_BLOCK_SIZE];
    bool      transfer;
  } chunkMeta;


typedef std::map<std::string, chunkMeta>  aMap;
typedef std::map<uint32_t,  std::string>  hMap;
typedef std::set<uint32_t>                aPool; // contains all available addresses in LM32 memory area
typedef hMap::iterator  itHm;
typedef aMap::iterator  itAm;
typedef aPool::iterator itAp;



class MemUnit {
  
  const uint8_t   cpu;
  const uint32_t  baseAdr;
  const uint32_t  poolSize;
  const uint32_t  bmpLen;
 

  
  const uint32_t  startOffs; // baseAddress + bmpLen rounded up to next multiple of MEM_BLOCK_SIZE to accomodate BMP
  const uint32_t  endOffs;   // baseAddress + poolSize rounded down to next multiple of MEM_BLOCK_SIZE, can only use whole blocks 
     
  Graph&  g;
  

  //ebdevice
  //eb socket

  
 aPool   memPool;
  
public:
  aMap allocMap;
  hMap hashMap;
  vBuf mgmtBmp; 
  MemUnit(uint8_t cpu, uint32_t baseAdr, uint32_t  poolSize, Graph& g) : cpu(cpu), baseAdr(baseAdr), 
          poolSize(poolSize), bmpLen( poolSize / _MEM_BLOCK_SIZE), 
          startOffs((((bmpLen + 8 -1)/8 + _MEM_BLOCK_SIZE -1) / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE),
          endOffs((poolSize / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE),
          g(g), mgmtBmp(vBuf( (bmpLen + 8 -1)/8) ) { initMemPool();}
  ~MemUnit() { };

  //MemPool Functions
  void updatememPoolFromBmp(); 
  void initMemPool();
  bool acquireChunk(uint32_t &adr);
  bool freeChunk(uint32_t &adr);

  //Management functions
  void updateBmpFromAlloc();
  void showAdrsFromBmp();
 

  //Allocation functions
  bool allocate(const std::string& name);
  bool insert(const std::string& name, uint32_t adr);
  bool deallocate(const std::string& name);
  chunkMeta* lookupName(const std::string& name) const;

  //Hash functions
  bool insertHash(const std::string& name, uint32_t &hash);
  bool removeHash(const uint32_t hash);
  chunkMeta* lookupHash(const uint32_t hash) const ;
  Graph& getGraph() const {return g;}
  vAdr vertices2addresses(const vVertices &v) const;

};



#endif

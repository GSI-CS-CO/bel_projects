#ifndef _MEM_UNIT_H_
#define _MEM_UNIT_H_
#include <stdint.h>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <stdlib.h>
#include "common.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_IDX 32
#define IDX_BMPS (MAX_IDX / 32)

#define MEM_BLOCK_SIZE  32



//typedef std::map< std::string, myData >::iterator itMap ;



  typedef struct  {
    uint32_t  adr;
    uint32_t  hash;
    vBuf      buf;
  } chunkMeta;

typedef std::map<std::string, chunkMeta> mMap;
typedef std::map<std::string, chunkMeta> aMap;
typedef std::map<uint32_t, std::string> hMap;
typedef   hMap::iterator itHm;
typedef   aMap::iterator itAm;
typedef   mMap::iterator itMm;



class MemUnit {

  uint8_t   cpu;
  uint32_t  baseAdr;
  uint32_t  poolSize;
  Graph&    g;

  std::set<uint32_t> memPool; // contains all available addresses in LM32 memory

  

  
  public:
  aMap allocMap;
  mMap mgmtMap;
  hMap  hashMap;
  MemUnit(uint8_t cpu, uint32_t baseAdr, uint32_t  poolSize, Graph& g) : cpu(cpu), baseAdr(baseAdr), poolSize(poolSize), g(g) { initMemPool();}
  ~MemUnit() { };

  //MemPool Functions
  void updatememPoolFromBmp32(uint32_t bmp, int bmpNo); 
  void initMemPool();
  bool acquireChunk(uint32_t &adr);
  bool freeChunk(uint32_t &adr);

  //Management functions
  void initMgmt() {clearMgmt(); mgmtMap["Entry00"] = {baseAdr, 0xdeadbeef, vBuf(32)}; } //generate hashtable entires !!!
  bool updateMgmt();
    //clear all mgmt nodes
    //get names of all designated entry points
    //calculate number of nodes to create for table
    //create nodes
    //assign entry point children
    //create edges to entry point children and next mgmt node
    //ready for serialisation
  bool clearMgmt(); { //delete all mgmt nodes
    for (itMm it = mgmtMap.begin(); it != mgmtMap.end(); it++) {freeChunk(it->second.adr);} mgmtMap.clear();
  }
     

  //Allocation functions
  bool allocate(const std::string& name);
  bool insert(const std::string& name, uint32_t adr);
  bool deallocate(const std::string& name);
  bool lookupName2Chunk(const std::string& name, chunkMeta*& chunk) ;

  //Hash functions
  bool insertHash(const std::string& name, uint32_t &hash);
  bool removeHash(const uint32_t hash);
  bool lookupHash2Name(const uint32_t hash, std::string& name);
  Graph getGraph() const;

};



#endif

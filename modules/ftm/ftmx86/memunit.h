#ifndef _MEM_UNIT_H_
#define _MEM_UNIT_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <boost/bimap.hpp>
#include <stdlib.h>
#include "common.h"
#include "ftm_common.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_IDX 32
#define IDX_BMPS (MAX_IDX / 32)





//typedef std::map< std::string, myData >::iterator itMap ;

  typedef struct  {
    vertex_t  v;
    uint32_t  hash;
    uint8_t   b[_MEM_BLOCK_SIZE];
  } parserMeta;

  typedef struct  {
    uint32_t  adr;
    uint32_t  hash;
    uint8_t   b[_MEM_BLOCK_SIZE];
    bool      transfer;
  } chunkMeta;


typedef std::map<std::string, chunkMeta>  aMap;
typedef std::map<uint32_t, parserMeta>  pMap;
//typedef std::map<uint32_t,  std::string>  hMap;
typedef boost::bimap< uint32_t, std::string > hBiMap;
typedef hBiMap::value_type hashValue;
typedef std::set<uint32_t>                aPool; // contains all available addresses in LM32 memory area
typedef boost::container::vector<chunkMeta*> vChunk;



class MemUnit {
  
  const uint8_t   cpu;
  const uint32_t  extBaseAdr;
  const uint32_t  intBaseAdr;
  const uint32_t  sharedOffs;
  const uint32_t  poolSize;
  const uint32_t  bmpLen;
  const uint32_t  startOffs; // baseAddress + bmpLen rounded up to next multiple of MEM_BLOCK_SIZE to accomodate BMP
  const uint32_t  endOffs;   // baseAddress + poolSize rounded down to next multiple of MEM_BLOCK_SIZE, can only use whole blocks 
  Graph&  gUp;
  Graph gDown;
  vBuf   uploadBmp;
  vBuf   downloadBmp;

  aPool  memPool;
  aMap   allocMap;
  pMap   parserMap;
  hBiMap hashMap;
  



  

public:  




  MemUnit(uint8_t cpu, uint32_t extBaseAdr, uint32_t intBaseAdr, uint32_t sharedOffs, uint32_t poolSize, Graph& g) 
        : cpu(cpu), extBaseAdr(extBaseAdr), intBaseAdr(intBaseAdr), sharedOffs(sharedOffs),
          poolSize(poolSize), bmpLen( poolSize / _MEM_BLOCK_SIZE), 
          startOffs(sharedOffs + ((((bmpLen + 8 -1)/8 + _MEM_BLOCK_SIZE -1) / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE)),
          endOffs(sharedOffs + ((poolSize / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE)),
          gUp(g), uploadBmp(vBuf( ((((bmpLen + 8 -1)/8 + _MEM_BLOCK_SIZE -1) / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE) )), downloadBmp(vBuf( ((((bmpLen + 8 -1)/8 + _MEM_BLOCK_SIZE -1) / _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE) )) { initMemPool();}
  ~MemUnit() { };

  Graph& getUpGraph() const {return gUp;}
  Graph getDownGraph() const {return gDown;}

  //MemPool Functions

  void initMemPool();
  bool acquireChunk(uint32_t &adr);
  bool freeChunk(uint32_t &adr);

  //Management functions

  uint32_t getFreeChunkQty() { return memPool.size(); }
  uint32_t getFreeSpace() { return memPool.size() * _MEM_BLOCK_SIZE; }
  uint32_t getUsedSpace() { return poolSize - (memPool.size() * _MEM_BLOCK_SIZE); }
  
  //Allocation functions
  bool allocate(const std::string& name);
  bool insert(const std::string& name, uint32_t adr);
  bool deallocate(const std::string& name);
  chunkMeta* lookupName(const std::string& name) const;
  vChunk getAllChunks() const;

  //Hash functions
  bool insertHash(const std::string& name, uint32_t &hash);
  bool removeHash(const uint32_t hash);
  const std::string& hash2name(const uint32_t hash)     const  { return hashMap.left.at(hash); }
  const uint32_t&    name2hash(const std::string& name) const  { return hashMap.right.at(name); }

  //Addr Functions
  const uint32_t extAdr2adr(const uint32_t ea)    const  { return ea - extBaseAdr; }
  const uint32_t intAdr2adr(const uint32_t ia)    const  { return ia - intBaseAdr; }
  const uint32_t extAdr2intAdr(const uint32_t ea) const  { return ea - extBaseAdr + intBaseAdr; }
  const uint32_t intAdr2extAdr(const uint32_t ia) const  { return ia - intBaseAdr + extBaseAdr; }
  const uint32_t adr2extAdr(const uint32_t a)     const  { return a + extBaseAdr; }
  const uint32_t adr2intAdr(const uint32_t a)     const  { return a + intBaseAdr; }
  
  //Upload Functions
  void prepareUpload();

  void createUploadBmp();
  vAdr getUploadAdrs();
  vBuf getUploadData();

  //Download Functions
  vAdr getDownloadAdrs();
  void parseDownloadData(vBuf downloadData);

};



#endif

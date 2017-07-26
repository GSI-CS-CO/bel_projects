#ifndef _MEM_UNIT_H_
#define _MEM_UNIT_H_

#include <stdint.h>
#include <string>
#include <iostream>
#include <set>
#include <boost/optional.hpp>
#include <stdlib.h>
#include "common.h"
#include "graph.h"
#include "ftm_common.h"
#include "hashmap.h"
#include "alloctable.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_IDX 32
#define IDX_BMPS (MAX_IDX / 32)

#define ALLOC_OK             (0)
#define ALLOC_NO_SPACE      (-1)
#define ALLOC_ENTRY_EXISTS  (-2)  




typedef std::set<uint32_t>                    aPool; // contains all available addresses in LM32 memory area




class MemUnit {
  
  const uint8_t   cpu;
  const uint32_t  extBaseAdr;
  const uint32_t  intBaseAdr;
  const uint32_t  sharedOffs;
  const uint32_t  nodeQty;
  const uint32_t  bmpBits;
  const uint32_t  bmpSize;
  const uint32_t  startOffs; // baseAddress + bmpLen rounded up to next multiple of MEM_BLOCK_SIZE to accomodate BMP
  const uint32_t  endOffs;   // baseAddress + nodeQty rounded down to next multiple of MEM_BLOCK_SIZE, can only use whole blocks 
  
  HashMap& hashMap;
  
  
  
  vBuf   uploadBmp;
  Graph&  gUp;
  AllocTable& atUp;

  vBuf   downloadBmp;
  Graph& gDown;
  AllocTable& atDown;

  aPool  memPool;

protected:

  
  

public:  


// get your own pool and bmps, but use common alloctables and graphs

  MemUnit(uint8_t cpu, uint32_t extBaseAdr, uint32_t intBaseAdr, uint32_t sharedOffs, uint32_t space, HashMap& hm, Graph& gUp, AllocTable& atUp, Graph& gDown, AllocTable& atDown)
        : cpu(cpu), extBaseAdr(extBaseAdr), intBaseAdr(intBaseAdr), sharedOffs(sharedOffs),
          nodeQty(space / _MEM_BLOCK_SIZE), 
          bmpBits(nodeQty),
          bmpSize((bmpBits + 8 * _MEM_BLOCK_SIZE -1) / (8 * _MEM_BLOCK_SIZE) * _MEM_BLOCK_SIZE), // (bmpBits + memBlockBits -1) / memBlockBits * memBlockBytes = bmpSize in bytes
          startOffs(sharedOffs + bmpSize), 
          endOffs(startOffs + (nodeQty * _MEM_BLOCK_SIZE)),
          hashMap(hm),
          uploadBmp(vBuf(bmpSize)),
          gUp(gUp),
          atUp(atUp),  
          downloadBmp(vBuf(bmpSize)),
          gDown(gDown),
          atDown(atDown)  
          {}
  ~MemUnit() { };

  Graph& getUpGraph()   {return gUp;}
  Graph& getDownGraph() {return gDown;}
  AllocTable& getDownAllocTable() {return atDown;}
  AllocTable& getUpAllocTable()   {return atUp;}

  //MemPool Functions
  void removeChunk(uint32_t adr) {memPool.erase(adr);};
  void initMemPool();
  void initMemPoolFromDownloadBMP();
  bool acquireChunk(uint32_t &adr);
  bool freeChunk(uint32_t &adr);

  //Management functions

  uint32_t getFreeChunkQty() { return memPool.size(); }
  uint32_t getFreeSpace() { return memPool.size() * _MEM_BLOCK_SIZE; }
  uint32_t getUsedSpace() { return nodeQty - (memPool.size() * _MEM_BLOCK_SIZE); }
  
  //Allocation functions
  int allocate(uint32_t hash, vertex_t v);
  //bool insert(uint32_t hash, uint32_t adr);
  bool deallocate(uint32_t hash);





  

  //Addr Functions
  const uint32_t extAdr2adr(const uint32_t ea)    const  { return (ea == LM32_NULL_PTR ? LM32_NULL_PTR : ea - extBaseAdr); }
  const uint32_t intAdr2adr(const uint32_t ia)    const  { return (ia == LM32_NULL_PTR ? LM32_NULL_PTR : ia - intBaseAdr); }
  const uint32_t extAdr2intAdr(const uint32_t ea) const  { return (ea == LM32_NULL_PTR ? LM32_NULL_PTR : ea - extBaseAdr + intBaseAdr); }
  const uint32_t intAdr2extAdr(const uint32_t ia) const  { return (ia == LM32_NULL_PTR ? LM32_NULL_PTR : ia - intBaseAdr + extBaseAdr); }
  const uint32_t adr2extAdr(const uint32_t a)     const  { return a + extBaseAdr; }
  const uint32_t adr2intAdr(const uint32_t a)     const  { return a + intBaseAdr; }
  
  //Upload Functions
  void prepareUpload(Graph& g);

  void createUploadBmp();
  vAdr getUploadAdrs() const;
  vBuf getUploadData();

  //Download Functions
  void setDownloadBmp(vBuf dlBmp) {downloadBmp = dlBmp;} // vHexDump ("DLBMP", downloadBmp, downloadBmp.size());}
  const vAdr getDownloadBMPAdrs() const;
  const vAdr getDownloadAdrs() const;
  void parseDownloadData(vBuf downloadData);

  const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio) const; 
  const uint32_t getCmdInc(uint32_t hash, uint8_t prio) const;

  void show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta );



};



#endif

#ifndef _MEM_UNIT_H_
#define _MEM_UNIT_H_
#include <stdint.h>
#include <string>
#include <iostream>
#include <boost/bimap.hpp>
#include <boost/lockfree/queue.hpp>
#include <stdlib.h>
#include "node.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_IDX 32
#define IDX_BMPS (MAX_IDX / 32)

typedef std::set<int16_t> number_pool;


typedef boost::bimap< std::string, int > results_bimap;
typedef results_bimap::value_type position;

class Allocator {

  uint8_t   cpu;
  uint32_t  baseAdr;
  Graph&    g;
  
  //table

  public:
  Allocator();
  ~Allocator();

  ebDownload();
  ebDownloadTable();
  
  bool ebUpload() const;
  bool ebUploadTable() const;
  bool ebUploadTableUpdate() const;
    

  int getBlockSize(vertex_t v) const;
  bool allocate(vertex_t v);
  bool deallocate(AllocationElement a);
  vertex_t findNodeDesc(std:string name) const; //names must be unique
  
}

class AllocationElement public Allocator {

  std:string name;
  uint16_t  idx;
  uint32_t  adr;
  uint32_t  size;
  vBuf       vB;

  public:
    AllocationInfo(std:string NodeName, uint16_t  idx, uint32_t  adr, uint32_t  size) : idx(idx), adr(adr), size(size);
    ~AllocationInfo() {vB.clear();}

    ebDownloadBlock() const { // call eb read on AllocationTable::getBaseAdr() + adr for size / 4 elements and put result in vB}
    ebUploadBlock() const;
    bool insertBlockNode const ( // parse Buffer and create Block and all event children at Allocator::get );
    bool destroyBlockNode(//destroy block and all event children, Allocator::findNodeDesc(name) );

  
}


class MemUnit {
  uint8_t curIdx;
  number_pool idxPool;

  void updateIdxPoolFromBmp32(uint32_t bmp, int bmpNo) {
    for(int16_t i=0; i < min(MAX_IDX, 32); i++) {
      if((bmp >> i) & 1) idxPool.insert(i + bmpNo * min(MAX_IDX, 32));
    }
  }

 
  void initIdxPool() { 
    idxPool.clear();
    for(int16_t i=0; i<MAX_IDX; i++) idxPool.insert(i);
  }

    
   bool drawIdx(int16_t &idx) {
    bool ret = true;
    
    if (!(idxPool.empty())) {
      idx = *(idxPool.begin());
      idxPool.erase(idx);
    } else {
      ret = false;
    }
    return ret;
  }

  bool dropIdx(int16_t idx) {
    bool ret = true;
    if (idxPool.count(idx) > 0) {ret = false;} //attempted double entry, throw exception
    else idxPool.insert(idx);
    return ret;
  }         

  public:
    
    
    updateIdxPool(itBuf ib) { //from LM32 table
      clearIdxPool();      
      for(uint8_t i; i<IDX_BMPS; i++) {
        updateIdxPoolFromBmp32(*(ib + i), i);
      }
    }
    reset() {
      clearIdxPool();
      
         
  

    };

public:
  //TimeBlock(uint64_t start, uint64_t period, bool cmdQ) : period(period), cmdQ(cmdQ) {}
  TimeBlock(uint64_t period, bool cmdQ) : cpu(0), idx(0), adr(0), period(period), cmdQ(cmdQ) {}
  ~TimeBlock()  {};
  virtual void accept(const VisitorVertexWriter& v)     const override { v.visit(*this); }
  virtual void accept(const VisitorCreateMemBlock & v)  const override { v.visit(*this); }
  virtual void accept(const VisitorAddEvtChildren & v)  const override { v.visit(*this); }

/*
           downloadTable() const
  void     allocate(uint8_t cpu, uint16_t idx, uint32_t adr ); //replace by call to allocator
  void     deallocate() const; //call to allocator free
*/
  uint8_t  getCpu()     const {return cpu;}
  uint16_t getIdx()     const {return idx;}
  uint32_t getAdr()     const {return adr;}
  uint64_t getTOffs()   const {return -1;}
  uint64_t getTPeriod() const {return period;}
  bool hasCmdQ()        const {return cmdQ;}
  void show(void)       const {show(0, "");}
  void show(uint32_t cnt, const char* sPrefix)  const {printf("*** Block %s, Period %llu\n", sPrefix, (unsigned long long)period);}
  void serialise(itBuf ib)  {printf("I'am a serialised Timeblock\n");}

  
/*
  uint32_t getEvtQty() const {return gsub.size() const;}
  uint32_t getSize() const {} //call to allocator map
  bool     hasQ() const {return cmdQ;}
*/

};





#endif

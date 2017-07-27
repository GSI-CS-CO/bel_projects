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





class MemUnit {
  
  
  
  HashMap& hashMap;
  
  
  

  Graph&  gUp;
  AllocTable& atUp;

  Graph& gDown;
  AllocTable& atDown;



protected:

  
  

public:  




// get your own pool and bmps, but use common alloctables and graphs

  MemUnit(HashMap& hm, Graph& gUp, AllocTable& atUp, Graph& gDown, AllocTable& atDown)
        : hashMap(hm),
          gUp(gUp),
          atUp(atUp),  
          gDown(gDown),
          atDown(atDown)  
          {}
  ~MemUnit() { };

  Graph& getUpGraph()   {return gUp;}
  Graph& getDownGraph() {return gDown;}
  AllocTable& getDownAllocTable() {return atDown;}
  AllocTable& getUpAllocTable()   {return atUp;}

  //Upload Functions
  void prepareUpload(Graph& g);

 
  vAdr getUploadAdrs() const;
  vBuf getUploadData();

  //Download Functions
 
  const vAdr getDownloadBMPAdrs() const;
  const vAdr getDownloadAdrs() const;
  void parseDownloadData(vBuf downloadData);

  const vAdr getCmdWrAdrs(uint32_t hash, uint8_t prio) const; 
  const uint32_t getCmdInc(uint32_t hash, uint8_t prio) const;

  void show(const std::string& title, const std::string& logDictFile, bool direction, bool filterMeta );



};



#endif

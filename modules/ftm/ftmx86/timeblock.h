#ifndef _TIME_BLOCK_H_
#define _TIME_BLOCK_H_
#include <stdint.h>
#include <stdlib.h>
#include <boost/shared_ptr.hpp>
#include <boost/container/vector.hpp>
#include "../ftm_common.h"
#include "event.h"

typedef boost::container::vector<Event> vEvt;
typedef boost::container::vector<Event>::iterator itEvt;
typedef boost::shared_ptr<Event> evt_ptr;



class MemBlock{
  
  uint16_t idx;
  
public:
  
  MemBlock() ;
  ~MemBlock();
  
  vBuf buf;
  uint16_t getIdx() {return idx;}
  uint32_t getSize();

  void deserialise();
  void serialise();
  
};




class TimeBlock : public MemBlock{

public:
  TimeBlock();
  ~TimeBlock();
  uint16_t getIdx() {return MemBlock::getIdx();}
  uint32_t getSize() {return MemBlock::getSize();}
};





#endif

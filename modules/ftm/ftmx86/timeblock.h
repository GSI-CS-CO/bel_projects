#ifndef _TIME_BLOCK_H_
#define _TIME_BLOCK_H_
#include <stdint.h>
#include "../ftm_common.h"

#include <stdlib.h>
#include <stdint.h>
#include <boost/container/list.hpp>
#include <boost/container/vector.hpp>

class TimeBlock {
  uint64_t tOffs;  
  uint16_t flags;

  

public:
  TimeBlock();
  ~TimeBlock();
  void show();
  uint8_t* serialise(uint8_t *buf);
  
  
};





#endif

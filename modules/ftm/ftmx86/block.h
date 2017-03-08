#ifndef _BLOCK_H_
#define _BLOCK_H_
#include <stdint.h>
#include "../ftm_common.h"

#include <stdlib.h>
#include <stdint.h>
#include <typeinfo.h>

class Block {
  uint64_t tOffs;  
  uint16_t type;
  uint16_t flags;
  Block    *next;

public:
  Block();
  ~Block();
  void show();
  uint8_t* serialise(uint8_t *buf);
  allocateLM32();
  deallocate();
};





#endif

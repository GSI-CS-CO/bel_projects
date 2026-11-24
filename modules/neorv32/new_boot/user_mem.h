#ifndef USER_MEM_H
#define USER_MEM_H

#include <stdint.h>


int  mem_setup(void);
int  mem_stream_get(uint32_t* rdata);

#endif
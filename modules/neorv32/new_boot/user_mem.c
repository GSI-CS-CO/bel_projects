#include <neorv32.h>
#include <config.h>
#include <user_mem.h>

#define BASE_ADDR (volatile uint32_t *)0x71000000

static volatile uint32_t *current_addr = BASE_ADDR;

int mem_setup(void){
    current_addr = BASE_ADDR;
    return 0;
}

int mem_stream_get(uint32_t* rdata){

    uint32_t tmp;
    tmp = *current_addr;
    *rdata = tmp;
    current_addr++;

    return 0;
} 

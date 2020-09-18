#ifndef __STACK_H__
#define __STACK_H__
#include <stdint.h>

#define ENDRAM_MAGIC 0xbadc0ffe

extern uint32_t _endram;
extern void check_stack(void);
extern void check_stack_fwid(uint32_t *fwid);

#endif /* __STACK_H__ */

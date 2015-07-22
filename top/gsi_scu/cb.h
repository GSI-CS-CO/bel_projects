#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

int cbisEmpty(volatile struct circ_buffer*, int);
int cbisFull(volatile struct circ_buffer*, int);
int cbgetCount(volatile struct circ_buffer*, int);
void cbWrite(volatile struct circ_buffer*, int, struct param_set*);
void cbRead(volatile struct circ_buffer*, int, struct param_set*);
void cbDump(volatile struct circ_buffer *cb, int num);
#endif

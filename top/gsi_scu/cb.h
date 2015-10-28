#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

int cbisEmpty(volatile struct channel_regs*, int);
int cbisFull(volatile struct channel_regs*, int);
int cbgetCount(volatile struct channel_regs*, int);
void cbWrite(volatile struct channel_buffer*, volatile struct channel_regs*, int, struct param_set*);
void cbRead(volatile struct channel_buffer*, volatile struct channel_regs*, int, struct param_set*);
void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs*, int num);
#endif

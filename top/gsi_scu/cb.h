#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

int cbisEmpty(struct circ_buffer*, int);
int cbisFull(struct circ_buffer*, int);
int cbgetCount(struct circ_buffer*, int);
void cbWrite(struct circ_buffer*, int, struct param_set*);
void cbRead(struct circ_buffer*, int, struct param_set*);



#endif

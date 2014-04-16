#include <stdlib.h>
#include <cb.h>
#include <fg.h>
#include <aux.h>

int cbisEmpty(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = cb[num].wr_ptr == cb[num].rd_ptr;
  atomic_off();
  return ret;
}

int cbisFull(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = (cb[num].wr_ptr + 1) % cb[num].size == cb[num].rd_ptr;
  atomic_off();
  return ret; 
}

int cbgetCount(struct circ_buffer* cb, int num) {
  int ret = 0;
  atomic_on();
  ret = abs(cb[num].wr_ptr - cb[num].rd_ptr);
  atomic_off();
  return ret;
}

void cbWrite(struct circ_buffer* cb, int num, struct param_set *pset) {}
void cbRead(struct circ_buffer *cb, int num, struct param_set *pset) {}



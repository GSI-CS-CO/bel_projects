#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

inline int cbisEmpty(volatile struct channel_regs* cr, int channel) {
  return cr[channel].wr_ptr == cr[channel].rd_ptr;
}

inline int cbgetCount(volatile struct channel_regs* cr, int channel) {
  if (cr[channel].wr_ptr > cr[channel].rd_ptr)
    return cr[channel].wr_ptr - cr[channel].rd_ptr;
  else if (cr[channel].rd_ptr > cr[channel].wr_ptr)
    return BUFFER_SIZE - cr[channel].rd_ptr + cr[channel].wr_ptr;
  else
    return 0;
}


inline int cbisFull(volatile struct channel_regs* cr, int channel) {
  int ret = 0;
  ret = (cr[channel].wr_ptr + 1) % (BUFFER_SIZE) == cr[channel].rd_ptr;
  return ret;
}


inline int cbRead(volatile struct channel_buffer *cb, volatile struct channel_regs* cr, int channel, struct param_set *pset) {
  unsigned int rptr = cr[channel].rd_ptr;
  unsigned int wptr = cr[channel].wr_ptr;
  /* check empty */
  if (wptr == rptr) {
    return 0;
  }
  /* read element */
  *pset = cb[channel].pset[rptr];
  /* move read pointer forward */
  cr[channel].rd_ptr = (rptr + 1) % (BUFFER_SIZE);
  return 1;
}

void cbWrite(volatile struct channel_buffer*, volatile struct channel_regs*, int, struct param_set*);
void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs*, int num);
#endif

#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

#define RING_SIZE   64
#define DAQ_RING_SIZE  2048 

/** @brief check if a channel buffer is empty
 *  @param cr channel register
 *  @param channel number of the channel
 */
inline int cbisEmpty(volatile struct channel_regs* cr, int channel) {
  return cr[channel].wr_ptr == cr[channel].rd_ptr;
}

/** @brief get the fill level  of a channel buffer
 *  @param cr channel register
 *  @param channel number of the channel
 */
inline int cbgetCount(volatile struct channel_regs* cr, int channel) {
  if (cr[channel].wr_ptr > cr[channel].rd_ptr)
    return cr[channel].wr_ptr - cr[channel].rd_ptr;
  else if (cr[channel].rd_ptr > cr[channel].wr_ptr)
    return BUFFER_SIZE - cr[channel].rd_ptr + cr[channel].wr_ptr;
  else
    return 0;
}


/** @brief check if a channel buffer is full
 *  @param cr channel register
 *  @param channel number of the channel
 */
inline int cbisFull(volatile struct channel_regs* cr, int channel) {
  int ret = 0;
  ret = (cr[channel].wr_ptr + 1) % (BUFFER_SIZE) == cr[channel].rd_ptr;
  return ret;
}

/** @brief read a parameter set from a channel buffer
 *  @param cb pointer to the first channel buffer
 *  @param cr pointer to the first channel register
 *  @param channel number of the channel
 *  @param pset the data from the buffer is written to this address
 */
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

struct msi {
   unsigned int  msg;
   unsigned int  adr;
   unsigned int  sel;
}; 

struct daq {
  unsigned int setvalue;
  unsigned int actvalue;
  unsigned int tmstmp_l;
  unsigned int tmstmp_h;
  unsigned int channel;
};

typedef uint32_t ring_pos_t;

struct message_buffer {
  ring_pos_t ring_head;
  ring_pos_t ring_tail;
  struct msi ring_data[RING_SIZE];
};

struct daq_buffer {
  ring_pos_t ring_head;
  ring_pos_t ring_tail;
  struct daq ring_data[DAQ_RING_SIZE];
};

void cbWrite(volatile struct channel_buffer *cb, volatile struct channel_regs*, int, struct param_set*);
void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs*, int num);
#endif

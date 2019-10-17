#ifndef __CB_H_
#define __CB_H_

#include <fg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RING_SIZE   64
#define DAQ_RING_SIZE  2048
//#define DAQ_RING_SIZE 512
//#define DAQ_RING_SIZE  1024
/** @brief check if a channel buffer is empty
 *  @param cr channel register
 *  @param channel number of the channel
 */
static
inline int cbisEmpty(volatile struct channel_regs* cr, int channel) {
  return cr[channel].wr_ptr == cr[channel].rd_ptr;
}

/** @brief get the fill level  of a channel buffer
 *  @param cr channel register
 *  @param channel number of the channel
 */
static
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
static inline
int cbRead(volatile struct channel_buffer *cb, volatile struct channel_regs* cr, int channel, struct param_set *pset) {
  unsigned int rptr = cr[channel].rd_ptr;
  unsigned int wptr = cr[channel].wr_ptr;
  /* check empty */
  if (wptr == rptr) {
    return 0;
  }
  /* read element */
#ifdef __lm32__
  *pset = cb[channel].pset[rptr];
#else
  //TODO Workaround, I don't know why yet!
  *pset = *((struct param_set*) &(cb[channel].pset[rptr]));
#endif
  /* move read pointer forward */
  cr[channel].rd_ptr = (rptr + 1) % (BUFFER_SIZE);
  return 1;
}
//#pragma pack(push, 1)
struct msi {
   uint32_t  msg;
   uint32_t  adr;
   uint32_t  sel;
} PACKED_SIZE;

struct daq {
  uint32_t   setvalue;
  uint32_t   actvalue;
  uint32_t   tmstmp_l;
  uint32_t   tmstmp_h;
  FG_MACRO_T fgMacro;
} PACKED_SIZE;

typedef uint32_t ring_pos_t;

struct message_buffer {
  ring_pos_t ring_head;
  ring_pos_t ring_tail;
  struct msi ring_data[RING_SIZE];
} PACKED_SIZE;

struct daq_buffer {
  ring_pos_t ring_head;
  ring_pos_t ring_tail;
  struct daq ring_data[DAQ_RING_SIZE];
} PACKED_SIZE;
//#pragma pack(pop)

void cbWrite(volatile struct channel_buffer *cb, volatile struct channel_regs*, int, struct param_set*);
void cbDump(volatile struct channel_buffer *cb, volatile struct channel_regs*, int num);
int add_msg(volatile struct message_buffer *mb, int queue, struct msi m);
struct msi remove_msg(volatile struct message_buffer *mb, int queue);
void add_daq_msg(volatile struct daq_buffer *db, struct daq d);
int has_msg(volatile struct message_buffer *mb, int queue);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif

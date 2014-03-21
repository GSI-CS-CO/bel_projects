#ifndef __FG_H_
#define __FG_H_

#include <stdint.h>

#define   MAX_FG_DEVICES  12
#define   MAX_SCU_SLAVES  12
#define   MAX_FG_PER_SLAVE 2
#define   BUFFER_SIZE 10

struct fg_dev {
  unsigned int dev_number;
  unsigned int version;
  unsigned int offset;
  char running;
  char timeout;
  int rampcnt;
  int endvalue; /* ramp value in case of timeout */
  struct scu_slave *slave;
};

struct scu_slave {
  struct scu_bus *bus;
  uint64_t unique_id; /* onewire id */
  unsigned int version;
  unsigned int cid_group;
  unsigned int cid_sys;
  unsigned int slot; /* 1 to 12 */
  struct fg_dev devs[MAX_FG_PER_SLAVE + 1];
};

struct scu_bus {
  uint64_t unique_id; /* onewire id */
  struct scu_slave slaves[MAX_SCU_SLAVES + 1];
};

struct fg_list {
  struct fg_dev *devs[MAX_FG_DEVICES];
};

struct param_set {
  unsigned short coeff_a;
  unsigned short coeff_b;
  unsigned int coeff_c;
  unsigned int control; /* Bit 2..0   step
                               5..3   freq
                              11..6   shift_b
                              17..12  shift_a */
                           
};

struct circ_buffer {
  unsigned int wr_ptr;
  unsigned int rd_ptr;
  unsigned int size;
  struct param_set pset[BUFFER_SIZE];
};

#endif

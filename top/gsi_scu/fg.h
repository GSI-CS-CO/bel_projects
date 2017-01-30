#ifndef __FG_H_
#define __FG_H_

#include <stdint.h>
#include <scu_bus.h>

#define   MAX_FG_MACROS     256
#define   MAX_FG_CHANNELS   12
#define   MAX_FG_PER_SLAVE  2
#define   MAX_WB_FG_MACROS  1
#define   BUFFER_SIZE       121
#define   THRESHOLD         BUFFER_SIZE * 40 / 100
#define   OUTPUT_BITS       24
#define   MIL_EXT           1
#define   MAX_SIO3          MAX_SCU_SLAVES 
#define   IFK_MAX_ADR       255
#define   GRP_IFA8          321

struct fg_dev {
  unsigned int dev_number;
  unsigned int version;
  unsigned int offset;
  unsigned char running;
  unsigned char timeout;
  unsigned int rampcnt;
  unsigned int endvalue; /* ramp value in case of timeout */
  unsigned char enabled;
  struct scu_slave *slave;
};

struct scu_slave {
  struct scu_bus *bus;
  uint64_t unique_id; /* onewire id */
  unsigned int version;
  unsigned int cid_group;
  unsigned int cid_sys;
  unsigned int slot; /* 1 to 12 */
  unsigned int fg_ver;
  struct fg_dev devs[MAX_FG_PER_SLAVE + 1];
};

struct scu_bus {
  uint64_t unique_id; /* onewire id */
  struct scu_slave slaves[MAX_SCU_SLAVES + MIL_EXT + MAX_SIO3 + 1];
};

struct fg_list {
  struct fg_dev *devs[MAX_FG_MACROS + 1];
};

struct param_set {
  signed short coeff_a;
  signed short coeff_b;
  signed int coeff_c;
  unsigned int control; /* Bit 2..0   step
                               5..3   freq
                              11..6   shift_b
                              17..12  shift_a */                           
};

struct channel_regs {
  unsigned int wr_ptr;
  unsigned int rd_ptr;
  unsigned int mbx_slot;
  unsigned int macro_number;
  unsigned int ramp_count;
  unsigned int tag;
  unsigned int state; // meaning private to LM32
};

struct channel_buffer {
  struct param_set pset[BUFFER_SIZE];
};

int scan_scu_bus(struct scu_bus *bus, uint64_t id, volatile unsigned short *base_adr, volatile unsigned int *mil_base);
int scan_for_fgs(struct scu_bus *bus, uint32_t *fglist);
void init_buffers(struct channel_regs *cr, int channel, uint32_t *macro, volatile unsigned short* scub_base);

#endif

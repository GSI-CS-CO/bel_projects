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
#define   GRP_IFA8          24

#define FG_RUNNING    0x4
#define FG_ENABLED    0x2
#define FG_DREQ       0x8
#define DRQ_BIT       (1 << 10)
#define DEV_DRQ       (1 << 0)
#define DEV_STATE_IRQ (1 << 1)
#define DEV_BUS_SLOT  13
#define FC_CNTRL_WR   0x14 << 8
#define FC_COEFF_A_WR 0x15 << 8
#define FC_COEFF_B_WR 0x16 << 8
#define FC_SHIFT_WR   0x17 << 8
#define FC_START_L_WR 0x18 << 8
#define FC_START_H_WR 0x19 << 8
#define FC_CNTRL_RD   0xa0 << 8
#define FC_COEFF_A_RD 0xa1 << 8
#define FC_COEFF_B_RD 0xa2 << 8
#define FC_IRQ_STAT   0xc9 << 8
#define FC_IRQ_MSK    0x12 << 8
#define FC_IRQ_ACT_RD 0xa7 << 8
#define FC_IRQ_ACT_WR 0x21 << 8


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
void init_buffers(struct channel_regs *cr, int channel, uint32_t *macro, volatile unsigned short* scub_base, volatile unsigned int* devb_base);

#endif

#ifndef __FG_H_
#define __FG_H_

#include <stdint.h>
#include <scu_bus.h>

// 12 SIOs with dev busses and 1 mil extension
#define   MAX_FG_MACROS     256
#define   MAX_FG_CHANNELS   12
#define   MAX_FG_PER_SLAVE  2
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
#define MIL_EXT_SLOT  13
#define DEV_SIO       0x20
#define DEV_MIL_EXT   0x10
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
#define FC_IFAMODE_WR 0x60 << 8
#define FC_BLK_WR     0x6b << 8

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

void scan_scu_bus(volatile unsigned short *base_adr, volatile unsigned int *mil_base, uint32_t *fglist);
void init_buffers(struct channel_regs *cr, int channel, uint32_t *macro, volatile unsigned short* scub_base, volatile unsigned int* devb_base);

#endif

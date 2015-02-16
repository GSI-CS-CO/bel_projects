#include <fg.h>
#include <scu_bus.h>
#include <string.h>
#include <unistd.h>

#define SYS_CSCO    55
#define SYS_PBRF    42

#define GRP_ADDAC1  3
#define GRP_ADDAC2  38
#define GRP_DIOB    26
#define GRP_FIB_DDS 1

extern unsigned int* wb_fg_base;

int scan_scu_bus(struct scu_bus *bus, uint64_t id, volatile unsigned short *base_adr) {
  int i, j = 0;
  memset(bus->slaves, 0, sizeof(bus->slaves));
  bus->unique_id = id;

  for (i = 1; i <= MAX_SCU_SLAVES; i++) {
    if (base_adr[i * (1<<16) + 0x10] != 0xdead) {
      bus->slaves[j].unique_id = (uint64_t)(base_adr[i * (1<<16) + 0x40]) << 48;
      bus->slaves[j].unique_id |= (uint64_t)(base_adr[i * (1<<16) + 0x41]) << 32;
      bus->slaves[j].unique_id |= (uint64_t)(base_adr[i * (1<<16) + 0x42]) << 16;
      bus->slaves[j].unique_id |= (uint64_t)(base_adr[i * (1<<16) + 0x43]);

      bus->slaves[j].slot = i;
      bus->slaves[j].cid_group = base_adr[i * (1<<16) + CID_GROUP];
      bus->slaves[j].cid_sys = base_adr[i * (1<<16) + CID_SYS];
      bus->slaves[j].version = base_adr[i * (1<<16) + SLAVE_VERSION];
      j++; /* next found slave */
    }
  }
  bus->slaves[j].unique_id = 0; /* end of list */
  return j; /* return number of slaves found */
}

int scan_for_fgs(struct scu_bus *bus, struct fg_list *list, struct fg_dev *wbfg) {
  int i = 0, j = 0;
  
  while(bus->slaves[i].unique_id) {
    if (bus->slaves[i].cid_sys == SYS_CSCO || bus->slaves[i].cid_sys == SYS_PBRF) {
      if (bus->slaves[i].cid_group == GRP_ADDAC1 ||
          bus->slaves[i].cid_group == GRP_ADDAC2 ||
          bus->slaves[i].cid_group == GRP_DIOB) {
        /* two FGs */
        bus->slaves[i].devs[0].dev_number = 0x0;
        bus->slaves[i].devs[0].version = 0x1;
        bus->slaves[i].devs[0].offset = FG1_BASE;
        bus->slaves[i].devs[0].slave = &(bus->slaves[i]);
        if (j < MAX_FG_DEVICES) {
          list->devs[j] = &(bus->slaves[i].devs[0]);j++;
        }
        bus->slaves[i].devs[1].dev_number = 0x1;
        bus->slaves[i].devs[1].version = 0x1;
        bus->slaves[i].devs[1].offset = FG2_BASE;
        bus->slaves[i].devs[1].slave = &(bus->slaves[i]);
        if (j < MAX_FG_DEVICES) {
          list->devs[j] = &(bus->slaves[i].devs[1]);j++;
        }
        
      } else if (bus->slaves[i].cid_group == GRP_FIB_DDS) { /* FIB */
        /* one FG */
        bus->slaves[i].devs[0].dev_number = 0x0;
        bus->slaves[i].devs[0].version = 0x1;
        bus->slaves[i].devs[0].offset = FG1_BASE;
        bus->slaves[i].devs[0].slave = &(bus->slaves[i]);
        if (j < MAX_FG_DEVICES) {
          list->devs[j] = &(bus->slaves[i].devs[0]);j++;
        }
      }
    }
    i++;
  }

  /* special solution for RF group, wb fg in scu */
  /* this fg dev is always last in the list */
  if (wb_fg_base && j < MAX_FG_DEVICES) {
    wbfg->dev_number = 0;
    wbfg->version = 0x2;
    wbfg->offset = (int)wb_fg_base;
    list->devs[j] = wbfg;
    j++;
  }

  list->devs[j] = 0;
  return j; //return number of found fgs
}

void init_buffers(struct circ_buffer *buf) {
  int i;
  for (i = 0; i < MAX_FG_DEVICES; i++) {
    buf[i].wr_ptr = 0;
    buf[i].rd_ptr = 0;
    buf[i].size = BUFFER_SIZE + 1; 
  }
}

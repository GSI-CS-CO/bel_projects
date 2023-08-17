/*
 * Onewire generic interface
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#ifndef __BATHOS_W1_H__
#define __BATHOS_W1_H__

#include <stdint.h>

#define W1_MAX_DEVICES 8 /* we have no alloc */

struct w1_dev {
	struct w1_bus *bus;
	uint64_t rom;
};

static inline int w1_class(struct w1_dev *dev)
{
	return dev->rom & 0xff;
}


struct w1_bus {
	unsigned long detail; /* gpio bit or whatever (driver-specific) */
	struct w1_dev devs[W1_MAX_DEVICES];
};

/*
 * The low-level driver is based on this set of operations. We expect to
 * only have one set of such operations in each build. (i.e., no bus-specific
 * operations, to keep the thing simple and small).
 */
struct w1_ops {
	int (*reset)(struct w1_bus *bus);	/* returns 1 on "present" */
	int (*read_bit)(struct w1_bus *bus);
	void (*write_bit)(struct w1_bus *bus, int bit);
};

/* Library functions */
extern int w1_scan_bus(struct w1_bus *bus);
extern void w1_write_byte(struct w1_bus *bus, int byte);
extern int w1_read_byte(struct w1_bus *bus);
extern void w1_match_rom(struct w1_dev *dev);

#define W1_CMD_SEARCH_ROM	0xf0
#define W1_CMD_READ_ROM		0x33
#define W1_CMD_MATCH_ROM	0x55
#define W1_CMD_SKIP_ROM		0xcc
#define W1_CMD_ASEARCH		0xec

/* commands for specific families */
#define W1_CMDT_CONVERT		0x44
#define W1_CMDT_W_SPAD		0x4e
#define W1_CMDT_R_SPAD		0xbe
#define W1_CMDT_CP_SPAD		0x48
#define W1_CMDT_RECALL		0xb8
#define W1_CMDT_R_PS		0xb4
/* EEPROM DS28EC20 */
#define W1_CMDR_W_SPAD		0x0f
#define W1_CMDR_R_SPAD		0xaa
#define W1_CMDR_C_SPAD		0x55
#define W1_CMDR_R_MEMORY	0xf0
#define W1_CMDR_EXT_R_MEMORY	0xa5

/* Temperature conversion takes time: by default wait, but allow flags */
#define W1_FLAG_NOWAIT		0x01	/* start conversion only*/
#define W1_FLAG_COLLECT		0x02	/* don't start, just get output */

/* These functions are dev-specific */
extern int32_t w1_read_temp(struct w1_dev *dev, unsigned long flags);
extern int w1_read_eeprom(struct w1_dev *dev,
			  int offset, uint8_t *buffer, int blen);
extern int w1_write_eeprom(struct w1_dev *dev,
			   int offset, const uint8_t *buffer, int blen);
extern int w1_erase_eeprom(struct w1_dev *dev, int offset, int blen);

/* These are generic, using the first suitable device in the bus */
extern int32_t w1_read_temp_bus(struct w1_bus *bus, unsigned long flags);
extern int w1_read_eeprom_bus(struct w1_bus *bus,
			    int offset, uint8_t *buffer, int blen);
extern int w1_write_eeprom_bus(struct w1_bus *bus,
			     int offset, const uint8_t *buffer, int blen);
extern int w1_erase_eeprom_bus(struct w1_bus *bus, int offset, int blen);

extern struct w1_ops wrpc_w1_ops;
extern struct w1_bus wrpc_w1_bus;
extern void wrpc_w1_init(void);

#endif /* __BATHOS_W1_H__ */

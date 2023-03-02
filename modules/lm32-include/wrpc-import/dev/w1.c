/*
 * Onewire generic interface
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#include <string.h>
#include <w1.h>
#include <unistd.h>

static const struct w1_ops *ops = &wrpc_w1_ops; /* local shorter name */

void w1_write_byte(struct w1_bus *bus, int byte)
{
	int i;

	for (i = 1; i < 0x100; i <<= 1)
		ops->write_bit(bus, byte & i ? 1 : 0);
}

int w1_read_byte(struct w1_bus *bus)
{
	int i, res = 0;

	for (i = 1; i < 0x100; i <<= 1)
		res |= ops->read_bit(bus) ? i : 0;
	usleep(100); /* inter-byte, for my eyes only */
	return res;

}

/* scan_bus requires this di-bit helper */
enum __bits {B_0, B_1, B_BOTH};

/* return what we get, select it if unambiguous or the one passed */
static enum __bits __get_dibit(struct w1_bus *bus, int select)
{
	int a, b;

	a = ops->read_bit(bus);
	b = ops->read_bit(bus);
	if (a != b) {
		ops->write_bit(bus, a);
		return a ? B_1 : B_0;
	}
	ops->write_bit(bus, select);
	return B_BOTH;
}

/*
 * This identifies one. Returns 0 if not found, -1 on error. The current mask
 * is used to return the conflicts we found: on each conflict, we follow
 *  what's already in our id->rom, but remember it for later scans.
 */
static int __w1_scan_one(struct w1_bus *bus, uint64_t *rom, uint64_t *cmask)
{
	uint64_t mask;
	int select;
	enum __bits b;

	if (ops->reset(bus) != 1)
		return -1;
	w1_write_byte(bus, 0xf0); /* search rom */

	/*
	 * Send all bits we have (initially, zero).
	 * On a conflict, follow what we have in rom and possibly mark it.
	 */
	*cmask = 0;
	for (mask = 1; mask; mask <<= 1) {
		select = *rom & mask;
		b = __get_dibit(bus, select);

		switch(b) {
		case B_1:
			*rom |= mask;
		case B_0:
			break;
		case B_BOTH:
			/* if we follow 1, it's resolved, else mark it */
			if (!select)
				*cmask |= mask;
			break;
		}
	}
	return 0;
}

int w1_scan_bus(struct w1_bus *bus)
{
	uint64_t mask;
	uint64_t cmask; /* current */
	struct w1_dev *d;
	int i;

	memset(bus->devs, 0, sizeof(bus->devs));

	if (!ops->reset)
		return 0; /* no devices */
	for (i = 0, cmask = 0; i < W1_MAX_DEVICES; i++) {
		d = bus->devs + i;
		d->bus = bus;

		if (i) { /* Not first: scan conflicts and resolve last */
			d->rom = bus->devs[i-1].rom;
			for (mask = (1ULL<<63); mask; mask >>= 1) {
				/*
				 * Warning: lm32 compiter treats as signed!
				 *
				 * Even if mask is uint64_t, the shift in the
				 * for loop above is signed, so fix it.
				 * I prefer not to change the loop, as the
				 * code is in use elsewhere and I prefer to
				 * keep differences to a minimum
				 */
				if (mask & (1ULL<<62))
					mask = (1ULL<<62);

				if (cmask & mask)
					break;
				d->rom &= ~mask;
			}
			if (!mask) {
				/* no conflicts to solve: done */
				return i;
			}
			d->rom |= mask; /* we'll reply 1 next loop */
			cmask &= ~mask;
		}
		if (__w1_scan_one(bus, &d->rom, &cmask)) {
			/* error on this one */
			return i;
		}
	}
	return i;
}

void w1_match_rom(struct w1_dev *dev)
{
	int i;

	ops->reset(dev->bus);
	w1_write_byte(dev->bus, W1_CMD_MATCH_ROM); /* match rom */
	for (i = 0; i < 64; i+=8) {
		w1_write_byte(dev->bus, (int)(dev->rom >> i) );
	}
}

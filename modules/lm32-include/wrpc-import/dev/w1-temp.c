/*
 * Temperature input for DS18S20 (family 0x10)
 * Alessandro Rubini, 2013 GNU GPL2 or later
 */
#include <wrc.h>
#include <w1.h>

int32_t w1_read_temp(struct w1_dev *dev, unsigned long flags)
{
	static uint8_t scratchpad[8];
	int class = w1_class(dev);
	int32_t res;
	int16_t cval;
	int i;

	/* The caller is expected to have checked the class. but still... */
	switch(class) {
	case 0x10: case 0x28: case 0x42:
		break; /* Supported, at least for temperature input */
	default:
		return 1<<31; /* very negative */
	}

	/* If so asked, jump over start-conversion and only collect result */
	if (flags & W1_FLAG_COLLECT)
		goto collect;

	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDT_CONVERT);

	/* If so asked, don't wait for the conversion to be over */
	if (flags & W1_FLAG_NOWAIT)
		return 0;

	while(wrpc_w1_ops.read_bit(dev->bus) == 0)
		;
collect:
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDT_R_SPAD);
	for (i = 0; i < sizeof(scratchpad); i++)
		scratchpad[i] = w1_read_byte(dev->bus);

	res = 0;
	cval = scratchpad[1] << 8 | scratchpad[0];

	switch(class) {
	case 0x10:
		/* 18S20: two bytes plus "count remain" value */
		res = (int32_t)cval << 15; /* 1 decimal points */
		res -= 0x4000; /* - 0.25 degrees */
		res |= scratchpad[6] << 12; /* 1/16th of degree each */
		break;

	case 0x28:
	case 0x42:
		/* 18B20 and DS28EA00: only the two bytes */
		res = (int32_t)cval << 12; /* 4 decimal points */
		break;
	}
	return res;
}

int32_t w1_read_temp_bus(struct w1_bus *bus, unsigned long flags)
{
	int i, class;

	for (i = 0; i < W1_MAX_DEVICES; i++) {
		class = w1_class(bus->devs + i);
		switch(class) {
		case 0x10: case 0x28: case 0x42:
			return w1_read_temp(bus->devs + i, flags);
		default:
			break;
		}
	}
	/* not found */
	return 1 << 31;
}

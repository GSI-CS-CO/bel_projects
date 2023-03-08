/*
 * Eeprom support (family 0x43)
 * Cesar Prados, Alessandro Rubini, 2013. GNU GPL2 or later
 */
#include "dev/w1.h"
#include "syscon.h" /* for usleep */

#define LSB_ADDR(X) ((X) & 0xFF)
#define MSB_ADDR(X) (((X) & 0xFF00)>>8)

static int w1_write_page(struct w1_dev *dev, int offset, const uint8_t *buffer,
			 int blen)
{
	int i, j, es;

	/* First, write scratchpad */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_W_SPAD);
	w1_write_byte(dev->bus, LSB_ADDR(offset));
	w1_write_byte(dev->bus, MSB_ADDR(offset));
	for(i = 0; i < blen; i++)
		w1_write_byte(dev->bus, buffer[i]);

	/* Then, read it back, and remember the return E/S */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_R_SPAD);
	if (w1_read_byte(dev->bus) != LSB_ADDR(offset))
		return -1;
	if (w1_read_byte(dev->bus) != MSB_ADDR(offset))
		return -2;
	es = w1_read_byte(dev->bus);
	for(i = 0; i < blen; i++) {
		j = w1_read_byte(dev->bus);
		if (j != buffer[i])
			return -3;
	}

	/* Finally, "copy scratchpad" to actually write */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_C_SPAD);
	w1_write_byte(dev->bus, LSB_ADDR(offset));
	w1_write_byte(dev->bus, MSB_ADDR(offset));
	w1_write_byte(dev->bus, es);
	usleep(10000); /* 10ms, in theory */

	/* Don't read back, as nothing useful is there (I get 0xf9, why?) */
	return blen;
}

static int w1_erase_page(struct w1_dev *dev, int offset, int blen)
{
	int i, j, es;

	/* First, write scratchpad */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_W_SPAD);
	w1_write_byte(dev->bus, LSB_ADDR(offset));
	w1_write_byte(dev->bus, MSB_ADDR(offset));
	for(i = 0; i < blen; i++)
		w1_write_byte(dev->bus, 0xFF);

	/* Then, read it back, and remember the return E/S */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_R_SPAD);
	if (w1_read_byte(dev->bus) != LSB_ADDR(offset))
		return -1;
	if (w1_read_byte(dev->bus) != MSB_ADDR(offset))
		return -2;
	es = w1_read_byte(dev->bus);
	for(i = 0; i < blen; i++) {
		j = w1_read_byte(dev->bus);
		if (j != 0xFF)
			return -3;
	}

	/* Finally, "copy scratchpad" to actually write */
	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_C_SPAD);
	w1_write_byte(dev->bus, LSB_ADDR(offset));
	w1_write_byte(dev->bus, MSB_ADDR(offset));
	w1_write_byte(dev->bus, es);
	usleep(10000); /* 10ms, in theory */

	/* Don't read back, as nothing useful is there (I get 0xf9, why?) */
	return blen;
}

int w1_write_eeprom(struct w1_dev *dev, int offset, const uint8_t *buffer,
		    int blen)
{
	int i, page, endpage;
	int ret = 0;

	/* Split the write into several page-local writes */
	page = offset / 32;
	endpage = (offset + blen - 1) / 32;

	/* Traling part of first page */
	if (offset % 32) {
		if (endpage != page)
			i = 32 - (offset % 32);
		else
			i = blen;
		ret += w1_write_page(dev, offset, buffer, i);
		if (ret < 0)
			return ret;
		buffer += i;
		offset += i;
		blen -= i;
	}

	/* Whole pages and leading part of last page */
	while (blen > 0 ) {
		i = blen;
		if (blen > 32)
			i = 32;
		i = w1_write_page(dev, offset, buffer, i);
		if (i < 0)
			return i;
		ret += i;
		buffer += 32;
		offset += 32;
		blen -= 32;
	}
	return ret;
}

int w1_read_eeprom(struct w1_dev *dev, int offset, uint8_t *buffer, int blen)
{
	int i;

	w1_match_rom(dev);
	w1_write_byte(dev->bus, W1_CMDR_R_MEMORY);

	w1_write_byte(dev->bus, LSB_ADDR(offset));
	w1_write_byte(dev->bus, MSB_ADDR(offset));

	/* There is no page-size limit in reading, just go on at will */
	for(i = 0; i < blen; i++)
		buffer[i] = w1_read_byte(dev->bus);

	return blen;
}

int w1_erase_eeprom(struct w1_dev *dev, int offset, int blen)
{
	int i, page, endpage;
	int ret = 0;

	/* Split the write into several page-local writes */
	page = offset / 32;
	endpage = (offset + blen - 1) / 32;

	/* Traling part of first page */
	if (offset % 32) {
		if (endpage != page)
			i = 32 - (offset % 32);
		else
			i = blen;
		ret += w1_erase_page(dev, offset, i);
		if (ret < 0)
			return ret;
		offset += i;
		blen -= i;
	}

	/* Whole pages and leading part of last page */
	while (blen > 0 ) {
		i = blen;
		if (blen > 32)
			i = 32;
		i = w1_erase_page(dev, offset, i);
		if (i < 0)
			return i;
		ret += i;
		offset += 32;
		blen -= 32;
	}
	return ret;
}

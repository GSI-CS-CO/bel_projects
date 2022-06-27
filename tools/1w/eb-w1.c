/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Author: Wesley W. Terpstra <w.terpstra@gsi.de>
 *         Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <string.h>
#include <etherbone.h>
#include <w1.h>
#include <hw/sockit_owm_regs.h>

extern eb_address_t BASE_ONEWIRE;
extern eb_device_t  device;

static inline uint32_t __wait_cycle(eb_device_t device)
{
	eb_data_t data;
	
	do {
	    eb_device_read(device, BASE_ONEWIRE, EB_DATA32|EB_BIG_ENDIAN, &data, 0, 0);
        } while (data & SOCKIT_OWM_CTL_CYC_MSK);

	return data;
}

static int w1_reset(struct w1_bus *bus)
{
	int portnum = bus->detail;
	uint32_t reg;
	
	eb_data_t data =  (portnum << SOCKIT_OWM_CTL_SEL_OFST)
			    | (SOCKIT_OWM_CTL_CYC_MSK)
			    | (SOCKIT_OWM_CTL_RST_MSK);
	eb_device_write(device, BASE_ONEWIRE, EB_DATA32|EB_BIG_ENDIAN, data, 0, 0);
	reg = __wait_cycle(device);
	
	/* return presence-detect pulse (1 if true) */
	return (reg & SOCKIT_OWM_CTL_DAT_MSK) ? 0 : 1;
}

static int w1_read_bit(struct w1_bus *bus)
{
	int portnum = bus->detail;
	uint32_t reg;

	eb_data_t data = (portnum << SOCKIT_OWM_CTL_SEL_OFST)
			    | (SOCKIT_OWM_CTL_CYC_MSK)
			    | (SOCKIT_OWM_CTL_DAT_MSK);
        eb_device_write(device, BASE_ONEWIRE, EB_DATA32|EB_BIG_ENDIAN, data, 0, 0);
	reg = __wait_cycle(device);
	
	return (reg & SOCKIT_OWM_CTL_DAT_MSK) ? 1 : 0;
}

static void w1_write_bit(struct w1_bus *bus, int bit)
{
	int portnum = bus->detail;
	
	eb_data_t data = (portnum << SOCKIT_OWM_CTL_SEL_OFST)
			    | (SOCKIT_OWM_CTL_CYC_MSK)
			    | (bit ? SOCKIT_OWM_CTL_DAT_MSK : 0);
	eb_device_write(device, BASE_ONEWIRE, EB_DATA32|EB_BIG_ENDIAN, data, 0, 0);
	__wait_cycle(device);
}

#define WB_CLOCK   62500000
#define CLK_DIV_NOR (WB_CLOCK / 200000 - 1)	/* normal mode */
#define CLK_DIV_OVD (WB_CLOCK / 1000000 - 1)	/* overdrive mode (not used) */
void wrpc_w1_init(void)
{
    eb_data_t data = ((CLK_DIV_NOR & SOCKIT_OWM_CDR_N_MSK) |
                      ((CLK_DIV_OVD << SOCKIT_OWM_CDR_O_OFST) &
                       SOCKIT_OWM_CDR_O_MSK));
    eb_device_write(device, BASE_ONEWIRE+4, EB_DATA32|EB_BIG_ENDIAN, data, 0, 0);
}

struct w1_ops wrpc_w1_ops = {
	.reset = w1_reset,
	.read_bit = w1_read_bit,
	.write_bit = w1_write_bit,
};

struct w1_bus wrpc_w1_bus;

/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2012 - 2015 CERN (www.cern.ch)
 * Author: Grzegorz Daniluk <grzegorz.daniluk@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#ifndef __SYSCON_H
#define __SYSCON_H

#include <inttypes.h>
#include <sys/types.h>
#include "board.h"

uint32_t timer_get_tics(void);
void timer_delay(uint32_t tics);

/* The following ones come from the kernel, but simplified */
#ifndef time_after
#define time_after(a,b)		\
	((long)(b) - (long)(a) < 0)
#define time_before(a,b)	time_after(b,a)
#define time_after_eq(a,b)	\
	 ((long)(a) - (long)(b) >= 0)
#define time_before_eq(a,b)	time_after_eq(b,a)
#endif

/* This can be used for up to 2^32 / TICS_PER_SECONDS == 42 seconds in wrs */
static inline void timer_delay_ms(int ms)
{
	timer_delay(ms * TICS_PER_SECOND / 1000);
}

/* usleep.c */
//extern void usleep_init(void);
//#ifndef unix
//extern int usleep(unsigned usec);
//#endif


#ifdef CONFIG_WR_NODE

#undef PACKED /* if we already included a regs file, we'd get a warning */
#include <hw/wrc_syscon_regs.h>

struct SYSCON_WB {
	uint32_t RSTR;		/*Syscon Reset Register */
	uint32_t GPSR;		/*GPIO Set/Readback Register */
	uint32_t GPCR;		/*GPIO Clear Register */
	uint32_t HWFR;		/*Hardware Feature Register */
	uint32_t HWIR;		/*Hardware Info Register */
	uint32_t SDBFS;		/*Flash SDBFS Info Register */
	uint32_t TCR;		/*Timer Control Register */
	uint32_t TVR;		/*Timer Counter Value Register */
	uint32_t DIAG_INFO;
	uint32_t DIAG_NW;
	uint32_t DIAG_CR;
	uint32_t DIAG_DAT;
};

/* GPIO pins */

extern const struct gpio_pin pin_sysc_led_link;
extern const struct gpio_pin pin_sysc_led_stat;
extern const struct gpio_pin pin_sysc_btn1;
extern const struct gpio_pin pin_sysc_btn2;
extern const struct gpio_pin pin_sysc_sfp_det;
extern const struct gpio_pin pin_sysc_spi_sclk;
extern const struct gpio_pin pin_sysc_spi_ncs;
extern const struct gpio_pin pin_sysc_spi_mosi;
extern const struct gpio_pin pin_sysc_spi_miso;
extern const struct gpio_pin pin_sysc_fmc_scl;
extern const struct gpio_pin pin_sysc_fmc_sda;
extern const struct gpio_pin pin_sysc_sfp_scl;
extern const struct gpio_pin pin_sysc_sfp_sda;
extern const struct gpio_pin pin_sysc_net_rst;

extern const struct i2c_bus dev_i2c_fmc;
extern const struct i2c_bus dev_i2c_sfp;
extern struct spi_flash_device wrc_flash_dev;

#define FMC_I2C_DELAY 15
#define SFP_I2C_DELAY 300

void timer_init(uint32_t enable);

extern struct spi_bus spi_wrc_flash;
extern struct spi_flash_device wrc_flash_dev;

#define HW_NAME_LENGTH 5 /* 4 letters + '\0' */
void get_hw_name(char *str);
void get_storage_info(int *memtype, uint32_t *sdbfs_baddr, uint32_t *blocksize);
int sysc_get_memsize(void);

#define DIAG_RW_BANK 0
#define DIAG_RO_BANK 1
void diag_read_info(uint32_t *id, uint32_t *ver, uint32_t *nrw, uint32_t *nro);
int diag_read_word(uint32_t adr, int bank, uint32_t *val);
int diag_write_word(uint32_t adr, uint32_t val);

void net_rst(void);

#endif /* CONFIG_WR_NODE */
#endif

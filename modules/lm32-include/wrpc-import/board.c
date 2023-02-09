#include "board.h"
#include "wrc-debug.h"
#include "dev/bb_spi.h"
#include "dev/bb_i2c.h"
#include "dev/w1.h"
#include "dev/spi_flash.h"
#include "dev/i2c_eeprom.h"
#include "dev/syscon.h"
#include "dev/endpoint.h"
#include "storage.h"

static struct i2c_bus i2c_wrc_eeprom;
static struct i2c_eeprom_device wrc_eeprom_dev;

int wrc_board_early_init()
{
	int memtype;
	uint32_t sdbfs_entry;
	uint32_t sector_size;

	if (HAS_W1_EEPROM
	    && storage_w1eeprom_create(&wrc_storage_dev, &wrpc_w1_bus) == 0) {
		/* Found.  */
	}
	else if (EEPROM_STORAGE) {
		/* EEPROM support */
		bb_i2c_create(&i2c_wrc_eeprom,
			      &pin_sysc_fmc_scl,
			      &pin_sysc_fmc_sda);
		bb_i2c_init(&i2c_wrc_eeprom);

		i2c_eeprom_create(&wrc_eeprom_dev, &i2c_wrc_eeprom, FMC_EEPROM_ADR, 2);
		storage_i2ceeprom_create( &wrc_storage_dev, &wrc_eeprom_dev );
	} else {
		/* Flash support */
		/*
		 * declare GPIO pins and configure their directions for bit-banging SPI
		 * limit SPI speed to 10MHz by setting bit_delay = CPU_CLOCK / 10^6
		 */
		bb_spi_create( &spi_wrc_flash,
			&pin_sysc_spi_ncs,
			&pin_sysc_spi_mosi,
			&pin_sysc_spi_miso,
			&pin_sysc_spi_sclk, CPU_CLOCK / 10000000 );

		spi_wrc_flash.rd_falling_edge = 1;

		/*
		 * Read from gateware info about used memory. Currently only base
		 * address and sector size for memtype flash is supported.
		 */
		get_storage_info(&memtype, &sdbfs_entry, &sector_size);

		/*
		 * Initialize SPI flash and read its ID
		 */
		spi_flash_create( &wrc_flash_dev, &spi_wrc_flash, sector_size, sdbfs_entry);

		/*
		 * Initialize storage subsystem with newly created SPI Flash
		 */
		storage_spiflash_create( &wrc_storage_dev, &wrc_flash_dev );
	}

	/*
	 * Mount SDBFS filesystem from storage.
	 */
	storage_mount( &wrc_storage_dev );

	return 0;
}

static int board_get_persistent_mac(uint8_t *mac)
{
	int i;
	struct w1_dev *d;
	
	/* Try from SDB */
	if (storage_get_persistent_mac(0, mac) == 0)
		return 0;

	/* Get from one-wire (derived from unique id) */
	if (HAS_W1) {
		for (i = 0; i < W1_MAX_DEVICES; i++) {
			d = wrpc_w1_bus.devs + i;
			if (d->rom) {
				mac[0] = 0x22;
				mac[1] = 0x33;
				mac[2] = 0xff & (d->rom >> 32);
				mac[3] = 0xff & (d->rom >> 24);
				mac[4] = 0xff & (d->rom >> 16);
				mac[5] = 0xff & (d->rom >> 8);
				return 0;
			}
		}
	}

	/* Not found */
	return -1;
}

int wrc_board_init()
{
	uint8_t mac_addr[6];
	/*
	 * Try reading MAC addr stored in flash
	 */
	if (board_get_persistent_mac(mac_addr) < 0) {
		board_dbg("Failed to get MAC address from the flash. Using fallback address.\n");
		mac_addr[0] = 0x22;
		mac_addr[1] = 0x33;
		mac_addr[2] = 0x44;
		mac_addr[3] = 0x55;
		mac_addr[4] = 0x66;
		mac_addr[5] = 0x77;
	}
	ep_set_mac_addr(&wrc_endpoint_dev, mac_addr);
	ep_pfilter_init_default(&wrc_endpoint_dev);

	return 0;
}

int wrc_board_create_tasks()
{
    return 0;
}

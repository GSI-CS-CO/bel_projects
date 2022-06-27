/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2013 CERN (www.cern.ch)
 * Author: Wesley W. Terpstra <w.terpstra@gsi.de>
 * Author: Alessandro Rubini <rubini@gnudd.com>
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <etherbone.h>

#include <w1.h>

#define W1_VENDOR	0xce42		/* CERN */
#define W1_DEVICE	0x779c5443	/* WR-Periph-1Wire */

char *prgname;
int verbose;

eb_address_t BASE_ONEWIRE;
eb_device_t device;
extern struct w1_bus wrpc_w1_bus;


static int write_w1(int w1base, int w1len)
{
	struct w1_dev *d;
	uint8_t buf[w1len];
	int i;

	wrpc_w1_init();
	w1_scan_bus(&wrpc_w1_bus);

	if (verbose) { /* code borrowed from dev/w1.c -- "w1" shell command */
		for (i = 0; i < W1_MAX_DEVICES; i++) {
			d = wrpc_w1_bus.devs + i;
			if (d->rom)
				fprintf(stderr, "device %i: %08x%08x\n", i,
					(int)(d->rom >> 32), (int)d->rom);
		}
	}

	if (verbose) {
		fprintf(stderr, "Writing device offset %i (0x%x), len %i\n",
			w1base, w1base, w1len);
	}

	if (isatty(fileno(stdin)))
		fprintf(stderr, "Reading from stdin, please type the data\n");
	i = fread(buf, 1, w1len, stdin);
	if (i != w1len) {
		fprintf(stderr, "%s: read error (%i, expeted %i)\n", prgname,
			i, w1len);
		return 1;
	}
	i = w1_write_eeprom_bus(&wrpc_w1_bus, w1base, buf, w1len);
	if (i != w1len) {
		fprintf(stderr, "Tried to write %i bytes, retval %i\n",
			w1len, i);
		return 1;
	}
	return 0;
}

/*
 * What follows is mostly generic, should be librarized in a way
 */


static int help(void)
{
	fprintf(stderr, "%s: Use: \"%s [-v] [-i <index>] <device> <addr> <len>\n",
		prgname, prgname);
	return 1;
}

static void die(const char *reason, eb_status_t status)
{
	fprintf(stderr, "%s: %s: %s\n", prgname, reason, eb_status(status));
	exit(1);
}

int main(int argc, char **argv)
{
	int c, i;
	eb_status_t status;
	eb_socket_t socket;
	struct sdb_device sdb[10];;
	char *tail;

	prgname = argv[0];
	i = -1;

	while ((c = getopt(argc, argv, "i:v")) != -1) {
		switch(c) {
		case 'i':
			i = strtol(optarg, &tail, 0);
			if (*tail != 0) {
				fprintf(stderr, "Specify a proper number, not '%s'!\n", optarg);
				exit(1);
			}
			break;
		case 'v':
			verbose++;
			break;
		default:
			exit(help());
		}
	}
	if (optind != argc - 3)
		exit(help());

	if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
		die("eb_socket_open", status);

	if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
		die(argv[optind], status);
	
	/* Find the W1 device */
	c = sizeof(sdb) / sizeof(struct sdb_device);
	if ((status = eb_sdb_find_by_identity(device, W1_VENDOR, W1_DEVICE, &sdb[0], &c)) != EB_OK)
		die("eb_sdb_find_by_identity", status);
	
	if (i == -1) {
		if (c > 1) {
			fprintf(stderr, "Found %d 1wire controllers on that device; pick one with -i #\n", c);
			exit(1);
		} else {
			i = 0;
		}
	}
	if (i >= c) {
		fprintf(stderr, "Could not find 1wire controller #%d on that device (%d total)\n", i, c);
		exit(1);
	}
	
	BASE_ONEWIRE = sdb[i].sdb_component.addr_first;
	
	return write_w1(atoi(argv[optind + 1]), atoi(argv[optind + 2]));
}

/** @file eb-flash.c
 *  @brief Program a flash device using Etherbone.
 *
 *  Copyright (C) 2013 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 *  A complete skeleton of an application using the Etherbone library.
 *
 *  @author Wesley W. Terpstra <w.terpstra@gsi.de>
 *
 *******************************************************************************
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

#define _POSIX_C_SOURCE 200112L /* strtoull */

#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>

#define GSI_ID   0x651
#define FLASH_ID 0x5cf12a1c

#define MAX_SECTOR_SIZE	4194304

static const char* program;
static const char* device_address;
static const char* firmware;

static eb_address_t address;
static int address_set;
static eb_address_t erase_address;
static int erase_address_set;
static eb_width_t address_width;
static eb_width_t data_width;
static eb_format_t endian;
static eb_address_t sector_size;
static eb_address_t page_size;
static long wait_us;
static int cycles_per_poll;
static int retries;
static int full;
static int probe;
static int verify;
static int invert;
static int verbose;
static int quiet;
static int old_erase;
static int erase_only;

static eb_address_t firmware_length;
static eb_format_t format;
static eb_data_t ffs;
static eb_data_t mask_0f;
static uint8_t* erase_bitmap;
static int erase_bitmap_size;
static FILE* firmware_f;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port> <firmware>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -t <address>   target flash base address   (sdb-auto-located)\n");
  fprintf(stderr, "  -e <address>   erase control register address (end-of-device)\n");
  fprintf(stderr, "  -a <width>     acceptable address bus widths     (8/16/32/64)\n");
  fprintf(stderr, "  -d <width>     acceptable data bus widths        (8/16/32/64)\n");
  fprintf(stderr, "  -b             big-endian operation                    (auto)\n");
  fprintf(stderr, "  -l             little-endian operation                 (auto)\n");
  fprintf(stderr, "  -i <0/1>       invert bit order of bytes in firmware   (auto)\n");
  fprintf(stderr, "  -s <bytes>     flash sector size                       (auto)\n");
  fprintf(stderr, "  -x <bytes>     flash page size                          (256)\n");
  fprintf(stderr, "  -w <seconds>   poll interval while erasing              (0.1)\n");
  fprintf(stderr, "  -c <cycles>    number of cycles per bridge access         (4)\n");
  fprintf(stderr, "  -r <retries>   number of times to attempt autonegotiation (3)\n");
  fprintf(stderr, "  -f             full; skip quick scan and erase everything\n");
  fprintf(stderr, "  -p             disable self-describing wishbone device probe\n");
  fprintf(stderr, "  -n             do not verify contents after programming\n");
  fprintf(stderr, "  -z             simply erase everything\n");
  fprintf(stderr, "  -v             verbose operation\n");
  fprintf(stderr, "  -q             quiet: do not display warnings\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <w.terpstra@gsi.de>\n");
}

static int bitmask_get(int offset) {
  return (erase_bitmap[offset/8] >> (offset&7)) & 1;
}

static void bitmask_set(int offset, int value) {
  uint8_t* val = &erase_bitmap[offset/8];
  int bit = 1 << (offset&7);
  *val = (*val&~bit)|(value?bit:0);
}

static eb_data_t flip_bits(eb_data_t x) {
  eb_data_t mask;
  if (!invert) return x;
  
  /* 0x0F0F */
  mask = mask_0f;;
  x = ((x >> 4) & mask) | ((x & mask) << 4);
  
  /* 0x0F0F -> 0x3333 */
  mask ^= (mask << 2);
  x = ((x >> 2) & mask) | ((x & mask) << 2);
  
  /* 0x3333 -> 0x5555 */
  mask ^= (mask << 1);
  x = ((x >> 1) & mask) | ((x & mask) << 1);
  
  return x;
}

static eb_status_t erase_sector(eb_device_t device, eb_address_t sector) {
  eb_cycle_t cycle;
  eb_format_t control;
  eb_status_t status;
  
  control = (format&EB_ENDIAN_MASK) | EB_DATA32;
  
  if (old_erase)
    return eb_device_write(device, erase_address, control, sector, 0, eb_block);
  
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return status;
  eb_cycle_write(cycle, erase_address, control, 0x6);        /* write enable */
  eb_cycle_write(cycle, erase_address, control, 0x80000000); /* execute */
  eb_cycle_write(cycle, erase_address, control, 0xD8);       /* sector erase */
  if (((erase_address-address) >> 24) != 0) 
    eb_cycle_write(cycle, erase_address, control, (sector >> 24) & 0xFF);
  eb_cycle_write(cycle, erase_address, control, (sector >> 16) & 0xFF);
  eb_cycle_write(cycle, erase_address, control, (sector >>  8) & 0xFF);
  eb_cycle_write(cycle, erase_address, control, (sector >>  0) & 0xFF);
  eb_cycle_write(cycle, erase_address, control, 0x80000000); /* execute */
  return eb_cycle_close(cycle);
}

static void wait_for_busy(eb_device_t device, long interval) {
  long delay, used;
  eb_status_t status;
  eb_data_t result;
  
  do {
    /* Sleep as requested */
    for (delay = interval; delay > 0; delay -= used) {
      used = eb_socket_run(eb_device_socket(device), delay);
    }
    
    result = 0;
    status = eb_device_read(device, erase_address, (format&EB_ENDIAN_MASK)|EB_DATA32, &result, 0, eb_block);
    if (status != EB_OK && status != EB_SEGFAULT) {
      fprintf(stderr, "%s: polling erase status failed: %s\n", program, eb_status(status));
      exit(1);
    }
  } while (status == EB_SEGFAULT);
}

static void find_sector_size(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  eb_address_t* sector_size = (eb_address_t*)user;
  eb_address_t result;
  
  if (status != EB_OK) {
    fprintf(stderr, "%s: pattern read failed: %s\n", program, eb_status(status));
    exit(1);
  }
  
  result = page_size;
  for (; op != EB_NULL; op = eb_operation_next(op)) {
    if (eb_operation_had_error(op)) {
      fprintf(stderr, "%s: wishbone segfault reading %s %s bits from address 0x%"EB_ADDR_FMT"\n",
                      program, 
                      eb_width_data(eb_operation_format(op)), 
                      eb_format_endian(eb_operation_format(op)), 
                      eb_operation_address(op));
      exit(1);
    }
    
    /* If the data is 0, then it was erased */
    if (eb_operation_data(op) != 0) {
      result <<= 1;
    } else {
      break;
    }
  }
  
  *sector_size = result;
}

static eb_address_t detect_sector_size(eb_device_t device) {
  eb_address_t sector_size;
  eb_address_t max_size;
  eb_address_t target;
  eb_address_t end;
  eb_status_t  status;
  eb_cycle_t   cycle;
  
  /* Plan:
   *   0- Find largest possible sector that fits between start and end
   *   1- Write 0s at positions 2^x-1 inside that sector
   *   2- Erase the start address of the sector
   *   3- Read the values at all the addresses that were zeroed.
   *   4- The last non-zero seen determines the sector size
   */
  
  if (!quiet) {
    printf("Autodetecting sector size ... ");
    fflush(stdout);
  }
  
  /* Find largest sector that fits by aligning target */
  end = address + firmware_length-1;
  target = address + firmware_length/2;
  
  sector_size = page_size;
  target &= ~(sector_size-1);
  while (address <= target && target+sector_size-1 <= end) {
    sector_size <<= 1;
    target &= ~(sector_size-1);
    if (sector_size >= MAX_SECTOR_SIZE) break; /* faster */
  }
  
  max_size = sector_size >> 1;
  target = (address + firmware_length/2) & ~(max_size-1);
  
  /* Start writing test pattern */
  for (sector_size = page_size; sector_size < max_size; sector_size <<= 1) {
    if (!quiet) {
      printf("\rAutodetecting sector size: write 0x%"EB_ADDR_FMT" ... ", target+sector_size);
      fflush(stdout);
    }
    status = eb_device_write(device, target+sector_size, format, 0, 0, eb_block);
    if (status != EB_OK) {
      fprintf(stderr, "\r%s: failed to write test pattern at 0x%"EB_ADDR_FMT": %s\n", program, target+sector_size, eb_status(status));
      exit(1);
    }
    wait_for_busy(device, 0);
  }
  
  if (!quiet) {
    printf("\rAutodetecting sector size: erasing 0x%"EB_ADDR_FMT" ... ", target);
    fflush(stdout);
  }
  if ((status = erase_sector(device, target)) != EB_OK) {
    fprintf(stderr, "\r%s: failed to erase test sector 0x%"EB_ADDR_FMT": %s\n", program, target, eb_status(status));
    exit(1);
  }
  
  wait_for_busy(device, wait_us);
  
  if (!quiet) {
    printf("\rAutodetecting sector size: scanning ...            ");
    fflush(stdout);
  }
  if ((status = eb_cycle_open(device, &sector_size, find_sector_size, &cycle)) != EB_OK) {
    fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
    exit(1);
  }
  
  for (sector_size = page_size; sector_size < max_size; sector_size <<= 1) {
    eb_cycle_read(cycle, target+sector_size, format, 0);
  }
  
  sector_size = 0;
  eb_cycle_close(cycle);
  while (sector_size == 0) {
    eb_socket_run(eb_device_socket(device), -1);
  }
  
  if (!quiet) {
    printf("\rAutodetecting sector size: found = 0x%"EB_ADDR_FMT"\n", sector_size);
  }
  
  return sector_size;
}

static void detect_decrement(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  int* counter = (int*)user;
  --*counter;
  
  if (status != EB_OK) {
    fprintf(stderr, "%s: scan operation failed: %s\n", program, eb_status(status));
    exit(1);
  }
  
  for (; op != EB_NULL; op = eb_operation_next(op)) {
    if (eb_operation_had_error(op)) {
      fprintf(stderr, "%s: wishbone segfault reading %s %s bits from address 0x%"EB_ADDR_FMT"\n",
                      program, 
                      eb_width_data(eb_operation_format(op)), 
                      eb_format_endian(eb_operation_format(op)), 
                      eb_operation_address(op));
      exit(1);
    }
    
    /* If the data is not FFFFFF, then set the bit indicating we must erase it */
    if (eb_operation_data(op) != ffs) {
      bitmask_set((eb_operation_address(op) - address) / sector_size, 1);
    }
  }
}

static int scan_count = 0;
static void quick_scan_gap(eb_device_t device, eb_address_t offset, eb_address_t len) {
  eb_address_t sector;
  eb_address_t first_sector;
  eb_address_t last_sector;
  eb_address_t position;
  eb_address_t last_position;
  eb_status_t status;
  eb_cycle_t cycle;
  int i;
  int size;
  int ops;
  int inflight;
  
  size = format & EB_DATAX;
  ops = 0;
  inflight = 0;
  
  if ((status = eb_cycle_open(device, &inflight, detect_decrement, &cycle)) != EB_OK) {
    fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
  }
  
  first_sector = (address/sector_size) * sector_size;
  last_sector = ((address+firmware_length+sector_size-1)/sector_size)*sector_size;
  
  i = 0;
  for (sector = first_sector; sector != last_sector; sector += sector_size) {
    if (bitmask_get(i++)) continue;
    
    last_position = sector+offset+len;
    for (position = sector+offset; position != last_position; position += size) {
      eb_cycle_read(cycle, position, format, 0);
      
      if (!quiet && (++scan_count % 65536) == 0) {
        scan_count = 0;
        printf("\rScanning offset 0x%"EB_ADDR_FMT" ... ", position);
        fflush(stdout);
      }
      
      /* use the same number of operations per cycle as programming/verifying */
      if (++ops == (page_size/size)) {
        ops = 0;
        ++inflight;
        eb_cycle_close(cycle);
        
        if ((status = eb_cycle_open(device, &inflight, detect_decrement, &cycle)) != EB_OK) {
          fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
        }
        
        while (inflight >= cycles_per_poll)
          eb_socket_run(eb_device_socket(device), -1);
      }
    }
  }
  
  if (ops > 0) {
    ++inflight;
    eb_cycle_close(cycle);
  } else {
    eb_cycle_abort(cycle);
  }
  
  while (inflight > 0) {
    eb_socket_run(eb_device_socket(device), -1);
  }
}

static void quick_scan(eb_device_t device) {
  eb_address_t offset, gap;
  
  if (!quiet) {
    printf("Scanning flash ... ");
    fflush(stdout);
  }
  
  gap = 8;
  scan_count = 0;
  quick_scan_gap(device, 0, gap);
  
  for (offset = gap; offset != sector_size; offset += gap, gap += gap) {
    quick_scan_gap(device, offset, gap);
  }
  
  if (!quiet) printf("done!\n");
}

static void erase_flash(eb_device_t device) {
  eb_address_t sector;
  eb_address_t first_sector;
  eb_address_t last_sector;
  eb_status_t status;
  int i;
  first_sector = (address/sector_size) * sector_size;
  last_sector = ((address+firmware_length+sector_size-1)/sector_size)*sector_size;
  
  if (!quiet)
    printf("Erasing flash ... ");
  
  i = 0;
  for (sector = first_sector; sector != last_sector; sector += sector_size) {
    if (!quiet) {
      printf("\rErasing 0x%"EB_ADDR_FMT" ... ", sector);
      fflush(stdout);
    }
    
    if (!bitmask_get(i++)) continue;
    
    if ((status = erase_sector(device, sector)) != EB_OK) {
      fprintf(stderr, "\r%s: failed to erase 0x%"EB_ADDR_FMT": %s\n", program, sector, eb_status(status));
      exit(1);
    }
    
    wait_for_busy(device, wait_us);
  }
  
  if (!quiet) printf("done!\n");
  
}

static void program_decrement(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  int* counter = (int*)user;
  --*counter;
  
  if (status != EB_OK) {
    fprintf(stderr, "%s: program operation failed: %s\n", program, eb_status(status));
    exit(1);
  }
  
  for (; op != EB_NULL; op = eb_operation_next(op)) {
    if (eb_operation_had_error(op)) {
      fprintf(stderr, "%s: wishbone segfault writing %s %s bits to address 0x%"EB_ADDR_FMT"\n",
                      program, 
                      eb_width_data(eb_operation_format(op)), 
                      eb_format_endian(eb_operation_format(op)), 
                      eb_operation_address(op));
      exit(1);
    }
  }
}

static void program_flash(eb_device_t device) {
  eb_status_t status;
  eb_cycle_t  cycle;
  eb_address_t offset;
  eb_address_t stop_offset;
  eb_data_t data;
  int size;
  int ops;
  int inflight;
  int byte;
  int may_skip_ffs;
  uint8_t buf[8];

  size = format & EB_DATAX;
  ops = 0;
  inflight = 0;
  may_skip_ffs = 1;
  
  if (!quiet) printf("Programming flash ... ");
  
  if ((status = eb_cycle_open(device, &inflight, program_decrement, &cycle)) != EB_OK) {
    fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
  }
  
  stop_offset = address + firmware_length;
  for (offset = address; offset != stop_offset; offset += size) {
    if (fread(buf, 1, size, firmware_f) != size) {
      fprintf(stderr, "\r%s: short read from '%s'\n", program, firmware);
      exit(1);
    }
    
    data = 0;
    if ((format & EB_ENDIAN_MASK) == EB_BIG_ENDIAN) {
      for (byte = 0; byte < size; ++byte) {
        data <<= 8;
        data |= buf[byte];
      }
    } else {
      for (byte = size-1; byte >= 0; --byte) {
        data <<= 8;
        data |= buf[byte];
      }
    }
    
    if (!quiet && offset % 65536 == 0) {
      printf("\rProgramming 0x%"EB_ADDR_FMT"... ", offset);
      fflush(stdout);
    }
    
    /* Don't write pages of FFs; skip them */
    if (may_skip_ffs && data == ffs) {
      continue;
    }
    
    if (offset == erase_address) {
      if (!quiet)
        fprintf(stderr, "\r%s: warning: firmware tried to write to erase address!\n", program);
      continue;
    }
    
    /* Flip bits in the bytes? */
    data = flip_bits(data);
    
    eb_cycle_write(cycle, offset, format, data);
    may_skip_ffs = 0;
    ++ops;
    
    /* Align cycles to page boundaries */
    if ((offset+size) % page_size == 0) {
      ops = 0;
      may_skip_ffs = 1;
      
      ++inflight;
      eb_cycle_close(cycle);
      
      if ((status = eb_cycle_open(device, &inflight, program_decrement, &cycle)) != EB_OK) {
        fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
      }
      
      while (inflight >= cycles_per_poll)
        eb_socket_run(eb_device_socket(device), -1);
    }
  }
  
  if (ops > 0) {
    ++inflight;
    eb_cycle_close(cycle);
  } else {
    eb_cycle_abort(cycle);
  }
  
  while (inflight > 0) {
    eb_socket_run(eb_device_socket(device), -1);
  }
  
  if (!quiet) printf("done!\n");
}

static void verify_decrement(eb_user_data_t user, eb_device_t dev, eb_operation_t op, eb_status_t status) {
  static eb_address_t last_offset = -1;
  eb_data_t data;
  uint8_t buf[8];
  int byte, size;
  int* counter = (int*)user;
  --*counter;
  
  if (status != EB_OK) {
    fprintf(stderr, "%s: verify operation failed: %s\n", program, eb_status(status));
    exit(1);
  }
  
  for (; op != EB_NULL; op = eb_operation_next(op)) {
    if (eb_operation_had_error(op)) {
      fprintf(stderr, "%s: wishbone segfault reading %s %s bits from address 0x%"EB_ADDR_FMT"\n",
                      program, 
                      eb_width_data(eb_operation_format(op)), 
                      eb_format_endian(eb_operation_format(op)), 
                      eb_operation_address(op));
      exit(1);
    }
    
    if (last_offset != eb_operation_address(op)) {
      last_offset = eb_operation_address(op);
      fseeko(firmware_f, last_offset-address, SEEK_SET);
    }
    
    size = eb_operation_format(op) & EB_DATAX;
    
    if (fread(buf, 1, size, firmware_f) != size) {
      fprintf(stderr, "\r%s: short verify read from '%s'\n", program, firmware);
      exit(1);
    }
    last_offset += size;
    
    data = 0;
    if ((format & EB_ENDIAN_MASK) == EB_BIG_ENDIAN) {
      for (byte = 0; byte < size; ++byte) {
        data <<= 8;
        data |= buf[byte];
      }
    } else {
      for (byte = size-1; byte >= 0; --byte) {
        data <<= 8;
        data |= buf[byte];
      }
    }
    
    data = flip_bits(data);
    
    if (eb_operation_data(op) != data) {
      fprintf(stderr, "%s: verify failed at address 0x%"EB_ADDR_FMT" (0x%"EB_DATA_FMT" != 0x%"EB_DATA_FMT")\n",
                      program, eb_operation_address(op), eb_operation_data(op), data);
      exit(1);
    }
  }
}

static void verify_flash(eb_device_t device) {
  eb_status_t status;
  eb_cycle_t  cycle;
  eb_address_t offset;
  eb_address_t stop_offset;
  int size;
  int ops;
  int inflight;

  size = format & EB_DATAX;
  ops = 0;
  inflight = 0;
  
  if (!quiet) printf("Verifying flash ... ");
  
  if ((status = eb_cycle_open(device, &inflight, verify_decrement, &cycle)) != EB_OK) {
    fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
  }
  
  stop_offset = address + firmware_length;
  for (offset = address; offset != stop_offset; offset += size) {
    if (offset % 65536 == 0) {
      if (!quiet) printf("\rVerifying 0x%"EB_ADDR_FMT"... ", offset);
      fflush(stdout);
    }
    
    eb_cycle_read(cycle, offset, format, 0);
    ++ops;
    
    if ((offset+size) % page_size == 0) {
      ops = 0;
      ++inflight;
      eb_cycle_close(cycle);
      
      if ((status = eb_cycle_open(device, &inflight, verify_decrement, &cycle)) != EB_OK) {
        fprintf(stderr, "\r%s: could not create cycle: %s\n", program, eb_status(status));
      }
      
      while (inflight >= cycles_per_poll)
        eb_socket_run(eb_device_socket(device), -1);
    }
  }
  
  if (ops > 0) {
    ++inflight;
    eb_cycle_close(cycle);
  } else {
    eb_cycle_abort(cycle);
  }
  
  while (inflight > 0) {
    eb_socket_run(eb_device_socket(device), -1);
  }
  
  if (!quiet) printf("done!\n");
}

int main(int argc, char** argv) {
  long value;
  char* value_end;
  int opt, error, i;
  
  eb_socket_t socket;
  eb_status_t status;
  eb_device_t device;
  eb_width_t line_width;
  eb_width_t device_support;
  
  /* Default arguments */
  program = argv[0];
  address = 0;
  address_set = 0;
  erase_address = 0;
  erase_address_set = 0;
  address_width = EB_ADDRX;
  data_width = EB_DATAX;
  endian = 0; /* auto-detect */
  sector_size = 0; /* auto */
  page_size = 256;
  wait_us = 100000; /* 100 ms */
  cycles_per_poll = 4;
  retries = 3;
  full = 0;
  probe = 1;
  verify = 1;
  invert = -1; /* auto */
  verbose = 0;
  quiet = 0;
  erase_only = 0;
  
  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "t:e:a:d:bli:s:w:c:r:fpnzvqh")) != -1) {
    switch (opt) {
    case 't':
      address = strtoull(optarg, &value_end, 0);
      address_set = 1;
      if (*value_end != 0) {
        fprintf(stderr, "%s: invalid <address> argument -- '%s'\n", program, optarg);
        error = 1;
      }
      break;
    case 'e':
      erase_address = strtoull(optarg, &value_end, 0);
      if (*value_end) {
        fprintf(stderr, "%s: invalid erase address -- '%s'\n", program, optarg);
        error = 1;
      }
      erase_address_set = 1;
      break;
    case 'a':
      value = eb_width_parse_address(optarg, &address_width);
      if (value != EB_OK) {
        fprintf(stderr, "%s: invalid address width -- '%s'\n", program, optarg);
        error = 1;
      }
      break;
    case 'd':
      value = eb_width_parse_data(optarg, &data_width);
      if (value != EB_OK) {
        fprintf(stderr, "%s: invalid data width -- '%s'\n", program, optarg);
        error = 1;
      }
      break;
    case 'b':
      endian = EB_BIG_ENDIAN;
      break;
    case 'l':
      endian = EB_LITTLE_ENDIAN;
      break;
    case 'i':
      invert = strtol(optarg, &value_end, 0);
      if (*value_end || invert < 0 || invert > 1) {
        fprintf(stderr, "%s: invalid invert option -- '%s'; must be 0 or 1\n", program, optarg);
        error = 1;
      }
      break;
    case 's':
      sector_size = strtoull(optarg, &value_end, 0);
      if (*value_end || sector_size < 256 || ((sector_size-1)&sector_size) != 0) {
        fprintf(stderr, "%s: invalid sector size -- '%s'; must be a large power of two\n", program, optarg);
        error = 1;
      }
      break;
    case 'x':
      page_size = strtoull(optarg, &value_end, 0);
      if (*value_end || page_size < 1 || ((page_size-1)&page_size) != 0) {
        fprintf(stderr, "%s: invalid page size -- '%s'; must be a power of two\n", program, optarg);
        error = 1;
      }
      break;
    case 'w':
      wait_us = strtof(optarg, &value_end) * 1000000.0;
      if (*value_end || wait_us < 100000 || wait_us > 10000000) {
        fprintf(stderr, "%s: invalid wait time -- '%s'; must be between 0.01 and 10 seconds\n", program, optarg);
        error = 1;
      }
      break;
    case 'c':
      cycles_per_poll = strtol(optarg, &value_end, 0);
      if (*value_end || cycles_per_poll <= 0 || cycles_per_poll > 100) {
        fprintf(stderr, "%s: invalid number of cycles -- '%s'; must be between 1 and 100\n", program, optarg);
        return 1;
      }
      break;
    case 'r':
      retries = strtol(optarg, &value_end, 0);
      if (*value_end || retries < 0 || retries > 100) {
        fprintf(stderr, "%s: invalid number of retries -- '%s'; must be between 0 and 100\n", program, optarg);
        return 1;
      }
      break;
    case 'f':
      full = 1;
      break;
    case 'p':
      probe = 0;
      break;
    case 'z':
      erase_only = 1;
      break;
    case 'n':
      verify = 0;
      break;
    case 'v':
      verbose = 1;
      break;
    case 'q':
      quiet = 1;
      break;
    case 'h':
      help();
      return 1;
    case ':':
    case '?':
      error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      error = 1;
    }
  }
  
  if (error) return 1;
  
  if (optind + 2 != argc) {
    fprintf(stderr, "%s: expecting two non-optional arguments: <proto/host/port> <firmware(rpd file)>\n", program);
    return 1;
  }
  
  device_address = argv[optind];
  
  firmware = argv[optind+1];
  
  if ((firmware_f = fopen(firmware, "r")) == 0) {
    fprintf(stderr, "%s: fopen, %s -- '%s'\n",
                    program, strerror(errno), firmware);
    return 1;
  }
  
  if (fseeko(firmware_f, 0, SEEK_END) != 0) {
    fprintf(stderr, "%s: fseeko, %s -- '%s'\n",
                    program, strerror(errno), firmware);
    return 1;
  }
  
  firmware_length = ftello(firmware_f);
  fseeko(firmware_f, 0, SEEK_SET);
  
  if (firmware_length % page_size != 0) {
    fprintf(stderr, "%s: invalid firmware image -- '%s'; must be aligned to a %d-byte page\n",
                    program, firmware, (int)page_size);
    return 1;
  }

  erase_bitmap_size = firmware_length / page_size / 8;  
  erase_bitmap = (uint8_t*)calloc(erase_bitmap_size, 1);
  if (!erase_bitmap) {
    fprintf(stderr, "%s: could not allocate erase bitmap (%d bytes)\n",
                    program, erase_bitmap_size);
    return 1;
  }
  
  if (verbose) 
    printf("Opened '%s' with 0x%"EB_ADDR_FMT" bytes of data\n", firmware, firmware_length);
  
  if (invert == -1) {
    if (strlen(firmware) > 4 && !strcmp(firmware+strlen(firmware)-4, ".rpd")) {
      invert = 1;
    } else {
      invert = 0;
    }
    if (verbose)
      printf("  based on filename, WILL%s invert bits within bytes\n", invert?"":" NOT");
  }
  
  if (verbose)
    fprintf(stdout, "Opening socket with %s-bit address and %s-bit data widths\n", 
                    eb_width_address(address_width), eb_width_data(data_width));
  
  if ((status = eb_socket_open(EB_ABI_CODE, 0, address_width|data_width, &socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  
  if (verbose)
    fprintf(stdout, "Connecting to '%s' with %d retry attempts...\n", device_address, retries);
  
  if ((status = eb_device_open(socket, device_address, EB_ADDRX|EB_DATAX, retries, &device)) != EB_OK) {
    fprintf(stderr, "%s: failed to open Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }
  
  line_width = eb_device_width(device);
  if (verbose)
    fprintf(stdout, "  negotiated %s-bit address and %s-bit data session.\n", 
                    eb_width_address(line_width), eb_width_data(line_width));
  
  if (probe) {
    struct sdb_device info;
    int num;
    
    if (verbose)
      fprintf(stdout, "Scanning remote bus for Wishbone devices...\n");

    if (address_set == 0) {
      num = 1;
      if ((status = eb_sdb_find_by_identity(device, GSI_ID, FLASH_ID, &info, &num)) != EB_OK) {
        fprintf(stderr, "%s: failed to find flash device: %s\n", program, eb_status(status));
        return 1;
      }
      if (num == 0) {
        fprintf(stderr, "%s: no known flash devices found\n", program);
        return 1;
      }
      if (num > 1) {
        fprintf(stderr, "%s: found %d flash devices -- choose one with '-t'.\n", program, num);
        return 1;
      }
      
      address = info.sdb_component.addr_first;
      address_set = 1;
      if (verbose)
        printf("  found flash device at address 0x%"EB_ADDR_FMT"\n", address);
    } else {
      if ((status = eb_sdb_find_by_address(device, address, &info)) != EB_OK) {
        fprintf(stderr, "%s: failed to find SDB record at 0x%"EB_ADDR_FMT": %s\n", program, address, eb_status(status));
        return 1;
      }
    }
    
    if ((info.bus_specific & SDB_WISHBONE_LITTLE_ENDIAN) != 0)
      device_support = EB_LITTLE_ENDIAN;
    else
      device_support = EB_BIG_ENDIAN;
    device_support |= info.bus_specific & EB_DATAX;
    
    if (!erase_address_set) {
      erase_address = info.sdb_component.addr_last-3;
      erase_address_set = 1;
      if (verbose)
        fprintf(stdout, "  using 0x%"EB_ADDR_FMT" as erase register\n", erase_address);
    }
    
    if (info.abi_ver_major == 1 && info.abi_ver_minor == 0) {
      if (!quiet) fprintf(stderr, "warning: old flash firmware detected! (falling back to slow erase)\n");
      if (wait_us < 4000000) {
        wait_us = 4000000;
      }
    }
    if (info.abi_ver_major == 1 && info.abi_ver_minor <= 1) {
      if (!quiet) fprintf(stderr, "warning: old flash firmware detected! (using hardware erase command)\n");
      old_erase = 1;
    } else {
      old_erase = 0;
    }
  } else {
    device_support = endian | EB_DATAX;
  }
  
  /* If unset by user and SDB */
  if (!address_set) {
    fprintf(stderr, "%s: could not auto-detect target flash address\n", program);
    return 1;
  }
  
  if (!erase_address_set) {
    fprintf(stderr, "%s: could not auto-detect erase register address\n", program);
    return 1;
  }
  
  if (address % page_size != 0) {
    fprintf(stderr, "%s: invalid target address -- 0x%"EB_ADDR_FMT"; must be aligned to a %d-byte page\n",
                    program, address, (int)page_size);
    return 1;
  }
  
  /* Did the user request a bad endian? We use it anyway, but issue warning. */
  if (endian != 0 && (device_support & EB_ENDIAN_MASK) != endian) {
    if (!quiet)
      fprintf(stderr, "%s: warning: target device is %s (writing as %s).\n",
                      program, eb_format_endian(device_support), eb_format_endian(endian));
  }
  
  if (endian == 0) {
    /* Select the probed endian. May still be 0 if device not found. */
    endian = device_support & EB_ENDIAN_MASK;
  }
  
  /* We need to know the endian */
  if (endian == 0) {
    fprintf(stderr, "%s: error: must know endian to program firmware\n",
                    program);
    return 1;
  }
  
  /* What widths does the connection support? */
  line_width &= EB_DATAX;
  line_width |= line_width-1;

  /* Pick the largest supported data width */
  format = device_support & line_width;
  format |= format >> 1;
  format |= format >> 2;
  format = (format+1) >> 1;
  
  if (format == 0) {
    fprintf(stderr, "%s: device widths %s do not fit this connection (widths %s)\n", 
                    program, eb_width_data(device_support), eb_width_data(line_width));
    return 1;
  }
  
  /* If the page size is smaller than the write format... wtf? */
  if (page_size < format) {
    fprintf(stderr, "%s: page size %d is smaller than device write size %d!\n",
                    program, (int)page_size, format);
    return 1;
  }

  format |= endian;
  if (verbose)
    printf("Will operate using %s bit %s operations\n",
      eb_format_data(format), eb_format_endian(format));
  
  if (!sector_size) {
    sector_size = detect_sector_size(device);
  }
  
  ffs = 0;
  ffs = ~ffs;
  ffs >>= (sizeof(eb_data_t) - (format&EB_DATAX))*8;
  
  mask_0f = 0x0f;
  for (i = 0; i < sizeof(eb_data_t); ++i)
    mask_0f |= mask_0f << 8;
  
  if (full) {
    memset(erase_bitmap, 0xFF, erase_bitmap_size);
  } else {
    quick_scan(device);
  }
  
  erase_flash(device);
  if (erase_only) return 0;
    
  program_flash(device);
  if (verify) verify_flash(device);
  
  if ((status = eb_device_close(device)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone device: %s\n", program, eb_status(status));
    return 1;
  }
  
  if ((status = eb_socket_close(socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to close Etherbone socket: %s\n", program, eb_status(status));
    return 1;
  }
  
  return 0;
}

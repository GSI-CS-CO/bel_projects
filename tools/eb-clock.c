#define _POSIX_C_SOURCE 200112L /* strtoull */

#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>
#include <math.h>

#define GSI_ID	0x651
#define SERDES_CLK_GEN_ID	0x5f3eaf43

#define BITS 8

const char *program;
static eb_device_t device;
static int selr, perr, fracr, normmaskr, skipmaskr, perhir, phofslr, phofshr;

struct Control {
  uint32_t period_integer;
  uint32_t period_high;
  uint32_t period_fraction;
  uint16_t bit_pattern_normal;     // size=2*BITS
  uint16_t bit_pattern_skip;
  uint64_t phase_offset;
};

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "  -c <channel>   channel number to access\n");
  fprintf(stderr, "  -H <hi-period> set high period (in nanosec.) on the channel\n");
  fprintf(stderr, "  -L <lo-period> set low period (in nanosec.) on the channel\n");
  fprintf(stderr, "  -p <ns>        set channel phase offset in nanoseconds\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <t.stana@gsi.de>\n");
}

static void die(const char* msg, eb_status_t status) {
  fprintf(stderr, "%s: %s: %s\n", program, msg, eb_status(status));
  exit(1);
}

static void clock(double hi, double lo, uint64_t phase, struct Control *control)
{
  double period, wide_period, cut_period;
  int i, j, fill_factor, cut_wide_period;

  if (hi != floor(hi))
    fprintf(stderr, "warning: fractional part of hi time ignored; period remains unaffected\n");

  period = hi + lo;
  fill_factor = ceil(BITS/period);
  wide_period = period*fill_factor; // >= BITS
  cut_wide_period = floor(wide_period);
  cut_period = (double)cut_wide_period / fill_factor;
  fprintf(stderr, "wide_period     = %f\n", wide_period);

  control->period_integer  = floor(wide_period);
  control->period_high     = floor(hi);
  control->period_fraction = round((wide_period - control->period_integer) * 4294967296.0);
  control->phase_offset    = phase;
  fprintf(stderr, "period_integer   = 0x%x\n", control->period_integer);
  fprintf(stderr, "period_high      = 0x%x\n", control->period_high);
  fprintf(stderr, "period_fraction  = 0x%x\n", control->period_fraction);
  fprintf(stderr, "phase_offset     = 0x%lx\n", control->phase_offset);

  control->bit_pattern_normal = 0;
  for (i = 0; i < fill_factor; ++i) {
    int offset  = ceil(i*cut_period); // ceil guarantees gap before 7 is small
    if (BITS-1 >= offset) // future bits
      control->bit_pattern_normal |= 1 << (BITS-1 - offset);
    if (cut_wide_period - offset <= BITS) // past bits
      control->bit_pattern_normal |= 1 << (BITS-1 - offset + cut_wide_period);
  }

  // fractional bit insertion (0) must happen at cut_wide_period-1
  uint16_t spot = (cut_wide_period >= 16) ? 0 : (0x8000 >> (15-cut_wide_period));
  uint16_t low_mask = spot-1;
  //low_mask = 0xFF; // old approach
  uint16_t high_mask = ~low_mask;
  control->bit_pattern_skip = (control->bit_pattern_normal & high_mask) |
                              ((control->bit_pattern_normal & low_mask) >> 1);

  fprintf(stderr, "bit_pattern_norm = ");
  for (j = 15; j >= 0; --j)
    fprintf(stderr, "%d", (control->bit_pattern_normal >> j) & 1);
  fprintf(stderr, "\n");
  fprintf(stderr, "bit_pattern_skip = ");
  for (j = 15; j >= 0; --j)
    fprintf(stderr, "%d", (control->bit_pattern_skip >> j) & 1);
  fprintf(stderr, "\n");
}

static void apply(int chan, struct Control *control)
{
  eb_status_t status;

  if ((status = eb_device_write(device, selr, EB_DATA32, chan, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, perr, EB_DATA32,
                    control->period_integer, 0, 0)) != EB_OK) {
    die("eb_device_write(perr)", status);
  }

  if ((status = eb_device_write(device, perhir, EB_DATA32,
                    control->period_high, 0, 0)) != EB_OK) {
    die("eb_device_write(perhir)", status);
  }

  if ((status = eb_device_write(device, fracr, EB_DATA32,
                    control->period_fraction, 0, 0)) != EB_OK) {
    die("eb_device_write(fracr)", status);
  }

  if ((status = eb_device_write(device, normmaskr, EB_DATA32,
                    control->bit_pattern_normal, 0, 0)) != EB_OK) {
    die("eb_device_write(normmaskr)", status);
  }

  if ((status = eb_device_write(device, skipmaskr, EB_DATA32,
                    control->bit_pattern_skip, 0, 0)) != EB_OK) {
    die("eb_device_write(skipmaskr)", status);
  }

  if ((status = eb_device_write(device, phofslr, EB_DATA32,
                    (uint32_t)(control->phase_offset), 0, 0)) != EB_OK) {
    die("eb_device_write(phofshr)", status);
  }

  if ((status = eb_device_write(device, phofshr, EB_DATA32,
                    (uint32_t)(control->phase_offset >> 32), 0, 0)) != EB_OK) {
    die("eb_device_write(phofslr)", status);
  }

}

int main(int argc, char** argv) {
  int opt, error, c;
  struct sdb_device sdb;
  eb_status_t status;
  eb_socket_t socket;

  int chan = 0;
  char port[16];

  double hi;
  double lo;
  uint64_t phase;
  struct Control control;

  int base;

  /* Default arguments */
  program = argv[0];

  /* Process the command-line arguments */
  error = 0;
  hi    = 0;
  lo    = 0;
  phase = 0;
  while ((opt = getopt(argc, argv, "hc:H:L:p:")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
    case 'c':
      chan = atoi(optarg);
      break;
    case 'H':
      hi = atof(optarg);
      break;
    case 'L':
      lo = atof(optarg);
      break;
    case 'p': phase = atoll(optarg);
      break;
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

  if (optind+1 != argc) {
    help();
    return 1;
  }

  /* Get the port from the command line */
  strcpy(port, argv[optind]);

  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
    die("eb_socket_open", status);

  if ((status = eb_device_open(socket, port, EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
    die(port, status);

  /* Set channel I/O as output using IO_HACK module */
  // TODO: change when IO_HACK removed
  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, 0x4d78adfd, &sdb, &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  if (c != 1) {
    fprintf(stderr, "Found %d IO_HACK identifiers on that device\n", c);
    exit(1);
  }
  
  /* Enable the channel's output using the IO_HACK module */
  base = sdb.sdb_component.addr_first;
  eb_data_t iodir;
  if ((status = eb_device_read(device, base + 4, EB_DATA32, &iodir, 0, NULL)) != EB_OK)
    die("eb_device_read(iodir)", status);
    
  fprintf(stderr, "IOHACK+4: %x\n", (unsigned int) iodir);
   
  iodir |= (1 << (chan-1));
  
  fprintf(stderr, "IOHACK+4 modify: %x\n", (unsigned int) iodir);
  
  if ((status = eb_device_write(device, base + 4, EB_DATA32, iodir, 0, NULL)) != EB_OK)
    die("eb_device_write(iodir)", status);

  /* Now find the clock generator module and set the addresses */
  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, SERDES_CLK_GEN_ID, &sdb, &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  if (c != 1) {
    fprintf(stderr, "Found %d clock gen identifiers on that device\n", c);
    exit(1);
  }

  base      = sdb.sdb_component.addr_first;
  selr      = base;
  perr      = base + 4;
  perhir    = base + 8;
  fracr     = base + 12;
  normmaskr = base + 16;
  skipmaskr = base + 20;
  phofslr   = base + 24;
  phofshr   = base + 28;

  /* Set and apply reg values to clock module */
  clock(hi, lo, phase, &control);
  apply(chan-1, &control);

  return 0;
}


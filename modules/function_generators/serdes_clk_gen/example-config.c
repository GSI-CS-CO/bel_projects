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
static int selr, pr, fracr, maskr;

struct Control {
  uint32_t period_integer;  // half-period = integer.fraction
  uint32_t period_fraction;
  uint16_t bit_pattern;     // size=2*BITS
};

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <w.terpstra@gsi.de>\n");
}

static void die(const char* msg, eb_status_t status) {
  fprintf(stderr, "%s: %s: %s\n", program, msg, eb_status(status));
  exit(1);
}

static void clk(double hi, double lo, struct Control *control)
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
  control->period_fraction = round((wide_period - control->period_integer) * 4294967296.0);
  fprintf(stderr, "period_integer  = 0x%x\n", control->period_integer);
  fprintf(stderr, "period_fraction = 0x%x\n", control->period_fraction);

  control->bit_pattern = 0;
  for (i = 0; i < fill_factor; ++i) {
    int offset  = ceil(i*cut_period); // ceil guarantees gap before 7 is small
    if (BITS-1 >= offset) // future bits
      control->bit_pattern |= 1 << (BITS-1 - offset);
    if (cut_wide_period - offset <= BITS) // past bits
      control->bit_pattern |= 1 << (BITS-1 - offset + cut_wide_period);
  }
  fprintf(stderr, "bit_pattern     = ");
  for (j = 15; j >= 0; --j)
    fprintf(stderr, "%d", (control->bit_pattern >> j) & 1);
  fprintf(stderr, "\n");
}

static void apply(int chan, struct Control *control)
{
  eb_status_t status;

  if ((status = eb_device_write(device, selr, EB_DATA32, chan, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, pr, EB_DATA32,
                    control->period_integer, 0, 0)) != EB_OK) {
    die("eb_device_write(hperr)", status);
  }

  if ((status = eb_device_write(device, fracr, EB_DATA32,
                    control->period_fraction, 0, 0)) != EB_OK) {
    die("eb_device_write(hperr)", status);
  }

  if ((status = eb_device_write(device, maskr, EB_DATA32,
                    control->bit_pattern, 0, 0)) != EB_OK) {
    die("eb_device_write(maskr)", status);
  }

}

int main(int argc, char** argv) {
  int opt, error, c;
  struct sdb_device sdb;
  eb_status_t status;
  eb_socket_t socket;

  double hi1, hi2, hi3;
  double lo1, lo2, lo3;
  struct Control control;

  /* Default arguments */
  program = argv[0];
  error = 0;

  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
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

  if (optind + 1 != argc) {
    fprintf(stderr, "%s: expecting one non-optional arguments: <proto/host/port> <num_cycles>\n", program);
    return 1;
  }

  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
    die("eb_socket_open", status);

  if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
    die(argv[optind], status);

  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, SERDES_CLK_GEN_ID, &sdb, &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  if (c != 1) {
    fprintf(stderr, "Found %d clk gen identifiers on that device\n", c);
    exit(1);
  }

  /* Clocking paraphernaelia */
  int first;
  first = sdb.sdb_component.addr_first;
  selr = first;
  pr = first + 4;
  fracr = first + 8;
  maskr = first + 12;

  /*-------------------------------------------------------------------------*/
  printf("hi1 = ");
  scanf("%lf", &hi1);
  printf("lo1 = ");
  scanf("%lf", &lo1);

  clk(hi1, lo1, &control);

  apply(0, &control);

  /*-------------------------------------------------------------------------*/
  printf("hi2 = ");
  scanf("%lf", &hi2);
  printf("lo2 = ");
  scanf("%lf", &lo2);

  clk(hi2, lo2, &control);

  apply(1, &control);

  /*-------------------------------------------------------------------------*/
  printf("hi3 = ");
  scanf("%lf", &hi3);
  printf("lo3 = ");
  scanf("%lf", &lo3);

  clk(hi3, lo3, &control);

  apply(2, &control);

  return 0;
}


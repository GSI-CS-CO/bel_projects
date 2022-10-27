/* Synopsis */
/* ==================================================================================================== */
/* Some I2C basics:
    - Bidirectional open collector/open drain
    - Allows multiple devices
    - Two bus lines are required:
      - SDA: Serial Data Line
      - SCL: Serial Clock Line
      - Each device, on the bus, has it's own address:
        - 7-bit slave address: Mandatory
        - 10-bit slave address: Optional
    - Speed modes:
      - Standard mode: 100 kbits/s
      - Fast mode: 400 kbit/s
      - Fast mode plus: 1Mbit/s
      - High speed mode: 3.4 Mbit/s
    - Logic levels
      - 0: <= 0.3 Vcc
      - 1: >= 0.7 Vcc
      - Vcc: ~2.0V up to ~5.0V
    - START and STOP conditions
      - A HIGH to LOW transition on the SDA line (while SCL is HIGH) defines a START condition
      - A LOW to HIGH transition on the SDA line (while SCL is HIGH) defines a STOP condition
    - Data size (every byte on SDA) must be eight bits long/aligned
    - Acknowledge (ACK) and Not Acknowledge (NACK):
      - The acknowledge takes place after every byte (on SCL)
      - The master will generate the ninth clock pulse (known as ACK pulse)
      - SDA line is released by the master
      - The slave will pull SDA to LOW during the ACK pulse
      - In case SDA stays HIGH during the ACK pulse, it's defined as NACK
    - Slave address and the read/wright bit (first bye on bus)
      - MSB                  LSB
        [7][6][5][4][3][2][1][0]
      - Address    => 7..1
      - Read/write =>    0 (0 = write, 1 = read)
*/

/* C Standard Includes */
/* ==================================================================================================== */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* GSI/WR/Devices Includes */
/* ==================================================================================================== */
#include "etherbone.h"
#include "../modules/i2c_wrapper/src/c/oc_i2c_master.h"

/* Defines */
/* ==================================================================================================== */
#define I2C_WRAP_GSI_ID   0x651      /* Vendor ID */
#define I2C_WRAP_ID       0x12575a95 /* Device ID */
#define I2C_SCL_SPEED_DEF 100000     /* 100kHz, default */
#define I2C_SCL_SPEED_DEB 7812500    /* 7.8125MHz, debug/simulation */
#define SYSTEM_SPEED      62500000   /* 62.5MHz */
#define PRESCALER_CONST   5          /* See slave documentation */
#define WRITE_BIT_ADDR    0          /* See I2C specification */
#define MAX_INTERFACES    15         /* 16 Interface IDs, 0..15 */
#define HIDDEN_IF_REG     0x05       /* Secret CERN addtion for interfaces */

/* Globals Variables */
/* ==================================================================================================== */
const char *program;          /* eb-i2c-master */
const char *device_name;      /* dev/ttyUSB0, dev/wbm0, ... */
struct sdb_device sdb;        /* SDB and Etherbone stuff */
eb_status_t status;           /* "" */
eb_socket_t socket;           /* "" */
eb_device_t device;           /* "" */
eb_data_t data;               /* "" */
int base = 0;                 /* Base address of I2C device/master */
int verbose = 0;              /* Be verbose? */
int error = 0;                /* Error counter */
int devices = 1;              /* Expected I2C devices/masters */
int current_base = 0;         /* Base address + register offset, helper variable */
int current_value = 0;        /* Read/write variable */
int interface = 0;            /* Interface ID */
bool write_op = false;        /* Write or read operation? */
unsigned int adr_size = 7;    /* Address size */
unsigned int addr = 0x0;      /* I2C slave address */
unsigned int data_byte = 0x0; /* Data to send */
bool got_address = false;     /* Got valid I2C slave address */
bool got_data = false;        /* Got data to send? */
bool high_speed_clk = false;  /* Set clock to maximum speed (simulation, debugging, ...) */

/* Function print_help */
/* ==================================================================================================== */
void print_help(void)
{
  printf("%s is a simple tool to control each WB I2C master.\n", program);
}

/* Function read_i2c_core_status */
/* ==================================================================================================== */
void read_i2c_core_status(void)
{
  eb_data_t status;

  /* Show control and status registers */
  if (verbose)
  {
    /* Get control register */
    eb_device_read(device, (eb_address_t)(base+(OC_I2C_CTR<<2)), EB_DATA32, &status, 0, NULL);
    printf("Info: Control register is 0x%x - EN: %x IEN: %x ...\n",
           (unsigned int)status, (unsigned int)(status&OC_I2C_EN)>>7, (unsigned int)(status&OC_I2C_IEN)>>6);

    /* Get status register */
    eb_device_read(device, (eb_address_t)(base+(OC_I2C_SR<<2)), EB_DATA32, &status, 0, NULL);
    printf("Info: Status register is 0x%x - RXACK: %x BUSY: %x TIP: %x IF: %x ...\n",
           (unsigned int)status, (unsigned int)(status&OC_I2C_RXACK)>>7, (unsigned int)(status&OC_I2C_BUSY)>>6,
           (unsigned int)(status&OC_I2C_TIP)>>1, (unsigned int)(status&OC_I2C_IF)>>0);
  }
}

/* Function setup_i2c_core */
/* ==================================================================================================== */
void setup_i2c_core(void)
{
  uint8_t prescale_low = 0x00;
  uint8_t prescale_high = 0x00;
  uint16_t prescale_sum = 0x0000;
  uint32_t i2c_clock_speed = 0x00000000;

  /* Disable I2C core, EN bit must be cleared to change the prescaler settings */
  current_base = base+(OC_I2C_CTR<<2);
  current_value = 0x00;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  read_i2c_core_status();

  /* Calculate and set prescaler */
  if (!high_speed_clk) { i2c_clock_speed = I2C_SCL_SPEED_DEF; } /* Default speed */
  else                 { i2c_clock_speed = I2C_SCL_SPEED_DEB; } /* Debug speed */
  prescale_sum = (SYSTEM_SPEED)/(PRESCALER_CONST*i2c_clock_speed);
  prescale_low = prescale_sum&0xff; /* Cut off higher bits */
  prescale_high = (prescale_sum&0xff00) >> 8; /* Must fit in uint8_t */

  current_base = base+(OC_I2C_PRER_LO<<2);
  current_value = prescale_low;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  if (verbose) { printf("Info: Presclaer low is 0x%x (Reg: 0x%x) ...\n", prescale_low, current_base); }

  current_base = base+(OC_I2C_PRER_HI<<2);
  current_value = prescale_high;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  if (verbose) { printf("Info: Presclaer high is 0x%x (Reg: 0x%x) ...\n", prescale_high, current_base); }

  if (verbose) { printf("Info: Prescaler: 0x%x (%dHz)\n", prescale_sum, i2c_clock_speed); }

  /* Set interface */
  current_base = base+(HIDDEN_IF_REG<<2);
  current_value = interface;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  read_i2c_core_status();

  /* Enable I2C core (again) */
  current_base = base+(OC_I2C_CTR<<2);
  current_value = OC_I2C_EN;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  read_i2c_core_status();

  if (verbose) { printf("Info: Core enabled (interrupts off) ...\n"); }
}

/* Function setup_i2c_core */
/* ==================================================================================================== */
void transfer_i2c_data(void)
{
  /* Set up WR/RD mode */
  uint8_t addr_rw_byte = 0x00;
  uint8_t config_byte = 0x00;

  /* Write address and optional write bit */
  current_base = base+(OC_I2C_TXR<<2);
  addr_rw_byte = (addr<<1)&0xff; /* Cut off higher bits */;
  if (write_op) { addr_rw_byte = addr_rw_byte|WRITE_BIT_ADDR; }
  if (verbose)
  {
    if (write_op) { printf("Info: Set up address/first byte (WR) 0x%x, address is 0x%x (Reg: 0x%x) ...\n", addr_rw_byte, addr, current_base); }
    else          { printf("Info: Set up address/first byte (RD) 0x%x, address is 0x%x (Reg: 0x%x) ...\n", addr_rw_byte, addr, current_base); }
  }
  current_value = addr_rw_byte;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  read_i2c_core_status();

  /* Generate start condition */
  current_base = base+(OC_I2C_CR<<2);
  if (write_op) { config_byte = OC_I2C_STA + OC_I2C_WR; }
  else          { config_byte = OC_I2C_STA; }
  current_value = config_byte;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  if (verbose) { printf("Info: Send start condition 0x%x (Reg: 0x%x) ...\n", config_byte, current_base); }
  read_i2c_core_status();

  /* To do: Check busy or RxAck flag on real hardware when pexarria10 is available */
  sleep(0.500);

  /* Generate stop condition */
  current_base = base+(OC_I2C_CR<<2);
  config_byte = OC_I2C_STO;
  current_value = config_byte;
  eb_device_write(device, (eb_address_t)(current_base), EB_DATA32, current_value, 0, NULL);
  if (verbose) { printf("Info: Send stop condition 0x%x (Reg: 0x%x) ...\n", config_byte, current_base); }
  read_i2c_core_status();
}

/* Function main */
/* ==================================================================================================== */
int main (int argc, char** argv)
{
  /* Helpers */
  int opt = 0;               /* Number of given options */
  char *pEnd = NULL;         /* Arguments parsing */

  /* Check argument counter */
  program = argv[0];
  if (argc < 2)
  {
    printf("Error: Missing arguments (got only %i)!\n", argc);
    error++;
  }
  else
  {
    device_name = argv[1];
  }

  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "hvswxi:a:d:")) != -1)
  {
    switch (opt)
    {
      case 'h':
        print_help();
        return 0;
      case 'v':
        verbose = 1;
        break;
      case 's':
        adr_size = 10;
        break;
      case 'w':
        write_op = true;
        break;
      case 'x':
        high_speed_clk = true;
        break;
      case 'i':
        if (argv[optind-1] != NULL)
        {
          interface = strtoul(argv[optind-1], &pEnd, 0);
          if (interface > MAX_INTERFACES)
          {
            printf("Error: Interface ID error, please select a value between 0 and 15!\n");
            error++;
          }
        }
        else
        {
          printf("Error: Missing interface id (0..15)!\n");
          error++;
        }
        break;
      case 'a':
        if (argv[optind-1] != NULL)
        {
          addr = strtoul(argv[optind-1], &pEnd, 0);
          got_address = true;
        }
        else
        {
          printf("Error: Missing address!\n");
          error++;
        }
        break;
      case 'd':
        if (argv[optind-1] != NULL)
        {
          data_byte = strtoul(argv[optind-1], &pEnd, 0);
          got_data = true;
        }
        else
        {
          printf("Error: Missing data!\n");
          error++;
        }
        break;
      case '?':
        error++;
        break;
      default:
        error++;
        break;
    }
  }

  /* Exit on error(s) */
  if (error) { printf("Error: Ambiguous arguments!\n"); return 1; }

  /* Find device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
  {
    printf("Error: Failed to open a socket!\n");
    return 1;
  }

  if ((status = eb_device_open(socket, device_name, EB_ADDRX|EB_DATAX, 3, &device)) != EB_OK)
  {
    printf("Error: Failed to open device (%s)!\n", device_name);
    return 1;
  }

  if ((status = eb_sdb_find_by_identity(device, I2C_WRAP_GSI_ID, I2C_WRAP_ID, &sdb, &devices)) != EB_OK)
  {
    printf("Error: Failed find I2C master!\n");
    return 1;
  }

  /* Display operation */
  if (devices == 0)
  {
    printf("Error: There is not I2C master on this device!\n");
    return 1;
  }

  base = (uint32_t) sdb.sdb_component.addr_first;
  if (verbose)
  {
    printf("Info: Found I2C master at 0x%x-0x%x\n", (uint32_t) sdb.sdb_component.addr_first,
                                                    (uint32_t) sdb.sdb_component.addr_last);
    if (got_address)
    {
      if (write_op) { printf("Info: Writing ...\n"); }
      else          { printf("Info: Reading ...\n"); }
      printf("Info: Address size: %d\n", (uint32_t) adr_size);
      printf("Info: Address: 0x%x\n", (uint32_t) addr);
      printf("Info: Interface: %d\n", (uint32_t) interface);
    }
  }

  /* Enable core */
  if (verbose && !got_address)
  {
    current_base = base+(OC_I2C_CTR<<2);
    eb_device_read(device, (eb_address_t)(current_base), EB_DATA32, &data, 0, NULL);
    printf("Info: Core status is 0x%x (0x%x)\n", (uint32_t) data, current_base);
  }

  /* Perform operation */
  if (got_address)
  {
    setup_i2c_core();
    transfer_i2c_data();
  }

  /* Done */
  return 0;
}


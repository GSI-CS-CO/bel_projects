// standard includes 
#include <unistd.h> // getopt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

// Etherbone
#include <etherbone.h>

// dm-unipz
#include <wr_mil_gw.h>

void die(const char *program, const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die

void help(const char *program) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -d <delay>     microseconds between trigger event and generated events\n");
  fprintf(stderr, "  -u <utc-delay> microsecinds between 5 generated utc events\n");
  fprintf(stderr, "  -e             configure WR-MIL gateway as ESR source\n");
  fprintf(stderr, "  -s             configure WR-MIL gateway as SIS source\n");
  fprintf(stderr, "  -p             pause WR-MIL gateway for 10 seconds\n");
  fprintf(stderr, "  -k             kill WR-MIL gateway, only reset or eb-fwload can recover (useful for eb-fwload)\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report Etherbone bugs to <etherbone-core@ohwr.org>\n");
  fprintf(stderr, "Version: %s\n%s\nLicensed under the LGPL v3.\n", eb_source_version(), eb_build_info());
}

int main(int argc, char *argv[])
{
  eb_device_t device;
  eb_status_t eb_status;
  eb_socket_t socket;

  char *value_end;
  int value;
  int delay     = -1;
  int utc_delay = -1;
  int sis       =  0;
  int esr       =  0;
  int opt,error =  0;

  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "d:u:seh")) != -1) {
    switch (opt) {
    case 'd':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0) {
        fprintf(stderr, "%s: invalid number of delay -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      delay = value;
      break;
    case 'u':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0) {
        fprintf(stderr, "%s: invalid number of utc delay -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      utc_delay = value;
      break;
    case 's':
      sis = 1;
      break;
    case 'e':
      esr = 1;
      break;
    case 'h':
      help(argv[0]);
      return 1;
    case ':':
    case '?':
      error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", argv[0]);
      return 1;
    }
  }

  if (error) return 1;
  
  if (optind + 1 != argc) {
    fprintf(stderr, "%s: expecting three non-optional arguments: <proto/host/port> \n", argv[0]);
    return 1;
  }
  
  //const char *devName = "tcp/scuxl0089.acc.gsi.de";;
  const char *devName = argv[optind];
  
  if (sis&&esr) 
  {
    fprintf(stderr, "%s: please specigy either -e (ESR) or -s (SIS), not both.\n", argv[0]);
    return 1;
  }

  /* open Etherbone device and socket */
  if ((eb_status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &socket)) != EB_OK) die(argv[0], "eb_socket_open", eb_status);
  if ((eb_status = eb_device_open(socket, devName, EB_ADDR32|EB_DATA32, 3, &device)) != EB_OK) die(argv[0], "eb_device_open", eb_status);

  #define MAX_DEVICES 8
  struct sdb_device devices[MAX_DEVICES];
  int num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, UINT64_C(0x651), UINT32_C(0x54111351), devices, &num_devices);
  //eb_sdb_find_by_identity(device, UINT64_C(0x651), UINT32_C(0xaa7bfb3c), &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no matching devices found\n", argv[0]);
    return 1;
  }


  //printf("found %d devices\n", num_devices);
  int device_idx = -1;
  uint64_t device_addr = 0;
  for (int i = 0; i < num_devices; ++i)
  {
    uint64_t addr_first = devices[i].sdb_component.addr_first;
    //uint64_t addr_last  = devices[i].sdb_component.addr_last ;

    //printf("device %d : adr = [%" PRIx64 ",%" PRIx64 "]\n",i,addr_first, addr_last);

    uint32_t magic_number;
    eb_status_t status;
    status = eb_device_read(device, addr_first+WR_MIL_GW_SHARED_OFFSET, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&magic_number, 0, eb_block);   
    if (status != EB_OK){
      printf("not ok\n");
      continue;
    }
    else{
      //printf("%d %x\n", (int)status, (int)magic_number);
    }
    
    if (magic_number == WR_MIL_GW_MAGIC_NUMBER)
    {
      device_idx = 1;
      device_addr = addr_first;
      break;
    }
  }

  if (device_idx == -1)
  {
    fprintf(stderr, "no WR-MIL gateway found\n");
    return 1;
  }

  //printf("wr-mil-gw is at index %d device addr = %" PRIx64 "\n", device_idx, device_addr);

  if (esr)
  {
    printf("%s: configure WR-MIL gateway as ESR source\n", argv[0]);
    eb_device_write(device, device_addr+WR_MIL_GW_SHARED_OFFSET+WR_MIL_GW_REG_COMMAND, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_CONFIG_ESR, 0, eb_block);
  }
  if (sis) 
  {
    printf("%s: configure WR-MIL gateway as SIS source\n", argv[0]);
    eb_device_write(device, device_addr+WR_MIL_GW_SHARED_OFFSET+WR_MIL_GW_REG_COMMAND, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_CONFIG_SIS, 0, eb_block);
  }
  if (delay >= 0) 
  {
    printf("%s: set delay = %d us\n", argv[0], delay);
    eb_device_write(device, device_addr+WR_MIL_GW_SHARED_OFFSET+WR_MIL_GW_REG_UTC_DELAY, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)delay, 0, eb_block);
  }
  if (utc_delay >= 0) 
  {
    printf("%s: set utc delay = %d us\n", argv[0], utc_delay);
    eb_device_write(device, device_addr+WR_MIL_GW_SHARED_OFFSET+WR_MIL_GW_REG_UTC_SEPARATION, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utc_delay, 0, eb_block);
  }

  return 0;
}
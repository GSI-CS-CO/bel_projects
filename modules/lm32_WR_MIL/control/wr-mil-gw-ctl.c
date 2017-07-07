// standard includes 
#include <unistd.h> // getopt
#include <getopt.h>
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

const char* state_str(uint32_t state)
{
  switch(state)
  {
    case WR_MIL_GW_STATE_INIT:         return "WR_MIL_GW_STATE_INIT";
    case WR_MIL_GW_STATE_UNCONFIGURED: return "WR_MIL_GW_STATE_UNCONFIGURED";
    case WR_MIL_GW_STATE_CONFIGURED:   return "WR_MIL_GW_STATE_CONFIGURED";
    case WR_MIL_GW_STATE_PAUSED:       return "WR_MIL_GW_STATE_PAUSED";
  }
  return "";
}

const char* event_source_str(uint32_t source)
{
  switch(source)
  {
    case WR_MIL_GW_EVENT_SOURCE_UNKNOWN:  return "WR_MIL_GW_EVENT_SOURCE_UNKNOWN";
    case WR_MIL_GW_EVENT_SOURCE_SIS:      return "WR_MIL_GW_EVENT_SOURCE_SIS";
    case WR_MIL_GW_EVENT_SOURCE_ESR:      return "WR_MIL_GW_EVENT_SOURCE_ESR";
  }
  return "";
}

void help(const char *program) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -d <delay>     microseconds between trigger event and generated events\n");
  fprintf(stderr, "  -u <utc-delay> microseconds between 5 generated utc events\n");
  fprintf(stderr, "  -t <EvtNo>     MIL event number that tirggers generation of UTC events\n");
  fprintf(stderr, "  -l <latency>   MIL event is generated latency microseconds. default is 100 us\n");
  fprintf(stderr, "  -e             configure WR-MIL gateway as ESR source\n");
  fprintf(stderr, "  -s             configure WR-MIL gateway as SIS source\n");
  fprintf(stderr, "  -r             reset WR-MIL gateway after 1 second pause\n");
  fprintf(stderr, "  -k             kill WR-MIL gateway, only reset or eb-fwload can recover (useful for eb-fwload)\n");
  fprintf(stderr, "  -i             print information about the WR-MIL gateway\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <m.reese@gsi.de>\n");
  fprintf(stderr, "Version: %s\n%s\nLicensed under the LGPL v3.\n", eb_source_version(), eb_build_info());
}

int main(int argc, char *argv[])
{
  eb_device_t device;
  eb_status_t eb_status;
  eb_socket_t socket;

  char *value_end;
  int value;
  int delay       = -1;
  int utc_delay   = -1;
  int utc_trigger = -1;
  int latency     =  0;
  int set_latency =  0;
  int sis         =  0;
  int esr         =  0;
  int reset       =  0;
  int kill        =  0;
  int info        =  0;
  int opt,error   =  0;

  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "l:d:u:t:sehrki")) != -1) {
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
    case 't':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0 || value > 255) {
        fprintf(stderr, "%s: invalid EvtNO for utc trigger -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      utc_trigger = value;
      break;
    case 'l':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0 || value > 255) {
        fprintf(stderr, "%s: invalid EvtNO for utc trigger -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      latency = value;
      set_latency = 1;
      break;
    case 's':
      sis = 1;
      break;
    case 'r':
      reset = 1;
      break;
    case 'k':
      kill = 1;
      break;
    case 'e':
      esr = 1;
      break;
    case 'i':
      info = 1;
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
    fprintf(stderr, "%s: expecting one non-optional argument: <proto/host/port> \n", argv[0]);
    return 1;
  }
  
  //const char *devName = "tcp/scuxl0089.acc.gsi.de";;
  const char *devName = argv[optind];
  
  if (sis&&esr) 
  {
    fprintf(stderr, "%s: please specify either -e (ESR) or -s (SIS), not both.\n", argv[0]);
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

    uint32_t magic_number;
    eb_status_t status;
    // find the correct user LM32 by reading the first value of the shared memory segment and expecting a certain value for the WR-MIL gateway
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

  // the register adresses 
  uint32_t reg_shared_addr          = device_addr+WR_MIL_GW_SHARED_OFFSET;
  uint32_t reg_magic_addr           = reg_shared_addr+WR_MIL_GW_REG_MAGIC_NUMBER;
  uint32_t reg_command_addr         = reg_shared_addr+WR_MIL_GW_REG_COMMAND;
  uint32_t reg_utc_trigger_addr     = reg_shared_addr+WR_MIL_GW_REG_UTC_TRIGGER;
  uint32_t reg_utc_separation_addr  = reg_shared_addr+WR_MIL_GW_REG_UTC_SEPARATION;
  uint32_t reg_utc_delay_addr       = reg_shared_addr+WR_MIL_GW_REG_UTC_DELAY;
  uint32_t reg_event_source_addr    = reg_shared_addr+WR_MIL_GW_REG_EVENT_SOURCE;
  uint32_t reg_latency_addr         = reg_shared_addr+WR_MIL_GW_REG_LATENCY;
  uint32_t reg_state_addr           = reg_shared_addr+WR_MIL_GW_REG_STATE;

  if (reset)
  {
    printf("%s: reset of WR-MIL gateway after 1 second pause\n", argv[0]);
    if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_RESET, 0, eb_block)) != EB_OK) {
      die(argv[0],"command WR_MIL_GW_CMD_RESET", eb_status);
    }
  }
  if (kill)
  {
    printf("%s: kill WR-MIL gateway\n", argv[0]);
    if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_KILL, 0, eb_block)) != EB_OK) {
      die(argv[0],"command WR_MIL_GW_CMD_KILL", eb_status);
    }
  }
  uint32_t state, source;
  eb_status = eb_device_read(device, reg_event_source_addr,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
  source = value;
  eb_status = eb_device_read(device, reg_state_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
  state = value;
  if (state == WR_MIL_GW_STATE_CONFIGURED &&
      (esr || sis || delay >=0 || utc_delay >= 0 || utc_trigger >= 0 || set_latency))
  {
    fprintf(stderr, "%s: cannot configure gateway. It is already running as %s\n", argv[0], event_source_str(source));
    esr = sis = 0; 
  }
  else
  {

    if (delay >= 0) 
    {
      printf("%s: set delay = %d us\n", argv[0], delay);
      if ((eb_status = eb_device_write(device, reg_utc_delay_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)delay, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_UTC_DELAY", eb_status);
      }
    }
    if (utc_delay >= 0) 
    {
      printf("%s: set utc delay = %d us\n", argv[0], utc_delay);
      if ((eb_status = eb_device_write(device, reg_utc_separation_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utc_delay, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_UTC_SEPARATION", eb_status);
      }
    }
    if (utc_trigger >= 0)
    {
      printf("%s: set utc trigger evtNo to 0x%x = %d \n", argv[0], utc_trigger, utc_trigger);
      if ((eb_status = eb_device_write(device, reg_utc_trigger_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utc_trigger, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_UTC_TRIGGER", eb_status);
      }
    }
    if (set_latency)
    {
      printf("%s: set event latency to %d us\n", argv[0], latency);
      if ((eb_status = eb_device_write(device, reg_latency_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)latency, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_LATENCY", eb_status);
      }
    }

    if (esr)
    {
      printf("%s: configure WR-MIL gateway as ESR source\n", argv[0]);
      if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_CONFIG_ESR, 0, eb_block)) != EB_OK) {
        die(argv[0],"command WR_MIL_GW_CMD_CONFIG_ESR", eb_status);
      }
    }
    if (sis) 
    {
      printf("%s: configure WR-MIL gateway as SIS source\n", argv[0]);
      if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_CONFIG_SIS, 0, eb_block)) != EB_OK) {
        die(argv[0],"command WR_MIL_GW_CMD_CONFIG_SIS", eb_status);
      }
    }    
  }

  if (info)
  {
    printf("%s: WR-MIL status regitster content:\n", argv[0]);
    uint32_t value;
    eb_status = eb_device_read(device, reg_magic_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_MAGIC_NUMBER:   0x%08x\n", value);
    eb_status = eb_device_read(device, reg_command_addr,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_COMMAND:        0x%08x\n", value);
    eb_status = eb_device_read(device, reg_utc_trigger_addr,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_UTC_TRIGGER:    0x%08x = %d\n", value, value);
    eb_status = eb_device_read(device, reg_utc_separation_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_UTC_SEPARATION: 0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_utc_delay_addr,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_UTC_DELAY:      0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_event_source_addr,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_EVENT_SOURCE:   0x%08x = %s\n", value, event_source_str(value));
    eb_status = eb_device_read(device, reg_latency_addr,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_LATENCY:        0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_state_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_STATE:          0x%08x = %s\n", value, state_str(value));
  }

  return 0;
}
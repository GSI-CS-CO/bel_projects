// standard includes 
#include <unistd.h> // getopt
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <string>

// Etherbone
#include <etherbone.h>

// dm-unipz
#include <wr_mil_gw.h>

#ifdef USEMASP
#include "MASP/Emitter/StatusEmitter.h"
#include "MASP/StatusDefinition/DeviceStatus.h"
#include "MASP/Util/Logger.h"
#include "MASP/Common/StatusNames.h"
#include "MASP/Emitter/End_of_scope_status_emitter.h"
#include <boost/thread/thread.hpp> // (sleep)
#include <iostream>
#include <string>
#endif // USEMASP

void die(const char *program, const char* where, eb_status_t status) {
  fprintf(stderr, "%s: %s failed: %s\n",
          program, where, eb_status(status));
  exit(1);
} //die

const char* state_str(uint32_t state, bool human_readable = false)
{
  switch(state)
  {
    case WR_MIL_GW_STATE_INIT:         return human_readable?"initial":"WR_MIL_GW_STATE_INIT";
    case WR_MIL_GW_STATE_UNCONFIGURED: return human_readable?"unconfigured":"WR_MIL_GW_STATE_UNCONFIGURED";
    case WR_MIL_GW_STATE_CONFIGURED:   return human_readable?"configured":"WR_MIL_GW_STATE_CONFIGURED";
    case WR_MIL_GW_STATE_PAUSED:       return human_readable?"paused":"WR_MIL_GW_STATE_PAUSED";
  }
  return "";
}

const char* event_source_str(uint32_t source, bool human_readable = false)
{
  switch(source)
  {
    case WR_MIL_GW_EVENT_SOURCE_UNKNOWN:  return human_readable?"unknown":"WR_MIL_GW_EVENT_SOURCE_UNKNOWN";
    case WR_MIL_GW_EVENT_SOURCE_SIS:      return human_readable?"SIS":"WR_MIL_GW_EVENT_SOURCE_SIS";
    case WR_MIL_GW_EVENT_SOURCE_ESR:      return human_readable?"ESR":"WR_MIL_GW_EVENT_SOURCE_ESR";
  }
  return "";
}

void help(const char *program) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -d <delay>      microseconds between trigger event and generated events\n");
  fprintf(stderr, "  -u <utc-delay>  microseconds between 5 generated utc events\n");
  fprintf(stderr, "  -o <utc-offset> zero of UTC seconds in TAI seconds: MIL-UTC[s] = TAI[s] - <utc-offset> \n");
  fprintf(stderr, "  -t <EvtNo>      MIL event number that tirggers generation of UTC events\n");
  fprintf(stderr, "  -l <latency>    MIL event is generated latency microseconds. default is 100 us\n");
  fprintf(stderr, "  -e              configure WR-MIL gateway as ESR source\n");
  fprintf(stderr, "  -s              configure WR-MIL gateway as SIS source\n");
  fprintf(stderr, "  -r              reset WR-MIL gateway after 1 second pause\n");
  fprintf(stderr, "  -k              kill WR-MIL gateway, only reset or eb-fwload can recover (useful for eb-fwload)\n");
  fprintf(stderr, "  -i              print information about the WR-MIL gateway (register content)\n");
  fprintf(stderr, "  -m              monitor gateway status registers and report irregularities on stdout\n");
#ifdef USEMASP
  fprintf(stderr, "  -M SIS|ESR      monitor gateway status registers and send MASP status as SIS or ESR nomen\n");
#endif // USEMASP
  fprintf(stderr, "  -h              display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report software bugs to <m.reese@gsi.de>\n");
  fprintf(stderr, "Licensed under the LGPL v3.\n");//, eb_source_version(), eb_build_info());
}

void fill_display_content(eb_device_t device, uint32_t reg_adr, bool op_ready, const char *SIS_or_ESR, uint64_t messages, uint32_t late_messages)
{
  eb_status_t eb_status;
  char buffer[66];
  if (op_ready) sprintf(buffer, "\rWR-MIL-%s\rOP_READY  \rMIL events\r%10ld\rlate      \r%10d",SIS_or_ESR,messages,late_messages);
  else          sprintf(buffer, "\rWR-MIL-%s\rNOT READY \r          \r          \r          \r          ",SIS_or_ESR);

  // reset OLED display
  if ((eb_status = eb_device_write(device, reg_adr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)0, 0, eb_block)) != EB_OK) {
    die("wr-mil-gateway","reset OLED display", eb_status);
  }
  for (int i = 0; i < 66; ++i)
  {
    if ((eb_status = eb_device_write(device, reg_adr+0x8, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)buffer[i], 0, eb_block)) != EB_OK) {
      die("wr-mil-gateway","write to OLED display", eb_status);
    }
  }
}

int main(int argc, char *argv[])
{
  eb_device_t device;
  eb_status_t eb_status;
  eb_socket_t socket;

  char *value_end;
  int64_t value;
  int     delay       = -1;
  int     utc_delay   = -1;
  int64_t utc_offset  = -1;
  int     utc_trigger = -1;
  int     latency     =  0;
  int     set_latency =  0;
  int     sis         =  0;
  int     esr         =  0;
  int     reset       =  0;
  int     kill        =  0;
  int     info        =  0;
  int     monitor     =  0;
  int     opt,error   =  0;

  std::string MASP_SIS_ESR;


  /* Process the command-line arguments */
  while ((opt = getopt(argc, argv, "l:d:u:o:t:sehrkimM:")) != -1) {
    switch (opt) {
    case 'd':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0) {
        fprintf(stderr, "%s: invalid number for delay -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      delay = value;
      break;
    case 'u':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0) {
        fprintf(stderr, "%s: invalid number for utc delay -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      utc_delay = value;
      break;
    case 'o':
      value = strtol(optarg, &value_end, 0);
      if (*value_end || value < 0) {
        fprintf(stderr, "%s: invalid number for utc offset -- '%s'\n", argv[0], optarg);
        error = 1;
      }
      utc_offset = value*1000; // convert from seconds to miliseconds
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
    case 'm':
      monitor = 1;
      break;
#ifdef USEMASP
    case 'M':
      monitor = 1;
      if (!optarg)
      {
        fprintf(stderr, "%s: specify MASP nomen \"%s\", use SIS or ESR\n", argv[0], optarg);
        error = 1;
        break;
      }
      printf("optarg = %s\n", optarg);
      MASP_SIS_ESR = std::string(optarg);
      if (MASP_SIS_ESR != "SIS" && MASP_SIS_ESR != "ESR")
      {
        fprintf(stderr, "%s: invalid MASP nomen \"%s\", use SIS or ESR\n", argv[0], optarg);
        error = 1;
      }
      break;
#endif // USEMASP
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

  // find user LM32 devices
  #define MAX_DEVICES 8
  struct sdb_device devices[MAX_DEVICES];
  int num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, UINT64_C(0x651), UINT32_C(0x54111351), devices, &num_devices);
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

  // find addr of OLED display
  eb_sdb_find_by_identity(device, UINT64_C(0x651), UINT32_C(0x93a6f3c4), devices, &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no matching OLED display found\n", argv[0]);
    return 1;
  }
  uint32_t oled_reg_reset          = devices[0].sdb_component.addr_first;

  fill_display_content(device, oled_reg_reset, 0, "   ", 42, 41);




  // the register adresses 
  uint32_t reg_shared_addr          = device_addr+WR_MIL_GW_SHARED_OFFSET;
  uint32_t reg_magic_addr           = reg_shared_addr+WR_MIL_GW_REG_MAGIC_NUMBER;
  uint32_t reg_command_addr         = reg_shared_addr+WR_MIL_GW_REG_COMMAND;
  uint32_t reg_utc_trigger_addr     = reg_shared_addr+WR_MIL_GW_REG_UTC_TRIGGER;
  uint32_t reg_utc_delay_addr       = reg_shared_addr+WR_MIL_GW_REG_UTC_DELAY;
  uint32_t reg_utc_offset_hi_addr   = reg_shared_addr+WR_MIL_GW_REG_UTC_OFFSET_HI;
  uint32_t reg_utc_offset_lo_addr   = reg_shared_addr+WR_MIL_GW_REG_UTC_OFFSET_LO;
  uint32_t reg_trig_utc_delay_addr  = reg_shared_addr+WR_MIL_GW_REG_TRIG_UTC_DELAY;
  uint32_t reg_event_source_addr    = reg_shared_addr+WR_MIL_GW_REG_EVENT_SOURCE;
  uint32_t reg_latency_addr         = reg_shared_addr+WR_MIL_GW_REG_LATENCY;
  uint32_t reg_state_addr           = reg_shared_addr+WR_MIL_GW_REG_STATE;
  uint32_t reg_num_events_hi_addr   = reg_shared_addr+WR_MIL_GW_REG_NUM_EVENTS_HI;
  uint32_t reg_num_events_lo_addr   = reg_shared_addr+WR_MIL_GW_REG_NUM_EVENTS_LO;
  uint32_t reg_late_events_addr     = reg_shared_addr+WR_MIL_GW_REG_LATE_EVENTS;

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
      if ((eb_status = eb_device_write(device, reg_trig_utc_delay_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)delay, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_TRIG_UTC_DELAY", eb_status);
      }
    }
    if (utc_delay >= 0) 
    {
      printf("%s: set utc delay = %d us\n", argv[0], utc_delay);
      if ((eb_status = eb_device_write(device, reg_utc_delay_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)utc_delay, 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_UTC_DELAY", eb_status);
      }
    }
    if (utc_offset >= 0) 
    {
      printf("%s: set utc offset = %ld ms\n", argv[0], utc_offset);
      if ((eb_status = eb_device_write(device, reg_utc_offset_hi_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)((utc_offset>>32) & UINT64_C(0xffffffff)), 0, eb_block)) != EB_OK || 
          (eb_status = eb_device_write(device, reg_utc_offset_lo_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)( utc_offset      & UINT64_C(0xffffffff)), 0, eb_block)) != EB_OK) {
        die(argv[0],"configure register WR_MIL_GW_REG_UTC_OFFSET", eb_status);
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
    printf("WR-MIL shared memory regitster content:\n");
    uint32_t magic_number;
    uint32_t gateway_state;
    uint32_t event_source;
    uint32_t value;
    uint64_t value64_bit;
    eb_status = eb_device_read(device, reg_magic_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_MAGIC_NUMBER:   0x%08x\n", value);
    magic_number = value; // is used later
    eb_status = eb_device_read(device, reg_command_addr,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_COMMAND:        0x%08x\n", value);
    eb_status = eb_device_read(device, reg_utc_trigger_addr,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_UTC_TRIGGER:    0x%08x = %d\n", value, value);
    eb_status = eb_device_read(device, reg_utc_delay_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_UTC_DELAY:      0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_utc_delay_addr,      EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_TRIG_UTC_DELAY: 0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_event_source_addr,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_EVENT_SOURCE:   0x%08x = %s\n", value, event_source_str(value));
    event_source = value;
    eb_status = eb_device_read(device, reg_latency_addr,        EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_LATENCY:        0x%08x = %d us\n", value, value);
    eb_status = eb_device_read(device, reg_state_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_STATE:          0x%08x = %s\n", value, state_str(value));
    gateway_state = value;
    eb_status = eb_device_read(device, reg_utc_offset_hi_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    value64_bit = value;
    value64_bit <<= 32;
    printf("    WR_MIL_GW_REG_UTC_OFFSET_HI:  0x%08x\n", value);
    eb_status = eb_device_read(device, reg_utc_offset_lo_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    value64_bit |= value;
    printf("    WR_MIL_GW_REG_UTC_OFFSET_LO:  0x%08x = %ld s\n", value, value64_bit/1000);
    eb_status = eb_device_read(device, reg_num_events_hi_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    value64_bit = value;
    value64_bit <<= 32;
    printf("    WR_MIL_GW_REG_NUM_EVENTS_HI:  0x%08x\n", value);
    eb_status = eb_device_read(device, reg_num_events_lo_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    value64_bit |= value;
    printf("    WR_MIL_GW_REG_NUM_EVENTS_LO:  0x%08x = %ld\n", value, value64_bit);
    eb_status = eb_device_read(device, reg_late_events_addr,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    printf("    WR_MIL_GW_REG_LATE_EVENTS:    0x%08x = %d\n", value, value);

    // see if the firmware is running (it should reset the CMD register to 0 after a command is put there)
    // submit a test command 
    if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_TEST, 0, eb_block)) != EB_OK) {
      die(argv[0],"command WR_MIL_GW_CMD_TEST", eb_status);
    }
    usleep(50000);
    eb_device_read(device, reg_command_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
    if (magic_number == WR_MIL_GW_MAGIC_NUMBER)
    {
      printf("\nWR-MIL-GATEWAY firmware was found on user LM32[%d]. \n", device_idx);
      if (value) printf("  firmware : not running!\n");
      else       printf("  firmware : running\n");
      printf("  state    : %s\n", state_str(gateway_state,true));
      printf("  source   : %s\n", event_source_str(event_source,true));
    }
    else
    {
      printf("\nNo WR-MIL-GATEWAY firmware was found\n");
    }
   
  }

  if (monitor)
  {
    uint32_t last_late_events = 0;
    uint64_t last_num_events = 0;
    uint32_t missing_events_message_written = 0;

#ifdef USEMASP
        // send MASP status
        std::string nomen("WR_MIL_GW");
        nomen.append("_");
        nomen.append(MASP_SIS_ESR);
        char hostname_cstr[100];
        gethostname(hostname_cstr,100);
        std::string hostname(hostname_cstr);
        std::string source_id(nomen);
        source_id.append(".");
        source_id.append(hostname);
        bool masp_productive = 
#ifdef PRODUCTIVE
            true;
#else 
            false;
#endif //PRODUCTIVE

        MASP::StatusEmitter emitter(MASP::StatusEmitterConfig(
            MASP::StatusEmitterConfig::CUSTOM_EMITTER_DEFAULT(),
            source_id, masp_productive ));
        //printf ("prepare MASP status emitter with source_id: %s, nomen: %s, productive: %\n");
        std::cout << "prepare MASP status emitter with "
                  << "source_id: " << source_id 
                  << ",  nomen: " << nomen
                  << ",  productive: " << (masp_productive?"true":"false")
                  << std::endl;

   MASP::no_logger no_log;
   MASP::Logger::middleware_logger = &no_log;

#endif  // USEMASP

    for (;;)
    {
      uint32_t value;
      uint64_t value64_bit;
      eb_status = eb_device_read(device, reg_num_events_hi_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
      value64_bit = value;
      value64_bit <<= 32;
      //printf("    WR_MIL_GW_REG_NUM_EVENTS_HI:  0x%08x\n", value);
      eb_status = eb_device_read(device, reg_num_events_lo_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
      value64_bit |= value;
      //printf("    WR_MIL_GW_REG_NUM_EVENTS_LO:  0x%08x = %ld\n", value, value64_bit);
      eb_status = eb_device_read(device, reg_late_events_addr,    EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
      //printf("    WR_MIL_GW_REG_LATE_EVENTS:    0x%08x = %d\n", value, value);

      if (last_num_events && last_num_events == value64_bit)
      {
        if (!missing_events_message_written)
        { // complain that no events are arriving, but complain only once
          printf("WR-MIL-GATEWAY WARNING: Number of translated MIL events did not increase!\n"
                 "  If lack of MIL events is not intentional: check if wr-mil-gateway and Data Master are both active\n");
          fflush(stdout);
          missing_events_message_written = 1;
        }
      }
      else
      {
        if (missing_events_message_written)
        {
          // the number of translated MIL events increased, we can reset the message indicator and log that MIL events are back
          printf("WR-MIL-GATEWAY: There are MIL events again!\n");
          fflush(stdout);
          missing_events_message_written = 0;
        }
      }
      if (last_late_events < value)
      {
        printf("WR-MIL-GATEWAY WARNING: Late MIL event occured!\n"
               "  Number of delayed events since last message/reset: %d/%d\n", value - last_late_events, value);
        fflush(stdout);
      }

      last_late_events = value;
      last_num_events  = value64_bit;



      // wait for 10 s until the number of events is checked again
      for (int i = 0; i < 60; ++i) 
      {
        // check if the firmware is still running (it should reset the CMD register to 0 after a command is put there)
        // submit a test command 
        if ((eb_status = eb_device_write(device, reg_command_addr, EB_BIG_ENDIAN|EB_DATA32, (eb_data_t)WR_MIL_GW_CMD_TEST, 0, eb_block)) != EB_OK) {
          die(argv[0],"command WR_MIL_GW_CMD_TEST", eb_status);
        }
        fflush(stdout);
        sleep(1);
        eb_device_read(device, reg_command_addr,  EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
        bool firmware_running = (value==0);
        // the firmware should have set value to 0, if not the firmware is not running
        if (!firmware_running) 
        { // the firmware is not running. This schould not happen!
          printf("WR-MIL-GATEWAY: firmware not running!\n");
        }
        // read gateway state
        eb_status = eb_device_read(device, reg_state_addr,          EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
        int gateway_state = value;
        bool gateway_active = (gateway_state==WR_MIL_GW_STATE_CONFIGURED);
        if (!gateway_active) 
        { // the firmware is not running. This schould not happen!
          printf("WR-MIL-GATEWAY: gateway is not active!\n");
        }

        // read gateway source type
        eb_status = eb_device_read(device, reg_event_source_addr,   EB_BIG_ENDIAN|EB_DATA32, (eb_data_t*)&value, 0, eb_block);
        int event_source = value;
        bool correct_source_type = false;
        if (MASP_SIS_ESR == "ESR" && event_source == WR_MIL_GW_EVENT_SOURCE_ESR) correct_source_type = true;
        if (MASP_SIS_ESR == "SIS" && event_source == WR_MIL_GW_EVENT_SOURCE_SIS) correct_source_type = true;

        if (!correct_source_type) 
        { // the monitor tool is started on the wrong gateway
          printf("WR-MIL-GATEWAY: gateway has wrong source type!\n");
        }
        // send MASP status
        if (MASP_SIS_ESR == "ESR" || MASP_SIS_ESR == "SIS")
        {
          //printf ("send MASP message\n");
          {
              bool op_ready = false;
              // send OP_READY only if firmware is running and the gateway is configured as the correct source type
              if (firmware_running && gateway_active && correct_source_type) op_ready = true;

              // update display
              fill_display_content(device, oled_reg_reset, op_ready, MASP_SIS_ESR.c_str(), last_num_events, last_late_events);

#ifdef USEMASP
              MASP::End_of_scope_status_emitter scoped_emitter(nomen,emitter);
              scoped_emitter.set_OP_READY(op_ready);
              //std::cout << "send op_ready " << op_ready << std::endl;
#endif  // USEMASP          
              //scoped_emitter.set_custom_status("TEST_SIGNAL",true);
          } // <--- status is send when the End_of_scope_emitter goes out of scope
        }

      }
    }
  }

  return 0;
}
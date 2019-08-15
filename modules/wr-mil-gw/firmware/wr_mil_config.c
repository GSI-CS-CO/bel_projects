#include <inttypes.h>

#include "wr_mil_config.h"
#include "mini_sdb.h"
#include "pp-printf.h"
#include "wr_mil_delay.h"
#include "wr_mil_eca_queue.h"
#include "wr_mil_events.h"
#include "wr_mil_gw.h"

extern volatile uint32_t _startshared[]; // provided in linker script "ram.ld"
#define SHARED __attribute__((section(".shared")))
uint64_t SHARED dummy = 0; // not sure if that variable is really needed
                           // the extern volatile uint32_t _startshared[] should be enough



volatile WrMilConfig *config_init()
{
  volatile WrMilConfig *config = (volatile WrMilConfig*)_startshared;
  config->magic                = WR_MIL_GW_MAGIC_NUMBER;
  config->cmd                  = WR_MIL_GW_CMD_NONE;
  config->utc_trigger          = MIL_EVT_BEGIN_CMD_EXEC;
  config->utc_delay            = 30; // us delay between utc events
  config->trigger_utc_delay    = 0;  // us delay between utc-trigger and first utc event
  config->event_source         = WR_MIL_GW_EVENT_SOURCE_UNKNOWN; // not configured by default
  config->latency              = 100; // us
  config->state                = WR_MIL_GW_STATE_INIT;
  config->utc_offset_ms.value  = UINT64_C(1199142000000);
  config->num_events.value     = UINT64_C(0);
  config->late_events          = UINT64_C(0);
  for (int i = 0; i < 255; ++i) {
    config->mil_histogram[i] = UINT32_C(0);
  }
  config->mb_slot              = UINT32_C(0xffffffff); // this is an invalid value
  config->op_ready             = UINT32_C(0);
  config->request_fill_evt     = UINT32_C(0);
  return config;
}

// read the config->cmd register and take action.
// this includes direct changes of the program state (config->state)
void config_command_handler(volatile WrMilConfig *config)
{
  if (config->cmd)
  {
    switch(config->cmd)
    {
      case WR_MIL_GW_CMD_KILL:
        pp_printf("stop MCU\n");
        config->state = WR_MIL_GW_STATE_PAUSED;
        while(1);
      case WR_MIL_GW_CMD_RESET:
        pp_printf("wr-mil-gw reset after pause of 1 sec\n");
        { 
          int current_state = config->state; 
          config->state = WR_MIL_GW_STATE_PAUSED;
          config->event_source = WR_MIL_GW_EVENT_SOURCE_UNKNOWN; //reset the source type
          for (int i = 0; i < 1000; ++i) DELAY1000us;
            
          config->state             = WR_MIL_GW_STATE_INIT;
          config->num_events.value  = UINT64_C(0);
          config->late_events       = UINT64_C(0);
        }
        break;
      case WR_MIL_GW_CMD_CONFIG_SIS: // allow configuration of PZ-id only if not configured yet
        if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
        {
          config->event_source = WR_MIL_GW_EVENT_SOURCE_SIS;
          config->state = WR_MIL_GW_STATE_CONFIGURED;
          pp_printf("wr-mil-gw configured as SIS event source\n");
        }
        break;
      case WR_MIL_GW_CMD_CONFIG_ESR: // allow configuration of PZ-id only if not configured yet
        if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
        {
          config->event_source = WR_MIL_GW_EVENT_SOURCE_ESR;
          config->state = WR_MIL_GW_STATE_CONFIGURED;
          pp_printf("wr-mil-gw configured as ESR event source\n");
        }
        break;
      case WR_MIL_GW_CMD_TEST: // do nothing 
        break;
      case WR_MIL_GW_CMD_UPDATE_OLED: 
        break;
      default:
      {
        pp_printf("wr-mil-gw unknown command %08x\n", config->cmd);
        break;
      }
    }
    config->cmd = UINT32_C(0);  
  }
}

// check if the command (cmd) register is != 0, 
//  do stuff according to its value, and set it 
//  back to 0
void config_poll(volatile WrMilConfig *config)
{
  switch(config->state)
  {
    case WR_MIL_GW_STATE_INIT:
      if (config->event_source == WR_MIL_GW_EVENT_SOURCE_UNKNOWN) {
        pp_printf("wr-mil-gw not configured\n");
        config->state = WR_MIL_GW_STATE_UNCONFIGURED;
      }
      else
      {
        config->state = WR_MIL_GW_STATE_CONFIGURED;
      }
      break;
    case WR_MIL_GW_STATE_UNCONFIGURED:
      if (config->event_source != WR_MIL_GW_EVENT_SOURCE_UNKNOWN) {
        config->state = WR_MIL_GW_STATE_CONFIGURED;
      }
      // fall through
    case WR_MIL_GW_STATE_CONFIGURED:
      config_command_handler(config);
      break;
    default:
      pp_printf("wr-mil-gw unknown state\n");
      config->state = WR_MIL_GW_STATE_INIT;
  }
}

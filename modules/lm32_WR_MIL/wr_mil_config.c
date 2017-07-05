#include "wr_mil_config.h"
#include "mini_sdb.h"
#include "mprintf.h"
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
  config->utc_trigger          = MIL_EVT_END_CYCLE;
  config->utc_delay            = 100; // 100 us delay
  config->trigger_utc_delay    = 200; // 200 us delay
  config->event_source         = WR_MIL_GW_EVENT_SOURCE_UNKNOWN; // not configured by default
  config->latency              = 100; // us
  config->state                = WR_MIL_GW_STATE_INIT;
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
        mprintf("stop MCU\n");
        config->state = WR_MIL_GW_STATE_PAUSED;
        while(1);
      case WR_MIL_GW_CMD_RESET:
        mprintf("wr-mil-gw reset after pause of 1 sec\n");
        { 
          int current_state = config->state; 
          config->state = WR_MIL_GW_STATE_PAUSED;
          config->event_source = WR_MIL_GW_EVENT_SOURCE_UNKNOWN; //reset the source type
          for (int i = 0; i < 1000; ++i) DELAY1000us;
          config->state = WR_MIL_GW_STATE_INIT;

        }
        break;
      case WR_MIL_GW_CMD_CONFIG_SIS: // allow configuration of PZ-id only if not configured yet
        if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
        {
          config->event_source = WR_MIL_GW_EVENT_SOURCE_SIS;
          config->state = WR_MIL_GW_STATE_CONFIGURED;
          mprintf("wr-mil-gw configured as SIS event source\n");
        }
        break;
      case WR_MIL_GW_CMD_CONFIG_ESR: // allow configuration of PZ-id only if not configured yet
        if (config->state == WR_MIL_GW_STATE_UNCONFIGURED)
        {
          config->event_source = WR_MIL_GW_EVENT_SOURCE_ESR;
          config->state = WR_MIL_GW_STATE_CONFIGURED;
          mprintf("wr-mil-gw configured as ESR event source\n");
        }
        break;
      default:
        mprintf("wr-mil-gw unknown command %08x\n", config->cmd);
        break;
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
        mprintf("wr-mil-gw not configured\n");
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
      mprintf("wr-mil-gw unknown state\n");
      config->state = WR_MIL_GW_STATE_INIT;
  }
}

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
  config->state                = WR_MIL_GW_STATE_INIT;
  return config;
}

void config_command_handler(volatile WrMilConfig *config)
{
  if (config->cmd)
  {
    switch(config->cmd)
    {
      case WR_MIL_GW_CMD_FULL_STOP:
        mprintf("stop MCU\n");
        while(1);
      case WR_MIL_GW_CMD_PAUSE_10S:
        mprintf("pause MCU for 10 sec\n");
        for (int i = 0; i < 10000; ++i) DELAY1000us;
        break;
      case WR_MIL_GW_CMD_CONFIG_SIS: 
        config->event_source = WR_MIL_GW_EVENT_SOURCE_SIS;
        config->state = WR_MIL_GW_STATE_CONFIGURED;
        mprintf("wr-mil-gw configured as SIS event source\n");
        break;
      case WR_MIL_GW_CMD_CONFIG_ESR: 
        config->event_source = WR_MIL_GW_EVENT_SOURCE_ESR;
        config->state = WR_MIL_GW_STATE_CONFIGURED;
        mprintf("wr-mil-gw configured as ESR event source\n");
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
      mprintf("wr-mil-gw not configured\n");
      config->state = WR_MIL_GW_STATE_CONFIGURED;
      break;
    case WR_MIL_GW_STATE_UNCONFIGURED: // fall through
    case WR_MIL_GW_STATE_CONFIGURED:
      config_command_handler(config);
      break;
    default:
      mprintf("wr-mil-gw unknown state\n");
      config->state = WR_MIL_GW_STATE_INIT;  
  }
}

#ifndef WR_MIL_CMD_H_
#define WR_MIL_CMD_H_

#include <stdint.h>

// states of the software
#define WR_MIL_GW_STATE_INIT         0
#define WR_MIL_GW_STATE_UNCONFIGURED 1
#define WR_MIL_GW_STATE_CONFIGURED   2

typedef struct 
{
	uint32_t magic;             // base + 0x0 : magic number to identify the LM32 that actually runs WR_MIL gateway
	uint32_t cmd;               // base + 0x4 : command to be executed
	uint32_t utc_trigger;       // base + 0x8 : the MIL event that triggers the generation of UTC events
	uint32_t utc_delay;         // base + 0xC : delay in us between the generated UTC events
	uint32_t trigger_utc_delay; // base + 0x10 : delay in us between the trigger event and the UTC (and other) generated events
	uint32_t event_source;      // base + 0x14: 1 = SIS ; 2 = ESR ; 0 not configured
	uint32_t state;             // base + 0x18: INITIAL, UNCONFIGURED, CONFIGURED
} WrMilConfig;


volatile WrMilConfig *config_init();

void config_poll(volatile WrMilConfig *cmd);

#endif

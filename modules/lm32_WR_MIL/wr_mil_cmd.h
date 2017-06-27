#ifndef WR_MIL_CMD_H_
#define WR_MIL_CMD_H_

#include <stdint.h>

typedef struct 
{
	uint32_t cmd;               // base + 0x0 : command to be executed
	uint32_t utc_trigger;       // base + 0x4 : the MIL event that triggers the generation of UTC events
	uint32_t utc_delay;         // base + 0x8 : delay in us between the generated UTC events
	uint32_t trigger_utc_delay; // base + 0xC : delay in us between the trigger event and the UTC (and other) generated events
	uint32_t event_source;      // base + 0x10: 1 = SIS ; 2 = ESR ; 0 not configured
} MilCmdRegs;

#define EVENT_SOURCE_NOT_CONFIGURED 0
#define EVENT_SOURCE_SIS            1
#define EVENT_SOURCE_ESR            2

volatile MilCmdRegs *MilCmd_init();

void MilCmd_poll(volatile MilCmdRegs *cmd);

#endif

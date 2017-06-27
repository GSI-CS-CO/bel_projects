#ifndef WR_MIL_CMD_H_
#define WR_MIL_CMD_H_

#include <stdint.h>

typedef struct 
{
	uint32_t cmd;          // base + 0 : command to be executed
	uint32_t utc_delay;    // base + 4 : delay in ns between the generated UTC events
	uint32_t utc_trigger;  // base + 8 : the MIL event that triggers the generation of UTC events
} MilCmdRegs;

volatile MilCmdRegs *MilCmd_init();

void MilCmd_poll(volatile MilCmdRegs *cmd);

#endif

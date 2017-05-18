#ifndef WR_MIL_CMD_H_
#define WR_MIL_CMD_H_

#include <stdint.h>

typedef struct 
{
	uint32_t status;    // base + 0
	uint32_t data;      // base + 4
	uint32_t cmd;       // base + 8
} MilCmdRegs;

volatile MilCmdRegs *MilCmd_init(uint32_t *device_addr);

void MilCmd_poll(volatile MilCmdRegs *cmd);


#endif

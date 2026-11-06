#ifndef WR_MIL_CMD_H_
#define WR_MIL_CMD_H_

#include <stdint.h>
#include "wr_mil_gw.h"
#include "wr_mil_value64bit.h"


typedef struct 
{
	uint32_t     magic;             // base + 0x0 : magic number to identify the LM32 that actually runs WR_MIL gateway
	uint32_t     cmd;               // base + 0x4 : command to be executed
	uint32_t     utc_trigger;       // base + 0x8 : the MIL event that triggers the generation of UTC events
	uint32_t     utc_delay;         // base + 0xC : delay in us between the generated UTC events
	uint32_t     trigger_utc_delay; // base + 0x10: delay in us between the trigger event and the UTC (and other) generated events
	uint32_t     event_source;      // base + 0x14: 1 = SIS ; 2 = ESR ; 0 not configured
	uint32_t     latency;           // base + 0x18: MIL event is generated 100us+latency after the WR event. 
	uint32_t     state;             // base + 0x1C: INITIAL, UNCONFIGURED, CONFIGURED
	Value64Bit_t utc_offset_ms;     // base + 0x20: delay [ms] between the TAI and the MIL-UTC, high word 
                                    // base + 0x24: delay [ms] between the trigger the MIL-UTC, low  word

	// The following registers are for monitoring, not for configuration. They should be read but not written by the config tool.
	Value64Bit_t num_events;        // base + 0x28: number of translated events from WR to MIL, high word
									// base + 0x2C: number of translated events from WR to MIL, low word
	uint32_t	 late_events;       // base + 0x30: number of translated events that could not be delivered in time
	uint32_t     late_histogram[16];// histogram of delays 
	                                //            [0] -> delay < 2^10 ns
	                                //            [1] -> delay < 2^11 ns
	                                //           ...
	                                //           [14] -> delay < 2^24 ns
	                                //           [15] -> delay >= 2^24 ns
	uint32_t     mil_histogram[256];// histogram of MIL events
	uint32_t     mb_slot;           // MSI slot number 
	uint32_t     op_ready;          // this is set by the host (used for the display only)
	uint32_t     request_fill_evt;  // 
} WrMilConfig;


volatile WrMilConfig *config_init();

void config_poll(volatile WrMilConfig *cmd);

#endif

#ifndef WR_MIL_GW_H_
#define WR_MIL_GW_H_

// Magic number to identify the LM32 (if more than one exists) that runs the WR-MIL gateway
#define WR_MIL_GW_MAGIC_NUMBER       0x1234abcd

#define WR_MIL_GW_SHARED_OFFSET      0x500 // offset to the shared memory section

// Commands to be written into the WR_MIL_GW_REG_COMMAND register
#define WR_MIL_GW_CMD_NONE           0x0   // empty command
#define WR_MIL_GW_CMD_KILL           0x1   // command to stop the LM32 from running
#define WR_MIL_GW_CMD_RESET          0x2   // command to stop the LM32 for 1sec and go into initial state
#define WR_MIL_GW_CMD_CONFIG_SIS     0x3   // command to configure the gateway for SIS operation
#define WR_MIL_GW_CMD_CONFIG_ESR     0x4   // command to configure the gateway for ESR operation
#define WR_MIL_GW_CMD_TEST           0x5   // command that does nothing. it is useful to initiate to see if the firmware cleans the command register (see if the firmware runs at all)
#define WR_MIL_GW_CMD_UPDATE_OLED    0x6   // redraw content on OLED


// Configuration register mapping in shared memory region
#define WR_MIL_GW_REG_MAGIC_NUMBER   0x00  // command to be executed
#define WR_MIL_GW_REG_COMMAND        0x04  // command to be executed
#define WR_MIL_GW_REG_UTC_TRIGGER    0x08  // the MIL event that triggers the generation of UTC events
#define WR_MIL_GW_REG_UTC_DELAY      0x0C  // delay [us] between the 5 generated UTC MIL events
#define WR_MIL_GW_REG_TRIG_UTC_DELAY 0x10  // delay [us] between the trigger event and the first UTC (and other) generated events
#define WR_MIL_GW_REG_EVENT_SOURCE   0x14  // for internal use: register to hold the source configuration: 1 = SIS ; 2 = ESR ; 0 not configured
#define WR_MIL_GW_REG_LATENCY        0x18  // MIL event is generated 100us+latency after the WR event. The value of latency can be negative
#define WR_MIL_GW_REG_STATE          0x1C  // for internal use: state of the program: INITIAL, UNCONFIGURED, CONFIGURED
#define WR_MIL_GW_REG_UTC_OFFSET_HI  0x20  // delay [ms] between the TAI and the MIL-UTC, high word 
#define WR_MIL_GW_REG_UTC_OFFSET_LO  0x24  // delay [ms] between the trigger the MIL-UTC, low  word
#define WR_MIL_GW_REG_NUM_EVENTS_HI  0x28  // number of translated events from WR to MIL, high word
#define WR_MIL_GW_REG_NUM_EVENTS_LO  0x2C  // number of translated events from WR to MIL, low word
#define WR_MIL_GW_REG_LATE_EVENTS    0x30  // number of translated events that could not be delivered in time
#define WR_MIL_GW_REG_LATE_HISTOGRAM 0x34  // dummy register to indicate position after the last valid register
#define WR_MIL_GW_REG_MIL_HISTOGRAM  0x74  // dummy register to indicate position after the last valid register
#define WR_MIL_GW_REG_MSI_SLOT       0x474 // MSI slot is stored here
#define WR_MIL_GW_REG_SET_OP_READY   0x478 // Host writes 1 if OP-READY, 0 otherwise
#define WR_MIL_GW_REG_REQUEST_FILL_EVT 0x47c // if this is written to 1, the gateway will send a fill event as soon as possible

// states of the software
#define WR_MIL_GW_STATE_INIT         0
#define WR_MIL_GW_STATE_UNCONFIGURED 1
#define WR_MIL_GW_STATE_CONFIGURED   2
#define WR_MIL_GW_STATE_PAUSED       3

// Constants for event source type configuration.
// The WR-MIL gateway can run the SIS or the ESR
// and has to be configured to one these. 
// At startup it is unconfigured.
#define WR_MIL_GW_EVENT_SOURCE_UNKNOWN 0x0
#define WR_MIL_GW_EVENT_SOURCE_SIS     0x1
#define WR_MIL_GW_EVENT_SOURCE_ESR     0x2

// Interrupts
#define WR_MIL_GW_MSI_LATE_EVENT       0x1
#define WR_MIL_GW_MSI_STATE_CHANGED    0x2
#define WR_MIL_GW_MSI_EVENT            0x3 // ored with (mil_event_number << 8)

#endif

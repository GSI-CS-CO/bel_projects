#ifndef WR_MIL_GW_H_
#define WR_MIL_GW_H_

// Commands to be written into the WR_MIL_GW_REG_COMMAND register
#define WR_MIL_GW_CMD_NONE           0x0   // empty command
#define WR_MIL_GW_CMD_FULL_STOP      0x1   // command to stop the LM32 from running
#define WR_MIL_GW_CMD_PAUSE_10S      0x2   // command to stop the LM32 for 10sec
#define WR_MIL_GW_CMD_CONFIG_SIS     0x3   // command to configure the gateway for SIS operation
#define WR_MIL_GW_CMD_CONFIG_ESR     0x4   // command to configure the gateway for ESR operation


// Configuration register mapping in shared memory region
#define WR_MIL_GW_REG_COMMAND        0x00  // command to be executed
#define WR_MIL_GW_REG_UTC_TRIGGER    0x04  // the MIL event that triggers the generation of UTC events
#define WR_MIL_GW_REG_UTC_SEPARATION 0x08  // delay [us] between the 5 generated UTC MIL events
#define WR_MIL_GW_REG_UTC_DELAY      0x0C  // delay [us] between the trigger event and the first UTC (and other) generated events
#define WR_MIL_GW_REG_EVENT_SOURCE   0x10  // for internal use: register to hold the source configuration: 1 = SIS ; 2 = ESR ; 0 not configured
#define WR_MIL_GW_REG_STATE          0x14  // for internal use: state of the program: INITIAL, UNCONFIGURED, CONFIGURED


// Constants for event source type configuration.
// The WR-MIL gateway can run the SIS or the ESR
// and has to be configured to one these. 
// At startup it is unconfigured.
#define WR_MIL_GW_EVENT_SOURCE_UNKNOWN 0x0
#define WR_MIL_GW_EVENT_SOURCE_SIS     0x1
#define WR_MIL_GW_EVENT_SOURCE_ESR     0x2

#endif

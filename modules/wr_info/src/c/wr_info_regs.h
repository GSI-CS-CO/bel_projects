#ifndef WR_INFO_REGS
#define WR_INFO_REGS

/* Device Specific Includes and Defines */
/* ==================================================================================================== */
#define WR_INFO_GSI_VENDOR_ID          0x0000000000000651LL
#define WR_INFO_GSI_DEVICE_ID          0x77722D69  /* "wr-i" */

/* Registers */
#define WR_INFO_STATUS_RO              0x00        /* status of primary WR port */
#define WR_INFO_AUX_STATUS_RO          0x04        /* status of auxiliary WR port (2nd NIC) */
#define WR_INFO_AUX_TIME_LATCH_RW      0x20        /* any access (read or write) latches the time of the auxiliary WR port; reads 0 */
#define WR_INFO_AUX_TIME_HI_RO         0x24        /* latched time of auxiliary WR port [ns], high word */
#define WR_INFO_AUX_TIME_LO_RO         0x28        /* latched time of auxiliary WR port [ns], low word */
#define WR_INFO_UPTIME_RO              0x30        /* uptime [s] since FPGA reset */

/* Status bits (WR_INFO_STATUS_RO, WR_INFO_AUX_STATUS_RO) */
#define WR_INFO_STATUS_TIME_VALID_MASK 0x00000001  /* bit 0: time valid (locked) */
#define WR_INFO_STATUS_LINK_VALID_MASK 0x00000002  /* bit 1: link valid */
#define WR_INFO_STATUS_PRESENT_MASK    0x00000008  /* bit 3, auxiliary WR port only: present (dual port gateware) */

/* Reading the time of the auxiliary WR port: access WR_INFO_AUX_TIME_LATCH_RW, then read
   WR_INFO_AUX_TIME_HI_RO and WR_INFO_AUX_TIME_LO_RO, preferably within one Etherbone cycle.
   The time is only valid if WR_INFO_STATUS_PRESENT_MASK and WR_INFO_STATUS_TIME_VALID_MASK are set
   in WR_INFO_AUX_STATUS_RO. */

#endif /* WR_INFO_REGS */

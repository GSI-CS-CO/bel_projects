#ifndef _PMC_CTRL_H_
#define _PMC_CTRL_H_

#define PMC_CTRL_SLAVE_VENDOR_ID 0x0000000000000651ull
#define PMC_CTRL_SLAVE_DEVICE_ID 0x98c59ec1

//| Address Map ------------------------ slave ----------------------------------------------|
#define SLAVE_HEX_SWITCH_GET           0x00  // r  _0x0000000f , Shows hex switch inputs
#define SLAVE_PUSH_BUTTON_GET          0x04  // r  _0x00000001 , Shows status of the push button
#define SLAVE_HEX_SWITCH_CPLD_GET      0x08  // r  _0x0000000f , Shows hex switch inputs (cpld)
#define SLAVE_PUSH_BUTTON_CPLD_GET     0x0c  // r  _0x00000001 , Shows status of the push button (cpld)
#define SLAVE_CLOCK_CONTROL_OE_RW      0x10  // rw _0x00000001 , External input clock output enable
#define SLAVE_LOGIC_CONTROL_OE_RW      0x14  // rw _0x0001ffff , External logic analyzer output enable
#define SLAVE_LOGIC_OUTPUT_RW          0x18  // rw _0x0001ffff , External logic analyzer output (write)
#define SLAVE_LOGIC_INPUT_GET          0x1c  // r  _0x0001ffff , External logic analyzer input (read)

#endif

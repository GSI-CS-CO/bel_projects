#ifndef _PMC_CTRL_H_
#define _PMC_CTRL_H_

#define PMC_CTRL_SLAVE_VENDOR_ID 0x0000000000000651ull
#define PMC_CTRL_SLAVE_DEVICE_ID 0x98c59ec1

//| Address Map ------------------------ slave ----------------------------------------------|
#define SLAVE_HEX_SWITCH_GET        0x0   // r  _0x0000000f , Shows hex switch inputs
#define SLAVE_PUSH_BUTTON_GET       0x4   // r  _0x00000001 , Shows status of the push button
#define SLAVE_CLOCK_CONTROL_RW      0x8   // rw _0x00000001 , Control external clock enable

#endif

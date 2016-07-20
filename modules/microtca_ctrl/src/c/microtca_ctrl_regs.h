#ifndef _MICROTCA_CTRL_H_
#define _MICROTCA_CTRL_H_

#define MICROTCA_CTRL_SLAVE_VENDOR_ID 0x0000000000000651ull
#define MICROTCA_CTRL_SLAVE_DEVICE_ID 0x0e10dd1a

//| Address Map ------------------------ slave ----------------------------------------------|
#define SLAVE_HEX_SWITCH_GET           0x00  // r  _0x0000000f , Shows hex switch inputs
#define SLAVE_PUSH_BUTTON_GET          0x04  // r  _0x00000001 , Shows status of the push button
#define SLAVE_HEX_SWITCH_CPLD_GET      0x08  // r  _0x0000000f , Shows hex switch inputs (CPLD)
#define SLAVE_PUSH_BUTTON_CPLD_GET     0x0c  // r  _0x00000001 , Shows status of the push button (CPLD)
#define SLAVE_CLOCK_CONTROL_OE_RW      0x10  // rw _0x00000001 , External input clock output enable
#define SLAVE_LOGIC_CONTROL_OE_RW      0x14  // rw _0x0001ffff , External logic analyzer output enable
#define SLAVE_LOGIC_OUTPUT_RW          0x18  // rw _0x0001ffff , External logic analyzer output (write)
#define SLAVE_LOGIC_INPUT_GET          0x1c  // r  _0x0001ffff , External logic analyzer input (read)
#define SLAVE_BACKPLANE_CONF0_RW       0x20  // rw _0x0000ffff , Backplane Config 0
#define SLAVE_BACKPLANE_CONF1_RW       0x24  // rw _0x0000ffff , Backplane Config 1
#define SLAVE_BACKPLANE_CONF2_RW       0x28  // rw _0x0000ffff , Backplane Config 2
#define SLAVE_BACKPLANE_CONF3_RW       0x2c  // rw _0x0000ffff , Backplane Config 3
#define SLAVE_BACKPLANE_CONF4_RW       0x30  // rw _0x0000ffff , Backplane Config 4
#define SLAVE_BACKPLANE_CONF5_RW       0x34  // rw _0x0000ffff , Backplane Config 5
#define SLAVE_BACKPLANE_CONF6_RW       0x38  // rw _0x0000ffff , Backplane Config 6
#define SLAVE_BACKPLANE_CONF7_RW       0x3c  // rw _0x0000ffff , Backplane Config 7
#define SLAVE_BACKPLANE_STAT0_GET      0x40  // r  _0x0000ffff , Backplane Status 0
#define SLAVE_BACKPLANE_STAT1_GET      0x44  // r  _0x0000ffff , Backplane Status 1
#define SLAVE_BACKPLANE_STAT2_GET      0x48  // r  _0x0000ffff , Backplane Status 2
#define SLAVE_BACKPLANE_STAT3_GET      0x4c  // r  _0x0000ffff , Backplane Status 3
#define SLAVE_BACKPLANE_STAT4_GET      0x50  // r  _0x0000ffff , Backplane Status 4
#define SLAVE_BACKPLANE_STAT5_GET      0x54  // r  _0x0000ffff , Backplane Status 5
#define SLAVE_BACKPLANE_STAT6_GET      0x58  // r  _0x0000ffff , Backplane Status 6
#define SLAVE_BACKPLANE_STAT7_GET      0x5c  // r  _0x0000ffff , Backplane Status 7

#endif

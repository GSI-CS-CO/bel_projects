#ifndef _DISP_H_
#define _DISP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <etherbone.h>

#include "disp-lcd.h"
#include "disp-oled.h"
#include "ssd1325_serial_driver.h"

#define ERROR     -1
#define NONE      0
#define OLED      1
#define LCD       2
#define SSD1325   3

typedef unsigned char t_disp_type;

extern volatile unsigned int* display;
extern volatile unsigned int desiredDisplay;
extern const char* program;
extern const char* netaddress;

t_disp_type init_disp(eb_device_t device);

extern void (*disp_put_loc_c)(eb_device_t device, char ascii, unsigned char row, unsigned char col);
extern void (*disp_put_c)(eb_device_t device, char ascii);
extern void (*disp_put_s)(eb_device_t device, const char* str);
extern void (*disp_put_line)(eb_device_t device, const char *sPtr, unsigned char row);

#endif

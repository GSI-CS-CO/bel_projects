#ifndef _DISP_OLED_H_
#define _DISP_OLED_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <etherbone.h>
#include "oled_regs.h"

#define ERROR	-1

extern volatile unsigned int* display;
extern const char ROW_LEN;

extern const char* program;
//extern const char* netaddress;
//void disp_reset();
void oled_disp_put_c(eb_device_t device, char ascii);
void oled_disp_put_loc_c(eb_device_t device, char ascii, unsigned char row, unsigned char col);
//
void oled_disp_put_s(eb_device_t device, const char *sPtr);
void oled_disp_put_line(eb_device_t device, const char *sPtr, unsigned char row);
//void disp_put_raw(char pixcol, unsigned int address);

#endif 

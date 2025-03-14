#ifndef _DISP_LCD_H_
#define _DISP_LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <etherbone.h>
#include "font5x7.h"

#define FB_SIZE 24
#define COL_MAX	6
#define ROW_MAX	3
#define CHR_WID 6
#define CHR_HEI 8
#define ERROR	-1

extern unsigned long long framebuffer[FB_SIZE];
extern volatile unsigned int* display;
extern const char* program;
extern const char* netaddress;

void disp_write(eb_device_t device);
void lcd_disp_put_loc_c(eb_device_t device, char ascii, unsigned char row, unsigned char col);
void lcd_disp_put_c(eb_device_t device, char ascii);
void lcd_disp_put_s(eb_device_t device, const char* str);
void lcd_disp_put_line(eb_device_t device, const char *sPtr, unsigned char row);

#endif

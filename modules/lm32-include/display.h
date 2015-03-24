#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#include <inttypes.h>
#include <stdint.h>
extern volatile uint32_t* pOledDisplay;

// Oled Display RegisterLayout
static const struct {
   uint32_t rst;
   uint32_t mode;
   uint32_t uart;
   uint32_t character;
   uint32_t raw;
   uint32_t mode_RAW;
   uint32_t mode_UART;
   uint32_t mode_CHAR;
   uint32_t mode_IDLE;
   uint32_t ROW_LEN;
   
} r_oledDisp = {  .rst        = 0x04 >> 2,
                  .mode       = 0x00 >> 2,
                  .uart       = 0x00010000 >> 2,
                  .character  = 0x00020000 >> 2,
                  .raw        = 0x00020000 >> 2,
                  .mode_RAW   = 0x03,
                  .mode_UART  = 0x01,
                  .mode_CHAR  = 0x02,
                  .mode_IDLE  = 0x00,
                  .ROW_LEN    = 11
};


void disp_put_c(char ascii);
void disp_put_int(int number);
void disp_put_str(const char *sPtr);
void disp_put_line(const char *sPtr, unsigned char row);

void disp_reset();

void disp_loc_c(char ascii, unsigned char row, unsigned char col);
void disp_put_raw(char pixcol, uint32_t address, char color);

uint32_t get_pixcol_addr(unsigned char x_in, unsigned char y_in);

uint32_t get_pixcol_val(unsigned char y_in);


#endif 

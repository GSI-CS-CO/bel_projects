#ifndef _DISPLAY_H_
#define _DISPLAY_H_
extern volatile unsigned int* pOledDisplay;

// Oled Display RegisterLayout
static const struct {
   unsigned int rst;
   unsigned int mode;
   unsigned int uart;
   unsigned int character;
   unsigned int raw;
   unsigned int mode_RAW;
   unsigned int mode_UART;
   unsigned int mode_CHAR;
   unsigned int mode_IDLE;
   unsigned int ROW_LEN;
   
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
void disp_put_raw(char pixcol, unsigned int address, char color);

unsigned int get_pixcol_addr(unsigned char x_in, unsigned char y_in);

unsigned int get_pixcol_val(unsigned char y_in);


#endif 

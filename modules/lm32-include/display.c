#
#include "display.h"

void disp_reset()
{
   *(pOledDisplay + r_oledDisp.rst) = 1;
}

void disp_put_c(char ascii)
{
   *(pOledDisplay + r_oledDisp.mode) = r_oledDisp.mode_UART;
   *(pOledDisplay + r_oledDisp.uart) = (unsigned int)ascii;
}

void disp_put_str(const char *sPtr)
{
   *(pOledDisplay + r_oledDisp.mode) = r_oledDisp.mode_UART;
   while(*sPtr != '\0') *(pOledDisplay + r_oledDisp.uart) = (unsigned int)*sPtr++;
}

void disp_put_line(const char *sPtr, unsigned char row)
{
   unsigned char col, outp, pad;
   pad = 0;

   for(col=0; col< r_oledDisp.ROW_LEN; col++)
   {
      if(*(sPtr+col) == '\0') pad = 1;
      
      if(pad) outp = ' ';
      else    outp = (unsigned int)*(sPtr+col);   
      disp_loc_c(outp, row, col);
   }
}

void disp_loc_c(char ascii, unsigned char row, unsigned char col)
{
   unsigned int rowcol;
   *(pOledDisplay + r_oledDisp.mode)               = r_oledDisp.mode_CHAR;
   rowcol   = ((0x07 & (unsigned int)row)<<6) + ((0x0f & (unsigned int)col)<<2);
   *(pOledDisplay + ((r_oledDisp.character + rowcol)>>2))    = (unsigned int)ascii;
}

void disp_put_raw(char pixcol, unsigned int address, char color)
{
   char oldpixcol;
   *(pOledDisplay + r_oledDisp.mode) = r_oledDisp.mode_RAW;
   oldpixcol = *(pOledDisplay + r_oledDisp.raw + ((address & 0x3FFF))); //read out old memcontent
   
   if(color) // 1 -> White
      *(pOledDisplay + r_oledDisp.raw  + ((address & 0x3FFF))) = (unsigned int)(pixcol | oldpixcol);
   else
      *(pOledDisplay + r_oledDisp.raw  + ((address & 0x3FFF))) = (unsigned int)(~pixcol & oldpixcol);

    *(pOledDisplay + r_oledDisp.mode)  = r_oledDisp.mode_IDLE;

}

unsigned int get_pixcol_addr(unsigned char x_in, unsigned char y_in)
{
   unsigned int addr_base = 0x230;
   unsigned int x, y;
   
   x = x_in & 0x3F;
   if(y_in < 48)  y = ((y_in>>3)<<8); //determine row. Shift by 8bit
   else           y = (5<<8);       //outside visible area, take start of bottom row instead

   return addr_base + y + x;
   
}

unsigned int get_pixcol_val(unsigned char y_in)
{
   return 1<<(y_in & 0x07); 
}

#include "disp-lcd.h"

unsigned char cursorCol = 1;
unsigned char cursorRow = 1;
unsigned long long framebuffer[FB_SIZE];


void disp_write(eb_device_t device)
{

  unsigned int* writeout;
  int i;
  eb_status_t status;
  eb_format_t format;
  eb_cycle_t cycle;
  

  format = EB_ADDR32|EB_DATA32;

  writeout = (unsigned int*)framebuffer;	

  //wrap frame buffer in EB packet
  for(i=0;i<FB_SIZE*2;i+=2) 
    {
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
	fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
	return;
      } 
      eb_cycle_write(cycle, (eb_address_t)(display+i), format, writeout[i+1]);      
      eb_cycle_write(cycle, (eb_address_t)(display+i+1), format, writeout[i]); 
      eb_cycle_close(cycle);
    }
  return;
}




static unsigned char* render_char(char ascii, unsigned char* pDotMatrix)
{
	unsigned char i, j, tmpRow;
	const unsigned char* pBmp = &font5x7[(ascii-0x20)*(CHR_WID-1)];

	//this is written 5x7, turn it into 8x8
	//no time or desire to compile my own table
	for(i=0;i<CHR_HEI-1;i++)
	{
		tmpRow = 0;		
		for(j=0;j<CHR_WID-1;j++) tmpRow |= ((pBmp[j]>>i)&0x01)<<(CHR_WID-2-j);
		pDotMatrix[i] = tmpRow;
	}	 
	pDotMatrix[CHR_HEI-1] = 0x00;

	return pDotMatrix;

				
}

void lcd_disp_put_loc_c(eb_device_t device, char ascii, unsigned char row, unsigned char col)
{
  unsigned char i, x, y;
  unsigned char bitmap[CHR_HEI];
  unsigned long long line, mask;
  const unsigned long long cMask = ((unsigned long long)(2^CHR_WID)-1)<<(63-CHR_WID); //make a mask CHR_WID bits wide and left align it in a 64bit word
  
  render_char(ascii, bitmap);
  
  if(col == 0) x = 0; 
  else x = (col <= COL_MAX) ? (col-1)*CHR_WID :  (COL_MAX-1)*CHR_WID;	
  
  if(row == 0) y = 0; 
  else y = (row <= ROW_MAX) ? (row-1)*CHR_HEI : (ROW_MAX-1)*CHR_HEI;
	
  //1x 64 bit word per line to cover 36x24pix. x is bitshift, y is word 
  mask = ~(cMask >> x);
  
  for(i=0;i<CHR_HEI;i++)
    {
      framebuffer[y+i] &= mask;	//clear the part in the framebuffer row
      line = ((unsigned long long)bitmap[i])<<(59-x);
      framebuffer[y+i] |= line;	//OR it with the bitmap row
    }
  
  
  disp_write(device);	 
}

void lcd_disp_put_c(eb_device_t device, char ascii)
{
  unsigned char i;	
  
  if(ascii == 0x0c)
    {
      cursorCol = 1;
      cursorRow = 1;
      for(i=0;i<FB_SIZE;i++) framebuffer[i] = 0;	
      disp_write(device);			
    }
  else if(ascii == '\n')
    {
      
      cursorCol = 1;
      if(cursorRow < 3) cursorRow++;
      else 							cursorRow=1;
      for(i=(cursorRow-1)*CHR_HEI;i<(cursorRow*CHR_HEI);i++) framebuffer[i] = 0;
      disp_write(device); 
    }	
  else
    {	
      
      lcd_disp_put_loc_c(device, ascii, cursorRow, cursorCol);
      if(cursorCol < 6) cursorCol++;
      else 
	{
	  cursorCol = 1;
	  if(cursorRow < 3) cursorRow++;
	  else 							cursorRow=1;
	  for(i=(cursorRow-1)*CHR_HEI;i<(cursorRow*CHR_HEI);i++) framebuffer[i] = 0;  
	}

    }
}

void lcd_disp_put_s(eb_device_t device, const char *sPtr)
{

  while(*sPtr != '\0') lcd_disp_put_c(device, *sPtr++);
}

void lcd_disp_put_line(eb_device_t device, const char *sPtr, unsigned char row)
{
  char col, outp, pad;
  pad = 0;
  
  for(col=0; col<COL_MAX; col++)
    {
      if(*(sPtr+col) == '\0') pad = 1;
      
      if(pad) outp = ' ';
      else 	outp = *(sPtr+col);	
      
      lcd_disp_put_loc_c(device, outp, row, col+1);
    }
  
  cursorCol = 1;
  if(row < 3) cursorRow = row+1;
  else 				cursorRow=1;
}




 

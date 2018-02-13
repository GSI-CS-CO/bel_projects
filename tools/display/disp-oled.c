#include "disp-oled.h"


const char ROW_LEN = 11;

void oled_disp_put_c(eb_device_t device, char ascii)
{
  eb_status_t status;
  eb_format_t format = EB_ADDR32|EB_DATA32;
  eb_cycle_t cycle;
  
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
    fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
    return;
  }
  
  eb_cycle_write(cycle, (eb_address_t)(display + (OLED_UART_OWR>>2)), format, (unsigned int)ascii); 
  eb_cycle_close(cycle);

  return;
}

void oled_disp_put_s(eb_device_t device, const char *sPtr)
{
  eb_status_t status;
  eb_format_t format = EB_ADDR32|EB_DATA32;
  eb_cycle_t cycle;
  	
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
    fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
    return;
  }
  while(*sPtr != '\0') eb_cycle_write(cycle, (eb_address_t)(display + (OLED_UART_OWR>>2)), format, (unsigned int)((unsigned int)*sPtr++)); 
  eb_cycle_close(cycle);
  
  return;
}

void oled_disp_put_loc_c(eb_device_t device, char ascii, unsigned char row, unsigned char col)
{
  eb_status_t status;
  eb_format_t format = EB_ADDR32|EB_DATA32;
  eb_cycle_t cycle;
  	

  unsigned int rowcol = ((0x07 & (unsigned int)row)<<12) + ((0x0f & (unsigned int)col)<<8);	

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
    fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
    return;
  }
  eb_cycle_write(cycle, (eb_address_t)(display + (OLED_CHAR_OWR >>2)), format, rowcol | (unsigned int)ascii);
  eb_cycle_close(cycle);

  return;
}

void oled_disp_put_line(eb_device_t device, const char *sPtr, unsigned char row)
{
  eb_status_t status;
  eb_format_t format = EB_ADDR32|EB_DATA32;
  eb_cycle_t cycle;
  	

  unsigned char col, outp, pad;
  unsigned int rowcol;
  pad = 0;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
    fprintf(stderr, "%s: failed to create cycle: %s\n", program, eb_status(status));
    return;
  }
  
  for(col=0; col<ROW_LEN; col++)
    {
      if(*(sPtr+col) == '\0') pad = 1;

      if(pad) outp = ' ';
      else 	outp = (unsigned int)*(sPtr+col);	
      rowcol = ((0x07 & (unsigned int)row)<<12) + ((0x0f & (unsigned int)col)<<8); 	
      eb_cycle_write(cycle, (eb_address_t)(display + (OLED_CHAR_OWR >>2)), format, rowcol | (unsigned int)outp);
    }	

	
  eb_cycle_close(cycle);
}



/*

void disp_put_raw(char pixcol, unsigned int address)
{
	*(display + (REG_MODE>>2)) = (unsigned int)MODE_RAW;
	*(display + (REG_RAW>>2) + ((address & 0x3FFF))) = (unsigned int)(pixcol);
}



void disp_draw(unsigned char x, unsigned char y, char color)
{
	char oldpixcol;
	unsigned int address;
	
	pixcol = 1<<(y & 0x07);
	address = get_pixcol_addr(x, y):
	*(display + (REG_MODE>>2)) = (unsigned int)MODE_RAW;
	oldpixcol = *(display + (REG_RAW>>2) + ((address & 0x3FFF))); //read out old memcontent
	
	if(color) // 1 -> White
		*(display + (REG_RAW>>2) + ((address & 0x3FFF))) = (unsigned int)(pixcol | oldpixcol);
	else
		*(display + (REG_RAW>>2) + ((address & 0x3FFF))) = (unsigned int)(~pixcol & oldpixcol);
}

static unsigned int get_pixcol_addr(unsigned char x_in, unsigned char y_in)
{
	unsigned int addr_base = 0x223;
	unsigned int x, y;
	
	x = x_in & 0x3F;
	if(y_in < 48) 	y = ((y_in>>3)<<8); //determine row. Shift by 8bit
	else 		y = (5<<8);	    //outside visible area, take start of bottom row instead

	return addr_base + y + x;
	
}
*/


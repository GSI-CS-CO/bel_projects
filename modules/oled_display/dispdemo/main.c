#include "display.h"


volatile unsigned int* display = (unsigned int*)0x00200000;




void _irq_entry(void) {
  /* Currently only triggered by DMA completion */
}


const char mytext[] = "Hallo Welt!...\n\n";

void main(void) {
  int j, xinc, yinc, x, y;

unsigned int time = 0;


	unsigned int addr_raw_off;

	char color = 0xFF;

  disp_reset();	
  disp_put_c('\f');
  disp_put_str(mytext);
  disp_put_line("ABC", 2);
  disp_put_line("456", 1);
  disp_put_line("123", 0);






	x = 0;
	y = 9;
	yinc = -1;
 	xinc = 1;
	addr_raw_off = 0;
	
  while (1) {
    /* Rotate the LEDs */
    


//	disp_put_raw( get_pixcol_val((unsigned char)y), get_pixcol_addr((unsigned char)x, (unsigned char)y), color);


	if(x == 63) xinc = -1;
	if(x == 0)  xinc = 1;

	if(y == 47) yinc = -1;
	if(y == 0)  yinc = 1;

	x += xinc;
	y += yinc;


	
	

      /* Each loop iteration takes 4 cycles.
       * It runs at 125MHz.
       * Sleep 0.2 second.
       */
      for (j = 0; j < 125000000/160; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }

	if(time++ > 500) {time = 0; color = ~color; }
	
    
  }
}

//#include "simple-display.h"


volatile unsigned int* display = (unsigned int*)0x00100d00;
volatile unsigned long* framebuffer;

void _irq_entry(void) {
  /* Currently only triggered by DMA completion */
}


const char mytext[] = "Hallo Welt!...\n\n";

void main(void) {
  int j, i;
	framebuffer = (volatile unsigned long*)display;
	const char str[] = "\fHal\nlo We\nlt";


	*display = 0xffffffff;
	for(i=0;i<24;i++) framebuffer[i] = 0xffffffffffffffff;	
	//		disp_put_s(str);
	
	

      /* Each loop iteration takes 4 cycles.
       * It runs at 125MHz.
       * Sleep 0.2 second.
       */
      for (j = 0; j < 125000000/160; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }
	
    
}






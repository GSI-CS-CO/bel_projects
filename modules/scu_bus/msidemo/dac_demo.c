#include <stdio.h>
#include "display.h"
#include "irq.h"
#include "scu_bus.h"

volatile unsigned int* display            = (unsigned int*)0x02900000;
volatile unsigned int* irq_slave          = (unsigned int*)0x02000d00;
volatile unsigned short* scu_bus_master   = (unsigned short*)0x02400000;



char* mat_sprinthex(char* buffer, unsigned long val)
{
	unsigned char i,ascii;
	const unsigned long mask = 0x0000000F;

	for(i=0; i<8;i++)
	{
		ascii= (val>>(i<<2)) & mask;
		if(ascii > 9) ascii = ascii - 10 + 'A';
	 	else 	      ascii = ascii      + '0';
		buffer[7-i] = ascii;		
	}
	
	buffer[8] = 0x00;
	return buffer;	
}

void show_msi()
{
  char buffer[12];
  
  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.src);
  disp_put_str("A ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, (unsigned long)global_msi.sel);
  disp_put_str("S ");
  disp_put_str(buffer);
  disp_put_c('\n');
}

void isr1()
{
  unsigned int j;
  
  disp_put_str("ISR1\n");
  //echo register of slave 5
  //*(scu_bus_master + 0xa0000 + 0x20) = 0xaffe;
  show_msi();

   for (j = 0; j < 125000000; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }
   disp_put_c('\f');   
}

void _irq_entry(void) {
  
  disp_put_c('\f');
  disp_put_str("IRQ_ENTRY\n");
  irq_process();
 
}

const char mytext[] = "Hallo Welt!...\n\n";

void main(void) {
  unsigned short dac_value = 0xffff;
  int i;
  char buffer[14];
  //enable dac
  scu_bus_master[0x50210] = 0x1;
  
  while (1) {
    for (i = 0; i < 125000; ++i) {
      asm("# noop");
    }
    scu_bus_master[0x50211] = dac_value++;
    disp_put_c('\f');
    mat_sprinthex(buffer, (unsigned long)dac_value);
    disp_put_str(buffer);
  }  
  
  //SCU Bus Master
  //enable slave irqs
  scu_bus_master[GLOBAL_IRQ_ENA] = 0x28;
  //enable slave irq for slave 5
  //scu_bus_master[SRQ_ENA] = 0x10;
  scu_bus_master[SRQ_ENA] = 0x0;  

  isr_table_clr();
//  isr_ptr_table[0]= isr0;
  isr_ptr_table[1]= isr1;  
  irq_set_mask(0x03);
  irq_enable();

  
  disp_reset();	
  disp_put_str(mytext);

}

#include "mini_sdb.h"
#include "pp-printf.h"
#include "irq.h"
#include "mprintf.h"

void* memset(void* s, int c, int n) {
	void *result = s;
	for (int i = 0; i < n/4; ++i) {
		((int*)s)[i] = c;
	}
	return result;
}
void *memcpy(void *dest, const void *src, int n)
{
	void *result = dest;
	for (int i = 0; i < n/4; ++i) {
		((int*)dest)[i] = ((int*)src)[i];
	}
	return result;
}


volatile char *uart_out;
int puts(const char *str) {
	while(*str) {
		*uart_out = *str++;
	}
	return 1;
}
void uart_write_byte(const char c) {
	*uart_out = c;
}

void irq_handler(int id) { 
	mprintf("irq_handler\n");
}

void init_irq_table() {
  isr_table_clr();
  isr_ptr_table[0] = &irq_handler;
  irq_set_mask(0x01);
  //msg_buf[IRQ].ring_head = msg_buf[IRQ].ring_tail; // clear msg buffer
  irq_enable();
  mprintf("IRQ table configured.\n");
}

void discover() {
  sdb_location found_sdb[20];
  uint32_t idx = 0;
  pCpuMsiBox   = 0;
  pMyMsi       = 0; 

  uart_out = (volatile char*) find_device_adr(GSI, SDB_UART_SIM);
  find_device_multi(&found_sdb[0], &idx, 1, GSI, MSI_MSG_BOX);   
  if(idx) {
    pCpuMsiBox    = (uint32_t*)getSdbAdr(&found_sdb[0]); 
    pMyMsi        = (uint32_t*)getMsiAdr(&found_sdb[0]); 
  } 

}

int main() {
	discover();
	init_irq_table();
	for(;;) {
		puts("hello, world!\n");
		mprintf("uart=0x%x\n",uart_out);
		mprintf("pCpuMsiBox=0x%x\n",pCpuMsiBox);
		mprintf("pMyMsi=0x%x\n",pMyMsi);
	}
	irq_disable();
	return 0;
}